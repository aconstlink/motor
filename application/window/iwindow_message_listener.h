#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "window_message.h"

namespace motor
{
    namespace application
    {
        // system level listener facility
        // nothing the user ever sees.
        class MOTOR_APPLICATION_API iwindow_message_listener
        {
        public: // callbacks

            virtual void_t on_screen( motor::application::screen_dpi_message_cref_t ) noexcept = 0 ;
            virtual void_t on_screen( motor::application::screen_size_message_cref_t ) noexcept = 0 ;

            virtual void_t on_resize( motor::application::resize_message_cref_t ) noexcept = 0 ;
            virtual void_t on_visible( motor::application::show_message_cref_t ) noexcept = 0 ;
            virtual void_t on_close( motor::application::close_message_cref_t ) noexcept = 0 ;

            virtual void_t on_vsync( motor::application::vsync_message_cref_t ) noexcept = 0 ;
            virtual void_t on_fullscreen( motor::application::fullscreen_message_cref_t ) noexcept = 0 ;
        };
        motor_typedef( iwindow_message_listener ) ;
    }
}