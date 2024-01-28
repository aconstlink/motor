
#pragma once

#include "../backend.h"

namespace motor
{
    namespace audio
    {
        namespace gen2
        {
            class MOTOR_AUDIO_API xaudio2_backend : public motor::audio::backend
            {
                motor_this_typedefs( xaudio2_backend ) ;

                struct pimpl ;
                pimpl* _pimpl = nullptr ;

            public:

                xaudio2_backend( void_t ) noexcept ;
                xaudio2_backend( this_cref_t ) = delete ;
                xaudio2_backend( this_rref_t ) noexcept ;
                virtual ~xaudio2_backend( void_t ) noexcept ;

                this_ref_t operator = ( this_rref_t ) noexcept ;

            public:

                virtual motor::audio::result configure( motor::audio::capture_type const, 
                    motor::audio::capture_object_mtr_t ) noexcept ;

                virtual motor::audio::result capture( motor::audio::capture_object_mtr_t, bool_t const b = true ) noexcept ;
                virtual motor::audio::result release( motor::audio::capture_object_mtr_t ) noexcept ;

                virtual motor::audio::result configure( motor::audio::buffer_object_mtr_t ) noexcept ;
                virtual motor::audio::result update( motor::audio::buffer_object_mtr_t ) noexcept ;
                virtual motor::audio::result execute( motor::audio::buffer_object_mtr_t, motor::audio::gen2::backend::execute_detail_cref_t ) noexcept ;
                virtual motor::audio::result release( motor::audio::buffer_object_mtr_t ) noexcept ;

            public:

                virtual void_t init( void_t ) noexcept ;
                virtual void_t release( void_t ) noexcept ;
                virtual void_t begin( void_t ) noexcept ;
                virtual void_t end( void_t ) noexcept ;
            };
            motor_typedef( xaudio2_backend ) ;
        }
    }
}