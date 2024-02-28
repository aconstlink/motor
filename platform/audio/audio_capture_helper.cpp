
#include "audio_capture_helper.h"

#if defined( MOTOR_TARGET_OS_WIN )
#include "wasapi/wasapi_capture_helper.h"
#endif

#include <motor/memory/global.h>
#include <motor/log/global.h>

using namespace motor::platform ;

audio_capture_helper_t::this_mtr_safe_t motor::platform::audio_capture_helper_t::create( void_t ) noexcept 
{
    #if defined( MOTOR_TARGET_OS_WIN )
    return motor::memory::create_ptr( motor::platform::wasapi_capture_helper(), "[audio_capture_helper_t] : create" ) ; 
    motor::log::global_t::status( "Creating wasapi audio capture helper" ) ;
    #else
    motor::log::global_t::status( "No defaul system audio capture available." ) ;
    #endif

    return nullptr ;
}