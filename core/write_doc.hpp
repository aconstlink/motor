

#pragma once

#include <motor/std/string>
#include <motor/std/vector>

#include <motor/memory/global.h>

namespace motor
{
    namespace core
    {
        // non-thread safe write document
        // for helping writing text based files
        // similar to code.
        class write_doc
        {
            motor_this_typedefs( write_doc ) ;

        private:

            char * _doc = nullptr ;
            size_t _indent = 0 ;
            size_t _cur_pos = 0 ;

            size_t _grow_by = 500 ;
            size_t _size = 1000 ;

        public:

            write_doc( void_t ) noexcept 
            {
                _doc = motor::memory::global::alloc_raw<char_t>( _size * sizeof( char ) ) ;
                _doc[0] = '\0' ;
            }

            write_doc( size_t const size ) noexcept : _size( size < _size ? _size : size )
            {
                _doc = motor::memory::global::alloc_raw<char_t>( size * sizeof( char ) ) ;
                _doc[0] = '\0' ;
            }

            write_doc( this_rref_t rhv ) noexcept
            {
                motor::memory::global::dealloc( _doc ) ;
                _doc = motor::move( rhv._doc ) ;
                _indent = rhv._indent ;
                _size = rhv._size ;
                _cur_pos = rhv._cur_pos ;
                _grow_by = rhv._grow_by ;

                std::memset( (void_ptr_t)&rhv, 0, sizeof( this_t ) ) ;
            }

            ~write_doc( void_t ) noexcept
            {
                motor::memory::global::dealloc_raw( _doc ) ;
            }

            this_ref_t section_open( char const ch ) noexcept
            {
                this_t::println( &ch, 1 ) ;
                ++_indent ;
                return *this ;
            }

            this_ref_t section_close( char const ch ) noexcept
            {
                assert( _indent > 0 ) ;
                --_indent ;
                return this_t::println( &ch, 1 ) ;
            }

            // print everything until '\0'
            this_ref_t println( char const * text, size_t num_chars = size_t(-1) ) noexcept
            {
                if( num_chars == size_t (-1) ) while( text[++num_chars] != '\0' ) ;

                this_t::ensure_fit( _indent + num_chars + 1 ) ;

                this_t::print_indent() ;

                std::memcpy( _doc + _cur_pos, text, num_chars ) ;


                _cur_pos += num_chars ;
                _doc[_cur_pos] = '\n' ;
                _doc[++_cur_pos] = '\0' ;

                return *this ;
            }

            this_ref_t println( std::string_view const & sv ) noexcept
            {
                return this_t::println( sv.data(), sv.size() ) ;
            }
            
            // every character, including escape sequences
            size_t size( void_t ) const noexcept
            {
                return _cur_pos ;
            }

            size_t reserved( void_t ) const noexcept
            {
                return _size ;
            }

            void_t print_to_std( void_t ) const noexcept
            {
                std::cout << _doc ;
            }

        private:

            void_t resize( size_t const num_elems ) noexcept
            {
                size_t const new_size = num_elems * sizeof( char ) + 1 ;
                size_t const actual = std::min( _size, new_size ) ;
                char_ptr_t tmp = motor::memory::global::alloc_raw<char_t>( new_size ) ;
                std::memcpy( tmp, _doc, actual ) ;
                motor::memory::global::dealloc_raw( _doc ) ;
                _doc = tmp ;
                _size = actual ;
                if( _cur_pos >= _size ) _cur_pos = _size-1 ;
            }

            this_ref_t ensure_fit( size_t const num_elems ) noexcept
            {
                if( this_t::reserved() <= _cur_pos + num_elems + 1 )
                {
                    this_t::resize( _cur_pos + num_elems + _grow_by ) ;
                }
                return *this ;
            }

            void_t print_indent( void_t ) noexcept
            {
                for( size_t i=0; i<_indent; ++i )
                {
                    _doc[_cur_pos++] = '\t' ;
                }
            }
        };
    }
}