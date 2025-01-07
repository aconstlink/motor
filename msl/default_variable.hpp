
#pragma once

#include "typedefs.h"
#include "enums.hpp"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

#include <motor/std/string>

namespace motor
{
    namespace msl
    {
        //****************************************************
        class idefault_value
        {
        public: virtual ~idefault_value( void_t ) noexcept {}
        };
        motor_typedef( idefault_value ) ;

        //****************************************************
        template< typename T >
        class generic_default_value : public idefault_value
        {
            motor_this_typedefs( generic_default_value< T > ) ;

        private:

            T _value ;

        public:

            generic_default_value( T const & v ) noexcept : _value( v )
            {}

            generic_default_value( this_rref_t rhv ) noexcept :
                _value( std::move( rhv._value ) )
            {}

            virtual ~generic_default_value( void_t ) noexcept {}

            T const & get( void_t ) const noexcept { return _value ; }
        };
        motor_typedefs( generic_default_value< float_t >, float_dv ) ;
        motor_typedefs( generic_default_value< int_t >, int_dv ) ;
        motor_typedefs( generic_default_value< uint_t >, uint_dv ) ;

        motor_typedefs( generic_default_value< motor::math::vec2f_t >, vec2f_dv ) ;
        motor_typedefs( generic_default_value< motor::math::vec3f_t >, vec3f_dv ) ;
        motor_typedefs( generic_default_value< motor::math::vec4f_t >, vec4f_dv ) ;

        motor_typedefs( generic_default_value< motor::math::vec2i_t >, vec2i_dv ) ;
        motor_typedefs( generic_default_value< motor::math::vec3i_t >, vec3i_dv ) ;
        motor_typedefs( generic_default_value< motor::math::vec4i_t >, vec4i_dv ) ;

        motor_typedefs( generic_default_value< motor::math::vec2ui_t >, vec2ui_dv ) ;
        motor_typedefs( generic_default_value< motor::math::vec3ui_t >, vec3ui_dv ) ;
        motor_typedefs( generic_default_value< motor::math::vec4ui_t >, vec4ui_dv ) ;
        
        motor_typedefs( generic_default_value< motor::string_t >, string_dv ) ;
    }
}