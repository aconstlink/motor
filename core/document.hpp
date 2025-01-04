

#pragma once

#include <motor/std/string>
#include <motor/std/vector>

#include <cstring>

namespace motor
{
    namespace core
    {
        class document
        {
            motor_this_typedefs( document ) ;

        private:

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

                size_t indent ;
                size_t first_token ;
                size_t num_token ;

                size_t dist( void_t ) const noexcept
                {
                    return e - s ;
                }
            };
            

        public: 

            class line_view
            {
                friend class document ;

            private:

                document const * _doc ;

                size_t _lidx ;
                size_t _indent ;
                std::string_view _line ;

                line_view( document const * doc, size_t const idx, size_t const indent, std::string_view && line ) noexcept:
                    _doc( doc ), _lidx( idx ), _indent( indent ), _line( line )
                {}

            public:

                std::string_view get_line( void_t ) const noexcept
                {
                    return _line ;
                }

                size_t get_num_tokens( void_t ) const noexcept 
                {
                    return _doc->_lines[ _lidx ].num_token ;
                }

                std::string_view get_token( size_t const tidx ) const noexcept
                {
                    return _doc->make_token( _doc->get_token_us( _lidx, tidx ) ) ;
                }

                size_t indent( void_t ) const noexcept { return _indent ; }
            };

            friend class line_view ;

        public:

            class section_guard
            {
            private:

                document * _doc ;

            public:

                section_guard( document * owner ) noexcept : _doc( owner )
                {
                    assert( _doc != nullptr ) ;
                    _doc->section_open() ;
                }

                ~section_guard( void_t ) noexcept
                {
                    _doc->section_close() ;
                }
            };

        private:

            char * _doc = nullptr ;
            size_t _cur_pos = 0 ;

            size_t _grow_by = 500 ;
            size_t _size = 1000 ;

            motor::vector< line > _lines ;
            motor::vector< token > _tokens ;

        private: // for writing to this document

            size_t _indent = 0 ;

        public:

            document( void_t ) noexcept
            {
                _doc = motor::memory::global::alloc_raw<char_t>( _size ) ;
                _doc[ 0 ] = '\0' ;
            }

            document( motor::string_in_t s ) noexcept
            {
                _size = s.size() ;
                _doc = motor::memory::global::alloc_raw<char_t>( _size + 1 ) ;
                std::memcpy( _doc, s.data(), _size ) ;
                _doc[ _size ] = '\0' ;
                _cur_pos = _size ;


                this_t::dissect_lines() ;
                this_t::exchange( '\t', ' ' ) ;
                this_t::remove_multi_whitespaces() ;
                this_t::tokenize_all() ;
            }

            document( this_rref_t rhv ) noexcept : _size( rhv._size ), _cur_pos( rhv._cur_pos ),
                _indent( rhv._indent ), _grow_by( rhv._grow_by ), _lines( std::move( rhv._lines ) ),
                _tokens( std::move( rhv._tokens) )
            {
                motor::memory::global::dealloc_raw( _doc ) ;
                _doc = motor::move( rhv._doc ) ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _size = rhv._size ;
                _cur_pos = rhv._cur_pos ;
                _indent = rhv._indent ;
                _grow_by = rhv._grow_by ;
                _lines = std::move( rhv._lines ) ;
                _tokens = std::move( rhv._tokens ) ;

                motor::memory::global::dealloc_raw( _doc ) ;
                _doc = motor::move( rhv._doc ) ;

                return *this ;
            }

            ~document( void_t ) noexcept
            {
                motor::memory::global::dealloc_raw( _doc ) ;
            }

            using line_string_funk_t = std::function< void_t ( std::string_view const &  ) > ;
            void_t for_each_line( line_string_funk_t f ) const noexcept
            {
                for( auto const & l : _lines )
                {
                    f( this_t::make_line_view( l ) ) ;
                }
            }

            using line_view_funk_t = std::function< void_t ( this_t::line_view const & ) > ;
            void_t for_each_line( line_view_funk_t f ) const noexcept
            {
                size_t i=0; 
                for ( auto const & l : _lines )
                {
                    f( this_t::line_view( this, i++, l.indent, this_t::make_line_view( l ) ) ) ;
                }
            }

            using for_each_token_funk_t = std::function< void_t ( size_t const, size_t const, std::string_view const & ) > ;

