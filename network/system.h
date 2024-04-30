

#pragma once

#include "api.h"
#include "typedefs.h"

#include "imodule.h"

namespace motor{ namespace network {

    class MOTOR_NETWORK_API system
    {
        motor_this_typedefs( system ) ;

    private:

    public:

        system( void_t ) noexcept ;
        
    };
    motor_typedef( system ) ;
}
}