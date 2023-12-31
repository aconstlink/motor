#pragma once

#include "api.h"
#include "result.h"
#include "typedefs.h"

#include "iapp.h"

#include "window/iwindow.h"
#include "window/window_info.h"

#include <thread>

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API carrier
        {
            motor_this_typedefs( carrier ) ;

        private:
            
            motor::application::iapp_mtr_t _app = nullptr ;

            struct shared_data
            {
                bool_t update_running = false ;
            };
            shared_data* _sd ;

            std::mutex _mtx ;
            std::thread _thr ;

        public:

            carrier( void_t ) noexcept ;
            carrier( this_cref_t ) = delete ;
            carrier( this_rref_t ) noexcept ;
            carrier( motor::application::iapp_mtr_shared_t ) noexcept ;
            carrier( motor::application::iapp_mtr_unique_t ) noexcept ;
            virtual ~carrier( void_t ) noexcept ;

        public:
            
            int_t exec( void_t ) noexcept ;

        public: // window creation interface

            virtual motor::application::iwindow_mtr_unique_t create_window( motor::application::window_info_cref_t info ) noexcept = 0 ;

        private:

            /// at the moment, the app object must be added by the applications' ctor
            /// this is due to multi-threading issues when creating a window in the app.
            motor::application::result set( motor::application::iapp_mtr_shared_t ) noexcept ;
            motor::application::result set( motor::application::iapp_mtr_unique_t ) noexcept ;

            motor::application::result start_update_thread( void_t ) noexcept ;
            void_t stop_update_thread( void_t ) noexcept ;

        public: // interface 


            // run the carrier. returns on close.
            virtual motor::application::result on_exec( void_t ) noexcept = 0 ;

            // close the carrier
            virtual motor::application::result close( void_t ) noexcept = 0 ;
        };
        motor_typedef( carrier ) ;
    }
}