            void_t for_each_token( for_each_token_funk_t funk ) noexcept
            {
                size_t line_number = 0 ;
                for( auto const & l : _lines )
                {
                    if( l.num_token == 0 ) continue ;
                    
                    for( size_t i=0; i<l.num_token; ++i )
                    {
                        funk( line_number, i, this_t::make_token( _tokens[ l.first_token + i] ) ) ;
                    }
                    ++line_number ;
                }
            }

        public: // 

            //************************************************************
            this_ref_t section_open( void_t ) noexcept
            {
                ++_indent ;
                return *this ;
            }

            //************************************************************
            this_ref_t section_close( void_t ) noexcept
            {
                assert( _indent > 0 ) ;
                --_indent ;
                return *this ;
            }

            //************************************************************
            this_ref_t println( char const * text, size_t len = size_t( -1 ) ) noexcept
            {
                if( len == size_t (-1) ) while( text[++len] != '\0' ) ;

                this_t::ensure_fit( len + 1 ) ;

                size_t const s = _cur_pos ;
                size_t const e = _cur_pos + len ;

                std::memcpy( _doc + _cur_pos, text, len * sizeof( char ) ) ;
                _doc[ e ] = '\0' ;

                if( _lines.size() == _lines.capacity() ) _lines.reserve( _lines.size() + 100 ) ;

                _lines.emplace_back( this_t::line { s, e, _indent, 0, 0 } ) ;
                
                _cur_pos = e ;

                this_t::tokenize_last_line() ;

                return *this ;
            }

            //************************************************************
            this_ref_t println( motor::string_in_t text ) noexcept
            {
                return this_t::println( text.c_str(), text.size() ) ;
            }

            //************************************************************
            this_ref_t println_if( std::string_view const sv, bool_t const cond ) noexcept
            {
                if( !cond ) return *this ;
                return this_t::println( sv.data(), sv.size() ) ;
            }

            //************************************************************
            motor::string_t to_string( void_t ) const noexcept
            {
                motor::string_t ret ;
                ret.reserve( _size + _size/2 ) ;

                this_t::for_each_line( [&] ( motor::core::document::line_view const & lv )
                {
                    if( lv.get_num_tokens() != 0 )
                    {
                        for( size_t i=0; i<lv.indent(); ++i ) ret += '\t' ;
                        for ( size_t t = 0 ; t < lv.get_num_tokens()-1; ++t )
                        {
                            auto const token = lv.get_token( t ) ;
                            ret += token ;
                            ret += " " ;
                        }
                        ret += lv.get_token( lv.get_num_tokens()-1 ) ;
                    }
                    ret += '\n' ;
                } ) ;

                return ret ;
            }

        private:

            void_t dissect_lines( void_t ) noexcept
            {
                // count number of lines
                {
                    size_t num_lines = 0 ;
                    for ( size_t i=0; i<_size; ++i )
                    {
                        if ( _doc[i] == '\n' ) ++num_lines ;
                    }
                    _lines.reserve( num_lines ) ;
                }

                // create line meta
                {
                    auto const doc = std::string_view( _doc ) ;

                    size_t s = 0 ;
                    size_t p =  doc.find_first_of( '\n' ) ;
                    while ( p != std::string::npos )
                    {
                        size_t const pb = p - 1 ;
                        size_t const e = ( pb < doc.size() && _doc[ pb ] == '\r' ) ? pb : p ;

                        // move start of line to first
                        // non whitespace char
                        {
                            size_t indent = 0 ;

                            do
                            {
                                if( _doc[s] == '\t' ) ++indent ;
                                
                                else if( _doc[s] != ' ' ) break ;
                                
                            } while( ++s < e ) ;

                            // s == e would be an empty line.
                            if ( s != e ) _lines.emplace_back( this_t::line { s, e, indent, 0, 0 } ) ;
                        }

                        s = p + 1 ;
                        p = doc.find_first_of( '\n', s ) ;
                    }
                    if( s < doc.size() )
                        _lines.emplace_back( this_t::line { s, doc.size(), 0, 0, 0 } ) ;
                }
            }

            // for debug purposes only!
            #if defined( _DEBUG )
            #define show_string_view 0
            #endif

