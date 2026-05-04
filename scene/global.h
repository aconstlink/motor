
#pragma once

#include "api.h"
#include "typedefs.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API global
        {
            motor_this_typedefs( global ) ;

        private:
        public:

            static void_t init( void_t ) noexcept ;
            static void_t deinit( void_t ) noexcept ;

        private:


        };

    }
}