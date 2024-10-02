
#pragma once

#include "iproperty.hpp"
#include "editor_hint.hpp"
#include "min_max.hpp"
#include "default_value.hpp"

#include <motor/std/string>

#include <type_traits>

namespace motor
{
    namespace property
    {
        template< typename T >
        class generic_property : public iproperty
        {
            motor_typedefs( T, value ) ;
            motor_this_typedefs( generic_property< T > ) ;
            using min_max_t_ = motor::property::min_max< T, 
                std::is_enum< T >::value, std::is_fundamental< T >::value || std::is_enum< T >::value > ;
            motor_typedefs( min_max_t_, min_max ) ;

        private:

            value_t _data = motor::property::default_value<T>() ;
            min_max_t _mm = min_max_t() ;
            motor::property::editor_hint _hint = 
                motor::property::default_editor_hint<T, std::is_enum<T>::value >::value ;

        public:

            generic_property( void_t ) noexcept{}

            generic_property( motor::property::editor_hint const h ) noexcept : 
                _hint( h ) {}

            generic_property( motor::property::editor_hint const h, this_t::min_max_in_t mm ) noexcept : 
                _mm( mm ), _hint( h ) {}

            generic_property( this_t::min_max_in_t mm ) noexcept : 
                _mm( mm ) {}

            generic_property( T const & v ) noexcept :
                _data( v ) {}

            generic_property( T const & v, motor::property::editor_hint const h ) noexcept :
                _data( v ), _hint( h ) {}

            generic_property( T const & v, this_t::min_max_in_t mm ) noexcept :
                _data( v ), _mm( mm ) {}

            generic_property( T const & v, motor::property::editor_hint const h, this_t::min_max_in_t mm ) noexcept :
                _data( v ), _hint( h ), _mm( mm ) {}

            virtual ~generic_property( void_t ) noexcept {}

        public:

            void_t set( value_cref_t data ) noexcept
            {
                _data = data ;
            }

            value_cref_t get( void_t ) const noexcept
            {
                return _data ;
            }

            value_ptr_t ptr( void_t ) noexcept
            {
                return &_data ;
            }

            motor::property::editor_hint get_hint( void_t ) const  noexcept
            {
                return _hint ;
            }

            void_t set_hint( motor::property::editor_hint const h ) noexcept
            {
                _hint = h ;
            }

            this_t::min_max_cref_t get_min_max( void_t ) const noexcept
            {
                return _mm ;
            }

            void_t set_min_max( this_t::min_max_in_t mm ) noexcept
            {
                _mm = mm ;
            }
        };
        motor_typedefs( generic_property< byte_t >, byte_property ) ;
        motor_typedefs( generic_property< bool_t >, bool_property ) ;
        motor_typedefs( generic_property< ushort_t >, ushort_property ) ;
        motor_typedefs( generic_property< short_t >, short_property ) ;
        motor_typedefs( generic_property< int_t >, int_property ) ;
        motor_typedefs( generic_property< uint_t >, uint_property ) ;
        motor_typedefs( generic_property< float_t >, float_property ) ;
        motor_typedefs( generic_property< double_t >, double_property ) ;
        motor_typedefs( generic_property< motor::string_t >, string_property ) ;
    }
}