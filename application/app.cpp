
#include "app.h"

#include "carrier.h"

using namespace motor::application ;

//**************************************************************************************************************
app::app( void_t ) noexcept
{
}

//**************************************************************************************************************
app::app( this_rref_t rhv ) noexcept
{
    _windows = std::move( rhv._windows ) ;
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
            size_t const i = std::distance( iter, _windows2.end() ) ;

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
                
            }
            ++iter ;
        }
        
        this_t::pop_windows() ;
    }

    if( this_t::before_device( dt_micro ) )
    {
        {
            for( auto & d : _windows )
            {
                d.imgui->update( _dev_ascii ) ;
                d.imgui->update( _dev_mouse ) ;
            }
        }

        {
            this_t::device_data_in_t dat = 
            {
                _dev_mouse,
                _dev_ascii
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
            this_t::audio_data ad {_first_audio};
            this->on_audio( fptr, ad ) ;
        } )  ;

        this_t::after_audio(0) ;
    }

    if ( this_t::before_profile( dt_micro ) )
    {
        this_t::profile_data_t dat ;

        #if MOTOR_MEMORY_OBSERVER
        motor::memory::observer_t::observable_data_t od = 
            motor::memory::global_t::get_observer()->swap_and_clear() ;

        for ( auto const & m : od.messages )
        {
            if ( m.type == motor::memory::observer_t::alloc_type::allocation ||
                 m.type == motor::memory::observer_t::alloc_type::managed )
            {
                _profiling_data.memory_allocations.insert( m.sib ) ;
                _profiling_data.memory_current.insert( m.sib ) ;
            }
            else if ( m.type == motor::memory::observer_t::alloc_type::deallocation )
            {
                _profiling_data.memory_deallocations.insert( m.sib ) ;
                _profiling_data.memory_current.remove( m.sib ) ;
            }
        }
        #endif
        
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
                                    this_t::display_profiling_data() ;
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
    
    for( auto & d : _destruction_queue ) 
    {
        if( !this_t::clear_out_window_data( d ) ) 
        {
            return false ;
        }
    }

    _destruction_queue.clear() ;

    for( auto & d : _windows ) 
    {
        if( !this_t::clear_out_window_data( d ) ) 
        {
            return false ;
        }
    }

    _windows.clear() ;

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
void_t app::display_profiling_data( void_t ) noexcept
{
    if ( ImGui::Begin( "Profiling Data" ) )
    {
        using histogram_t = decltype( _profiling_data.memory_current  ) ;
        motor::vector< int_t > values( _profiling_data.memory_current.get_num_entries() )  ;
        _profiling_data.memory_current.for_each_entry( [&] ( size_t const i, histogram_t::data_cref_t d )
        {
            values[ i ] = (int_t)d.count ;
        } ) ;

        int_t const max_value = (int_t)_profiling_data.memory_current.get_max_count() ;

        if ( ImPlot::BeginPlot( "My Plot" ) ) 
        {
            ImPlot::PlotBars( "My Bar Plot", values.data(), (int_t)values.size() );
            
            
            ImPlot::EndPlot();
        }
    }
    ImGui::End() ;
}