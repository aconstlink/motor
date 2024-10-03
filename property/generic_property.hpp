
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
        template< typename T, bool is_enum = std::is_enum< T >::value >
        class generic_property ; 

        template< typename T >
        class generic_property< T, false > : public iproperty
        {
            motor_typedefs( T, value ) ;
            motor_this_typedefs( generic_property< T > ) ;
            using min_max_t_ = motor::property::min_max< T, std::is_fundamental< T >::value > ;
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

            generic_property( motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept : 
                _mm( mm ), _hint( h ) {}

            generic_property( typename this_t::min_max_in_t mm ) noexcept : 
                _mm( mm ) {}

            generic_property( T const & v ) noexcept :
                _data( v ) {}

            generic_property( T const & v, motor::property::editor_hint const h ) noexcept :
                _data( v ), _hint( h ) {}

            generic_property( T const & v, typename this_t::min_max_in_t mm ) noexcept :
                _data( v ), _mm( mm ) {}

            generic_property( T const & v, motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept :
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

            typename this_t::min_max_cref_t get_min_max( void_t ) const noexcept
            {
                return _mm ;
            }

            void_t set_min_max( typename this_t::min_max_in_t mm ) noexcept
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


        class any_enum_property : public iproperty
        {

        public:

            virtual ~any_enum_property( void_t ) noexcept {}

        public:

            using strings_funk_t = std::function< std::pair< char_cptrc_t *, size_t > ( void_t ) > ;

            virtual std::pair< char_cptrc_t*, size_t > get_strings( void_t ) const noexcept = 0 ;
            virtual char_cptrc_t get_char_ptr( void_t ) const noexcept = 0 ;
            virtual size_t as_index( void_t ) const noexcept = 0 ;
            virtual void_t set_by_name( motor::string_in_t name ) noexcept = 0 ;
            virtual void_t set_by_ptr( char_cptrc_t name ) noexcept = 0 ;
            virtual void_t set_by_index( size_t const idx ) noexcept = 0  ;
            virtual motor::property::editor_hint get_hint( void_t ) const  noexcept = 0 ;
        };
        motor_typedef( any_enum_property ) ;

        template< typename T >
        class generic_property< T, true > : public any_enum_property
        {
            motor_typedefs( T, value ) ;
            motor_this_typedefs( generic_property< T > ) ;

        private:

            value_t _data = motor::property::default_value<T>() ;
            motor::property::editor_hint _hint = motor::property::editor_hint::combo_box ;
                        
        private: 

            static inline strings_funk_t _strings_funk = []( void_t ){ return std::make_pair( nullptr, 0 ) ; } ;

        public:

            static void_t set_strings_funk( strings_funk_t f ) noexcept { _strings_funk = f ; } 

        public:

            generic_property( void_t ) noexcept{}
            generic_property( value_t const e ) noexcept : _data( e ){}

        public: // virtuals

            virtual std::pair< char_cptrc_t*, size_t > get_strings( void_t ) const noexcept { return _strings_funk() ; }

            virtual char_cptrc_t get_char_ptr( void_t ) const noexcept
            {
                return this_t::get_strings().first[size_t(_data)] ;
            }

            virtual size_t as_index( void_t ) const noexcept
            {
                return size_t( _data ) ;
            }

            virtual void_t set_by_name( motor::string_in_t name ) noexcept
            {
                auto p = this_t::get_strings() ; 
                for( size_t i=0; i<p.second; ++i )
                {
                    if( motor::string_t( p.first[i] ) == name  )
                    {
                        _data = T(i) ;
                        break ;
                    }
                }
            }

            virtual void_t set_by_ptr( char_cptrc_t name ) noexcept
            {
                auto p = this_t::get_strings() ;
                for ( size_t i = 0; i < p.second; ++i )
                {
                    if ( p.first[ i ] == name )
                    {
                        _data = T( i ) ;
                        break ;
                    }
                }
            }

            virtual void_t set_by_index( size_t const idx ) noexcept
            {
                _data = T(idx) ;
            }

            virtual motor::property::editor_hint get_hint( void_t ) const  noexcept
            {
                return _hint ;
            }

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
        } ;
    }
}