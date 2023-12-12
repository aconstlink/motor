
#include "carrier.h"
//#include "../app.h"

#include <motor/device/global.h>
#include <motor/memory/global.h>
#include <motor/log/global.h>

using namespace motor ;
using namespace motor::application ;

//******************************************************
carrier::carrier( void_t ) noexcept
{
    _sd = motor::memory::global::alloc< this_t::shared_data >() ;
}

//******************************************************
carrier::carrier( this_rref_t rhv ) noexcept
{
    _thr = std::move( rhv._thr ) ;
    motor_move_member_ptr( _sd, rhv ) ;
    _app = motor::move( rhv._app ) ;
}

//******************************************************
carrier::carrier( motor::application::iapp_mtr_shared_t app ) noexcept : this_t()
{
    this_t::set( std::move( app ) ) ;
}

//******************************************************
carrier::carrier( motor::application::iapp_mtr_unique_t app ) noexcept : this_t()
{
    this_t::set( std::move( app ) ) ;
}

//******************************************************
carrier::~carrier( void_t ) noexcept
{
    motor::memory::global_t::dealloc( _sd ) ;
}

//******************************************************
motor::application::result carrier::set( motor::application::iapp_mtr_shared_t app ) noexcept
{
    if( _app != nullptr )
        return motor::application::result::failed ;

    _app = app ;

    return motor::application::result::ok ;
}

//******************************************************
motor::application::result carrier::set( motor::application::iapp_mtr_unique_t app ) noexcept
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
