#pragma once

#include "../platform_window.h"
#include "../window_info.h"
#include "../toggle_window.h"

#include <natus/ntd/vector.hpp>
#include <natus/math/vector/vector4.hpp>

#include <X11/Xlib.h>

namespace natus
{
    namespace application
    {
        namespace xlib
        {
            class window : public platform_window
            {
                natus_this_typedefs( window ) ;
                friend class xlib_application ;

            private:

                Display * _display ;
                Window _handle ;

                bool_t _is_fullscreen = false ;
                bool_t _is_cursor = false ;

                natus::math::vec4i_t _dims = natus::math::vec4i_t( 0, 0, 100, 100 ) ;

            public:

                window( void_t ) ;
                window( window_info const & ) ;
                window( Display * display, Window wnd ) ;

                window( this_rref_t rhv ) ;
                virtual ~window( void_t ) ;

            public: // interface

                Window get_handle( void_t ) ;
                Display * get_display( void_t ) ;
                void_t send_toggle( natus::application::toggle_window_in_t ) ;
                void_t show_window(  window_info const & wi ) ;
                virtual void_t check_for_messages( void_t ) noexcept ;

            private:

                void_t create_window( window_info const & ) ;
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
            natus_res_typedef( window ) ;
        }
    }
}
