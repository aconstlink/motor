
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

            player_state do_tool( motor::string_cref_t label, motor::tool::imgui_view_t ) noexcept ;

            bool_t is_play_clicked( void_t ) const noexcept ;
            bool_t is_pause_clicked( void_t ) const noexcept ;
            
        };
        motor_typedef( player_controller ) ;
    }
}