
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
                    motor::release( motor::move( iter->first ) ) ;
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
    }
}