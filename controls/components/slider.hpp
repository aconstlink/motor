
#pragma once

#include "../component.hpp"

namespace motor
{
    namespace controls
    {
        namespace components
        {
            // slider component for vertical/horizontal slider
            class slider : public input_component
            {
                motor_this_typedefs( slider ) ;

            private:

                bool_t _changed = false ;
                float_t _value = 0.0f ;

            public:

                slider( void_t ) noexcept{}

                slider( this_cref_t rhv ) noexcept : input_component( rhv )
                {
                    _changed = rhv._changed ;
                    _value = rhv._value ;
                }

                slider( this_rref_t rhv ) noexcept : input_component( std::move( rhv ) )
                {
                    _changed = rhv._changed ;
                    _value = rhv._value ;
                }

                virtual ~slider( void_t ) noexcept {}

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
            motor_typedef( slider ) ;
        }
    }
}