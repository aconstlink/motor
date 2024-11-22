
#pragma once

#include "iproperty.hpp"
#include "generic_property.hpp"

#include <motor/std/string>
#include <motor/std/map>

namespace motor
{
    namespace property
    {
        class property_sheet
        {
            motor_this_typedefs( property_sheet ) ;

        private:

            motor::map< motor::string_t, iproperty_mtr_t > _properties ;

        public:

            property_sheet( void_t ) noexcept{}
            property_sheet( this_cref_t ) = delete ;

            property_sheet( this_rref_t rhv ) noexcept
            {
                _properties = std::move( rhv._properties ) ;
            }

            ~property_sheet( void_t ) noexcept
            {
                this_t::clear() ;
            }

            void_t clear( void_t ) noexcept
            {
                for ( auto & i : _properties )
                {
                    motor::release( motor::move( i.second ) ) ;
                }
            }

        public:

            template< typename T >
            bool_t add_property( motor::string_in_t name, motor::property::generic_property< T > && p ) noexcept
            {
                auto iter = _properties.find( name ) ;
                if( iter != _properties.end() ) 
                {
                    return false ;
                }

                _properties[ name ] = motor::shared( std::move( p ), "property" ) ;

                return true ;
            }

            template< typename T >
            bool_t exchange_property( motor::string_in_t name, motor::property::generic_property< T > && p ) noexcept
            {
                auto iter = _properties.find( name ) ;
                if ( iter != _properties.end() )
                {
                    motor::release( motor::move( iter->second ) ) ;
                }

                _properties[ name ] = motor::shared( std::move( p ), "property" ) ;

                return true ;
            }

            template< typename T >
            bool_t set_value( motor::string_cref_t name, T const & v ) noexcept
            {
                auto iter = _properties.find( name ) ;
                if( iter == _properties.end() ) 
                {
                    auto prop =  motor::shared( motor::property::generic_property< T >(), "property" ) ;
                    prop->set( v ) ;
                    _properties[ name ] = motor::move( prop ) ;
                    return true ;
                }
                
                auto * ptr = dynamic_cast< generic_property< T >* > ( iter->second ) ;
                if( ptr == nullptr ) return false ;

                ptr->set( v ) ;
                return true ;
            }

            template< typename T >
            bool_t get_value( motor::string_cref_t name, T & v_out ) const noexcept
            {
                auto iter = _properties.find( name ) ;
                if( iter == _properties.end() ) return false ;

                auto * ptr = dynamic_cast< generic_property< T > const * >( iter->second ) ;
                if( ptr == nullptr ) return false ;

                v_out = ptr->get() ;

                return true ;
            }

            template< typename T >
            motor::property::generic_property< T > * borrow_property( motor::string_in_t name ) noexcept
            {
                for( auto item : _properties )
                {
                    if( item.first == name ) 
                    {
                        return dynamic_cast< motor::property::generic_property< T > * >( item.second ) ;
                    }
                }
                return nullptr ;
            }

            template< typename T >
            bool_t get_property( motor::string_in_t name, motor::core::mtr_safe< motor::property::generic_property< T > > & res ) noexcept
            {
                auto * ret = this_t::borrow_property<T>( name ) ;
                if( ret == nullptr ) return false ;
                res = motor::shared( ret )  ;
                return true ;
            }

            typedef std::function< void_t ( motor::string_cref_t, motor::property::iproperty_cptr_t ) > for_each_cfunk_t ;

            void_t read_for_each( for_each_cfunk_t f ) const noexcept
            {
                for( auto i : _properties )
                {
                    f( i.first, i.second ) ;
                }
            }

            typedef std::function< void_t ( motor::string_cref_t, motor::property::iproperty_mtr_t ) > for_each_funk_t ;

            void_t for_each( for_each_funk_t f ) const noexcept
            {
                for ( auto i : _properties )
                {
                    f( i.first, i.second ) ;
                }
            }

        };
        motor_typedef( property_sheet ) ;


        template< typename T >
        static bool_t add_is_property( motor::string_in_t name, motor::wire::iinput_slot_ptr_t is, motor::property::property_sheet_inout_t props )
        {
            using type_t = motor::wire::input_slot< T > ;
            using property_t = motor::property::generic_property< type_t > ;
            property_t p ;

            if ( property_t::create_property( is, p ) )
            {
                if( !props.add_property( name, std::move( p ) ) )
                {
                    return props.borrow_property< type_t >( name ) != nullptr ;
                }
                return true ;
            }
            return false ;
        }
    }
}