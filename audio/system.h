
#pragma once

#include "api.h"
#include "typedefs.h"
#include "frontend.hpp"
#include "command_engine.h"
#include "backend.h"

#include <functional>
#include <thread>

namespace motor
{
    namespace audio
    {
        class MOTOR_AUDIO_API system
        {
            motor_this_typedefs( system ) ;

        private:

            motor::audio::backend_ptr_t _bptr ;
            motor::audio::command_engine_t _come ;

            std::thread _thr ;
            bool_t _run = false ;

        public:

            system( void_t ) noexcept ;
            system( this_cref_t ) = delete ;
            system( this_rref_t ) noexcept ;
            ~system( void_t ) noexcept ;

        public: // using entity
            
            // pass audio frontend to the user.
            bool_t on_audio( std::function< void_t ( motor::audio::frontend_ptr_t fptr ) > funk ) noexcept ;


        public:

            struct controller
            {
                void_t start( this_ptr_t ptr, motor::audio::backend_ptr_t bptr ) noexcept { ptr->start_system(bptr) ;}
                void_t stop( this_ptr_t ptr, bool_t const wait_for_it ) noexcept { ptr->shutdown_system( wait_for_it ) ; }
            };

        private: // controlling entity

            void_t start_system( motor::audio::backend_ptr_t ) noexcept ;
            void_t shutdown_system( bool_t const ) noexcept ;
        };
        motor_typedef( system ) ;
    }
}