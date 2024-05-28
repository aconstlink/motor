
#include "app.h"

#include "carrier.h"

#include <motor/profiling/global.h>
#include <motor/tool/imgui/custom_widgets.h>

using namespace motor::application ;

class motor::application::app::_app_client_handler_wrapper : public motor::network::iclient_handler
{
    motor::network::iclient_handler_mtr_t _handler ;
    motor::application::app_ptr_t _app ;

    bool_t _shutdown = false ;

public:

    _app_client_handler_wrapper( motor::application::app_ptr_t owner, motor::network::iclient_handler_ptr_t hnd ) noexcept :
        _handler( hnd ), _app( owner ) {}

    _app_client_handler_wrapper( _app_client_handler_wrapper && rhv ) noexcept :
        _handler( motor::move( rhv._handler ) ), _app( motor::move( rhv._app ) ) {}

    virtual ~_app_client_handler_wrapper( void_t ) noexcept
    {
        motor::memory::release_ptr( _handler ) ;
    }

    virtual motor::network::user_decision on_connect( motor::network::connect_result const res, size_t const tries ) noexcept
    {
        auto const ret_res = _handler->on_connect( res, tries ) ;
        if ( ret_res == motor::network::user_decision::shutdown )
        {
            _app->remove( this ) ;
        }
        return ret_res ;
    }

    virtual motor::network::user_decision on_sync( void_t ) noexcept
    {
        auto const ret_res = _handler->on_sync() ;
        if ( ret_res == motor::network::user_decision::shutdown )
        {
            _app->remove( this ) ;
        }
        return ret_res ;
    }

    virtual motor::network::user_decision on_update( void_t ) noexcept
    {
        auto const ret_res = _handler->on_update() ;
        if ( ret_res == motor::network::user_decision::shutdown ||
            _shutdown )
        {
            _app->remove( this ) ;
            return motor::network::user_decision::shutdown ;
        }
        return ret_res ;
    }

    virtual void_t on_receive( byte_cptr_t buffer, size_t const sib ) noexcept
    {
        _handler->on_receive( buffer, sib ) ;
    }

    virtual void_t on_received( void_t ) noexcept
    {
        _handler->on_received() ;
    }

    virtual void_t on_send( byte_cptr_t & buffer, size_t & num_sib ) noexcept
    {
        _handler->on_send( buffer, num_sib ) ;
    }

    virtual void_t on_sent( motor::network::transmit_result const res ) noexcept
    {
        _handler->on_sent( res ) ;
    }

    void_t shutdown( void_t ) noexcept
    {
        _shutdown = true ;
    }
};

//**************************************************************************************************************
app::app( void_t ) noexcept
{
}

//**************************************************************************************************************
app::app( this_rref_t rhv ) noexcept
{
    _windows = std::move( rhv._windows ) ;
    
    _destruction_queue = std::move( rhv._destruction_queue ) ;
    _closed = rhv._closed ;
    _first_audio = rhv._first_audio ;
    _shutdown_called = rhv._shutdown_called ;

    _engine_profiling = std::move( rhv._engine_profiling ) ;
    _display_engine_stats = rhv._display_engine_stats ;

    _dev_mouse = motor::move( rhv._dev_mouse ) ;
    _dev_ascii = motor::move( rhv._dev_ascii ) ;
    _networks = std::move( rhv._networks ) ;
}

//**************************************************************************************************************
app::~app( void_t ) noexcept
{
    // must be done during carrier shutdown
    assert( _windows.size() == 0 ) ;
}

//**************************************************************************************************************
app::window_id_t app::create_window( motor::application::window_info_cref_t wi ) noexcept 
{
    motor::application::iwindow_mtr_t wnd = _carrier->create_window( wi ) ;

    motor::application::window_message_listener_mtr_t msgl = motor::memory::create_ptr<
            motor::application::window_message_listener>( "[app::create_window] : out message listener" ) ;

    wnd->register_out( motor::share( msgl ) ) ;

    wnd->send_message( motor::application::show_message_t( {true} ) ) ;

    size_t ret = size_t(-1) ;
    {
        std::lock_guard< std::mutex > lk( _mtx_windows ) ;

        ret = _windows.size() ;

        _windows.emplace_back( this_t::window_data
            { static_cast<motor::application::window_ptr_t>(wnd), msgl, nullptr, 
                motor::shared( motor::tool::imgui_t( motor::to_string(ret) ) ) } ) ;
    }

    return ret ;
}

//**************************************************************************************************************
void_t app::send_window_message( this_t::window_id_t const wid, 
    std::function< void_t ( this_t::window_view & ) > funk ) 
{
    if( wid >= _windows.size() ) return ;

    std::lock_guard< std::mutex > lk( _mtx_windows ) ;
    this_t::window_view accessor( _windows[wid].wnd ) ;
    funk( accessor ) ;
}

