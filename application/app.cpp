
#include "app.h"

#include "carrier.h"

#include <motor/profiling/probe_guard.hpp>
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

    size_t wid = size_t( -1 ) ;

    {
        std::lock_guard< std::mutex > lk( _mtx_wid ) ;
        wid = ++_wid ;
    }

    // put new window into creation queue
    {
        std::lock_guard< std::mutex > lk( _mtx_window_cq ) ;
        _creation_queue.emplace_back( this_t::window_data
            { wid, static_cast<motor::application::window_ptr_t>( wnd ), msgl, nullptr,
            motor::shared( motor::tool::imgui_t( motor::to_string( wid ) ) ) } ) ;
    }

    return wid ;
}

//**************************************************************************************************************
void_t app::send_window_message( this_t::window_id_t const wid, 
    std::function< void_t ( this_t::window_view & ) > funk ) 
{
    {
        std::lock_guard< std::mutex > lk( _mtx_windows ) ;
        auto const iter = std::find_if(_windows.begin(), _windows.end(), [&]( this_t::window_data const & wd )
        {
            return wd.wid == wid ;
        } ) ;

        if( iter != _windows.end() ) 
        {
            this_t::window_view accessor( iter->wnd ) ;
            funk( accessor ) ;
            return ;
        }
    }
    {
        std::lock_guard< std::mutex > lk( _mtx_window_cq ) ;
        auto const iter = std::find_if( _creation_queue.begin(), _creation_queue.end(), [&] ( this_t::window_data const & wd )
        {
            return wd.wid == wid ;
        } ) ;

        if ( iter != _creation_queue.end() )
        {
            this_t::window_view accessor( iter->wnd ) ;
            funk( accessor ) ;
            return ;
        }
    }
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
    MOTOR_PROBE( "system", "carrier_update" ) ;

    std::chrono::microseconds dt_micro ;
    float_t dt_sec = 0.0f ;

    // compute dt micro seconds. Here it also yield if the micro seconds == 0
    // this can happen in small projects, so we have to wait until micro != 0
    // I think this is not that much of a problem, because in more complex
    // applications, that will be != 0 anyways
    {
        dt_micro = std::chrono::duration_cast<std::chrono::microseconds>( this_t::platform_clock_t::now() - _tp_platform ) ;

        while( dt_micro == std::chrono::microseconds(0) )
        {
            // best way I figured to slow down the thread.
            std::this_thread::yield() ;
            dt_micro = std::chrono::duration_cast<std::chrono::microseconds>( this_t::platform_clock_t::now() - _tp_platform ) ;
        }

        dt_sec = float_t( double_t( dt_micro.count() ) / 1000000.0 ) ;
        _tp_platform = this_t::platform_clock_t::now() ;
    }

    {
        std::lock_guard< std::mutex > lk( _mtx_window_cq ) ;
        _windows.reserve( _windows.size() + _creation_queue.size() ) ;
        for( auto & wd : _creation_queue )
        {
            _windows.emplace_back( std::move( wd ) ) ;
        }
        _creation_queue.clear() ;
    }

    // do window message updates -> on_event
    {        
        for( auto iter=_windows.begin() ; iter!=_windows.end(); )
        {
            auto & d = *iter ;

            motor::application::window_message_listener_t::state_vector_t sv ;
            if( d.lst->swap_and_reset( sv ) )
            {
                this->on_event( iter->wid, sv ) ;

                if( sv.create_changed )
                {
                    d.fe = d.wnd->borrow_frontend() ;
                }
                
                if( sv.close_changed )
                {
                    _destruction_queue.emplace_back( *iter ) ;
                    iter = _windows.erase( iter ) ;

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
        this_t::graphics_data_t dat_graphic ;

        // do single on_graphics for updating 
        // user graphics system data.
        {
            dat_graphic.micro_dt = _render_residual.count() ;
            dat_graphic.sec_dt = float_t( double_t(_render_residual.count()) / 1000000.0 ) ;
            dat_graphic.milli_dt = dat_graphic.micro_dt / 1000 ;

            this->on_graphics( dat_graphic ) ;
        }

        // render all the present windows
        {
            this_t::render_data_t dat ;
            dat.micro_dt = _render_residual.count() ;
            dat.sec_dt = float_t( double_t(_render_residual.count()) / 1000000.0 ) ;
            dat.milli_dt = dat.micro_dt / 1000 ;
            
            for( auto & d : _windows )
            {
                dat.first_frame = d.first_frame ;
                d.first_frame = false ;

                auto * re = d.fe->borrow_render_engine() ;
                if( re->enter_frame() )
                {
                    if( auto * fe = dynamic_cast<motor::graphics::gen4::frontend_ptr_t>(d.fe); fe != nullptr ) 
                    {
                        this->on_render( d.wid, fe, dat ) ;

                        if( this_t::before_tool( dt_micro ) )
                        {
                            d.imgui->execute( [&] ( void_t )
                            {
                                this_t::tool_data_t td 
                                { 
                                    fe, d.imgui,
                                    dat_graphic.sec_dt,
                                    dat_graphic.micro_dt,
                                    dat_graphic.milli_dt
                                } ;
                                if( this->on_tool( d.wid, td ) )
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

    if( _closed )
    {
        for ( auto iter = _creation_queue.begin(); iter != _creation_queue.end(); )
        {
            if ( !this_t::clear_out_window_data( *iter ) )
            {
                ++iter ;
                continue ;
            }
            iter = _creation_queue.erase( iter ) ;
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
// this function is supposed to be called repeatedly, 
// until it returns true!
{
    // destroy all windows first
    // @note render engine need to be cleared 
    // due to borrowed graphics objects
    {
        for ( auto & d : _windows )
        {
            _destruction_queue.emplace_back( std::move( d ) ) ;
        }
        _windows.clear() ;
    }
    
    {
        for ( auto iter = _destruction_queue.begin(); iter != _destruction_queue.end(); )
        {
            if ( !this_t::clear_out_window_data( *iter ) )
            {
                ++iter ;
                continue ;
            }
            iter = _destruction_queue.erase( iter ) ;
        }

        if ( _destruction_queue.size() > 0 ) return false ;
    }

    // clear out network clients
    {
        for( auto & ns : _networks )
        {
            ns.wrapper->shutdown() ;
            motor::memory::release_ptr( motor::move( ns.wrapper ) ) ;
        }
    }

    if ( !_shutdown_called )
    {
        this->on_shutdown() ;
        _shutdown_called = true ;
    }

    return true ;
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

//************************************************************************
bool_t app::before_physics( std::chrono::microseconds const & dt ) noexcept
{
    _physics_residual += dt ;
    return _physics_residual >= _physics_interval ;
}

//************************************************************************
bool_t app::after_physics( size_t const iter ) 
{
    _physics_residual -= iter * _physics_interval ;
    return true ;
}

//************************************************************************
bool_t app::before_render( std::chrono::microseconds const & dt ) noexcept
{
    _render_residual += dt ;
    
    size_t windows = _windows.size() ;

    // check if async system is ready
    for( auto & d : _windows )
    {
        if( d.fe != nullptr && d.fe->can_enter_frame() )
            --windows ;
    }

    return windows == 0 ;
}

//************************************************************************
bool_t app::after_render( size_t const ) noexcept
{
    //++_render_count ;
    _render_residual = decltype(_render_residual)(0) ;

    return true ;
}

//************************************************************************
bool_t app::before_audio( std::chrono::microseconds const & dt ) noexcept
{
    _audio_residual += dt ;
    return _audio_residual >= _audio_interval ;
}

//************************************************************************
void_t app::after_audio( size_t const ) noexcept
{
    _physics_residual = std::chrono::microseconds( 0 ) ;
    _first_audio = false ;
}

//************************************************************************
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

//************************************************************************
void_t app::display_profiling_data( void_t ) noexcept
{
    _engine_profiling.display() ;
}

//************************************************************************
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

//************************************************************************
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