

#pragma once

#include "iwindow.h"
#include "window_message.h"
#include "iwindow_message_listener.h"

#include <motor/std/vector>

#include <algorithm>

namespace motor
{
    namespace application
    {
        // the default window implementation with a listener 
        // interface. The final implementation requires to check for platform
        // window messages and sending them to the user.
        class MOTOR_APPLICATION_API window : public motor::application::iwindow
        {
            motor_this_typedefs( window ) ;

        private:

            // this window -> other entity
            motor::vector< motor::application::iwindow_message_listener_mtr_t > _ins ;

            // other entity -> this window
            motor::vector< motor::application::iwindow_message_listener_mtr_t > _outs ;

        public:

            window( void_t ) noexcept ;
            window( this_cref_t ) = delete ;
            window( this_rref_t rhv ) noexcept ;
            virtual ~window( void_t ) noexcept ;

        public: // ins

            void_t register_in( motor::application::iwindow_message_listener_mtr_shared_t l ) noexcept ;
            void_t register_in( motor::application::iwindow_message_listener_mtr_unique_t l ) noexcept ;
            void_t unregister_in( motor::application::iwindow_message_listener_mtr_t l ) noexcept ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_listener_funk_t ;
            void_t foreach_in( foreach_listener_funk_t funk ) noexcept ;

        public: // outs

            void_t register_out( motor::application::iwindow_message_listener_mtr_shared_t l ) noexcept ;
            void_t register_out( motor::application::iwindow_message_listener_mtr_unique_t l ) noexcept ;
            void_t unregister_out( motor::application::iwindow_message_listener_mtr_t l ) noexcept ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_out_funk_t ;
            void_t foreach_out( foreach_out_funk_t funk ) noexcept ;

        public: // iwindow interface 

            virtual void_t check_for_messages( void_t ) noexcept = 0 ;
        };
        motor_typedef( window ) ;
    }
}
