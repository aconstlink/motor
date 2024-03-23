
#pragma once

#include "../component.hpp"

namespace motor
{
    namespace controls
    {
        namespace components
        {
            // scroll component for vertical/horizontal scroll
            // relative value
            // value == 0.0f : no scroll
            // value > 0.0f : up/right scroll
            // value < 0.0f : down/left scroll
            class scroll : public input_component
            {
                motor_this_typedefs( scroll ) ;

            private:

                bool_t _changed = false ;
                float_t _value = 0.0f ;

            public:

                scroll( void_t )
                {
                }

                scroll( this_cref_t rhv ) noexcept : input_component( rhv )
                {
                    _changed = rhv._changed ;
                    _value = rhv._value ;
                }

                scroll( this_rref_t rhv ) noexcept : input_component( std::move( rhv ) )
                {
                    _changed = rhv._changed ;
                    _value = rhv._value ;
                }

                virtual ~scroll( void_t ) noexcept {}

            public:

                this_ref_t operator = ( float_t const v ) noexcept
                {
                    _changed = true ;
                    _value = v ;
                    return *this ;
                }

                bool_t has_changed( void_t ) const { return _changed ; }
                float_t value ( void_t ) const noexcept { return _value ; }

                virtual void_t update( void_t ) noexcept final
                {
                    _changed = false ;
                    _value = 0.0f ;
                }
            };
            motor_typedef( scroll ) ;
        }
    }
}