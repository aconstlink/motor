
#pragma once

#include "../types.h"
#include "vertex_attribute.h"

#include <motor/std/vector>
#include <cstring>

namespace motor
{
    namespace graphics
    {
        struct index_buffer
        {
            motor_this_typedefs( index_buffer ) ;

        private:

            motor::graphics::type _type ;
            size_t _num_elems = 0 ;
            void_ptr_t _data = nullptr ;

        public:

            index_buffer( void_t ) noexcept {}
            index_buffer( this_cref_t rhv ) noexcept
            {
                *this = rhv ;
            }

            index_buffer( this_rref_t rhv ) noexcept
            {
                *this = std::move( rhv ) ;
            }

            ~index_buffer( void_t ) noexcept
            {
                motor::memory::global_t::dealloc( _data ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _type = rhv._type ;
                this_t::resize( rhv._num_elems ) ;
                std::memcpy( _data, rhv._data, rhv.get_sib() ) ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _type = rhv._type ;
                _num_elems = rhv._num_elems ;
                motor_move_member_ptr( _data, rhv ) ;
                return *this ;
            }

        public:

            this_ref_t resize( size_t const ne, double_t const thres = 0.5 )
            {
                if( ne == _num_elems ) return *this ;

                // check realloc when getting smaller
                {
                    auto const c = double_t(ne) / double_t(_num_elems) ;
                    if( c < 1.0f && c > thres ) return *this ;
                }

                _num_elems = ne ;
                size_t const sib = this_t::get_sib() ;
                motor::memory::global_t::dealloc( _data ) ;
                _data = motor::memory::global_t::alloc( sib, "index buffer" ) ;
                return *this ;
            }

            void_cptr_t data( void_t ) const noexcept { return _data ; }

            template< typename index_t >
            this_ref_t update( std::function< void_t ( index_t* array, size_t const ne ) > funk )
            {
                funk( static_cast< index_t* >( _data ), _num_elems ) ;
                return *this ;
            }

            // array starts at start
            template< typename index_t >
            this_ref_t update( size_t start, size_t end, 
                std::function< void_t ( index_t * array, size_t const ne ) > funk )
            {
                start = std::min( start, end ) ;
                end = std::max( start, end ) ;
                start = std::min( _num_elems, start ) ;
                end = std::min( _num_elems, end ) ;
                size_t const ne = end - start ;
                funk( reinterpret_cast< index_t* >( _data ) + start, ne ) ;
                return *this ;
            }

            this_ref_t set_layout_element( motor::graphics::type const t ) noexcept
            {
                _type = t ;
                return *this ;
            }

            size_t get_element_sib( void_t ) const noexcept
            {
                return motor::graphics::size_of( _type ) ;
            }

            size_t get_sib( void_t ) const noexcept
            {
                return this_t::get_element_sib() * _num_elems ;
            }

            size_t get_num_elements( void_t ) const noexcept { return _num_elems ; }
        };
        motor_typedef( index_buffer ) ;
    }
}
