
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
    for( auto & d : _windows )
    {
        motor::memory::release_ptr( d.wnd ) ;
        motor::memory::release_ptr( d.lst ) ;
    }
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
        _windows.emplace_back( this_t::window_data{ wnd, msgl } ) ;
        ret = _windows.size() ;
    }

    return ret ;
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
        decltype(_windows) tmp ;

        // race condition prevention
        {
            std::lock_guard< std::mutex > lk( _mtx_windows ) ;
            tmp = std::move( _windows ) ;
        }

        for( auto & d : tmp )
        {
            motor::application::window_message_listener_t::state_vector_t sv ;
            if( d.lst->swap_and_reset( sv ) )
            {
                if( sv.create_changed )
                {
                    // on_event -> window created
                }
                if( sv.close_changed )
                {
                    // on_event -> window closed
                }
            }
        }
        
        // merge back
        {
            std::lock_guard< std::mutex > lk( _mtx_windows ) ;
            if( _windows.size() == 0 ) _windows = std::move( tmp ) ;
            else {for( auto & d : tmp ) _windows.emplace_back( std::move(d) ) ;}
        }
    }

    if( this_t::before_update( dt_micro ) )
    {
        this_t::update_data_t dat ;
        dat.micro_dt = _update_interval.count() ;
        dat.sec_dt = float_t( double_t(_update_interval.count()) / 1000000.0 ) ;
        dat.milli_dt = dat.micro_dt / 1000 ;

        size_t const num_iter = _update_residual / _update_interval ;
        
        // could do : for( 0, num_iter )
        auto const res = this->on_update( dat ) ;

        if( res == motor::application::result::close )
            this_t::close() ;
            

        this_t::after_update( num_iter ) ;
    }

    return true ;
}

//**************************************************************************************************************
bool_t app::carrier_shutdown( void_t ) noexcept 
{
    this->on_shutdown() ;
    return true ;
}

//**************************************************************************************************************
bool_t app::before_tool( std::chrono::microseconds const & ) noexcept
{
    // looking for device. It is managed, so pointer must be copied.
    _carrier->device_system()->search( [&] ( motor::device::idevice_borrow_t::mtr_t dev_in )
    {
        if( auto * ptr1 = dynamic_cast<motor::device::three_device_mtr_t>(dev_in); ptr1 != nullptr )
        {
            _dev_mouse = ptr1 ;
        }
        else if( auto * ptr2 = dynamic_cast<motor::device::ascii_device_mtr_t>(dev_in); ptr2 != nullptr )
        {
            _dev_ascii = ptr2 ;
        }
    } ) ;

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
    _dev_mouse = nullptr ;
    _dev_ascii = nullptr ;

    return true ;
}

//**************************************************************************************************************
bool_t app::before_device( std::chrono::microseconds const & dt ) noexcept 
{
    #if 0
    _device_residual += dt ;

    if( _device_residual >= _device_interval )
    {
        motor::device::global_t::system()->update() ;
        return true ;
    }
    #endif
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
bool_t app::before_update( std::chrono::microseconds const & dt ) noexcept
{
    _update_residual += dt ;

    if( _update_residual >= _update_interval )
    {
        //*_access = false ;

        #if 0
        size_t id = 0 ;
        for( auto & pwi : _windows )
        {
            // check messages from the window
            {
                motor::application::window_message_receiver_t::state_vector sv ;
                if( pwi.msg_recv->swap_and_reset( sv ) )
                {

                    motor::graphics::backend_t::window_info_t wi ;
                    if( sv.resize_changed )
                    {
                        wi.width = sv.resize_msg.w ;
                        wi.height = sv.resize_msg.h ;

                        motor::tool::imgui_t::window_data_t wd ;
                        wd.width = int_t( wi.width ) ;
                        wd.height = int_t( wi.height ) ;
                        pwi.imgui->update( wd ) ;
                    }

                    this_t::window_event_info_t wei ;
                    wei.w = uint_t( wi.width ) ;
                    wei.h = uint_t( wi.height ) ;
                    this->on_event( id++, wei ) ;
                }
            }

            // check and send message to the window
            if( pwi.msg_send->has_any_change() )
            {
                pwi.wnd->check_for_messages() ;
            }
        }
        #endif

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
    #if 0
    _render_residual += dt ;

    size_t windows = _windows.size() ;

    // check if async system is ready
    for( auto & pwi : _windows )
    {
        if( pwi.async->enter_frame() )
            --windows ;
    }
    return windows == 0 ;
    #else
    return false ;
    #endif
}

//***
bool_t app::after_render( size_t const )
{
    #if 0
    ++_render_count ;
    for( auto& pwi : _windows )
    {
        pwi.async->leave_frame() ;
    }

    _render_residual = decltype(_render_residual)(0) ;
    #endif
    return true ;
}

//***
bool_t app::before_audio( std::chrono::microseconds const & dt ) noexcept
{
    #if 0
    _audio_residual += dt ;

    size_t audio = _audios.size() ;

    // check if async system is ready
    for( auto& pwi : _audios )
    {
        if( pwi.async->enter_frame() )
            --audio ;
    }

    return audio == 0 ;
    #else
    return false ;
    #endif
}

//***
bool_t app::after_audio( size_t const ) 
{
    #if 0
    ++_audio_count ;
    for( auto& pwi : _audios )
    {
        pwi.async->leave_frame() ;
    }

    _audio_residual = decltype(_audio_residual)(0) ;
    #endif
    return true ;
}