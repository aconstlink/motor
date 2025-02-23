#pragma once

#include "../../api.h"
#include "../../controls/win32/rawinput_module.h"
#include "../../controls/win32/xinput_module.h"
#include "../../controls/win32/midi_module.h"

#include <motor/application/window/window.h>
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

                using _clock_t = std::chrono::high_resolution_clock ;

            private:

                bool_t _done = false ;
                                
                motor::platform::win32::rawinput_module_mtr_t _rawinput = nullptr ;
                motor::platform::win32::xinput_module_mtr_t _xinput  = nullptr ;
                motor::platform::win32::midi_module_mtr_t _midi = nullptr ;

            private: // win32 window data

                struct win32_window_data
                {
                    HWND hwnd ;
                    motor::application::window_mtr_t wnd ;
                    motor::application::window_message_listener_mtr_t lsn ;
                    // can be used to store messages to be used continuously.
                    // Use for vsync, cursor, resize messages.
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
                    HWND hwnd ;
                    wgl_pimpl * ptr ;
                    size_t micro_rnd ;
                };
                motor_typedef( wgl_window_data ) ;

                motor::vector< wgl_window_data_t > _wgl_windows ;

            private: // d3d11 window data

                struct d3d11_pimpl ;

                struct d3d11_window_data
                {
                    HWND hwnd ;
                    d3d11_pimpl * ptr ;
                    _clock_t::time_point rnd_beg ;
                    size_t micro_rnd ;
                    uint_t frame_miss ;
                };
                motor_typedef( d3d11_window_data ) ;

                motor::vector< d3d11_window_data_t > _d3d11_windows ;

            private: // win32 windows queue

                struct window_queue_msg
                {
                    motor::application::window_info_t wi ;
                    motor::application::window_mtr_t wnd ;
                    motor::application::window_message_listener_mtr_t lsn ;
                };
                motor_typedef( window_queue_msg ) ;

                std::mutex _mtx_queue ;
                motor::vector< window_queue_msg_t > _queue ;

            private: // destruction queue 

                motor::vector< HWND > _destroy_queue ;

            public:

                win32_carrier( void_t ) noexcept ;
                win32_carrier( motor::application::app_mtr_safe_t ) noexcept ;
                win32_carrier( this_rref_t ) noexcept ;
                virtual ~win32_carrier( void_t ) noexcept ;

                win32_carrier( this_cref_t ) = delete ;

            private: // interface

                virtual motor::application::result on_exec( void_t ) noexcept ;
                virtual motor::application::result close( void_t ) noexcept ;

                virtual motor::application::iwindow_mtr_safe_t create_window( motor::application::window_info_cref_t info ) noexcept override ;

            private:

                void_t create_and_register_device_modules( void_t ) noexcept ;
                void_t create_and_register_audio_backend( void_t ) noexcept ;
                void_t create_and_register_network_modules( void_t ) noexcept ;


            private: // window functions

                /// used to pass to windows for the callback. It is the wrapper that calls the real proc function.
                static LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) ;

                HWND create_win32_window( motor::application::window_info_cref_t ) noexcept ;



            private: // send messages -> outgoing messages
               
                // send listeners of the window that is about to die
                void_t send_destroy( win32_window_data_in_t ) noexcept ;
                void_t send_create( win32_window_data_in_t ) noexcept ;
                void_t send_resize( win32_window_data_inout_t ) noexcept ;
                void_t send_mouse( win32_window_data_inout_t, motor::application::mouse_message_t::state_type const ) noexcept ;

            private: // handle messages -> incoming messages

                void_t handle_messages( win32_window_data_inout_t, motor::application::window_message_listener_t::state_vector_in_t ) noexcept ;


            private:

                void_t handle_destroyed( void_t ) noexcept ;
                bool_t handle_destroyed_hwnd( HWND ) noexcept ;

            private:

                using find_window_info_funk_t = std::function< void_t ( win32_carrier::this_t::win32_window_data_ref_t ) > ;
                bool_t find_window_info( HWND hwnd, find_window_info_funk_t ) noexcept ;


            };
            motor_typedef( win32_carrier ) ;
        }
    }
}