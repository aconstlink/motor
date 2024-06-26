

#pragma once

#include "iwindow.h"
#include "window_message.h"
#include "iwindow_message_listener.h"
#include <motor/graphics/render_engine.h>
#include <motor/graphics/frontend/ifrontend.h>

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
            std::mutex _mtx_outs ;
            motor::vector< motor::application::iwindow_message_listener_mtr_t > _outs ;

            motor::graphics::render_engine_ptr_t _re = nullptr ;
            motor::graphics::ifrontend_ptr_t _fe = nullptr ;


            std::mutex _mtx_rnd ;
            size_t _borrow = 0 ;

        public:

            window( void_t ) noexcept ;
            window( this_cref_t ) = delete ;
            window( this_rref_t rhv ) noexcept ;
            virtual ~window( void_t ) noexcept ;

        public:

            size_t set_renderable( motor::graphics::render_engine_ptr_t, motor::graphics::ifrontend_ptr_t ) noexcept ;
            
        public: // ins

            virtual void_t register_in( motor::application::iwindow_message_listener_mtr_safe_t l ) noexcept override;
            virtual void_t unregister_in( motor::application::iwindow_message_listener_mtr_t l ) noexcept override ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_listener_funk_t ;
            virtual void_t foreach_in( foreach_listener_funk_t funk ) noexcept override ;

        public: // outs

            virtual void_t register_out( motor::application::iwindow_message_listener_mtr_safe_t l ) noexcept override ;
            virtual void_t unregister_out( motor::application::iwindow_message_listener_mtr_t l ) noexcept override ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_out_funk_t ;
            virtual void_t foreach_out( foreach_out_funk_t funk ) noexcept override ;

        public:

            virtual void_t send_message( motor::application::show_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::resize_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::create_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::close_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::screen_dpi_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::screen_size_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::vsync_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::fullscreen_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::cursor_message_cref_t ) noexcept ;
            virtual void_t send_message( motor::application::mouse_message_cref_t ) noexcept ;

        public:

            virtual motor::graphics::ifrontend_ptr_t borrow_frontend( void_t ) noexcept ;
            virtual void_t return_borrowed( motor::graphics::ifrontend_ptr_t ptr ) noexcept ;
        };
        motor_typedef( window ) ;
    }
}
