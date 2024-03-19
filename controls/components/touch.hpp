
#pragma once

#include "../component.hpp"

#include <motor/math/vector/vector2.hpp>

namespace motor
{
    namespace controls
    {
        namespace components
        {
            enum class touch_state
            {
                none,
                pressed,
                pressing,
                released,
                num_keys
            };

            // values are in [-1.0;1.0]
            class touch : public input_component
            {
                motor_this_typedefs( touch ) ;

            private:

                touch_state _ss = touch_state::none ;
                // normalized centered coordinate
                motor::math::vec2f_t _ncc ;

            public:

                touch( void_t ) noexcept
                {
                }

                touch( this_cref_t rhv ) noexcept : input_component( rhv )
                {
                    _ss = rhv._ss ;
                    _ncc = rhv._ncc ;
                }

                touch( this_rref_t rhv ) noexcept : input_component( std::move( rhv ) )
                {
                    _ss = rhv._ss ;
                    _ncc = rhv._ncc ;
                }

                virtual ~touch( void_t ) noexcept {}

            public:

                this_ref_t operator = ( motor::math::vec2f_cref_t v ) noexcept
                {
                    _ncc = v ;
                    return *this ;
                }

                this_ref_t operator = ( motor::controls::components::touch_state const v ) noexcept
                {
                    _ss = v ;
                    return *this ;
                }

                this_ref_t operator = ( this_cref_t rhv ) noexcept
                {
                    _ncc = rhv._ncc ;
                    _ss = rhv._ss ;
                    return *this ;
                }

                motor::math::vec2f_cref_t value ( void_t ) const noexcept { return _ncc ; }
                motor::controls::components::touch_state state( void_t ) const noexcept { return _ss ; }

                virtual void_t update( void_t ) noexcept final
                {
                    if( _ss == motor::controls::components::touch_state::pressed )
                    {
                        _ss = motor::controls::components::touch_state::pressing ;
                    }
                    else if( _ss == motor::controls::components::touch_state::released )
                    {
                        _ss = motor::controls::components::touch_state::none ;
                    }
                }
            };
            motor_typedef( touch ) ;
        }
    }
}
