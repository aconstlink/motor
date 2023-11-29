#pragma once

#include "typedefs.h"
#include <motor/ntd/vector.hpp>
#include <motor/ntd/string/split.hpp>
#include <motor/concurrent/mrsw.hpp>

namespace motor
{
    namespace msl
    {
        class symbol_table
        {
            motor_this_typedefs( symbol_table ) ;

        public:

            class symbol
            {
                motor_this_typedefs( symbol ) ;

            private:

                // the namespace like liba.libb.libc
                motor::vector< motor::string_t > _spaces ;

                // the symbols' name like xyz
                motor::string_t _name ;

                // shader version
                motor::string_t _version ;

            public:
                
                symbol( void_t ) {}
                symbol( this_cref_t rhv ) 
                {
                    _spaces = rhv._spaces ;
                    _name = rhv._name ;
                    _version = rhv._version ;
                }

                symbol( this_rref_t rhv ) 
                {
                    _spaces = std::move( rhv._spaces ) ;
                    _name = std::move( rhv._name ) ;
                    _version = std::move( rhv._version ) ;
                }

                // @param name something like liba.libb.libc.name
                // @param version something like msl, gl3, es3, hlsl9...
                symbol( motor::string_cref_t name, motor::string_cref_t /*version*/ ) noexcept
                {
                    size_t off = 0 ;
                    size_t n = name.find_first_of( '.' ) ;
                    while( n != std::string::npos )
                    {
                        _spaces.emplace_back( name.substr( off, n - off ) ) ;
                        off += n + 1 ;
                        n = name.find_first_of( '.' ) ;
                    }
                }

                this_ref_t operator = ( this_cref_t ) noexcept
                {
                    return *this ;
                }

                this_ref_t operator = ( this_rref_t ) noexcept
                {
                    return *this ;
                }

            public:

                bool_t operator == ( this_cref_t s ) const noexcept
                {
                    return s.full_symbol_name() == this_t::full_symbol_name() ;
                }

                bool_t operator == ( motor::string_cref_t s ) const noexcept
                {
                    return s == this_t::namespace_name() ;
                }

            public:

                // only returns the namespace name in dot notation like
                // liba.libb.libc
                motor::string_t namespace_name( void_t ) const noexcept
                {
                    motor::string_t ret ;
                    for( auto const & s : _spaces )
                    {
                        ret += s + "." ;
                    }
                    ret = ret.substr( 0, ret.size() - 1 ) ;

                    return std::move( ret ) ;
                }

                // returns the full qualified name like
                // liba.libb.libc.name
                motor::string_t symbol_name( void_t ) const noexcept
                {
                    motor::string_t ret ;
                    ret = this_t::namespace_name() + "." + _name ;
                    return std::move( ret ) ;
                }

                // returns the full qualified name like
                // liba.libb.libc.name
                motor::string_t full_symbol_name( void_t ) const noexcept
                {
                    return this_t::symbol_name() + "." + _version ;
                }
            };
            motor_typedef( symbol ) ;

            typedef motor::vector< symbol_t > symbols_t ;
            symbols_t _syms ;

            mutable motor::concurrent::mrsw_t _ac ;

        public:

            symbol_table( void_t ) noexcept
            {}

            symbol_table( this_cref_t ) noexcept
            {}

            symbol_table( this_rref_t ) noexcept
            {}

            ~symbol_table( void_t ) noexcept
            {}

            this_ref_t operator = ( this_cref_t ) noexcept
            {
                return *this ;
            }

            this_ref_t operator = ( this_rref_t ) noexcept
            {
                return *this ;
            }

        public:

            this_ref_t insert( this_t::symbol_cref_t s ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;

                auto iter = std::find_if( _syms.begin(), _syms.end(), [&] ( this_t::symbol_cref_t ls ) 
                { 
                    return ls == s ;
                } ) ;

                if( iter == _syms.end() )
                {
                    _syms.emplace_back( s ) ;
                }

                return *this ;
            }

            bool_t find( motor::string_cref_t, this_t::symbol_out_t ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;

                return false;
            }

            typedef std::function< bool_t ( this_t::symbol_cref_t ) > find_funk_t ;
            bool_t find_if( find_funk_t funk ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;

                for( auto const & s : _syms )
                {
                    if( funk( s ) ) return true ;
                }

                return false;
            }
        };
        motor_typedef( symbol_table ) ;
    }
}