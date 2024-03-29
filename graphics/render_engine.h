
#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/std/vector>
#include <motor/log/global.h>

#include <mutex>
#include <functional>

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API render_engine
        {
            motor_this_typedefs( render_engine ) ;

        public:

            using command_t = std::function< void_t ( void_t ) > ;

        private:

            mutable std::mutex _mtx_has_commands ;
            bool_t _has_commands = false ;

            // upstream commands
            motor::vector< command_t > _coms_up ;

            // execute commands
            motor::vector< command_t > _coms_ex ;

        public:

            render_engine( void_t ) noexcept ;
            render_engine( this_cref_t ) = delete ;
            render_engine( this_rref_t rhv ) noexcept ;
            ~render_engine( void_t ) noexcept ;

        public:

            // clear out command queue
            void_t force_clear( void_t ) noexcept ;

            bool_t can_execute( void_t ) noexcept ;

            // returns true if a frame has been rendered
            // otherwise false
            bool_t execute_frame( void_t ) noexcept ;

        public: // interface for external entity

            bool_t can_enter_frame( void_t ) const noexcept ;

            // only enters if no commands are in the back buffer
            bool_t enter_frame( void_t ) noexcept;

            void_t leave_frame( void_t ) noexcept;

            // !not thread safe! Only use within enter_frame/leave_frame
            void_t send_upstream( this_t::command_t c ) noexcept ;

            // !not thread safe! Only use within enter_frame/leave_frame
            void_t send_execute( this_t::command_t c ) noexcept ;

        private: // has commands specific
            
            void_t swap_has_commands( void_t ) noexcept ;

            bool_t has_frame_commands( void_t ) const noexcept ;

        };
        motor_typedef( render_engine ) ;
    }
}