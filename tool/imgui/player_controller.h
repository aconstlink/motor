
#pragma once

#include "imgui.h"

#include <motor/graphics/object/image_object.h>
#include <motor/io/database.h>
#include <motor/std/string>
#include <motor/std/vector>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API player_controller
        {
            motor_this_typedefs( player_controller ) ;

        public:

            enum class player_state
            {
                invalid, 
                no_change,
                play,
                stop,
                pause
            };

        private:

            bool_t _internal_play = false ;

            bool_t _play = false ;
            bool_t _pause = false ;

        public:

            player_state do_tool( char const *, bool_t const play_toggle = false ) noexcept ;

            bool_t is_play_clicked( void_t ) const noexcept ;
            bool_t is_pause_clicked( void_t ) const noexcept ;

            player_state get_state( void_t ) const noexcept 
            {
                if( _play ) return player_state::play ;
                else if( _pause ) return player_state::pause ;
                return player_state::stop ;
            }

            void_t set_play( void_t ) noexcept
            {
                _internal_play = true ;
            }

            void_t set_pause( void_t ) noexcept
            {
                _internal_play = false ;
                _play = false ;
                _pause = true ;
            }

            void_t set_stop( void_t ) noexcept
            {
                _internal_play = false ;
                _play = false ;
                _pause = false ;
            }
        };
        motor_typedef( player_controller ) ;
    }
}