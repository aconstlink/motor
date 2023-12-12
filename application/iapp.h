
#pragma once

#include "api.h"
#include "typedefs.h"

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API iapp
        {
        public:
            virtual ~iapp( void_t ) noexcept{}
        };
        motor_typedef( iapp ) ;
    }
}