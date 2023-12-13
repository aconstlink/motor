
#pragma once

#include "iwindow_message_listener.h"

#include <mutex>

namespace motor
{
    namespace application
    {
        // default window message listener. Stores message so its called receiver.
        class MOTOR_APPLICATION_API window_message_listener : public motor::application::iwindow_message_listener
        {
            motor_this_typedefs( window_message_listener ) ;

        public: 

            struct state_vector
            {
                motor::application::resize_message resize_msg ;
                bool_t resize_changed = false ;

                motor::application::show_message show_msg ;
                bool_t show_changed = false ;

                motor::application::create_message create_msg ;
                bool_t create_changed = false ;

                motor::application::close_message close_msg ;
                bool_t close_changed = false ;

                motor::application::screen_dpi_message dpi_msg ;
                bool_t dpi_msg_changed = false ;

                motor::application::screen_size_message msize_msg ;
                bool_t msize_msg_changed = false ;

                motor::application::vsync_message vsync_msg ;
                bool_t vsync_msg_changed = false ;

                motor::application::fullscreen_message fulls_msg ;
                bool_t fulls_msg_changed = false ;

                motor::application::cursor_message cursor_msg ;
                bool_t cursor_msg_changed = false ;
            };
            motor_typedef( state_vector ) ;
            typedef bool_t change_state_t ;

        private:

            /// lock the states vector
            std::mutex _mtx ;
            state_vector_t _states ;
            change_state_t _has_any_change = false ;

        public:

            window_message_listener( void_t ) noexcept ;
            window_message_listener( this_rref_t rhv ) noexcept ;
            window_message_listener( this_cref_t rhv ) noexcept ;
            virtual ~window_message_listener( void_t ) noexcept ;

            bool_t swap_and_reset( state_vector_out_t states ) noexcept ;

            bool_t has_any_change( void_t ) const noexcept ;

        private:

            void_t reset_change_flags( void_t ) noexcept ;

        public: // listener interface

            virtual void_t on_message( screen_dpi_message_cref_t msg ) noexcept ;
            virtual void_t on_message( screen_size_message_cref_t msg ) noexcept ;
            virtual void_t on_message( resize_message_cref_t msg ) noexcept ;
            virtual void_t on_message( show_message_cref_t msg ) noexcept ;
            virtual void_t on_message( motor::application::create_message_cref_t ) noexcept ;
            virtual void_t on_message( close_message_cref_t msg ) noexcept ;
            virtual void_t on_message( vsync_message_cref_t msg ) noexcept ;
            virtual void_t on_message( fullscreen_message_cref_t msg ) noexcept ;
            virtual void_t on_message( motor::application::cursor_message_cref_t ) noexcept ;
        };
        motor_typedef( window_message_listener ) ;
    }
}