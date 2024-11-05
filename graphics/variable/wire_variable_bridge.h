#pragma once
#include "../api.h"
#include "../typedefs.h"

#include "variable_set.hpp"

#include <motor/wire/slot/sheet.hpp>

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API wire_variable_bridge
        {
            motor_this_typedefs( wire_variable_bridge ) ;

        private:
            
            motor::graphics::variable_set_mtr_t _vs = nullptr ;
            motor::wire::inputs_t _inputs ;

            // will only hold borrowed pointers.
            // ref counted pointers are stored above.
            // binding in this case means:
            // shader var to slot binding
            struct variable_binding
            {
                using pull_funk_t = std::function< void_t ( variable_binding & ) >  ;
                motor::graphics::ivariable_ptr_t gvar ;
                motor::wire::iinput_slot_ptr_t slot ;
                pull_funk_t pull_funk ;
            };

            motor::vector< variable_binding > _bindings ;

        public:

            wire_variable_bridge( void_t ) noexcept ;
            wire_variable_bridge( motor::graphics::variable_set_mtr_safe_t ) noexcept ;
            wire_variable_bridge( this_rref_t ) noexcept ;
            wire_variable_bridge( this_cref_t ) = delete ;
            ~wire_variable_bridge( void_t ) noexcept ;

        public: // update interface

            // pull data from inputs slots to graphics variables
            void_t pull_data( void_t ) noexcept ;

            // redo graphics variables to inputs bindings
            void_t update_bindings( void_t ) noexcept ;

            // redo graphics variables to inputs bindings
            // by introducing a new variable set
            void_t update_bindings( motor::graphics::variable_set_mtr_safe_t ) noexcept ;

        public: // get/set

            motor::wire::inputs_ptr_t borrow_inputs( void_t ) noexcept ;
            motor::wire::inputs_cptr_t borrow_inputs( void_t ) const noexcept ;

        private:

            void_t create_bindings( void_t ) noexcept ;
            
            template< typename T >
            bool_t make_binding( motor::string_in_t name, motor::graphics::ivariable_ptr_t var ) noexcept
            {
                using type_t = T ;

                auto [a, b] = motor::graphics::cast_data_variable< type_t >( var ) ;
                if ( a )
                {
                    using slot_t = motor::wire::input_slot< type_t > ;
                    using var_t = motor::graphics::data_variable< type_t > ;

                    auto s = _inputs.get_or_add( name, motor::shared( slot_t( b->get() ) ) ) ;
                    b->set( ((slot_t*)s.mtr())->get_value() ) ;

                    _bindings.emplace_back( variable_binding
                    {
                        var, motor::move( s ),
                        [=] ( this_t::variable_binding & v )
                        {
                            auto * s_local = reinterpret_cast<slot_t *>( v.slot ) ;
                            auto * v_local = reinterpret_cast<var_t *>( v.gvar ) ;

                            v_local->set( s_local->get_value() ) ;
                        }
                    } ) ;
                    return true ;
                }
                return false ;
            }
        };
        motor_typedef( wire_variable_bridge ) ;
    }
}