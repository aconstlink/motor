#pragma once

#include "api.h"
#include "typedefs.h"
#include "result.h"

#include "object/capture_object.hpp"
#include "object/buffer_object.hpp"

namespace motor
{
    namespace audio
    {
        class MOTOR_AUDIO_API backend
        {
            motor_this_typedefs( backend ) ;

        private:

            size_t _backend_id = size_t( -1 ) ;

        public:

            struct execute_detail
            {
                motor::audio::execution_options to ;
                size_t sample = size_t( -1 ) ;
                float_t sec = float_t( -1 ) ;
                bool_t loop = false ;
            };
            motor_typedef( execute_detail ) ;

        public: // ctor

            backend( void_t ) noexcept ;

            backend( this_cref_t ) = delete ;
            backend( this_rref_t rhv ) noexcept
            {
                _backend_id = rhv._backend_id ;
                rhv._backend_id = 0 ;
            }
            virtual ~backend( void_t ) noexcept {}

        public:

            virtual motor::audio::result configure( motor::audio::capture_type const, 
                motor::audio::capture_object_mtr_t ) noexcept = 0 ;
            virtual motor::audio::result capture( motor::audio::capture_object_mtr_t, bool_t const b = true ) noexcept = 0 ;
            virtual motor::audio::result release( motor::audio::capture_object_mtr_t ) noexcept = 0 ;

            virtual motor::audio::result configure( motor::audio::buffer_object_mtr_t ) noexcept = 0 ;
            virtual motor::audio::result update( motor::audio::buffer_object_mtr_t ) noexcept = 0 ;
            virtual motor::audio::result execute( motor::audio::buffer_object_mtr_t, motor::audio::backend::execute_detail_cref_t ) noexcept = 0 ;
            virtual motor::audio::result release( motor::audio::buffer_object_mtr_t ) noexcept = 0 ;

            virtual void_t init( void_t ) noexcept = 0 ;
            virtual void_t release( void_t ) noexcept = 0 ;
            virtual void_t begin( void_t ) noexcept = 0 ;
            virtual void_t end( void_t ) noexcept = 0 ;

        protected:
            
            size_t get_bid( void_t ) const noexcept { return _backend_id ; }

        private:

            size_t create_backend_id( void_t ) noexcept
            {
                static std::mutex mtx ;
                std::lock_guard< std::mutex > lk( mtx ) ;
                static size_t id = 0 ;
                return id++ ;
            }

        };
        motor_typedef( backend ) ;
    }
}