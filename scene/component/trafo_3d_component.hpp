
#pragma once

#include "icomponent.h"

#include <motor/math/utility/3d/transformation.hpp>

namespace motor
{
    namespace scene
    {
        class trafo_3d_component : public icomponent
        {
            motor_this_typedefs( trafo_3d_component ) ;

        private:

            motor::math::m3d::trafof_t _trafo ;

        public:

            trafo_3d_component( motor::math::m3d::trafof_in_t t ) noexcept : _trafo( t )
            {}

            ~trafo_3d_component( void_t ) noexcept {}

            motor::math::m3d::trafof_t get_trafo( void_t ) const noexcept{}
            void_t set_trafo( motor::math::m3d::trafof_in_t t ) noexcept { _trafo = t ; }
        };
        motor_typedef( trafo_3d_component ) ;
    }
}