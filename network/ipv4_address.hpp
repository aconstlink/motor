#pragma once

#include "typedefs.h"

#include <motor/log/global.h>
#include <motor/std/vector>


namespace motor { namespace network { namespace ipv4 {

    class address
    {
        motor_this_typedefs( address ) ;

    private:

        uint_t _x ;
        uint_t _y ;
        uint_t _z ;
        uint_t _w ;

    public:

        address( void ) noexcept
        {
            _x = 127 ;
            _y = 0 ;
            _z = 0 ;
            _w = 1 ;
        }

        address( uint_t a, uint_t b, uint_t c, uint_t d ) noexcept
        {
            _x = a ; _y = b ; _z = c ; _w = d ;
        }

        address( this_cref_t rhv ) noexcept
        {
            _x = rhv._x ; _y = rhv._y ; _z = rhv._z ; _w = rhv._w ;
        }

        bool_t operator == ( this_cref_t rhv ) const noexcept
        {
            return _x == rhv._x && _y == rhv._y && _z == rhv._z && _w == rhv._w ;
        }

        uint_t get_a( void ) const noexcept { return _x ; }
        uint_t get_b( void ) const noexcept { return _y ; }
        uint_t get_c( void ) const noexcept { return _z ; }
        uint_t get_d( void ) const noexcept { return _w ; }

    public:

        motor::string_t to_string( void ) const noexcept
        {
            return motor::to_string( _x ) + "." + motor::to_string( _y ) +
                "." + motor::to_string( _z ) + "." + motor::to_string( _w ) ;
        }

        static this_t from_string( motor::string_in_t host ) noexcept
        {
            if ( host == "localhost" ) return this_t( 127, 0, 0, 1 ) ;

            motor::vector< motor::string_t > tokens ;

            {
                size_t start = 0 ;
                size_t pos = 0 ;
                motor::string_t const delimiter = "." ;

                motor::string_t token;
                while ( ( pos = host.find( delimiter, start ) ) != std::string::npos )
                {
                    token = host.substr( start, pos - start );
                    tokens.push_back( token ) ;
                    start = pos + 1 ;
                }

                if ( start != host.size() )
                {
                    token = host.substr( start, ( host.size() ) - start );
                    tokens.push_back( token ) ;
                }
            }

            if ( tokens.size() != 4 )
            {
                motor::log::global_t::error( "[ip4_address::from_string] : invalid host address" ) ;
                return this_t( 127, 0, 0, 1 ) ;
            }

            return this_t( std::atol( tokens[ 0 ].c_str() ), std::atol( tokens[ 1 ].c_str() ), std::atol( tokens[ 2 ].c_str() ), std::atol( tokens[ 3 ].c_str() ) ) ;
        }
    };
    motor_typedef( address ) ;
    motor_typedefs( motor::vector< address_t >, addresses ) ;

    struct binding_point
    {
        uint_t port ;
        address_t address ;

        static binding_point localhost_at( uint_t const port ) noexcept
        {
            return binding_point { port, address_t() } ;
        }

        motor::string_t to_string( void_t ) const noexcept
        {
            return address.to_string() + ":" + motor::to_string( port ) ;
        }
    };
    motor_typedef( binding_point ) ;

} } }