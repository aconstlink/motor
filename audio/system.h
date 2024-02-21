
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

            motor::audio::frontend_ptr_t _fptr = nullptr ;
            motor::audio::backend_mtr_t _bptr = nullptr ;
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
                this_ptr_t _ptr ;
                controller( this_ptr_t ptr ) noexcept : _ptr(ptr) {}
                void_t start( motor::audio::backend_mtr_safe_t bptr ) noexcept { _ptr->start_system(motor::move(bptr)) ;}
                void_t stop( bool_t const wait_for_it ) noexcept { _ptr->shutdown_system( wait_for_it ) ; }
            };

        private: // controlling entity

            void_t start_system( motor::audio::backend_mtr_safe_t ) noexcept ;
            void_t shutdown_system( bool_t const ) noexcept ;
        };
        motor_typedef( system ) ;
    }
}