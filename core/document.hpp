

#pragma once

#include <motor/std/string>
#include <motor/std/vector>

namespace motor
{
    namespace core
    {
        class document
        {
            motor_this_typedefs( document ) ;

        private:

            motor::string_t _doc ;

            struct line
            {
                size_t s ;
                size_t e ;

                size_t dist( void_t ) const noexcept
                {
                    return e - s ;
                }
            };
            motor::vector< line > _lines ;

        public:

            document( motor::string_in_t s ) noexcept : _doc( s )
            {
                this_t::dissect_lines() ;
                this_t::remove_multi_whitespaces() ;
            }

            document( motor::string_rref_t s ) noexcept : _doc( std::move( s ) )
            {
            }

            using line_string_funk_t = std::function< void_t ( std::string_view const &  ) > ;
            void_t for_each_line( line_string_funk_t f ) const noexcept
            {
                for( auto const & l : _lines )
                {
                    f( std::string_view( _doc.data() + l.s, l.dist() ) ) ;
                }
            }

        private:

            void_t dissect_lines( void_t ) noexcept
            {
                // count number of lines
                {
                    size_t num_lines = 0 ;
                    for ( auto const & c : _doc )
                    {
                        if ( c == '\n' ) ++num_lines ;
                    }
                    _lines.reserve( num_lines ) ;
                }

                // create line meta
                {
                    size_t s = 0 ;
                    size_t p = _doc.find_first_of( '\n' ) ;
                    while ( p != std::string::npos )
                    {
                        size_t const pb = p - 1 ;
                        size_t const e = ( pb < _doc.size() && _doc[ pb ] == '\r' ) ? pb : p ;
                        
                        // s == e would be an empty line.
                        if( s != e ) _lines.emplace_back( this_t::line{ s, e } ) ;

                        s = p + 1 ;
                        p = _doc.find_first_of( '\n', s ) ;
                    }
                }
            }

            void_t remove_multi_whitespaces( void_t ) noexcept
            {
                // the tmp buffer is so freaking fast compared
                // to using C++ substr or similar C++
                char buffer[ 4096 ] ;

                size_t kill_beg = 0 ;
                for( auto & l : _lines )
                {
                    // this is really optional
                    {
                        size_t const len = l.s - kill_beg ;
                        _doc.replace( kill_beg, len, len, 'x' ) ;
                    }

                    std::memcpy( buffer, _doc.data()+l.s, l.dist() ) ;
                    size_t const new_dist = this_t::remove_multi_whitespaces( buffer, l.dist() ) ;

                    buffer[new_dist] = '\n' ;
                    buffer[new_dist + 1] = '\0' ;
                    _doc.replace( l.s, new_dist+1, buffer ) ;
                    l.e = l.s + new_dist ;
                    kill_beg = l.e + 1 ;
                }
            }

            size_t remove_multi_whitespaces( char * buffer, size_t const chars ) noexcept
            {
                size_t bp = 0 ;

                size_t s = 0 ;
                size_t p = 0 ;
                while ( p < chars )
                {
                    // find first white space
                    while ( buffer[ p ] != ' ' && p < chars ) ++p ;

                    size_t const dist = p - s ;
                    std::memcpy( buffer + bp, buffer + s, dist ) ;
                    bp += dist ;

                    if ( p == chars ) break ;

                    // find first white space
                    while ( buffer[ p ] == ' ' && p < chars ) ++p ;
                    if ( p == chars ) break ;

                    buffer[ bp++ ] = ' ' ;

                    s = p ;
                }

                return bp ;
            }
        };
    }
}