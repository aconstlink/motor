#pragma once

#include "ivisitor.h"
#include <motor/std/string>

namespace motor
{
    namespace scene
    {
        /// implements the ivisitor interface with default
        /// behavior. The default behavior is to do nothing.
        class MOTOR_SCENE_API log_visitor : public ivisitor
        {
            motor_this_typedefs( log_visitor ) ;

        private:

            size_t _indent = 0 ;

            motor::string_t indent( void_t ) const noexcept ;
            void_t print( motor::string_in_t s ) const noexcept ;

        public:

            virtual ~log_visitor( void_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::group_ptr_t )  noexcept ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept;
            virtual motor::scene::result visit( motor::scene::decorator_ptr_t ) noexcept;
            virtual motor::scene::result post_visit( motor::scene::decorator_ptr_t, motor::scene::result const ) noexcept;
            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept;
        };
        motor_typedef( log_visitor ) ;
    }
}