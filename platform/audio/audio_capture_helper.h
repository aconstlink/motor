
#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/audio/enums.h>
#include <motor/std/vector>

namespace motor
{
    namespace platform
    {
        class MOTOR_PLATFORM_API audio_capture_helper
        {
            motor_this_typedefs( audio_capture_helper ) ;

        public:

            virtual bool_t init( motor::audio::channels const = motor::audio::channels::mono, 
                motor::audio::frequency const = motor::audio::frequency::freq_48k ) noexcept = 0  ;
            virtual void_t release( void_t ) noexcept = 0 ;
            virtual void_t start( void_t ) noexcept = 0 ;
            virtual void_t stop( void_t ) noexcept = 0 ;
            virtual bool_t capture( motor::vector< float_t > & samples ) noexcept = 0 ;
                
        public:

            // create system specific system audio capture object
            static this_ptr_t create( void_t ) noexcept ;
            
        };
        motor_typedef( audio_capture_helper ) ;
    }
}