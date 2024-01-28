
#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
    namespace audio
    {
        class object
        {
            motor_this_typedefs( object ) ;

        private:

            // backends can store an id for their own.
            typedef struct
            {
                size_t bid ;
                size_t oid ;
            } id_t ;

            id_t _id ;

        public:

            object( void_t ) noexcept {}
            object( this_cref_t rhv ) noexcept : _id( rhv._id ) {}
            object( this_rref_t rhv ) noexcept : _id( rhv._id )
            { 
                rhv._id = {size_t(-1), size_t(-1) } ;
            }
            virtual ~object( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _id = rhv._id ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                rhv._id = {size_t(-1), size_t(-1) } ;
                return *this ;
            }

        public:

            size_t set_oid( size_t const bid, size_t const oid ) noexcept
            {
                if( _id.bid != bid ) return size_t(-1) ;

                _id.bid = bid ;
                _id.oid = oid ;
                return _id.oid ;
            }

            size_t get_oid( size_t const bid ) const noexcept 
            {
                if( _id.bid != bid ) return size_t(-1) ;
                return _id.oid ; 
            }
            
        };
    }
}
