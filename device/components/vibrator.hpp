
#pragma once

#include "../component.hpp"

#include <motor/std/string>

namespace motor
{
    namespace device
    {
        namespace components
        {
            enum class vibrator_state
            {
                none,
                on,
                running,
                off,
                num_states
            };

            static motor::string_t to_string( motor::device::components::vibrator_state const s ) noexcept
            {
                using bs_t = motor::device::components::vibrator_state ;
                static char_t const * const __states[] = { "none", "on", "running", "off", "invalid" } ;
                size_t const i = size_t( s ) >= size_t( bs_t::num_states ) ? size_t( bs_t::num_states ) : size_t( s ) ;
                return motor::string_t( __states[ i ] ) ;
            }

            class vibrator : public output_component
            {
                motor_this_typedefs( vibrator ) ;

            private:

                motor::device::components::vibrator_state _s ;
                float_t _value = 0.0f ;

            public:

                vibrator( void_t )
                {
                }

                vibrator( this_cref_t rhv ) noexcept : output_component( rhv )
                {
                    _s = rhv._s ;
                    _value = rhv._value ;
                }

                vibrator( this_rref_t rhv ) noexcept : output_component( std::move( rhv ) )
                {
                    _s = rhv._s ; rhv._s = motor::device::components::vibrator_state::none ;
                    _value = rhv._value ;
                }

                virtual ~vibrator( void_t ) noexcept {}

            public:

                this_ref_t operator = ( float_t const v ) noexcept
                {
                    _value = v ;
                    return *this ;
                }

                this_ref_t operator = ( motor::device::components::vibrator_state const s ) noexcept
                {
                    _s = s ;
                    return *this ;
                }

                float_t value ( void_t ) const noexcept { return _value ; }
                vibrator_state state( void_t ) const noexcept { return _s ; }

                virtual void_t update( void_t ) noexcept final
                {
                    if( _s == motor::device::components::vibrator_state::on )
                    {
                        _s = motor::device::components::vibrator_state::running ;
                    }
                    else if( _s == motor::device::components::vibrator_state::off )
                    {
                        _s = motor::device::components::vibrator_state::none ;
                    }
                }
            };
            motor_typedef( vibrator ) ;
        }
    }
}
