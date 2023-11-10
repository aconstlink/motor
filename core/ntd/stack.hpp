#pragma once

#include "typedefs.h"
#include "vector.hpp"

#include <functional>

namespace natus
{
    namespace ntd
    {
        /// not thread safe stack
        template< typename O, size_t pre_alloc >
        class stack
        {
            typedef natus::ntd::stack< O, pre_alloc > __stack_t ;
            natus_this_typedefs( __stack_t ) ;


            natus_typedefs( O, object ) ;
            typedef natus::ntd::vector< object_t > stack_t ;

            typedef ::std::function< void_t( object_cref_t, bool_t const, bool_t const ) > for_each_funk_t ;

        private:

            natus::ntd::vector< object_t > _stack ;

            /// one above top of stack
            /// top of stack is last pushed
            size_t _cur_pos = 0 ;

        private:

            /// checks the size of the stack and resizes
            /// by pre_alloc
            void_t check_and_resize( void_t )
            {
                if( _cur_pos < _stack.size() ) return ;
                _stack.resize( _stack.size() + pre_alloc ) ;
            }

        public:

            stack( void_t )
            {
                _stack.resize( pre_alloc ) ;
            }

            stack( this_cref_t rhv )
            {
                *this = rhv ;
            }

            stack( this_rref_t rhv )
            {
                *this = ::std::move( rhv ) ;
            }

            this_ref_t operator = ( this_cref_t rhv )
            {
                _cur_pos = rhv._cur_pos ;
                _stack = rhv._stack ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv )
            {
                _cur_pos = rhv._cur_pos ;
                rhv._cur_pos = 0 ;
                _stack = ::std::move( rhv._stack ) ;
                return *this ;
            }

        public:

            void_t push( object_cref_t rhv )
            {
                check_and_resize() ;
                _stack[ _cur_pos ] = rhv ;
                ++_cur_pos ;
            }

            object_t pop( void_t )
            {
                natus_assert( _cur_pos > 0 ) ;
                --_cur_pos ;
                return ::std::move( _stack[ _cur_pos ] ) ;
            }

            void_t pop( object_ref_t obj_out )
            {
                natus_assert( _cur_pos > 0 ) ;
                --_cur_pos ;
                obj_out = ::std::move( _stack[ _cur_pos ] ) ;
            }

            size_t size( void_t ) const
            {
                return _cur_pos ;
            }

            bool_t has_item( void_t ) const
            {
                return this_t::size() != 0 ;
            }

            object_cref_t top( void_t ) const
            {
                natus_assert( _cur_pos != 0 ) ;
                return _stack.at( _cur_pos - 1 ) ;
            }

            bool_t top( object_ref_t out_ ) const
            {
                if( _cur_pos == 0 ) return false ;
                out_ = top() ;
                return true ;
            }

            object_cref_t operator [] ( size_t const i ) const noexcept
            {
                natus_assert( _cur_pos >= i ) ;
                return _stack[ i ] ;
            }

        public:

            void_t bottom_to_top( for_each_funk_t funk )
            {
                if( _cur_pos == 0 ) return ;

                for( size_t i = 0; i < _cur_pos; ++i )
                {
                    funk( _stack[ i ], i == 0, i == _cur_pos - 1 ) ;
                }
            }

            void_t top_to_bottom( for_each_funk_t funk )
            {
                if( _cur_pos == 0 ) return ;

                for( size_t i = _cur_pos - 1; i != size_t( -1 ); --i )
                {
                    funk( _stack[ i ], i == 0, i == _cur_pos - 1 ) ;
                }
            }
        };
    }
}