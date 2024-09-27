#pragma once

#include "logic_decorator.h"
#include <motor/math/utility/3d/transformation.hpp>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API trafo3d_node : public motor::scene::logic_decorator
        {
            typedef motor::scene::logic_decorator base_t ;
            motor_this_typedefs( trafo3d_node ) ;

            motor_core_dd_id_fn() ;

        private:

            motor::math::m3d::trafof_t _trafo ;

            // the one the visitor computes.
            motor::math::m3d::trafof_t _computed ;

        public:

            trafo3d_node( void_t ) noexcept ;
            trafo3d_node( motor::math::m3d::trafof_cref_t ) noexcept ;
            trafo3d_node( this_rref_t ) noexcept ;
            trafo3d_node( this_cref_t ) = delete ;
            virtual ~trafo3d_node( void_t ) noexcept ;

        public:

            motor::math::m3d::trafof_t get_computed( void_t ) const noexcept { return _computed ; }
            motor::math::m3d::trafof_t get_trafo( void_t ) const noexcept { return _trafo ; }
            void_t set_trafo( motor::math::m3d::trafof_in_t t ) noexcept { _trafo = t ; }

        public:

            class visitor_interface
            {
                trafo3d_node * _owner ;

            public:

                visitor_interface( trafo3d_node * owner ) noexcept : _owner( owner ) {}
                void_t set_computed( motor::math::m3d::trafof_cref_t trafo ) noexcept { _owner->set_computed( trafo ) ; }
            };
            motor_typedef( visitor_interface ) ;

        private:

            friend class visitor_interface ;
            void_t set_computed( motor::math::m3d::trafof_cref_t trafo ) noexcept { _computed = trafo ; }

        public:

            //virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ptr ) noexcept ;
        };
        motor_typedef( trafo3d_node ) ;
    }
}
