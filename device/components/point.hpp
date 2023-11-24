
#pragma once

#include "../component.hpp"

#include <motor/math/vector/vector2.hpp>

namespace motor
{
    namespace device
    {
        namespace components
        {
            enum class move_state
            {
                none,
                moved,
                moving,
                unmoved,
                num_states
            };

            class point : public input_component
            {
                motor_this_typedefs( point ) ;

            private:
                
                motor::math::vec2f_t _old ;
                motor::math::vec2f_t _value ;
                move_state _state ;

            public:

                point( void_t )
                {
                }

                point( this_cref_t rhv ) noexcept : input_component( rhv )
                {
                    _old = rhv._old ;
                    _value = rhv._value ;
                    _state = rhv._state ;
                }

                point( this_rref_t rhv ) noexcept : input_component( std::move( rhv ) )
                {
                    _old = rhv._old ;
                    _value = rhv._value ;
                    _state = rhv._state ;
                }

                virtual ~point( void_t ) noexcept {}

            public:

                this_ref_t operator = ( motor::math::vec2f_cref_t v ) noexcept
                {
                    _value = v ;
                    _state = _state == move_state::moving ? _state : move_state::moved ;
                    return *this ;
                }

                this_ref_t operator = ( motor::device::components::move_state const ms ) noexcept
                {
                    if( ms == move_state::unmoved && _state != move_state::none )
                        _state = ms ;
                    return *this ;
                }

                this_ref_t operator = ( this_cref_t rhv ) noexcept 
                {
                    _value = rhv._value ;
                    _state = rhv._state ;

                    return *this ;
                }

                motor::math::vec2f_t rel( void_t ) const noexcept { return _value - _old ; }
                motor::math::vec2f_cref_t value ( void_t ) const noexcept { return _value ; }
                bool_t has_changed( void_t ) const noexcept { return _state != move_state::none ; }
                move_state state( void_t ) const noexcept { return _state ; }

                virtual void_t update( void_t ) noexcept final
                {
                    _old = _value ;

                    if( _state == motor::device::components::move_state::moved )
                    {
                        _state = motor::device::components::move_state::moving ;
                    }
                    else if( _state == motor::device::components::move_state::unmoved )
                    {
                        _state = motor::device::components::move_state::none ;
                    }
                }
            };
            motor_typedef( point ) ;
        }
    }
}