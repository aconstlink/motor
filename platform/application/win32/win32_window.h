#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/application/window/window_message_listener.h>
#include <motor/application/window/toggle_window.h>
#include <motor/application/window/window_info.h>
#include <motor/application/window/window.h>

#include <vector>
#include <windows.h>

namespace motor
{
    namespace platform
    {
        namespace win32
        {
            class window
            {
                motor_this_typedefs( window ) ;

            private:

                HWND _handle = NULL ;

                bool_t _is_fullscreen = false ;
                bool_t _is_cursor = false ;

                HCURSOR _cursor = NULL ;

                motor::string_t _name = "WIN32 Window";

                motor::application::window_message_listener_mtr_t _lsn_in = nullptr ;

            public:

                window( void_t ) noexcept ;
                window( motor::application::window_info_cref_t ) noexcept ;
                window( this_cref_t rhv ) = delete ;
                window( this_rref_t rhv ) noexcept ;
                virtual ~window( void_t ) noexcept ;

                this_ref_t operator = ( this_rref_t ) noexcept ;

            public: // interface

                HWND get_handle( void_t ) noexcept ;

                void_t send_toggle( motor::application::toggle_window_in_t ) noexcept ;
                void_t send_resize( motor::application::resize_message_in_t ) noexcept  ;

                void_t show_window(  motor::application::window_info const & wi ) ;

                virtual void_t check_for_messages( void_t ) noexcept ;

            private:

                HWND create_window( motor::application::window_info const& ) ;
                void_t destroy_window( void_t ) ;


            protected: // virtual

                /// Here the real code is executed in order to do the callback actions.
                virtual LRESULT CALLBACK WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) ;

            public:

                /// used to pass to windows for the callback. It is the wrapper that calls the real proc function.
                static LRESULT CALLBACK StaticWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) ;

            private:

                void_t send_show( WPARAM ) ;
                void_t send_resize( HWND ) ;
                void_t send_destroy( void_t ) ;
            };
            motor_typedef( window ) ;            
        }
    }
}