//**************************************************************************************************************
bool_t app::carrier_init( motor::application::carrier_ptr_t carrier ) noexcept
{
    if( _carrier != nullptr ) return true ;
    _carrier = carrier ;

    this->on_init() ;

    return true ;
}

//**************************************************************************************************************
bool_t app::carrier_update( void_t ) noexcept
{
    std::chrono::microseconds dt_micro ;
    float_t dt_sec = 0.0f ;

    {
        dt_micro = std::chrono::duration_cast<std::chrono::microseconds>( this_t::platform_clock_t::now() - _tp_platform ) ;
        dt_sec = float_t( double_t( dt_micro.count() ) / 1000000.0 ) ;
        _tp_platform = this_t::platform_clock_t::now() ;
    }

    // do window message updates -> on_event
    {
        this_t::push_windows() ;
        
        for( auto iter=_windows2.begin() ; iter!=_windows2.end(); )
        {
            size_t const i = std::distance( _windows2.begin(), iter ) ;

            auto & d = *iter ;

            motor::application::window_message_listener_t::state_vector_t sv ;
            if( d.lst->swap_and_reset( sv ) )
            {
                this->on_event( i, sv ) ;

                if( sv.create_changed )
                {
                    d.fe = d.wnd->borrow_frontend() ;
                }
                
                if( sv.close_changed )
                {
                    _destruction_queue.emplace_back( *iter ) ;
                    iter = _windows2.erase( iter ) ;

                    continue ;
                }

                if( sv.resize_changed )
                {
                    d.imgui->update( {(int_t)sv.resize_msg.w,(int_t)sv.resize_msg.h} ) ;
                }

                // check if mouse is in tool window
                if ( sv.mouse_msg_changed )
                {
                    if ( sv.mouse_msg.state == motor::application::mouse_message::state_type::enter )
                    {
                        d.is_mouse_over = true ;
                    }
                    else if ( sv.mouse_msg.state == motor::application::mouse_message::state_type::leave )
                    {
                        d.is_mouse_over = false ;
                    }
                }
                
            }
            ++iter ;
        }
        
        this_t::pop_windows() ;
    }

    if( this_t::before_device( dt_micro ) )
    {
        bool_t mouse_over_ui = false ;

        {
            for( auto & d : _windows )
            {
                if ( d.is_mouse_over )
                {
                    d.imgui->update( _dev_ascii ) ;
                    auto const over_ui = d.imgui->update( _dev_mouse ) ;
                    mouse_over_ui |= over_ui ;
                }
            }
        }

        {
            this_t::device_data_in_t dat = 
            {
                _dev_mouse,
                _dev_ascii,
                mouse_over_ui
            } ;
            this->on_device( dat ) ;
        }

        this_t::after_device(0) ;
    }

    if( this_t::before_update( dt_micro ) )
    {
        this_t::update_data_t dat ;
        dat.micro_dt = _update_interval.count() ;
        dat.sec_dt = float_t( double_t(_update_interval.count()) / 1000000.0 ) ;
        dat.milli_dt = dat.micro_dt / 1000 ;

        size_t const num_iter = _update_residual / _update_interval ;
        
        for( size_t i=0; i<num_iter; ++i )
            this->on_update( dat ) ;

        this_t::after_update( num_iter ) ;
    }

    if( this_t::before_audio( dt_micro ) )
    {
        bool_t const exec = _carrier->audio_system()->on_audio( [&]( motor::audio::frontend_ptr_t fptr )
        {
            size_t const micro_dt = _update_interval.count() ;
            size_t const milli_dt = micro_dt / 1000 ;
            float_t const sec_dt = float_t( double_t( _update_interval.count() ) / 1000000.0 ) ;

            this->on_audio( fptr, this_t::audio_data { _first_audio, sec_dt, micro_dt, milli_dt } ) ;
        } )  ;

        this_t::after_audio(0) ;
    }

    if ( this_t::before_profile( dt_micro ) )
    {
        this_t::profile_data_t dat ;

        _engine_profiling.update() ;
        
        this->on_profile( dat ) ;

        this_t::after_profile() ;
    }

    // @todo on_graphics could be called already if 
    // only the shared user/engine data has been send upstream
    if( this_t::before_render(dt_micro) )
    {
        // do single on_graphics for updating 
        // user graphics system data.
        {
            this_t::graphics_data_t dat ;
            dat.micro_dt = _render_residual.count() ;
            dat.sec_dt = float_t( double_t(_render_residual.count()) / 1000000.0 ) ;
            dat.milli_dt = dat.micro_dt / 1000 ;

            this->on_graphics( dat ) ;
        }

        // render all the present windows
        {
            this_t::render_data_t dat ;
            dat.micro_dt = _render_residual.count() ;
            dat.sec_dt = float_t( double_t(_render_residual.count()) / 1000000.0 ) ;
            dat.milli_dt = dat.micro_dt / 1000 ;

            size_t i = 0 ;
            for( auto & d : _windows2 )
            {
                dat.first_frame = d.first_frame ;
                d.first_frame = false ;

                auto * re = d.fe->borrow_render_engine() ;
                if( re->enter_frame() )
                {
                    if( auto * fe = dynamic_cast<motor::graphics::gen4::frontend_ptr_t>(d.fe); fe != nullptr ) 
                    {
                        this->on_render( i, fe, dat ) ;

                        if( this_t::before_tool( dt_micro ) )
                        {
                            d.imgui->execute( [&] ( void_t )
                            {
                                this_t::tool_data_t td ;
                                if( this->on_tool( i, td ) )
                                {
                                    this_t::display_engine_stats() ;
                                    d.imgui->render( fe ) ;
                                }
                            } ) ;

                            this_t::after_tool(0) ;
                        }
                    }
                    re->leave_frame() ;
                }
                ++i ;
            }
        }

        // call it for the frame
        {
            this->on_frame_done() ;
        }

        this_t::after_render(0) ;
    }

    // handle windows to be destroyed
    {
        for( auto iter = _destruction_queue.begin(); iter != _destruction_queue.end(); ) 
        {
            if( !this_t::clear_out_window_data( *iter ) ) 
            {
                ++iter ;
                continue ;
            }
            iter = _destruction_queue.erase( iter ) ;
        }
    }

    return true ;
}

