
#pragma once

#include <motor/concurrent/mrsw.hpp>
#include <motor/std/vector>

namespace motor
{
    namespace core
    {
        #define motor_core_dd_id_vfn() \
        public:\
            virtual size_t id( void_t ) const noexcept = 0 ; \
        private:

        #define motor_core_dd_id_fn()                               \
        public:                                     \
            virtual size_t id( void_t ) const noexcept { return sid() ; }                     \
            static size_t sid( void_t ) noexcept { return __dd_id__ ; } \
            static void_t assign_id( size_t const id_ ) noexcept { __dd_id__ = id_ ; }    \
        private:                                    \
            static size_t __dd_id__ 

        #define motor_core_dd_id_init( class_name ) size_t class_name::__dd_id__ = size_t(-1)

        // double dispatches like this:
        //
        // icaller->some_funk( icallee ) 
        //
        // will translate into this:
        //
        // caller->some_funk( callee )
        template< class icaller_t, class icallee_t, typename funk_set_t >
        class double_dispatcher
        {
            using this_t_ = double_dispatcher<icaller_t, icallee_t, funk_set_t> ;
            motor_this_typedefs( this_t_ ) ;

        private:

            std::mutex _mtx_caller_id ;
            std::mutex _mtx_callee_id ;
            
            mutable motor::concurrent::mrsw_t _mtx_table ;

            size_t _caller_id = size_t( -1 ) ;
            size_t _callee_id = size_t( -1 ) ;
            
            motor::vector< motor::vector< funk_set_t > > _table ;

        private:

            size_t gen_caller_id( void_t ) noexcept
            {
                std::lock_guard< std::mutex > lk( _mtx_caller_id ) ;
                return ++ _caller_id ;
            }

            size_t gen_callee_id( void_t ) noexcept
            {
                std::lock_guard< std::mutex > lk( _mtx_caller_id ) ;
                return ++_callee_id ;
            }

        public:

            template< class caller_t, class callee_t >
            void register_funk_set( funk_set_t fs ) noexcept
            {
                // #1 : first check ids
                {
                    if ( caller_t::sid() == size_t( -1 ) )
                    {
                        caller_t::assign_id( this_t::gen_caller_id() ) ;
                    }

                    if ( callee_t::sid() == size_t( -1 ) )
                    {
                        callee_t::assign_id( this_t::gen_callee_id() ) ;
                    }
                }

                // #2 : check table size
                {
                    motor::concurrent::mrsw_t::writer_lock lk( _mtx_table ) ;
                    if ( caller_t::sid() >= _table.size() )
                    {
                        _table.resize( caller_t::sid() + 1 ) ;
                    }

                    if( callee_t::sid() >= _table[ caller_t::sid() ].size() )
                    {
                        _table[ caller_t::sid() ].resize( callee_t::sid() + 1 ) ;
                    }
                }

                // #3 : place function
                {
                    motor::concurrent::mrsw_t::reader_lock_t lk( _mtx_table ) ;
                    _table[ caller_t::sid() ][ callee_t::sid() ] = fs ;
                }
            }

            funk_set_t resolve( icaller_t * c1, icallee_t * c2 ) const noexcept
            {
                size_t const caller_id = c1->id() ;
                size_t const callee_id = c2->id() ;

                if( caller_id == size_t(-1) || callee_id == size_t( -1 ) )
                {
                    return funk_set_t() ;
                }

                // at this point, table must have enough entries because caller 
                // must have been registered with any callee
                assert( caller_id < _table.size() ) ;

                // ... but maybe another caller registered the callee, so the 
                // callee entry could not exist.
                
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx_table ) ;

                if( callee_id >= _table[caller_id].size() )
                {
                    return funk_set_t() ;
                }

                // if here, the caller will call the callee
                return _table[ caller_id ][ callee_id ] ;
            }
        };
    }
}