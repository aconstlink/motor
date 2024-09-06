#pragma once

#include "ivisitor.h"

#include <motor/math/utility/3d/transformation.hpp>
#include <motor/std/stack>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API trafo_visitor : public ivisitor
        {
            motor_this_typedefs( trafo_visitor ) ;
        private:

            motor::stack< motor::math::m3d::trafof_t, 10 > _trafos ;

        public:

            trafo_visitor( void_t ) noexcept ;
            virtual ~trafo_visitor( void_t ) noexcept ;

            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept ;

            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept ;

            virtual void_t on_start( void_t ) noexcept final ;
            virtual void_t on_finish( void_t ) noexcept final ;

        private:

            motor::math::m3d::trafof_t check_component_and_compute_global( motor::scene::node_ptr_t ) noexcept ;
        };
        motor_typedef( trafo_visitor ) ;
    }
}