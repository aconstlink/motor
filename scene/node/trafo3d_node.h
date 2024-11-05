#pragma once

#include "logic_decorator.h"

#include <motor/wire/slot/input_slot.h>
#include <motor/wire/slot/output_slot.h>
#include <motor/math/utility/3d/transformation.hpp>

namespace motor
{
    namespace scene
    {
        // @todo add relative and absolute transformation
        // relative: the transformation is computed throughout the tree traversal
        // absolute: should push transformation on the stack
        class MOTOR_SCENE_API trafo3d_node : public motor::scene::logic_decorator
        {
            typedef motor::scene::logic_decorator base_t ;
            motor_this_typedefs( trafo3d_node ) ;

            motor_core_dd_id_fn() ;

        private:

            motor_typedefs( motor::wire::input_slot< motor::math::m3d::trafof_t >, in_slot ) ;
            motor_typedefs( motor::wire::output_slot< motor::math::m3d::trafof_t >, out_slot ) ;

            in_slot_mtr_t _trafo = motor::shared( this_t::in_slot_t() ) ; // relative or absolute @todo insert enum for that
            out_slot_mtr_t _computed = motor::shared( this_t::out_slot_t() ) ; // the one the visitor computes.

        public:

            trafo3d_node( void_t ) noexcept ;
            trafo3d_node( motor::math::m3d::trafof_cref_t ) noexcept ;
            trafo3d_node( this_rref_t ) noexcept ;
            trafo3d_node( this_cref_t ) = delete ;
            virtual ~trafo3d_node( void_t ) noexcept ;

        public:

            motor::math::m3d::trafof_t get_computed( void_t ) const noexcept { return _computed->get_value() ; }
            motor::math::m3d::trafof_t get_trafo( void_t ) const noexcept { return _trafo->get_value() ; }
            void_t set_trafo( motor::math::m3d::trafof_in_t t ) noexcept { *_trafo = t ; }
            
            in_slot_mtr_safe_t get_input_slot( void_t ) noexcept { return motor::share( _trafo ) ; }
            out_slot_mtr_safe_t get_output_slot( void_t ) noexcept { return motor::share( _computed ) ; }

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
            void_t set_computed( motor::math::m3d::trafof_cref_t trafo ) noexcept { *_computed = trafo ; }

        public:

            virtual bool_t inputs( motor::wire::inputs_out_t ) noexcept ;
            //virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ptr ) noexcept ;
        };
        motor_typedef( trafo3d_node ) ;
    }
}
