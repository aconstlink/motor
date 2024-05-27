

#pragma once

#include "api.h"
#include "typedefs.h"

#include "imodule.h"

namespace motor{ namespace network {

    class MOTOR_NETWORK_API system
    {
        motor_this_typedefs( system ) ;

    private:

        motor::vector< motor::network::imodule_mtr_t > _mods ;

    public:

        system( void_t ) noexcept ;
        system( this_rref_t ) noexcept ;
        system( this_cref_t ) = delete ;
        ~system( void_t ) noexcept ;

    public:

        void_t add_module( motor::network::imodule_mtr_rref_t ) noexcept ;

        using for_each_module_funk_t = std::function< bool_t ( motor::network::imodule_mtr_t ) > ;
        void_t modules( for_each_module_funk_t ) noexcept ;

        
    };
    motor_typedef( system ) ;
}
}