
#pragma once

#include "../component.hpp"

namespace motor
{
    namespace controls
    {
        namespace components
        {
            // binary_led component for gradually intensifying on/off leds
            template< typename value_t >
            class intesity_led : public output_component
            {
                motor_this_typedefs( intesity_led ) ;

            private:

                bool_t _changed = false ;
                value_t _value = false ;

            public:

                intesity_led( void_t ) noexcept{}

                intesity_led( this_cref_t rhv ) noexcept : output_component( rhv )
                {
                    _changed = rhv._changed ;
                    _value = rhv._value ;
                }

                intesity_led( this_rref_t rhv ) noexcept : output_component( std::move( rhv ) )
                {
                    _changed = rhv._changed ;
                    _value = rhv._value ;
                }

                virtual ~intesity_led( void_t ) noexcept {}

            public:

                this_ref_t operator = ( value_t const v ) noexcept
                {
                    _changed = true ;
                    _value = v ;
                    return *this ;
                }

                bool_t has_changed( void_t ) const { return _changed ; }
                value_t value ( void_t ) const noexcept { return _value ; }

                virtual void_t update( void_t ) noexcept final
                {
                    _changed = false ;
                }
            };
            motor_typedefs( intesity_led< bool_t >, binary_led ) ;
            motor_typedefs( intesity_led< float_t >, linear_led ) ;

            class multi_led : public output_component
            {
                motor_this_typedefs( multi_led ) ;

            private:

                bool_t _changed = false ;
                bool_t _onoff = false ;
                byte_t _led_color = 0 ;

            public:

                multi_led( void_t ) noexcept{}

                multi_led( this_cref_t rhv ) noexcept : output_component( rhv )
                {
                    _changed = rhv._changed ;
                    _led_color = rhv._led_color ;
                }

                multi_led( this_rref_t rhv ) noexcept : output_component( std::move( rhv ) )
                {
                    _changed = rhv._changed ;
                    _led_color = rhv._led_color ;
                }

                virtual ~multi_led( void_t ) noexcept {}

            public:

                // led_color == 0 is off
                this_ref_t operator = ( size_t const led_color ) noexcept
                {
                    _changed = true ;
                    _led_color = byte_t( led_color ) ;
                    return *this ;
                }

                this_ref_t operator = ( bool_t const onoff ) noexcept
                {
                    _changed = true ;
                    _onoff = onoff ;
                    return *this ;
                }

                bool_t has_changed( void_t ) const { return _changed ; }
                bool_t onoff ( void_t ) const noexcept { return _onoff ; }
                byte_t led_color( void_t ) const noexcept { return _led_color ; }

                virtual void_t update( void_t ) noexcept final
                {
                    _changed = false ;
                }
            };
            motor_typedef( multi_led ) ;
        }
    }
}