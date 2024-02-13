#pragma once

#include "typedefs.h"

namespace motor
{
    namespace property
    {
        class iproperty
        {
        public: 
            virtual ~iproperty( void_t ) {}
        };
        motor_typedef( iproperty ) ;
    }
}