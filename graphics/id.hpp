#pragma once

#include "api.h"
#include "typedefs.h"
#include "result.h"
#include "backend/types.h"

// memset
#include <cstring>

namespace motor
{
    namespace graphics
    {       
        class id
        {
            motor_this_typedefs( id ) ;

            // backend id
            //size_t _bid = size_t( -1 ) ;

            // object id 
            size_t _oids[motor::graphics::max_backends] ;

        public:

            id( void_t ) noexcept
            {
                for( size_t i =0; i<motor::graphics::max_backends; ++i )
                {
                    _oids[ i ] = size_t( -1 ) ;
                }
            }

            id( size_t const bid, size_t const oid ) noexcept: id()
            {
                _oids[ bid ] = oid ;
            }
            
            id( this_cref_t ) = delete ;

            id( this_rref_t rhv ) noexcept
            { 
                //_bid = rhv._bid ;
                //rhv._bid = size_t( -1 ) ;

                std::memcpy( (void_ptr_t)&_oids, (void_ptr_t)&rhv._oids, sizeof( size_t ) * motor::graphics::max_backends ) ;
                std::memset(&rhv._oids, (int_t(-1)), sizeof(size_t)*motor::graphics::max_backends ) ;
            }

            virtual ~id( void_t ) noexcept
            {
                // stuff must be cleaned up!
                //motor_assert( _bid == size_t( -1 ) ) ;

                for( auto & oid : _oids )
                {
                    assert( oid == size_t( -1 ) ) ;
                }
            }

            bool_t is_valid( size_t const bid ) const noexcept
            {
                return _oids[ bid ] != size_t( -1 ) ;
            }

            bool_t is_not_valid( size_t const bid ) const noexcept
            {
                return !this_t::is_valid( bid ) ;
            }


            size_t get_oid( size_t const bid ) const noexcept { return _oids[ bid ] ; }
            void_t set_oid( size_t const bid, size_t const oid ) noexcept
            {
                _oids[ bid ] = oid ;
            }

        public:

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                std::memcpy( ( void_ptr_t ) &_oids, ( void_ptr_t ) &rhv._oids, sizeof( size_t ) * motor::graphics::max_backends ) ;
                std::memset( &rhv._oids, ( int_t( -1 ) ), sizeof( size_t ) * motor::graphics::max_backends ) ;

                return *this ;
            }

            this_ref_t operator = ( this_cref_t ) = delete ;

        };
        motor_typedef( id ) ;
    }
}
