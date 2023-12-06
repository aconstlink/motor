#pragma once

#include "../api.h"
#include "../types.h"

#include <motor/std/vector>

#include <functional>

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API data_buffer
        {
            motor_this_typedefs( data_buffer ) ;

        public:
            
            struct layout_element
            {
                motor::graphics::type type ;
                motor::graphics::type_struct type_struct ;
            };
            motor_typedef( layout_element ) ;
            motor_typedefs( motor::vector< layout_element_t >, layout ) ;

        private: 

            layout_t _layout ;

            size_t _num_elems = 0 ;
            void_ptr_t _data = nullptr ;

        public:

            data_buffer( void_t ) noexcept 
            {
            }

            data_buffer( this_cref_t rhv ) noexcept
            {
                *this = rhv ;
            }

            data_buffer( this_rref_t rhv ) noexcept 
            {
                *this = std::move( rhv ) ;
            }

            ~data_buffer( void_t ) noexcept
            {
                motor::memory::global_t::dealloc( _data ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept 
            {
                _layout = rhv._layout ;
                this_t::resize( rhv._num_elems ) ;
                std::memcpy( _data, rhv._data, rhv.get_sib() ) ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _layout = std::move( rhv._layout ) ;
                _num_elems = rhv._num_elems ;
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

                size_t const old_sib = this_t::get_sib() ;

                _num_elems = ne ;
                size_t const sib = this_t::get_sib() ;
                
                void_ptr_t tmp = motor::memory::global_t::alloc( sib, "[data_buffer::resize] : data buffer" ) ;
                
                if( _data != nullptr )
                {
                    std::memcpy( tmp, _data, std::min( sib, old_sib ) ) ;
                    motor::memory::global_t::dealloc( _data ) ;
                }
                

                _data = tmp ;

                return *this ;
            }
            
            template< typename item_t >
            this_ref_t update( std::function< void_t ( item_t * array, size_t const ne ) > funk ) noexcept
            {
                funk( static_cast< item_t* >( _data ), _num_elems ) ;
                return *this ;
            }

            template< typename item_t >
            this_ref_t update( size_t const i, item_t const & item ) noexcept
            {
                static_cast< item_t* >( _data )[i] = item ;
                return *this ;
            }

            void_cptr_t data( void_t ) const noexcept { return _data ; }
            
            this_ref_t add_layout_element( motor::graphics::type const t, 
                motor::graphics::type_struct const ts ) noexcept
            {
                this_t::layout_element_t d ;
                d.type = t ;
                d.type_struct = ts ;

                _layout.push_back( d ) ;

                return *this ;
            }

            layout_element_t get_layout_element( size_t const n ) const noexcept
            {
                if( n > _layout.size() ) return layout_element_t() ;
                return _layout[n] ;
            }
            
            void_t for_each_layout_element( std::function< void_t ( layout_element_cref_t d ) > funk ) noexcept
            {
                for( auto & le : _layout )
                {
                    funk( le ) ;
                }
            }

            size_t get_layout_sib( void_t ) const noexcept
            {
                size_t sib = 0 ;
                for( auto const& d : _layout )
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

            size_t get_num_elements( void_t ) const noexcept { return _num_elems ; }

        };
        motor_typedef( data_buffer ) ;
    }
}