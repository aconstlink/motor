#pragma once

#include "../../api.h"
#include "../../device/win32/rawinput_module.h"
#include "../../device/win32/xinput_module.h"

#include <motor/application/window/window_message_listener.h>
#include <motor/application/carrier.h>

#include <mutex>

namespace motor
{
    namespace platform
    {
        namespace win32
        {
            class MOTOR_PLATFORM_API win32_carrier : public motor::application::carrier
            {
                using base_t = motor::application::carrier_t ;
                motor_this_typedefs( win32_carrier ) ;

            private:

                bool_t _done = false ;
                                
                motor::platform::win32::rawinput_module_mtr_t _rawinput = nullptr ;
                motor::platform::win32::xinput_module_mtr_t _xinput  = nullptr ;

            private: // win32 window data

                struct win32_window_data
                {
                    HWND hwnd ;
                    motor::application::iwindow_mtr_t wnd ;
                    motor::application::window_message_listener_mtr_t lsn ;
                    // can be used to store messages to be used continuously.
                    // this needs to be done with cursor messages.
                    motor::application::window_message_listener_t::state_vector_t sv ;

                    // store window text for later alteration.
                    motor::string_t window_text ;
                };
                motor_typedef( win32_window_data ) ;

                motor::vector< win32_window_data_t > _win32_windows ;


            private: // wgl window data

                struct wgl_pimpl ;

                struct wgl_window_data
                {
                    size_t idx_win32_window ;
                    wgl_pimpl * ptr ;
                };
                motor_typedef( wgl_window_data ) ;

                motor::vector< wgl_window_data_t > _wgl_windows ;

            private: // win32 windows queue

                struct window_queue_msg
                {
                    motor::application::window_info_t wi ;
                    motor::application::iwindow_mtr_t wnd ;
                    motor::application::window_message_listener_mtr_t lsn ;
                };
                motor_typedef( window_queue_msg ) ;

                std::mutex _mtx_queue ;
                motor::vector< window_queue_msg_t > _queue ;


            private: // graphics windows queue

                struct graphics_queue_msg
                {
                    motor::application::graphics_window_info_t gi ;
                    motor::application::iwindow_mtr_t wnd ;
                    motor::application::window_message_listener_mtr_t lsn ;
                };
                motor_typedef( graphics_queue_msg ) ;

                std::mutex _mtx_gqueue ;
                motor::vector< graphics_queue_msg_t > _gqueue ;

            private: // destruction queue 

                motor::vector< HWND > _destroy_queue ;

            public:

                win32_carrier( void_t ) noexcept ;
                win32_carrier( motor::application::iapp_mtr_shared_t ) noexcept ;
                win32_carrier( motor::application::iapp_mtr_unique_t ) noexcept ;
                win32_carrier( this_rref_t ) noexcept ;
                virtual ~win32_carrier( void_t ) noexcept ;

                win32_carrier( this_cref_t ) = delete ;


            private: // interface

                virtual motor::application::result on_exec( void_t ) noexcept ;
                virtual motor::application::result close( void_t ) noexcept ;

                virtual motor::application::iwindow_mtr_shared_t create_window( motor::application::window_info_cref_t info ) noexcept override ;
                virtual motor::application::iwindow_mtr_shared_t create_window( motor::application::graphics_window_info_in_t info ) noexcept override ; 

            private:

                void_t create_and_register_device_modules( void_t ) noexcept ;


            private: // window functions

                /// used to pass to windows for the callback. It is the wrapper that calls the real proc function.
                static LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) ;

                HWND create_win32_window( motor::application::window_info_cref_t ) noexcept ;



            private: // send messages -> outgoing messages
               
                // send listeners of the window that is about to die
                void_t send_destroy( win32_window_data_in_t ) noexcept ;
                void_t send_create( win32_window_data_in_t ) noexcept ;


            private: // handle messages -> incoming messages

                void_t handle_messages( win32_window_data_inout_t, motor::application::window_message_listener_t::state_vector_in_t ) noexcept ;


            private:

                void_t handle_destroyed_hwnd( HWND ) noexcept ;
            };
            motor_typedef( win32_carrier ) ;
        }
    }
}