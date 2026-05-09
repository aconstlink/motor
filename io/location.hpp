#pragma once

#include "typedefs.h"

namespace motor
{
    namespace io
    {
        // resource location
        // valid: loc1.loc2.loc3
        // invalid: loc1.loc.loc3. <- invalid trailing .
        class location
        {
            motor_this_typedefs( location ) ;

        private:

            motor::string_t _loc ;

        public:

            location( void_t ) noexcept
            {}

            location( motor::string_cref_t loc ) noexcept : _loc( loc )
            {}

            location( this_cref_t rhv ) noexcept : _loc( rhv._loc )
            {}

            location( this_rref_t rhv ) noexcept : _loc( std::move( rhv._loc ) )
            {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _loc = rhv._loc ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept 
            {
                _loc = std::move( rhv._loc ) ;
                return *this ;
            }

            bool_t operator == ( this_cref_t other ) const noexcept
            {
                return _loc == other._loc ;
            }

            operator motor::string_t( void_t ) noexcept
            {
                return _loc ;
            }

            operator motor::io::path_t( void_t ) noexcept
            {
                return this_t::as_path() ;
            }

            // returns the number of separated parts in
            // the location
            // e.g. loc1.loc2.loc3 -> 3
            size_t num_parts( void_t ) const
            {
                size_t ret = 0 ;

                size_t off = _loc.find_first_of('.') ;
                while( off != std::string::npos )
                {
                    ++ret ;
                    off = _loc.find_first_of('.', off + 1) ;
                }

                return ++ret ;
            }

            // returns the sub location from the beginning.
            // if np__ < 0 the sub location will be taken from the end.
            // @return "loc1.loc2.loc3".sub_location( 2 ) -> "loc1.loc2"
            // @return "loc1.loc2.loc3".sub_location( -2 ) -> "loc1"
            this_t sub_location( int_t const np__ ) const 
            {
                size_t np = this_t::num_parts() ;
                if( np__ < 0 && (std::abs(np__)<=np) ) np += np__ ;
                else
                {
                    np = np__ ;
                    if( this_t::num_parts() <= np ) 
                        return _loc ;
                }

                size_t count = 0 ;
                motor::string_t ret ;

                size_t off = _loc.find_first_of('.') ;
                size_t p0 = 0 ;
                while( off != std::string::npos && (count++<np)  )
                {
                    ret += _loc.substr( p0, (off+1)-p0 ) ;
                    p0 = off + 1 ;
                    off = _loc.find_first_of('.', p0 ) ;
                }
                ret = ret.substr( 0, ret.size() - 1 ) ;
                return ret ;
            }

            this_t operator + ( this_cref_t loc ) const
            {
                return _loc + "." + loc.as_string() ;
            }
            
            motor::io::path_t as_path( void_t ) const noexcept
            {
                motor::io::path_t pout = "." ;
                
                size_t off = _loc.find_first_of( '.', 0 ) ;
                size_t last = size_t(0) ;
                while( off != std::string::npos )
                {
                    pout /= _loc.substr( last, off - last ) ;

                    last = off +1 ;
                    off = _loc.find_first_of( '.', last ) ;
                }
                pout += "." + _loc.substr( last, _loc.size() - 1 ) ;

                return std::move( pout ) ;
            }

            motor::io::path_t parent_path( void_t ) const noexcept
            {
                return this_t::as_path().parent_path() ;
            }

            motor::string_cref_t as_string( void_t ) const noexcept
            {
                return _loc ;
            }

            motor::string_t extension( bool_t const dotted = true ) const noexcept
            {
                motor::string_t ext = motor::from_std( this_t::as_path().extension().string() ) ;
                if( !dotted )
                {
                    ext = ext.substr( 1, ext.size() - 1 ) ;
                }
                return ext ;
            }

            static this_t from_path( motor::io::path_cref_t p ) noexcept
            {
                motor::string_t loc ;

                for( auto p_ : p.lexically_normal() )
                {
                    loc += motor::from_std( p_.string() ) + "." ;
                }
                loc = loc.substr( 0, loc.size() - 1 ) ;

                return this_t( loc ) ;
            }
        };
        motor_typedef( location ) ;
    }
}