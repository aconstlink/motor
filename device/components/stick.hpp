
#pragma once

#include "../component.hpp"

#include <motor/math/vector/vector2.hpp>

namespace motor
{
    namespace device
    {
        namespace components
        {
            enum class stick_state
            {
                none,
                tilted,
                tilting,
                untilted,
                num_keys
            };

            // values are in [-1.0;1.0]
            class stick : public input_component
            {
                motor_this_typedefs( stick ) ;

            private:

                stick_state _ss = stick_state::none ;
                motor::math::vec2f_t _value ;

            public:

                stick( void_t )
                {
                }

                stick( this_cref_t rhv ) noexcept : input_component( rhv )
                {
                    _ss = rhv._ss ;
                    _value = rhv._value ;
                }

                stick( this_rref_t rhv ) noexcept : input_component( std::move( rhv ) )
                {
                    _ss = rhv._ss ;
                    _value = rhv._value ;
                }

                virtual ~stick( void_t ) noexcept {}

            public:

                this_ref_t operator = ( motor::math::vec2f_cref_t v ) noexcept
                {
                    _value = v ;
                    return *this ;
                }

                this_ref_t operator = ( motor::device::components::stick_state const v ) noexcept
                {
                    _ss = v ;
                    return *this ;
                }

                this_ref_t operator = ( this_cref_t rhv ) noexcept
                {
                    _value = rhv._value ;
                    _ss = rhv._ss ;
                    return *this ;
                }

                motor::math::vec2f_cref_t value ( void_t ) const noexcept { return _value ; }
                motor::device::components::stick_state state( void_t ) const noexcept { return _ss ; }

                virtual void_t update( void_t ) noexcept final
                {
                    if( _ss == motor::device::components::stick_state::tilted )
                    {
                        _ss = motor::device::components::stick_state::tilting ;
                    }
                    else if( _ss == motor::device::components::stick_state::untilted )
                    {
                        _ss = motor::device::components::stick_state::none ;
                    }
                }
            };
            motor_typedef( stick ) ;
        }
    }
}
