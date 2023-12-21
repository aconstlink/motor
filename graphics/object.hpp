
#pragma once

#include "api.h"
#include "protos.h"
#include "typedefs.h"

#include <motor/concurrent/mrsw.hpp>
#include <motor/std/vector>

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API object
        {
            motor_this_typedefs( object ) ;

        private:

            // backends can store an id for their own.
            typedef struct
            {
                size_t bid ;
                size_t oid ;
            } id_t ;

            mutable motor::concurrent::mrsw_t _mutex ;
            motor::vector< id_t > _ids ;

        public:

            object( void_t ) noexcept{}
            object( this_cref_t rhv ) noexcept : _ids( rhv._ids ) {}
            object( this_rref_t rhv ) noexcept : _ids( std::move( rhv._ids ) ){}
            ~object( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _ids = rhv._ids ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _ids = std::move( rhv._ids ) ;
                return *this ;
            }

        public:

            size_t set_oid( size_t const bid, size_t const oid ) noexcept
            {
                // update oid ...
                {
                    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex ) ;
                    for( auto & id : _ids ) 
                    {
                        if( id.bid == bid ) 
                        {
                            id.oid = oid ;
                            return oid ;
                        }
                    }
                }

                // ... or create new
                {
                    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex ) ;
                    _ids.emplace_back( id_t({bid, oid}) ) ;
                }

                return oid ;
            }

            size_t get_oid( size_t const bid ) const noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mutex ) ;
                for( auto & id : _ids ) 
                {
                    if( id.bid == bid ) 
                    {
                        return id.oid ;
                    }
                }
                return size_t(-1) ;
            }

            void_t remove_bid( size_t const bid ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mutex ) ;
                auto iter = std::find_if( _ids.begin(), _ids.end(), [&]( id_t const & d )
                {
                    return d.bid == bid ;
                } ) ;

                if( iter == _ids.end() ) return ;

                _ids.erase( iter ) ;
            }
        };
    }
}
