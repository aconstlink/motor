
#include "global.h"


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
motor::application::carrier_mtr_unique_t global::create_carrier( void_t ) noexcept 
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
    
    return motor::application::carrier_mtr_unique_t::make() ;
}

//********************************************************************************
motor::application::carrier_mtr_unique_t global::create_carrier( motor::application::iapp_mtr_shared_t app ) noexcept 
{
    #if defined( MOTOR_GRAPHICS_WGL )

    return motor::memory::create_ptr( motor::platform::win32::win32_carrier_t( std::move(app) ),
        "[platform::global] : win32 carrier" ) ;

    #elif defined( MOTOR_GRAPHICS_GLX )

    return motor::memory::create_ptr( motor::platform::xlib::xlib_carrier_t( std::move(app) ),
        "[platform::global] : xlib carrier" ) ;

    #elif defined( MOTOR_GRAPHICS_EGL )

    return todo ;
    #endif

    return motor::application::carrier_mtr_unique_t::make() ;
}

//********************************************************************************
motor::application::carrier_mtr_unique_t global::create_carrier( motor::application::iapp_mtr_unique_t app ) noexcept 
{
    #if defined( MOTOR_GRAPHICS_WGL )

    return motor::memory::create_ptr( motor::platform::win32::win32_carrier_t( std::move(app) ),
        "[platform::global] : win32 carrier" ) ;

    #elif defined( MOTOR_GRAPHICS_GLX )

    return motor::memory::create_ptr( motor::platform::xlib::xlib_carrier_t( std::move(app) ),
        "[platform::global] : xlib carrier" ) ;

    #elif defined( MOTOR_GRAPHICS_EGL )

    return todo ;
    #endif

    return motor::application::carrier_mtr_unique_t::make() ;
}

//********************************************************************************
int_t global::create_and_exec_carrier( motor::application::iapp_mtr_shared_t app ) noexcept 
{
    auto r = this_t::create_carrier( std::move( app ) )->exec() ;

    return r ;
}

//********************************************************************************
int_t global::create_and_exec_carrier( motor::application::iapp_mtr_unique_t app ) noexcept 
{
    auto r = this_t::create_carrier( std::move( app ) )->exec() ;

    return r ;
}
