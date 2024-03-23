
#pragma once

#include "../component.hpp"

#include <motor/std/string>

namespace motor
{
    namespace controls
    {
        namespace components
        {
            enum class button_state
            {
                none,
                pressed,
                pressing,
                released,
                num_keys
            };

            static motor::string_t to_string( motor::controls::components::button_state const s ) noexcept
            {
                using bs_t = motor::controls::components::button_state ;
                static char_t const * const __button_states[] = { "none", "pressed", "pressing", "released", "invalid" } ;
                size_t const i = size_t( s ) >= size_t( bs_t::num_keys ) ? size_t( bs_t::num_keys ) : size_t( s ) ;
                return motor::string_t( __button_states[ i ] ) ;
            }

            class button : public input_component
            {
                motor_this_typedefs( button ) ;

            private:

                motor::controls::components::button_state _bs = button_state::none ;
                float_t _value = 0.0f ;

            public:

                button( void_t ) 
                {
                }

                button( this_cref_t rhv ) noexcept : input_component( rhv )
                {
                    _bs = rhv._bs ;
                    _value = rhv._value ;
                }

                button( this_rref_t rhv ) noexcept : input_component( std::move(rhv) )
                {
                    _bs = rhv._bs ; rhv._bs = motor::controls::components::button_state::none ;
                    _value = rhv._value ; 
                }

                virtual ~button( void_t ) noexcept {}

            public:

                this_ref_t operator = ( float_t const v ) noexcept
                {
                    _value = v ;
                    return *this ;
                }

                this_ref_t operator = ( button_state const bs ) noexcept
                {
                    _bs = bs ;
                    return *this ;
                }

                this_ref_t operator = ( this_cref_t rhv ) 
                {
                    _bs = rhv._bs ;
                    _value = rhv._value ;
                    return *this ;
                }

                float_t value ( void_t ) const noexcept { return _value ; }
                button_state state( void_t ) const noexcept { return _bs ; }


                virtual void_t update( void_t ) noexcept final
                {
                    if( _bs == motor::controls::components::button_state::pressed )
                    {
                        _bs = motor::controls::components::button_state::pressing ;
                    }
                    else if( _bs == motor::controls::components::button_state::released )
                    {
                        _bs = motor::controls::components::button_state::none ;
                    }
                }
            };
            motor_typedef( button ) ;
        }
    }
}
