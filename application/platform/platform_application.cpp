
#include "platform_application.h"
#include "../app.h"

#include <motor/device/global.h>
#include <motor/memory/global.h>
#include <motor/log/global.h>

using namespace motor ;
using namespace motor::application ;

//******************************************************
platform_application::platform_application( void_t ) 
{
    _sd = motor::memory::global::alloc< this_t::shared_data >() ;
}

//******************************************************
platform_application::platform_application( this_rref_t rhv )
{
    _thr = ::std::move( rhv._thr ) ;
    motor_move_member_ptr( _sd, rhv ) ;
    _app = ::std::move( rhv._app ) ;
}

//******************************************************
platform_application::platform_application( motor::application::app_res_t app ) 
{
    this_t::set( app ) ;
    _sd = motor::memory::global::alloc< this_t::shared_data >( motor::memory::purpose_t("[platform_application] : shader_data") ) ;
}

//******************************************************
platform_application::~platform_application( void_t )
{
    if( motor::core::is_not_nullptr(_sd) )
    {
        motor::memory::global_t::dealloc( _sd ) ;
    }
}

//******************************************************
motor::application::result platform_application::set( motor::application::app_res_t app )
{
    if( _app.is_valid() )
        return motor::application::result::failed ;

    _app = app ;

    return motor::application::result::ok ;
}

//******************************************************
motor::application::result platform_application::start_update_thread( void_t ) 
{
    if( motor::core::is_not( _app.is_valid() ) )
    {
        motor::log::global_t::error( motor_log_fn("No app object") ) ;
        return motor::application::result::no_app ;
    }

    _sd->update_running = true ;
    
    _thr = motor::concurrent::thread_t( [=]( void_t )
    {
        _app->platform_init() ;

        while( _sd->update_running )
        {
            _app->platform_update() ;
        }
        
        _app->on_shutdown() ;

        
    } ) ;

    return motor::application::result::ok ;
}

//******************************************************
void_t platform_application::stop_update_thread( void_t ) 
{
    if( motor::core::is_not( _sd->update_running ) )
        return ;

    _sd->update_running = false ;
    _thr.join() ;
}

//******************************************************
int_t platform_application::exec( void_t )
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
