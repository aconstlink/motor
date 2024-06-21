
#pragma once

#include "icomponent.h"
#include <functional>

namespace motor 
{
    namespace scene
    {
        class MOTOR_SCENE_API code_component : public icomponent
        {
            motor_this_typedefs( code_component ) ;

        public:

            using funk_t = std::function< void_t ( motor::scene::node_mtr_t ) > ;

        private:

            funk_t _funk ;

        public:

            code_component( funk_t f ) noexcept : _funk( f ) {}
            virtual ~code_component( void_t ) noexcept {}
            virtual void_t execute( motor::scene::node_mtr_t ) noexcept ;
        };
        motor_typedef( code_component ) ;
    }
}