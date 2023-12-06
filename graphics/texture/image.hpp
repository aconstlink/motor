

#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../types.h"

#include <motor/math/vector/vector3.hpp>
#include <motor/memory/global.h>
#include <motor/log/global.h>
#include <cstring>

namespace motor
{
    namespace graphics
    {
        // default: 2d rgba 8 bit/channel image
        class MOTOR_GRAPHICS_API image
        {
            motor_this_typedefs( image ) ;

        public:

            motor_typedefs( motor::math::vector3< size_t >, dims ) ;

        private:

            motor::graphics::image_format _if = motor::graphics::image_format::rgba ;
            motor::graphics::image_element_type _iet = motor::graphics::image_element_type::uint8 ;
            
            size_t _width = 0 ;
            size_t _height = 0 ;
            size_t _depth = 0 ;

            void_ptr_t _data = nullptr ;

        public:

            image( void_t ){}

            image( this_t::dims_in_t dims )
            {
                this_t::resize( dims ) ;
            }

            image( motor::graphics::image_format const imf, motor::graphics::image_element_type iet )
                : _if( imf ), _iet( iet ){}

            image( motor::graphics::image_format const imf, motor::graphics::image_element_type iet,
                this_t::dims_in_t dims )
                : _if( imf ), _iet( iet ) 
            {
                this_t::resize( dims ) ;
            }

            image( this_cref_t rhv ) 
            {
                *this = rhv ;
            }
            image( this_rref_t rhv )
            {
                *this = std::move( rhv ) ;
            }
            ~image( void_t ) 
            {
                motor::memory::global_t::dealloc( _data ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _if = rhv._if ;
                _iet = rhv._iet ;
                _width = rhv._width ;
                _height = rhv._height ;
                _depth = rhv._depth ;

                this_t::resize( _width, _height, _depth ) ;
                std::memcpy( _data, rhv._data, this_t::sib() ) ;

                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _if = rhv._if ;
                _iet = rhv._iet ;
                _width = rhv._width ;
                _height = rhv._height ;
                _depth = rhv._depth ;

                motor::memory::global_t::dealloc( _data ) ;
                motor_move_member_ptr( _data, rhv ) ;

                return *this ;
            }


        public:

            size_t sib( void_t ) const noexcept 
            {
                return motor::graphics::size_of( _iet )* motor::graphics::size_of( _if )*
                    _width* _height* _depth ;
            }

            this_ref_t resize( this_t::dims_in_t dims ) noexcept
            {
                return this_t::resize( dims.x(), dims.y(), dims.z() ) ;
            }

            this_ref_t resize( size_t const w, size_t const h ) noexcept
            {
                return this_t::resize( w, h, 0 ) ;
            }

            this_ref_t resize( size_t const w, size_t const h, size_t const d ) noexcept
            {
                _width = w ;
                _height = h ;
                _depth = d ;

                size_t const sib = this_t::sib() ;

                _data = motor::memory::global_t::alloc( sib, 
                    "[graphics::image::resize] : image" ) ;

                return *this ;
            }

            this_ref_t clear_data( void_t ) noexcept
            {
                _width = 0 ;
                _height = 0 ;
                _depth = 0 ;

                motor::memory::global_t::dealloc( _data ) ;
                _data = nullptr ;

                return *this ;
            }

            dims_t get_dims( void_t ) const noexcept
            {
                return motor::math::vector3< size_t >( _width, _height, _depth ) ;
            }
            
            motor::graphics::image_format get_image_format( void_t ) const noexcept 
            {
                return _if ;
            }

            motor::graphics::image_element_type get_image_element_type( void_t ) const noexcept 
            {
                return _iet ;
            }

            void_cptr_t get_image_ptr( void_t ) const noexcept { return _data ; }
            void_cptr_t get_image_ptr( size_t const z ) const noexcept 
            { 
                if( z >= _depth ) return nullptr ;

                size_t const esib = motor::graphics::size_of( _iet ) * motor::graphics::size_of(_if ) ;

                return reinterpret_cast< void_ptr_t >( size_t(_data) + _width * _height * z * esib ) ; 
            }

        public:

            typedef std::function< void_t ( this_ptr_t, dims_in_t, void_ptr_t ) > update_funk_t ;
            this_ref_t update( update_funk_t funk ) noexcept
            {
                funk( this, this_t::get_dims(), _data ) ;
                return *this ;
            }

            /// this function allows to add another image to this image
            /// The resulting width and height is the maximum of all added images.
            /// each time an image is added, the depth is incremented.
            this_ref_t append( this_cref_t other ) noexcept
            {
                if( _iet != other.get_image_element_type() ||
                    _if != other.get_image_format() )
                {
                    motor::log::global_t::error("[image] : can not append image. "
                        "element type or image format mismatch." ) ;
                    return *this ;
                }

                size_t const esib = motor::graphics::size_of( _iet ) * motor::graphics::size_of(_if ) ;

                auto const odims = other.get_dims() ;

                size_t const w = std::max( _width, odims.x() ) ;
                size_t const h = std::max( _height, odims.y() ) ;
                size_t const d = std::max( _depth+1, odims.z() ) ;

                this_t tmp_img( _if, _iet, this_t::dims_t( w, h, d ) ) ;

                tmp_img.update( [&]( this_ptr_t, dims_in_t /*dims*/, void_ptr_t data ) 
                {
                    size_t base = 0 ;
                    for( size_t z = 0; z<_depth; ++z )
                    {
                        for( size_t y=0; y<_height; ++y )
                        {
                            for( size_t x=0; x<_width; ++x )
                            {
                                size_t const widx = base + x ;
                                size_t const ridx = z * _width * _height + y * _width + x ;

                                std::memcpy( 
                                    reinterpret_cast< void_ptr_t >( size_t(data)+widx*esib), 
                                    reinterpret_cast< void_cptr_t >( size_t(_data)+ridx*esib), 
                                    motor::graphics::size_of( _iet ) * motor::graphics::size_of(_if ) ) ;
                            }
                            base += w ;
                        }
                        base = w * h * (z+1) ;
                    }

                    for( size_t z = 0; z < odims.z() ; ++z )
                    {
                        for( size_t y=0; y < odims.y() ; ++y )
                        {
                            for( size_t x=0; x < odims.x() ; ++x )
                            {
                                size_t const widx = base + x ;
                                size_t const ridx = z * odims.x() * odims.y() + y * odims.x() + x ;

                                void_cptr_t odata = other.get_image_ptr() ;
                                std::memcpy( 
                                    reinterpret_cast< void_ptr_t >( size_t(data)+widx*esib), 
                                    reinterpret_cast< void_cptr_t >( size_t(odata)+ridx*esib), 
                                    motor::graphics::size_of( _iet ) * motor::graphics::size_of(_if ) ) ;
                            }
                            base += w ;
                        }
                    }
                    
                } ) ;

                *this = std::move( tmp_img ) ;
                return *this ;
            }
        };
        motor_typedef( image ) ;
    }
}
