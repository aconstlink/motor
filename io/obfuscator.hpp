
#pragma once

#include "typedefs.h"
#include <motor/memory/malloc_guard.hpp>

namespace motor
{
    namespace io
    {
        // carrier of the result of the en/decryption functions
        class obfuscated
        {
            motor_this_typedefs( obfuscated ) ;

        private:

            motor::memory::malloc_guard< char_t > _mem ;

        public:

            obfuscated( motor::memory::malloc_guard< char_t > && mem ) noexcept
            {
                _mem = std::move( mem ) ;
            }

            obfuscated( motor::string_cref_t s ) noexcept
            {
                _mem = motor::memory::malloc_guard< char_t >( s.c_str(), s.size() ) ;
            }

            obfuscated( char_cptr_t ptr, size_t const sib ) noexcept
            {
                _mem = motor::memory::malloc_guard< char_t >( ptr, sib ) ;
            }

            obfuscated( char_ptr_t ptr, size_t const sib ) noexcept
            {
                _mem = motor::memory::malloc_guard< char_t >( ptr, sib ) ;
            }

            obfuscated( this_cref_t ) = delete ;
            obfuscated( this_rref_t rhv ) noexcept 
            {
                _mem = std::move( rhv._mem ) ;
            }

            ~obfuscated( void_t ) noexcept {}

        public:

            size_t sib( void_t ) const noexcept { return _mem.size() ; }
            char_cptr_t ptr( void_t ) const noexcept { return _mem.get() ; }
            char_ptr_t move_ptr( void_t ) noexcept { return _mem.move_ptr() ;  }

        public:

            operator motor::string_t( void_t ) noexcept
            {
                return motor::string_t( this_t::ptr(), this_t::sib() ) ;
            }
                
        };
        motor_typedef( obfuscated ) ;

        // carrier of the en-decryption functions
        class obfuscator
        {
            motor_this_typedefs( obfuscator ) ;

        public:

            typedef std::function< motor::memory::malloc_guard< char_t > ( motor::memory::malloc_guard< char_t > && ) > encode_funk_t ;
            typedef std::function< motor::memory::malloc_guard< char_t > ( motor::memory::malloc_guard< char_t > && ) > decode_funk_t ;

        private:

            encode_funk_t _efunk ;
            decode_funk_t _dfunk ;

        public:

            obfuscator( void_t ) noexcept
            {
                _efunk = [] ( motor::memory::malloc_guard< char_t >&& mem ) { return std::move( mem ); } ;
                _dfunk = [] ( motor::memory::malloc_guard< char_t >&& mem ) { return std::move( mem ); } ;
            }

            obfuscator( this_t::encode_funk_t ef, this_t::decode_funk_t df ) noexcept
            {
                _efunk = ef ;
                _dfunk = df ;
            }

            obfuscator( this_cref_t rhv ) noexcept
            {
                _efunk = rhv._efunk ;
                _dfunk = rhv._dfunk ;
            }

            obfuscator( this_rref_t rhv ) noexcept
            {
                _efunk = std::move( rhv._efunk ) ;
                _dfunk = std::move( rhv._dfunk ) ;
            }
            ~obfuscator( void_t ) noexcept {}

        public:

            motor::io::obfuscated_t encode( motor::string_cref_t s ) noexcept
            {
                return this_t::encode( s.c_str(), s.size() ) ;
            }

            // const version does copy
            motor::io::obfuscated_t encode( char_cptr_t ptr, size_t const sib ) noexcept
            {
                return motor::io::obfuscated_t( _efunk( motor::memory::malloc_guard< char_t >( ptr, sib ) ) ) ;
            }

            // non const version does inplace
            motor::io::obfuscated_t encode( char_ptr_t ptr, size_t const sib ) noexcept
            {
                return motor::io::obfuscated_t( _efunk( motor::memory::malloc_guard< char_t >( ptr, sib ) ) ) ;
            }

            motor::io::obfuscated_t decode( motor::string_cref_t s ) noexcept
            {
                return this_t::decode( s.c_str(), s.size() ) ;
            }

            motor::io::obfuscated_t decode( char_cptr_t ptr, size_t const sib ) noexcept
            {
                return motor::io::obfuscated_t( _dfunk( motor::memory::malloc_guard< char_t >( ptr, sib ) ) ) ;
            }

            motor::io::obfuscated_t decode( char_ptr_t ptr, size_t const sib ) noexcept
            {
                // take over ptr !
                return motor::io::obfuscated_t( _dfunk( motor::memory::malloc_guard< char_t >( ptr, sib ) ) ) ;
            }

        private:

            this_ref_t endecode( this_t::encode_funk_t ef, this_t::decode_funk_t df ) noexcept
            {
                _efunk = ef ;
                _dfunk = df ;
                return *this ;
            }

        public:

            static this_t as_user_funk( this_rref_t other, this_t::encode_funk_t ef, this_t::decode_funk_t df )
            {
                return std::move( std::move( other ).endecode( ef, df ) ) ;
            }

            // null endecode
            static this_t variant_0( this_rref_t other = this_t() ) noexcept
            {
                return this_t::as_user_funk( std::move( other ),
                    [] ( motor::memory::malloc_guard< char_t >&& mem ) { return std::move( mem ); },
                    [] ( motor::memory::malloc_guard< char_t >&& mem ) { return std::move( mem ); } ) ;
            }

            // variant 1 endecode with some xoring.
            static this_t variant_1( this_rref_t other = this_t() ) noexcept
            {
                this_t::encode_funk_t ef = [] ( motor::memory::malloc_guard< char_t > && mem )
                {
                    static const char_t lut[ 8 ] = { 'a', 'f', 'c', 'd', '5', 'c', '\n', '5' } ;

                    for( size_t i = 0; i < mem.size(); ++i )
                    {
                        mem[ i ] = mem[ i ] ^ lut[ i % 8 ] ;
                    }

                    return std::move( mem );
                } ;

                this_t::decode_funk_t df = [] ( motor::memory::malloc_guard< char_t > && mem )
                {
                    static const char_t lut[ 8 ] = { 'a', 'f', 'c', 'd', '5', 'c', '\n', '5' } ;

                    for( size_t i = 0; i < mem.size(); ++i )
                    {
                        mem[ i ] = mem[ i ] ^ lut[ i % 8 ] ;
                    }

                    return std::move( mem );
                } ;

                return this_t::as_user_funk( std::move( other ), ef, df ) ;
            }
        };
        motor_typedef( obfuscator ) ;
    }
}