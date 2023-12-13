#pragma once


//#include "../toggle_window.h"

#include <motor/application/window/window_info.h>
#include <motor/std/vector>
#include <motor/math/vector/vector4.hpp>

#include <X11/Xlib.h>

namespace motor
{
    namespace platform
    {
        namespace xlib
        {
            class window //: public platform_window
            {
                motor_this_typedefs( window ) ;
                friend class xlib_application ;

            private:

                Display * _display ;
                Window _handle ;

                bool_t _is_fullscreen = false ;
                bool_t _is_cursor = false ;

                motor::math::vec4i_t _dims = motor::math::vec4i_t( 0, 0, 100, 100 ) ;

            public:

                window( void_t ) ;
                window( motor::application::window_info const & ) ;
                window( Display * display, Window wnd ) ;

                window( this_rref_t rhv ) ;
                virtual ~window( void_t ) ;

            public: // interface

                Window get_handle( void_t ) ;
                Display * get_display( void_t ) ;
                //void_t send_toggle( motor::application::toggle_window_in_t ) ;
                void_t show_window(  motor::application::window_info const & wi ) ;
                virtual void_t check_for_messages( void_t ) noexcept ;

            private:

                void_t create_window( motor::application::window_info const & ) ;
                void_t create_window( Display * display, Window wnd ) ;

                void_t destroy_window( void_t ) ;

            public:

                /// allows to set a xevent straight to the window.
                /// this is primarilly used by the xlib application.
                void_t xevent_callback( XEvent const & event ) ;

            private:

                /// we store the this pointer within xlib so it
                /// can be retrieved later on in the application's
                /// main loop
                void_t store_this_ptr_in_atom( Display * display, Window wnd ) ;
            };
            motor_typedef( window ) ;
        }
    }
}
