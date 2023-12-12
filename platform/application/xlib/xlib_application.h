#pragma once

#include "../../api.h"
#include "../platform_application.h"

#include <natus/device/modules/linux/xlib_module.h>
#include <X11/Xlib.h>

namespace natus
{
    namespace application
    {
        namespace xlib
        {
            class xlib_application : public platform_application
            {
                typedef platform_application base_t ;
                natus_this_typedefs( xlib_application ) ;

            private:

                /// the main display connection. Required to be
                /// a singleton in order to be able for single
                /// windows to be connected to this display.
                /// I would like to manage all windows and dispatch
                /// all window messaged centralized in this 
                /// application.
                static Display * _display ;
                static size_t _display_use_count ;

                static Display * connect_display( void_t ) ;
                static void_t disconnect_display( void_t ) ;
                static Display * move_display( void_t ) ;

                natus::device::xlib::xlib_module_res_t _device_module ;

            public:

                xlib_application( void_t ) ;
                xlib_application( this_rref_t ) ;
                xlib_application( natus::application::app_res_t ) ;
                virtual ~xlib_application( void_t ) ;

                /// singleton. It is required due to the fact
                /// that windows and application can not be 
                /// brought together.
                static Display * get_display( void_t ) ;

            public:

                static this_ptr_t create( void_t ) ;
                static this_ptr_t create( this_rref_t ) ;
                static void_t destroy( this_ptr_t ) ;

            public:

                virtual natus::application::result on_exec( void_t ) ;
            };
            natus_res_typedef( xlib_application ) ;
            typedef natus::memory::res_t< xlib_application > application_res_t ;
        }
    }
}
