
#include "carrier.h"
//#include "../app.h"

#include <motor/memory/global.h>
#include <motor/log/global.h>

using namespace motor ;
using namespace motor::application ;

//******************************************************
carrier::carrier( void_t ) noexcept
{
    _sd = motor::memory::global::alloc< this_t::shared_data >("shared data") ;
    _dev_system = motor::memory::global_t::alloc( motor::device::system_t(), 
        "[carrier] : device system" ) ;
    _audio_system = motor::memory::global_t::alloc( motor::audio::system_t(), 
        "[carrier] : audio system" ) ;
}

//******************************************************
carrier::carrier( this_rref_t rhv ) noexcept
{
    _thr = std::move( rhv._thr ) ;
    motor_move_member_ptr( _sd, rhv ) ;
    _app = motor::move( rhv._app ) ;

    _dev_system = motor::move( rhv._dev_system ) ;
    _audio_system = motor::move( rhv._audio_system ) ;
}

//******************************************************
carrier::carrier( motor::application::iapp_mtr_safe_t app ) noexcept : this_t()
{
    this_t::set( motor::move( app ) ) ;
}

//******************************************************
carrier::~carrier( void_t ) noexcept
{
    motor::memory::global_t::dealloc( _sd ) ;
    motor::memory::global_t::dealloc( _dev_system ) ;
    motor::memory::global_t::dealloc( _audio_system ) ;
}

//******************************************************
motor::application::result carrier::set( motor::application::iapp_mtr_safe_t app ) noexcept
{
    if( _app != nullptr )
        return motor::application::result::failed ;

    _app = app ;

    return motor::application::result::ok ;
}

//******************************************************
motor::application::result carrier::start_update_thread( void_t ) noexcept 
{
    if( _app == nullptr )
    {
        motor::log::global_t::error( motor_log_fn("No app object") ) ;
        return motor::application::result::no_app ;
    }

    _sd->update_running = true ;
    
    _thr = std::thread( [=]( void_t )
    {
            #if 0
        _app->platform_init() ;

        while( _sd->update_running )
        {
            _app->platform_update() ;
        }
        
        _app->on_shutdown() ;
        #endif
        
    } ) ;

    return motor::application::result::ok ;
}

//******************************************************
void_t carrier::stop_update_thread( void_t ) noexcept
{
    if( !_sd->update_running ) return ;

    _sd->update_running = false ;
    _thr.join() ;
}

//******************************************************
int_t carrier::exec( void_t ) noexcept
{
    {
        auto const res = this_t::start_update_thread() ;
        if( motor::application::no_success( res ) )
        {
            motor::log::global_t::warning( motor_log_fn(
                "No update thread started. Proceed anyway.") ) ;
        }
    }

    // virtual call
    {
        this->on_exec() ;
    }

    this_t::stop_update_thread() ;

    return 0 ;
}

//******************************************************
motor::device::system_ptr_t carrier::get_dev_system( void_t ) noexcept 
{
    return _dev_system ;
}

//******************************************************
motor::audio::system_ptr_t carrier::get_audio_system( void_t ) noexcept 
{
    return _audio_system ;
}

//******************************************************
void_t carrier::update_device_system( void_t ) noexcept 
{
    _dev_system->update() ;
}

//******************************************************
motor::device::system_mtr_t carrier::device_system( void_t ) noexcept 
{
    return _dev_system ;
}

//******************************************************
motor::audio::system_ptr_t carrier::audio_system( void_t ) noexcept 
{
    return _audio_system ;
}