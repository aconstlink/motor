
#include "global.h"

// required for deinit //
#include <motor/io/global.h>
#include <motor/log/global.h>
#include <motor/concurrent/global.h>
#include <motor/profiling/global.h>
//////////////////////////

#if defined( MOTOR_GRAPHICS_WGL )

#include "application/wgl/wgl_context.h"
#include "application/win32/win32_carrier.h"

#elif defined( MOTOR_GRAPHICS_GLX )

//#include "application/glx/glx_context.h"
#include "application/xlib/xlib_carrier.h"

#elif defined( MOTOR_GRAPHICS_EGL )

#endif

using namespace motor ;
using namespace motor::platform ;

//********************************************************************************
motor::application::carrier_mtr_safe_t global::create_carrier( void_t ) noexcept 
{
    #if defined( MOTOR_GRAPHICS_WGL )

    return motor::memory::create_ptr<motor::platform::win32::win32_carrier_t>( 
        "[platform::global] : win32 carrier" ) ;
    
    #elif defined( MOTOR_GRAPHICS_GLX )

    return motor::memory::create_ptr<motor::platform::xlib::xlib_carrier_t>( 
        "[platform::global] : xlib carrier" ) ;

    #elif defined( MOTOR_GRAPHICS_EGL )

    return motor::application::xlib::application_res_t(
        motor::application::xlib::xlib_application_t() ) ;

    #endif
    
    return motor::application::carrier_mtr_safe_t::make() ;
}

//********************************************************************************
motor::application::carrier_mtr_safe_t global::create_carrier( motor::application::app_mtr_safe_t app ) noexcept 
{
    #if defined( MOTOR_GRAPHICS_WGL )

    return motor::memory::create_ptr( motor::platform::win32::win32_carrier_t( motor::move(app) ),
        "[platform::global] : win32 carrier" ) ;

    #elif defined( MOTOR_GRAPHICS_GLX )

    return motor::memory::create_ptr( motor::platform::xlib::xlib_carrier_t( motor::move(app) ),
        "[platform::global] : xlib carrier" ) ;

    #elif defined( MOTOR_GRAPHICS_EGL )

    return todo ;
    #endif

    return motor::application::carrier_mtr_safe_t::make() ;
}

//********************************************************************************
int_t global::create_and_exec_carrier_( motor::application::app_mtr_safe_t app ) noexcept
{
    auto ptr = this_t::create_carrier( motor::move( app ) ) ;
    auto const r = ptr->exec() ;
    motor::memory::release_ptr( motor::move( ptr ) ) ;

    motor::concurrent::global::deinit() ;
    motor::log::global::deinit() ;
    motor::profiling::global::deinit() ;
    motor::io::global::deinit() ;    

    return r ;
}