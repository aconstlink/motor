
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

            // the one the visitor computes.
            motor::math::m3d::trafof_t _computed ;

        public:

            trafo_3d_component( motor::math::m3d::trafof_in_t t ) noexcept : _trafo( t ){}
            virtual ~trafo_3d_component( void_t ) noexcept {}

            motor::math::m3d::trafof_t get_computed( void_t ) const noexcept{ return _computed ; }
            motor::math::m3d::trafof_t get_trafo( void_t ) const noexcept{ return _trafo ; }
            void_t set_trafo( motor::math::m3d::trafof_in_t t ) noexcept { _trafo = t ; }

        public:

            class visitor_interface
            {
                trafo_3d_component * _comp ;

            public:

                visitor_interface( trafo_3d_component * comp ) noexcept : _comp( comp ) {}
                void_t set_computed( motor::math::m3d::trafof_cref_t trafo ) noexcept { _comp->set_computed( trafo ) ; }
            };
            motor_typedef( visitor_interface ) ;

        private: 

            friend class visitor_interface ;
            void_t set_computed( motor::math::m3d::trafof_cref_t trafo ) noexcept { _computed = trafo ; }
        };
        motor_typedef( trafo_3d_component ) ;
    }
}