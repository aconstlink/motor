#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include "../../device/linux/xlib_module.h"

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
                static Display * _display ;
                static size_t _display_use_count ;

                static Display * connect_display( void_t ) noexcept ;
                static void_t disconnect_display( void_t ) noexcept ;
                static Display * move_display( void_t ) noexcept ;

                //motor::device::xlib::xlib_module_res_t _device_module ;

            public:

                xlib_carrier( void_t ) noexcept ;
                xlib_carrier( this_rref_t ) noexcept ;
                xlib_carrier( motor::application::iapp_mtr_shared_t ) noexcept ;
                xlib_carrier( motor::application::iapp_mtr_unique_t ) noexcept ;
                virtual ~xlib_carrier( void_t ) noexcept ;

                /// singleton. It is required due to the fact
                /// that windows and application can not be 
                /// brought together.
                static Display * get_display( void_t ) noexcept ;

            public:

                virtual motor::application::result on_exec( void_t ) noexcept override ;
                virtual motor::application::result close( void_t ) noexcept override ;

                virtual motor::application::iwindow_mtr_shared_t create_window( motor::application::window_info_cref_t info ) noexcept override ;
                virtual motor::application::iwindow_mtr_shared_t create_window( motor::application::graphics_window_info_in_t info ) noexcept override ; 
            };
            motor_typedef( xlib_carrier ) ;
        }
    }
}