            //*********************************************************************************
            // tokenize only the last line
            void_t tokenize_last_line( void_t ) noexcept
            {
                auto & ll = _lines.back() ;

                #if show_string_view
                auto const __lv__ = this_t::make_line_view( ll ) ;
                #endif

                // roughly counting tokens
                // and reserving space
                {
                    size_t num_tokens = 0 ;
                    for ( size_t i = ll.s; i < ll.e; ++i )
                    {
                        if ( _doc[ i ] == ' ' ) ++num_tokens ;
                    }
                    size_t const required = _tokens.size() + (++num_tokens) ;
                    if ( _tokens.capacity() <= required ) _tokens.reserve( required + 100 ) ;
                }

                size_t const tokens_before = _tokens.size() ;

                // make meta for token
                {
                    size_t s = ll.s ;
                    size_t e = ll.s ;

                    while( e < ll.e )
                    {
                        // seek end of token
                        while ( _doc[ e ] != ' ' && e < ll.e ) ++e ;

                        if ( ( e - s ) >= 1 )
                        {
                            _tokens.emplace_back( this_t::token { s, e } ) ;
                        }

                        #if show_string_view
                        auto const __tv__ = this_t::make_token( this_t::token { s, e } ) ;
                        #endif

                        s = e = e + 1 ;
                    }
                }

                ll.first_token = tokens_before ;
                ll.num_token = _tokens.size() - tokens_before ;
            }

            //*********************************************************************************
            // tokenize the whole document
            // determine tokens per line
            // @precondition requires lines to be white space handled.
            void_t tokenize_all( void_t ) noexcept
            {
                // count number of tokens
                {
                    size_t num_tokens = 0 ;
                    for ( size_t i=0; i<_size; ++i )
                    {
                        if ( _doc[i] == ' ' ) ++num_tokens ;
                    }
                    _tokens.reserve( ++num_tokens ) ;
                }

                size_t cur_token = 0 ;
                for( auto & l : _lines )
                {
                    // count tokens
                    {
                        size_t num_tokens = 0 ;
                        for ( size_t i = l.s; i <= l.e; ++i )
                        {
                            if ( _doc[ i ] == ' ' ) ++num_tokens ;
                        }

                        l.first_token = cur_token ;
                        l.num_token = ++num_tokens ;
                        cur_token += num_tokens ;
                    }

                    // add token meta
                    {
                        size_t s = l.s ;
                        for ( size_t i = l.s; i <= l.e; ++i )
                        {
                            size_t e = s ;
                            while( _doc[e] != ' ' && e <= l.e ) ++e ;
                            if( e >= l.e ) break ;

                            _tokens.emplace_back( this_t::token { s, e } ) ;

                            s = e + 1 ;
                        }
                        _tokens.emplace_back( this_t::token { s, l.e } ) ;
                    }
                }
            }

            // go through the document and exchange a single character
            void_t exchange( char_t const from, char_t const to ) noexcept
            {
                for( size_t i=0; i<_size; ++i )
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
                        //size_t const len = l.s - kill_beg ;
                        //_doc.replace( kill_beg, len, len, 'x' ) ;

                        for( size_t i=kill_beg; i<l.s; ++i ) _doc[i] = 'x' ;
                    }

                    std::memcpy( buffer, _doc + l.s, l.dist() ) ;
                    size_t const new_dist = this_t::remove_multi_whitespaces( buffer, l.dist() ) ;

                    buffer[new_dist] = '\n' ;
                    buffer[new_dist + 1] = '\0' ;
                    //_doc.replace( l.s, new_dist+1, buffer ) ;
                    for( size_t j=0, i=l.s; i<l.s+new_dist+1; ++i, ++j ) _doc[i] = buffer[j] ;
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
                return _tokens[ _lines[lidx].first_token + tidx ] ;
            }

            std::string_view make_line_view( this_t::line const & l ) const noexcept
            {
                return std::string_view( _doc + l.s, l.dist() ) ;
            }

            std::string_view make_token( this_t::token const & t ) const noexcept
            {
                return std::string_view( _doc + t.s, t.dist() ) ;
            }

        private: // size issues

            void_t resize( size_t const num_elems ) noexcept
            {
                size_t const new_size = num_elems * sizeof( char ) + 1 ;
                size_t const actual = std::min( _size, new_size ) ;
                char_ptr_t tmp = motor::memory::global::alloc_raw<char_t>( new_size ) ;
                std::memcpy( tmp, _doc, actual ) ;
                motor::memory::global::dealloc_raw( _doc ) ;
                _doc = tmp ;
                _size = actual ;
                if ( _cur_pos >= _size ) _cur_pos = _size - 1 ;
            }

            this_ref_t ensure_fit( size_t const num_elems ) noexcept
            {
                if ( this_t::reserved() <= _cur_pos + num_elems + 1 )
                {
                    this_t::resize( _cur_pos + num_elems + _grow_by ) ;
                }
                return *this ;
            }

            size_t reserved( void_t ) const noexcept
            {
                return _size ;
            }
        };
    }
}