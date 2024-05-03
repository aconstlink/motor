

#pragma once

#include "../typedefs.h"

#include <motor/std/string>
#include <motor/std/vector>

namespace motor { namespace social { namespace twitch
{
    class irc_parser
    {
        motor_this_typedefs( irc_parser ) ;

    private:

        motor::string_t _raw_msg ;

    private: // working on 

        motor::vector< motor::string_t > lines ;

        motor::vector< std::pair< motor::string_t, motor::string_t> > _caps ;

    public:

        irc_parser( void_t ) noexcept {}
        irc_parser( motor::string_cref_t raw_msg ) noexcept : _raw_msg( raw_msg ) {}
        irc_parser( motor::string_rref_t raw_msg ) noexcept : _raw_msg( std::move( raw_msg ) ) {}
        irc_parser( this_rref_t rhv ) noexcept : _raw_msg( std::move( rhv._raw_msg ) ) {}
        ~irc_parser( void_t ) noexcept {}

        this_ref_t operator = ( this_rref_t rhv ) noexcept
        {
            _raw_msg = std::move( rhv._raw_msg ) ;
            _caps = std::move( rhv._caps ) ;
        }

    public:

        bool_t parse( motor::string_cref_t new_msg ) noexcept
        {
            _raw_msg = new_msg ;
            return this_t::parse() ;
        }

        bool_t parse( motor::string_rref_t new_msg ) noexcept
        {
            _raw_msg = std::move( new_msg ) ;
            return this_t::parse() ;
        }

        bool_t parse( void_t ) noexcept
        {
            lines.reserve( 100 ) ;

            // separate \r\n lines
            {
                size_t start = 0 ;
                while ( true )
                {
                    size_t const pos = _raw_msg.find_first_of( '\r', start ) ;
                    if ( pos != std::string::npos )
                    {
                        lines.emplace_back( _raw_msg.substr( start, pos - start ) ) ;
                        start = pos + 2 ;
                    }

                    if ( pos == std::string::npos ) break ;
                }
            }

            // parser single message
            {
                motor::vector< motor::string_t > tokens ;
                for ( auto const & line : lines )
                {
                    // tokenize
                    {
                        size_t start = 0 ;
                        while ( true )
                        {
                            size_t const pos = line.find_first_of( ' ', start ) ;
                            if ( pos != std::string::npos )
                            {
                                tokens.emplace_back( line.substr( start, pos - start ) ) ;
                                start = pos + 1 ;
                            }

                            if ( pos == std::string::npos )
                            {
                                tokens.emplace_back( line.substr( start, line.size() - start ) ) ;
                                break ;
                            }
                        }
                    }

                    // analyze
                    {
                        auto const & token = tokens[ 0 ] ;
                        size_t const start = token.find_first_of( '!', 1 ) ;
                        size_t const end = token.find_first_of( '@', 1 ) ;

                        if ( start != std::string::npos )
                        {
                            size_t const p0 = start + 1 ;
                            size_t const p1 = end ;
                            motor::string_t const user = token.substr( p0, p1 - p0 ) ;

                            int bp = 0 ;
                        }
                        
                    }
                    
                    tokens.clear() ;
                }
            }

            return true ;
        }

    };

}}}