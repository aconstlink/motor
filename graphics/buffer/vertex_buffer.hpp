
#pragma once

#include "../types.h"
#include "vertex_attribute.h"

#include <motor/std/vector>

#include <cstring>
#include <algorithm>

namespace motor
{
    namespace graphics
    {
        struct vertex_buffer
        {
            motor_this_typedefs( vertex_buffer ) ;

        public:

            struct data
            {
                motor::graphics::vertex_attribute va ;
                motor::graphics::type type ;
                motor::graphics::type_struct type_struct ;
            };
            motor_typedef( data ) ;
            motor_typedefs( motor::vector< data_t >, datas ) ;

        private:

            datas_t _vertex_layout ;

            size_t _num_elems = 0 ;
            void_ptr_t _data = nullptr ;

            motor::graphics::usage_type _ut = usage_type::buffer_static ;

        public:

            vertex_buffer( void_t ) noexcept {}
            vertex_buffer( motor::graphics::usage_type const ut ) noexcept : _ut(ut){}
            vertex_buffer( this_cref_t rhv ) noexcept 
            {
                *this = rhv ;
            }

            vertex_buffer( this_rref_t rhv ) noexcept
            {
                *this = std::move( rhv ) ;
            }

            ~vertex_buffer( void_t ) noexcept
            {
                motor::memory::global_t::dealloc( _data ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept 
            {
                _vertex_layout = rhv._vertex_layout ;
                this_t::resize( rhv._num_elems ) ;
                std::memcpy( _data, rhv._data, rhv.get_sib() ) ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _vertex_layout = std::move( rhv._vertex_layout ) ;
                _num_elems = rhv._num_elems ;
                motor::memory::global_t::dealloc( _data ) ;
                motor_move_member_ptr( _data, rhv ) ;
                return *this ;
            }

        public:

            this_ref_t resize( size_t const ne, double_t const thres = 0.5 ) noexcept
            {
                if( ne == _num_elems ) return *this ;

                // check realloc when getting smaller
                // do not realloc if larger than thes
                {
                    auto const c = double_t(ne) / double_t(_num_elems) ;
                    if( c < 1.0f && c > thres ) return *this ;
                }
                
                _num_elems = ne ;
                size_t const sib = this_t::get_sib() ;
                motor::memory::global_t::dealloc( _data ) ;
                _data = motor::memory::global_t::alloc( sib, "vertex buffer" ) ;

                return *this ;
            }

            
            template< typename vertex_t >
            this_ref_t update( std::function< void_t ( vertex_t * array, size_t const ne ) > funk ) noexcept
            {
                funk( static_cast< vertex_t* >( _data ), _num_elems ) ;
                return *this ;
            }

            // array starts at start
            template< typename vertex_t >
            this_ref_t update( size_t start, size_t end, 
                std::function< void_t ( vertex_t * array, size_t const ne ) > funk ) noexcept
            {
                start = std::min( start, end ) ;
                end = std::max( start, end ) ;
                start = std::min( _num_elems, start ) ;
                end = std::min( _num_elems, end ) ;
                size_t const ne = end - start ;
                funk( reinterpret_cast< vertex_t* >( _data ) + start, ne ) ;
                return *this ;
            }

            void_cptr_t data( void_t ) const noexcept { return _data ; }

            this_ref_t add_layout_element( motor::graphics::vertex_attribute const va, 
                motor::graphics::type const t, motor::graphics::type_struct const ts ) noexcept
            {
                this_t::data_t d ;
                d.va = va ;
                d.type = t ;
                d.type_struct = ts ;

                _vertex_layout.push_back( d ) ;

                return *this ;
            }
            
            void_t for_each_layout_element( std::function< void_t ( data_cref_t d ) > funk ) const noexcept
            {
                for( auto const & le : _vertex_layout )
                {
                    funk( le ) ;
                }
            }

            void_t for_each_layout_element( std::function< void_t ( data_ref_t d ) > funk ) noexcept
            {
                for( auto & le : _vertex_layout )
                {
                    funk( le ) ;
                }
            }

            data_t get_layout_element_zero( void_t ) const noexcept
            {
                if( _vertex_layout.size() == 0 ) return data_t() ;
                return _vertex_layout[0] ;
            }

            size_t get_layout_element_sib( motor::graphics::vertex_attribute const va ) const noexcept
            {
                auto iter = std::find_if( _vertex_layout.begin(), _vertex_layout.end(), 
                    [&]( data_cref_t d )
                {
                    return d.va == va ;
                } ) ;

                if( iter == _vertex_layout.end() ) return 0 ;

                size_t const sib = motor::graphics::size_of( iter->type ) *
                    motor::graphics::size_of( iter->type_struct ) ;

                return sib ;
            }

            size_t get_layout_sib( void_t ) const noexcept
            {
                size_t sib = 0 ;
                for( auto const& d : _vertex_layout )
                {
                    sib += motor::graphics::size_of( d.type ) *
                        motor::graphics::size_of( d.type_struct ) ;
                }
                return sib ;
            }

            size_t get_sib( void_t ) const noexcept 
            {
                return this_t::get_layout_sib() * _num_elems ;
            }

            void_t copy_done( void_t )
            {
                if( _ut == usage_type::buffer_static )
                    this_t::resize( 0 ) ;
            }

            size_t get_num_layout_elements( void_t ) const noexcept { return _vertex_layout.size() ; }

            size_t get_num_elements( void_t ) const noexcept { return _num_elems ; }
        };
        motor_typedef( vertex_buffer ) ;
    }
}