//**************************************************************************************************************
bool_t app::clear_out_window_data( window_data & d ) noexcept 
{
    // have to wait until the render engine 
    // is free of any more commands.
    auto * re = d.fe->borrow_render_engine() ;
    if( !re->can_enter_frame() ) 
    {
        return false ;
    }

    d.wnd->return_borrowed( d.fe ) ;
    d.fe = nullptr ;
    motor::memory::release_ptr( d.lst ) ;
    motor::memory::release_ptr( d.wnd ) ;
    motor::memory::release_ptr( d.imgui ) ;

    return true ;
}

//**************************************************************************************************************
bool_t app::carrier_shutdown( void_t ) noexcept 
{
    if( !_shutdown_called ) 
    {
        this->on_shutdown() ;
        _shutdown_called = true ;
    }
    
    {
        for ( auto & d : _destruction_queue )
        {
            if ( !this_t::clear_out_window_data( d ) )
            {
                return false ;
            }
        }

        _destruction_queue.clear() ;

        for ( auto & d : _windows )
        {
            if ( !this_t::clear_out_window_data( d ) )
            {
                return false ;
            }
        }

        _windows.clear() ;
    }

    // clear out network clients
    {
        for( auto & ns : _networks )
        {
            ns.wrapper->shutdown() ;
            motor::memory::release_ptr( motor::move( ns.wrapper ) ) ;
        }
    }

    return true ;
}

//**************************************************************************************************************
size_t app::push_windows( void_t ) noexcept 
{
    std::lock_guard< std::mutex > lk( _mtx_windows ) ;
    _windows2 = std::move( _windows ) ;
    return _windows2.size() ;
}

//**************************************************************************************************************
void_t app::pop_windows( void_t ) noexcept 
{
    // merge back
    {
        std::lock_guard< std::mutex > lk( _mtx_windows ) ;
        if( _windows.size() == 0 ) _windows = std::move( _windows2 ) ;
        else {for( auto & d : _windows2 ) _windows.emplace_back( std::move(d) ) ;}
    }
}

//**************************************************************************************************************
bool_t app::before_tool( std::chrono::microseconds const & ) noexcept
{
    if( _dev_mouse == nullptr )
    {
        return false ;
    }

    if( _dev_ascii == nullptr )
    {
        return false ;
    }

    return true ;
}

//**************************************************************************************************************
bool_t app::after_tool( size_t const iter ) noexcept
{
    return true ;
}

//**************************************************************************************************************
bool_t app::before_device( std::chrono::microseconds const & dt ) noexcept 
{
    _device_residual += dt ;

    if( _device_residual >= _device_interval )
    {
        // looking for device. It is managed, so pointer must be copied.
        _carrier->device_system()->search( [&] ( motor::controls::device_borrow_t::mtr_t dev_in )
        {
            if( auto * ptr1 = dynamic_cast<motor::controls::three_device_mtr_t>(dev_in); ptr1 != nullptr )
            {
                _dev_mouse = ptr1 ;
            }
            else if( auto * ptr2 = dynamic_cast<motor::controls::ascii_device_mtr_t>(dev_in); ptr2 != nullptr )
            {
                _dev_ascii = ptr2 ;
            }
        } ) ;

        _carrier->device_system()->update() ;
        return true ;
    }
    return false ;
}

