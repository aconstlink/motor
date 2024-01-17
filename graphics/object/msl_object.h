
#pragma once

#include "../object.hpp"

namespace motor
{
    namespace graphics
    {
        enum class msl_api_type
        {
            none,
            msl_4_0         // gen 4 shader
        };

        class MOTOR_GRAPHICS_API msl_object : public object
        {
            motor_this_typedefs( msl_object ) ;

        private:

            // msl shader itself
            // render object (variables, render states)
            // shader object (generated shaders and i/o)
            // geometry object maybe

            motor::string_t _name ;

        public:

            msl_object( void_t ) noexcept {}
            virtual ~msl_object( void_t ) noexcept {}


        };
        motor_typedef( msl_object ) ;
    }
}
