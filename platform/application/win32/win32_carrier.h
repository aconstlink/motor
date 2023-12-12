#pragma once

#include "win32_window.h"

#include "../../api.h"
#include "../../device/win32/rawinput_module.h"
#include "../../device/win32/xinput_module.h"

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

                /// can be used for special escape key pattern
                size_t _escape_sequence = 0 ;
                
                motor::platform::win32::rawinput_module_mtr_t _rawinput = nullptr ;
                motor::platform::win32::xinput_module_mtr_t _xinput  = nullptr ;

                struct win32_window_data
                {
                    HWND hwnd ;
                    motor::application::iwindow_mtr_t wnd ;
                    //motor::platform::win32::window_mtr_t _win32 ;
                };
                motor_typedef( win32_window_data ) ;

                
                motor::vector< win32_window_data_t > _win32_windows ;

                struct window_queue_msg
                {
                    motor::application::window_info_t wi ;
                    motor::application::iwindow_mtr_t wnd ;
                };
                motor_typedef( window_queue_msg ) ;

                std::mutex _mtx_queue ;
                motor::vector< window_queue_msg_t > _queue ;

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
                motor::application::iwindow_mtr_shared_t create_window( motor::application::window_info_cref_t info ) noexcept ;

            private:

                void_t create_and_register_device_modules( void_t ) noexcept ;


            private: // window functions

                /// used to pass to windows for the callback. It is the wrapper that calls the real proc function.
                static LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) ;

                HWND create_win32_window( motor::application::window_info_cref_t ) noexcept ;



            private: 
               
                void_t send_destroy( win32_window_data_in_t ) noexcept ;
            };
            motor_typedef( win32_carrier ) ;
        }
    }
}