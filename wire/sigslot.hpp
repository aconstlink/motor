
#pragma once

#include "variable.hpp"

namespace motor
{
    namespace wire
    {
        template< typename T >
        class signal ;

        template< typename T >
        class slot ;

        template< typename T >
        class sigslot ;


        template< typename T >
        class signal
        {
            motor_this_typedefs( signal< T > ) ;

        private:

            motor::wire::variable< T > _var ;

        public:

            bool_t connect( )
        };
    }
}