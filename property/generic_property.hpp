
#pragma once

#include "iproperty.hpp"
#include "editor_hint.hpp"
#include "min_max.hpp"
#include "default_value.hpp"

#include <motor/wire/slot/input_slot.h>
#include <motor/std/string>

#include <type_traits>

namespace motor
{
    namespace property
    {
        template< typename T >
        class numerical_traits : public iproperty
        {
            motor_this_typedefs( numerical_traits< T > ) ;

        public:

            motor_typedefs( T, value ) ;
            using min_max_t_ = motor::property::min_max< T, std::is_fundamental< T >::value > ;
            motor_typedefs( min_max_t_, min_max ) ;

        private:

            min_max_t _mm = min_max_t() ;
            motor::property::editor_hint _hint =
                motor::property::default_editor_hint<T, std::is_enum<T>::value >::value ;

        public:

            numerical_traits( void_t ) noexcept {}
            numerical_traits( motor::property::editor_hint const h ) noexcept :
                _hint( h ) {}

            numerical_traits( motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept :
                _mm( mm ), _hint( h ) {}

            numerical_traits( typename this_t::min_max_in_t mm ) noexcept :
                _mm( mm ) {}

            numerical_traits( this_rref_t rhv ) noexcept : _mm( rhv._mm ), _hint( rhv._hint ) {}
            virtual ~numerical_traits( void_t ) noexcept {}

        public:

            virtual void_t set( value_cref_t data ) noexcept = 0 ;
            virtual value_cref_t get( void_t ) const noexcept = 0 ;

        public:

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

        motor_typedefs( numerical_traits< byte_t >, byte_traits ) ;
        motor_typedefs( numerical_traits< bool_t >, bool_traits ) ;
        motor_typedefs( numerical_traits< ushort_t >, ushort_traits ) ;
        motor_typedefs( numerical_traits< short_t >, short_traits ) ;
        motor_typedefs( numerical_traits< int_t >, int_traits ) ;
        motor_typedefs( numerical_traits< uint_t >, uint_traits ) ;
        motor_typedefs( numerical_traits< float_t >, float_traits ) ;
        motor_typedefs( numerical_traits< double_t >, double_traits ) ;
        motor_typedefs( numerical_traits< motor::string_t >, string_traits ) ;

        template< typename T, bool is_enum = std::is_enum< T >::value >
        class generic_property ; 

        template< typename T >
        class generic_property< T, false > : public numerical_traits< T >
        {
            motor_this_typedefs( generic_property< T > ) ;
            motor_typedefs( numerical_traits< T >::value_t, value ) ;

        private:

            value_t _data = motor::property::default_value<T>() ;

        public:

            generic_property( void_t ) noexcept{}

            generic_property( motor::property::editor_hint const h ) noexcept : 
                numerical_traits( h ) {}

            generic_property( motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept : 
                numerical_traits( h, mm ) {}

            generic_property( typename this_t::min_max_in_t mm ) noexcept : 
                numerical_traits( mm ) {}

            generic_property( T const & v ) noexcept :
                _data( v ) {}

            generic_property( T const & v, motor::property::editor_hint const h ) noexcept :
                _data( v ), numerical_traits( h ) {}

            generic_property( T const & v, typename this_t::min_max_in_t mm ) noexcept :
                _data( v ), numerical_traits( mm ){}

            generic_property( T const & v, motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept :
                _data( v ), numerical_traits( h, mm ){}

            generic_property( this_rref_t rhv ) noexcept : numerical_traits( std::move( rhv ) ), _data( rhv._data ) {}

            virtual ~generic_property( void_t ) noexcept {}

        public: // numerical_traits interface

            virtual void_t set( value_cref_t data ) noexcept final
            {
                _data = data ;
            }

            virtual value_cref_t get( void_t ) const noexcept final
            {
                return _data ;
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


        template< typename T >
        class generic_property< motor::wire::input_slot< T >, false > : public numerical_traits< T >
        {
            motor_this_typedefs( generic_property< motor::wire::input_slot< T > > ) ;

            motor_typedefs( numerical_traits< T >, base ) ;
            motor_typedefs( numerical_traits< T >::value_t, value ) ;

        private:

            using is_t = motor::wire::input_slot< T > ;
            using is_mtr_safe_t = motor::core::mtr_safe< motor::wire::input_slot< T > > ;
            using is_mtr_t = motor::wire::input_slot< T > * ;

            is_mtr_t _is = motor::shared( is_t( motor::property::default_value<T>() ) ) ;

        public:

            generic_property( void_t ) noexcept {}

            generic_property( motor::property::editor_hint const h ) noexcept : 
                base_t( h ) {}

            generic_property( motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept :
                base_t( h, mm ) {}

            generic_property( typename this_t::min_max_in_t mm ) noexcept :
                base_t( mm ) {}

            generic_property( T const & v ) noexcept :
            {
                _is->set_value( v ) ;
            }

            generic_property( T const & v, motor::property::editor_hint const h ) noexcept : base_t( h ) 
            {
                _is->set_value( v ) ;
            }

            generic_property( T const & v, typename this_t::min_max_in_t mm ) noexcept : base_t( mm ) 
            {
                _is->set_value( v ) ;
            }

            generic_property( T const & v, motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept :
                base_t( h, mm ) 
            {
                _is->set_value( v ) ;
            }

            generic_property( is_mtr_safe_t input ) noexcept 
            {
                motor::release( motor::move( _is ) ) ;
                _is = motor::move( input ) ;
            }

            generic_property( is_mtr_safe_t input, motor::property::editor_hint const h ) noexcept :
                base_t( h ) 
            {
                motor::release( motor::move( _is ) ) ;
                _is = motor::move( input ) ;
            }

            generic_property( is_mtr_safe_t input, typename this_t::min_max_in_t mm ) noexcept :
                base_t( mm ) 
            {
                motor::release( motor::move( _is ) ) ;
                _is = motor::move( input ) ;
            }

            generic_property( is_mtr_safe_t input, motor::property::editor_hint const h, typename this_t::min_max_in_t mm ) noexcept :
                base_t( h, mm )
            {
                motor::release( motor::move( _is ) ) ;
                _is = motor::move( input ) ;
            }

            generic_property( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
            {
                _is->disconnect() ;
                motor::release( motor::move( _is ) ) ;
                _is = motor::move( rhv._is ) ;
            }

            virtual ~generic_property( void_t ) noexcept 
            {
                motor::release( motor::move( _is ) ) ;
            }

        public: // numerical_traits interface

            virtual void_t set( value_cref_t data ) noexcept final
            {
                _is->set_value( data ) ;
            }

            virtual value_cref_t get( void_t ) const noexcept final
            {
                return _is->get_value() ;
            }

        public: 

            typename this_t::is_mtr_safe_t get_is( void_t ) noexcept
            {
                return motor::share( _is ) ;
            }

            typename this_t::is_mtr_t borrow_is( void_t ) noexcept 
            {
                return _is ;
            }
        };
        motor_typedefs( generic_property< motor::wire::input_slot< byte_t > >, byte_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< bool_t > >, bool_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< ushort_t > >, ushort_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< short_t > >, short_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< int_t > >, int_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< uint_t > >, uint_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< float_t > >, float_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< double_t > >, double_is_property ) ;
        motor_typedefs( generic_property< motor::wire::input_slot< motor::string_t > >, string_is_property ) ;
    }
}