//***
bool_t app::after_device( size_t const iter ) noexcept 
{
    _device_residual = decltype(_device_residual)(0) ;
    return true ;
}

//***
bool_t app::before_logic( std::chrono::microseconds const & dt ) noexcept 
{
    _logic_residual += dt ;
    return _logic_residual >= _logic_interval  ;
}

//***
bool_t app::after_logic( size_t const iter ) noexcept 
{
    _logic_residual -= iter * _logic_interval ;
    return true ;
}

//***
bool_t app::before_profile( std::chrono::microseconds const & dt ) noexcept
{
    _profile_residual += dt ;
    return _profile_residual >= _profile_interval ;
}

//***
void_t app::after_profile( void_t ) noexcept
{
    _profile_residual = decltype( _profile_residual )(0) ;
}

//***
bool_t app::before_update( std::chrono::microseconds const & dt ) noexcept
{
    _update_residual += dt ;

    if( _update_residual >= _update_interval )
    {
        //*_access = false ;


        return true ;
    }

    return false ;
}

//***
bool_t app::after_update( size_t const iter )
{
    _update_residual -= iter * _update_interval ;

    return true ;
}

//***
bool_t app::before_physics( std::chrono::microseconds const & dt ) noexcept
{
    _physics_residual += dt ;
    return _physics_residual >= _physics_interval ;
}

bool_t app::after_physics( size_t const iter ) 
{
    _physics_residual -= iter * _physics_interval ;
    return true ;
}

//***
bool_t app::before_render( std::chrono::microseconds const & dt ) noexcept
{
    _render_residual += dt ;
    
    size_t windows = this_t::push_windows() ;

    // check if async system is ready
    for( auto & d : _windows2 )
    {
        if( d.fe != nullptr && d.fe->can_enter_frame() )
            --windows ;
    }

    if( windows != 0 )
    {
        this_t::pop_windows() ;
    }

    return windows == 0 ;
}

//***
bool_t app::after_render( size_t const ) noexcept
{
    //++_render_count ;
    _render_residual = decltype(_render_residual)(0) ;

    this_t::pop_windows() ;

    return true ;
}

//***
bool_t app::before_audio( std::chrono::microseconds const & dt ) noexcept
{
    _audio_residual += dt ;
    return _audio_residual >= _audio_interval ;
}

//***
void_t app::after_audio( size_t const ) noexcept
{
    _physics_residual = std::chrono::microseconds( 0 ) ;
    _first_audio = false ;
}

//***
void_t app::display_engine_stats( void_t ) noexcept
{    
    if ( ImGui::IsKeyReleased( ImGuiKey_F2 ) )
    {
        _display_engine_stats = !_display_engine_stats ;
    }

    if ( !_display_engine_stats && motor::tool::custom_imgui_widgets::overlay_begin( "EngineStatsOverlay" ) )
    {
        ImGui::Text( "Press F2 for Engine Stats" );
        motor::tool::custom_imgui_widgets::overlay_end() ;
    }
    else
    {
        this_t::display_profiling_data() ;
    }
}

//***
void_t app::display_profiling_data( void_t ) noexcept
{
    _engine_profiling.display() ;
}

//**********************************************************************
void_t app::create_tcp_client( motor::string_in_t name, motor::network::ipv4::binding_point_host_in_t bp, 
    motor::network::iclient_handler_mtr_rref_t handler ) noexcept 
{
    _carrier->network_system()->modules( [&]( motor::network::imodule_mtr_t mod )
    {
         _app_client_handler_wrapper * wrapper = motor::shared( 
             _app_client_handler_wrapper( this, motor::move(handler) ),
        "[app::create_tcp_client] : network_client_handler_wrapper" ) ;

        auto const sid = mod->create_tcp_client( { name, bp, motor::share( wrapper ) } ) ;

        if( sid != motor::network::socket_id_t( -1 ) )
        {
            std::lock_guard< std::mutex > lk( _mtx_networks ) ;
            _networks.emplace_back( this_t::network_store{ sid, wrapper } ) ;
            return true ;
        }
        return false ;
    } ) ;
}

//**********************************************************************
void_t app::remove( _app_client_handler_wrapper * handler ) noexcept 
{
    std::lock_guard< std::mutex > lk( _mtx_networks ) ;

    auto const iter = std::find_if( _networks.begin(), _networks.end(), [&]( this_t::network_store const & d )
    {
        return d.wrapper == handler ;
    } ) ;

    if( iter == _networks.end() ) return ;
    motor::memory::release_ptr( iter->wrapper ) ;
    _networks.erase( iter ) ;
}