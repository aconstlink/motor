#pragma once

#include "api.h"
#include "result.h"
#include "typedefs.h"

#include "app.h"

#include "window/iwindow.h"
#include "window/window_info.h"

#include <motor/controls/system.h>
#include <motor/audio/system.h>
#include <motor/network/system.h>

#include <thread>

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API carrier
        {
            motor_this_typedefs( carrier ) ;

        private:
            
            motor::application::app_mtr_t _app = nullptr ;

            struct shared_data
            {
                bool_t update_running = false ;
            };
            shared_data* _sd ;

            std::mutex _mtx ;
            std::thread _thr ;

            motor::controls::system_ptr_t _dev_system = nullptr ;
            motor::audio::system_ptr_t _audio_system = nullptr ;
            motor::network::system_ptr_t _network_system = nullptr ;

            static std::chrono::microseconds sleep_for_micro ;

        public:

            carrier( void_t ) noexcept ;
            carrier( this_cref_t ) = delete ;
            carrier( this_rref_t ) noexcept ;
            carrier( motor::application::app_mtr_safe_t ) noexcept ;
            virtual ~carrier( void_t ) noexcept ;

        public:
            
            int_t exec( void_t ) noexcept ;
            motor::controls::system_mtr_t device_system( void_t ) noexcept ;
            void_t update_device_system( void_t ) noexcept ;

            motor::audio::system_ptr_t audio_system( void_t ) noexcept ;
            motor::network::system_ptr_t network_system( void_t ) noexcept ;

        public: // window creation interface

            virtual motor::application::iwindow_mtr_safe_t create_window( motor::application::window_info_cref_t info ) noexcept = 0 ;

        protected:

            motor::controls::system_ptr_t get_dev_system( void_t ) noexcept ;
            motor::audio::system_ptr_t get_audio_system( void_t ) noexcept ;
            motor::network::system_ptr_t get_network_system( void_t ) noexcept ;

        private:

            /// at the moment, the app object must be added by the applications' ctor
            /// this is due to multi-threading issues when creating a window in the app.
            motor::application::result set( motor::application::app_mtr_safe_t ) noexcept ;

            motor::application::result start_update_thread( void_t ) noexcept ;
            void_t stop_update_thread( void_t ) noexcept ;

        public: // interface 


            // run the carrier. returns on close.
            virtual motor::application::result on_exec( void_t ) noexcept = 0 ;

            // close the carrier
            virtual motor::application::result close( void_t ) noexcept = 0 ;

        public:

            static std::chrono::microseconds set_cpu_sleep( std::chrono::microseconds const ) noexcept ;
            static std::chrono::microseconds get_cpu_sleep( void_t ) noexcept ;
        };
        motor_typedef( carrier ) ;
    }
}