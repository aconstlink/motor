

#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "iwindow_message_listener.h"

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API iwindow
        {
        public:

            virtual ~iwindow( void_t ) {}

        public: // ins

            virtual void_t register_in( motor::application::iwindow_message_listener_mtr_shared_t l ) noexcept = 0 ;
            virtual void_t register_in( motor::application::iwindow_message_listener_mtr_unique_t l ) noexcept = 0 ;
            virtual void_t unregister_in( motor::application::iwindow_message_listener_mtr_t l ) noexcept = 0 ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_listener_funk_t ;
            virtual void_t foreach_in( foreach_listener_funk_t funk ) noexcept = 0 ;

        public: // outs

            virtual void_t register_out( motor::application::iwindow_message_listener_mtr_shared_t l ) noexcept = 0 ;
            virtual void_t register_out( motor::application::iwindow_message_listener_mtr_unique_t l ) noexcept = 0 ;
            virtual void_t unregister_out( motor::application::iwindow_message_listener_mtr_t l ) noexcept = 0 ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_out_funk_t ;
            virtual void_t foreach_out( foreach_out_funk_t funk ) noexcept = 0 ;

        public:

            virtual void_t send_message( motor::application::show_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::resize_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::create_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::close_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::screen_dpi_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::screen_size_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::vsync_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::fullscreen_message_cref_t ) noexcept = 0 ;

        public:

            // check for user messages and pass further to window implementation.
            virtual void_t check_for_messages( void_t ) noexcept = 0 ;
        } ;
        motor_typedef( iwindow ) ;
    }
}