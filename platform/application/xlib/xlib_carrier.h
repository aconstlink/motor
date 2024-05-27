#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include "../../controls/linux/xlib_module.h"

#include <motor/application/window/window.h>
#include <motor/application/window/window_message_listener.h>
#include <motor/application/carrier.h>

#include <X11/Xlib.h>

namespace motor
{
    namespace platform
    {
        namespace xlib
        {
            class xlib_carrier : public motor::application::carrier
            {
                using base_t = motor::application::carrier_t ;
                motor_this_typedefs( xlib_carrier ) ;

            private:

                /// the main display connection. Required to be
                /// a singleton in order to be able for single
                /// windows to be connected to this display.
                /// I would like to manage all windows and dispatch
                /// all window messaged centralized in this 
                /// application.
                Display * _display = nullptr ;
                size_t _display_use_count = 0 ;

                Display * connect_display( void_t ) noexcept ;
                void_t disconnect_display( void_t ) noexcept ;
                Display * move_display( void_t ) noexcept ;

                motor::platform::xlib::xlib_module_mtr_t _device_module ;
            
                bool_t _done = false ;

            private:

                struct xlib_window_data
                {
                    Window hwnd ;
                    motor::application::window_mtr_t wnd ;
                    motor::application::window_message_listener_mtr_t lsn ;
                    // can be used to store messages to be used continuously.
                    motor::application::window_message_listener_t::state_vector_t sv ;

                    // store window text for later alteration.
                    motor::string_t window_text ;

                    int_t x ;
                    int_t y ;
                    int_t width ;
                    int_t height ;
                };
                motor_typedef( xlib_window_data ) ;

                motor::vector< xlib_window_data_t > _xlib_windows ;

            private: // wgl window data

                struct glx_pimpl ;

                struct glx_window_data
                {
                    Window hwnd ;
                    glx_pimpl * ptr ;
                };
                motor_typedef( glx_window_data ) ;

                motor::vector< glx_window_data_t > _glx_windows ;

            private: // destruction queue 

                motor::vector< Window > _destroy_queue ;

            private: // xlib windows queue

                struct window_queue_msg
                {
                    motor::application::window_info_t wi ;
                    motor::application::window_mtr_t wnd ;
                    motor::application::window_message_listener_mtr_t lsn ;
                };
                motor_typedef( window_queue_msg ) ;

                std::mutex _mtx_queue ;
                motor::vector< window_queue_msg_t > _queue ;

            public:

                xlib_carrier( void_t ) noexcept ;
                xlib_carrier( this_rref_t ) noexcept ;
                xlib_carrier( motor::application::app_mtr_safe_t ) noexcept ;
                virtual ~xlib_carrier( void_t ) noexcept ;

            public:

                virtual motor::application::result on_exec( void_t ) noexcept override ;
                virtual motor::application::result close( void_t ) noexcept override ;

                virtual motor::application::iwindow_mtr_safe_t create_window( motor::application::window_info_cref_t info ) noexcept override ;

            private:

                Window create_xlib_window( motor::application::window_info_cref_t ) noexcept ;
                bool_t handle_destroyed_hwnd( Window hwnd ) noexcept ;
                void_t send_destroy( xlib_window_data_in_t d ) noexcept ;
                void_t send_create( xlib_window_data_in_t ) noexcept ;
                void_t send_resize( xlib_window_data_in_t ) noexcept ;

                using find_window_info_funk_t = std::function< void_t ( this_t::xlib_window_data_ref_t ) > ;
                bool_t find_window_info( Window hwnd, find_window_info_funk_t ) noexcept ;

                void_t create_and_register_device_modules( void_t ) noexcept ;
                void_t create_and_register_network_modules( void_t ) noexcept ;
            };
            motor_typedef( xlib_carrier ) ;
        }
    }
}
