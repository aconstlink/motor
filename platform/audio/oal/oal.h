

#pragma once

#include "../../api.h"
#include "../../typedefs.h"
#include <motor/audio/backend.h>

namespace motor
{
    namespace platform
    {
        class MOTOR_PLATFORM_API oal_backend : public motor::audio::backend
        {
            motor_this_typedefs( oal_backend ) ;

        private:

            struct pimpl ;
            pimpl* _pimpl = nullptr ;

            size_t _what_u_hear_count = 0 ;

        public:

            oal_backend( void_t ) noexcept ;
            oal_backend( this_cref_t ) = delete ;
            oal_backend( this_rref_t ) noexcept ;
            virtual ~oal_backend( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            virtual motor::audio::result configure( motor::audio::capture_type const, 
                motor::audio::capture_object_mtr_t ) noexcept ;

            virtual motor::audio::result capture( motor::audio::capture_object_mtr_t, bool_t const b = true ) noexcept ;
            virtual motor::audio::result release( motor::audio::capture_object_mtr_t ) noexcept ;

            virtual motor::audio::result configure( motor::audio::buffer_object_mtr_t ) noexcept ;
            virtual motor::audio::result update( motor::audio::buffer_object_mtr_t ) noexcept ;
            virtual motor::audio::result execute( motor::audio::buffer_object_mtr_t, motor::audio::backend::execute_detail_cref_t ) noexcept ;
            virtual motor::audio::result release( motor::audio::buffer_object_mtr_t ) noexcept ;

        public:

            virtual bool_t init( void_t ) noexcept ;
            virtual void_t release( void_t ) noexcept ;
            virtual void_t begin( void_t ) noexcept ;
            virtual void_t end( void_t ) noexcept ;

        };
        motor_typedef( oal_backend ) ;
    }
}