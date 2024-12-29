

#pragma once

#include <motor/std/string>
#include <motor/std/vector>

namespace motor
{
    namespace core
    {
        class read_doc
        {
            motor_this_typedefs( read_doc ) ;

        private:

            motor::string_t _doc ;

            struct token
            {
                size_t s ;
                size_t e ;

                size_t dist( void_t ) const noexcept
                {
                    return e - s ;
                }
            };

            struct line
            {
                size_t s ;
                size_t e ;

                size_t dist( void_t ) const noexcept
                {
                    return e - s ;
                }

                motor::vector< token > tokens ;
            };
            motor::vector< line > _lines ;

        public: 

            class line_view
            {
                friend class read_doc ;

            private:

                read_doc const * _doc ;

                size_t _lidx ;
                std::string_view _line ;

                line_view( read_doc const * doc, size_t const idx, std::string_view && line ) noexcept:
                    _doc( doc ), _lidx( idx ), _line( line )
                {}

            public:

                std::string_view get_line( void_t ) const noexcept
                {
                    return _line ;
                }

                size_t get_num_tokens( void_t ) const noexcept 
                {
                    return _doc->_lines[ _lidx ].tokens.size() ;
                }

                std::string_view get_token( size_t const tidx ) const noexcept
                {
                    return _doc->make_token( _doc->get_token_us( _lidx, tidx ) ) ;
                }
            };

            friend class line_view ;

        public:

            read_doc( motor::string_in_t s ) noexcept : _doc( s )
            {
                this_t::exchange( '\t', ' ' ) ;
                this_t::dissect_lines() ;
                this_t::remove_multi_whitespaces() ;
                this_t::tokenize() ;
            }

            read_doc( motor::string_rref_t s ) noexcept : _doc( std::move( s ) )
            {
                this_t::exchange( '\t', ' ' ) ;
                this_t::dissect_lines() ;
                this_t::remove_multi_whitespaces() ;
                this_t::tokenize() ;
            }

            using line_string_funk_t = std::function< void_t ( std::string_view const &  ) > ;
            void_t for_each_line( line_string_funk_t f ) const noexcept
            {
                for( auto const & l : _lines )
                {
                    f( this_t::make_view( l ) ) ;
                }
            }

            using line_view_funk_t = std::function< void_t ( this_t::line_view const & ) > ;
            void_t for_each_line( line_view_funk_t f ) const noexcept
            {
                size_t i=0; 
                for ( auto const & l : _lines )
                {
                    f( this_t::line_view( this, i++, this_t::make_view( l ) ) ) ;
                }
            }

            using for_each_token_funk_t = std::function< void_t ( size_t const, std::string_view const & ) > ;

            void_t for_each_token( for_each_token_funk_t funk ) noexcept
            {
                for( auto const & l : _lines )
                {
                    if( l.tokens.empty() ) continue ;
                    
                    size_t i = 0 ;
                    for( auto const & t : l.tokens )
                    {
                        funk( i++, this_t::make_token( t ) ) ;
                    }
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
                        
                        // move start of line to first
                        // non whitespace char
                        {
                            while ( _doc[ s ] == ' ' && s < e ) ++s ;
                        }

                        // s == e would be an empty line.
                        if( s != e ) _lines.emplace_back( this_t::line{ s, e } ) ;

                        s = p + 1 ;
                        p = _doc.find_first_of( '\n', s ) ;
                    }
                }
            }

            // determine tokens per line
            // @precondition requires lines to be white space handled.
            void_t tokenize( void_t ) noexcept
            {
                for( auto & l : _lines )
                {
                    // count tokens
                    {
                        size_t num_tokens = 0 ;
                        for( size_t i=l.s; i<=l.e; ++i )
                        {
                            if( _doc[i] == ' ' ) ++num_tokens ;
                        }
                        // there is no whitespace at the end, so
                        // increment by one.
                        l.tokens.resize( ++num_tokens ) ;
                    }

                    // add token meta
                    {
                        size_t t = 0 ;
                        size_t s = l.s ;
                        for ( size_t i = l.s; i <= l.e; ++i )
                        {
                            size_t e = s ;
                            while( _doc[e] != ' ' && e <= l.e ) ++e ;
                            if( e >= l.e ) break ;

                            l.tokens[t++] = this_t::token { s, e } ;

                            s = e + 1 ;
                        }
                        l.tokens[t] = this_t::token { s, l.e } ;
                    }
                }
            }

            // go through the document and exchange a single character
            void_t exchange( char_t const from, char_t const to ) noexcept
            {
                for( size_t i=0; i<_doc.size(); ++i )
                {
                    if( _doc[i] == from ) _doc[i] = to ;
                }
            }

            // run over all lines
            void_t remove_multi_whitespaces( void_t ) noexcept
            {
                // the tmp buffer is so freaking fast compared
                // to using C++ substr or similar C++
                char buffer[ 4096 ] ;

                size_t kill_beg = 0 ;
                for( auto & l : _lines )
                {
                    // this is really optional
                    // @todo this can be removed from release code.
                    // those positions are not used anyways.
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

            // removes whitespaces from the buffer.
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

                    // find next character
                    while ( buffer[ p ] == ' ' && p < chars ) ++p ;
                    if ( p == chars ) break ;

                    buffer[ bp++ ] = ' ' ;

                    s = p ;
                }

                return bp ;
            }

            this_t::token get_token_us( size_t const lidx, size_t const tidx ) const noexcept
            {
                return _lines[lidx].tokens[tidx] ;
            }

            std::string_view make_view( this_t::line const & l ) const noexcept
            {
                return std::string_view( _doc.data() + l.s, l.dist() ) ;
            }

            std::string_view make_token( this_t::token const & t ) const noexcept
            {
                return std::string_view( _doc.data() + t.s, t.dist() ) ;
            }
        };
    }
}