#pragma once

#include "../object.hpp"

#include "../texture/image.hpp"

#include <motor/std/vector>

#include <algorithm>

namespace motor
{
    namespace graphics
    {
        class image_object : public object
        {
            motor_this_typedefs( image_object ) ;

        private:

            motor::string_t _name ;
            motor::graphics::image_t _img ;

            motor::graphics::texture_type _texture_type = motor::graphics::texture_type::texture_2d ;
            motor::graphics::texture_wrap_type _wrap_types[ (size_t)motor::graphics::texture_wrap_mode::size ] ;
            motor::graphics::texture_filter_type _filter_types[ (size_t)motor::graphics::texture_filter_mode::size ] ;

        public:

            image_object( void_t ) 
            {
                for( size_t i=0; i<(size_t)motor::graphics::texture_wrap_mode::size; ++i )
                {
                    _wrap_types[ i ] = motor::graphics::texture_wrap_type::clamp ;
                }

                for( size_t i = 0; i < ( size_t ) motor::graphics::texture_filter_mode::size; ++i )
                {
                    _filter_types[ i ] = motor::graphics::texture_filter_type::nearest ;
                }
            }
            image_object( motor::string_in_t name ) : _name( name )
            {
                for( size_t i = 0; i < ( size_t ) motor::graphics::texture_wrap_mode::size; ++i )
                {
                    _wrap_types[ i ] = motor::graphics::texture_wrap_type::clamp ;
                }

                for( size_t i = 0; i < ( size_t ) motor::graphics::texture_filter_mode::size; ++i )
                {
                    _filter_types[ i ] = motor::graphics::texture_filter_type::nearest ;
                }
            }
            image_object( motor::string_in_t name, motor::graphics::image_rref_t img) :
                image_object( name )
            {
                _img = std::move( img ) ;
            }

            image_object( this_cref_t rhv ) : object( rhv )
            {
                _name = rhv._name ;
                _img = rhv._img ;
                _texture_type = rhv._texture_type ;

                memcpy( ( void_ptr_t ) _wrap_types, ( void_cptr_t )rhv._wrap_types, sizeof(_wrap_types) ) ;
                memcpy( ( void_ptr_t ) _filter_types, ( void_cptr_t )rhv._filter_types, sizeof(_filter_types) ) ;
            }

            image_object( this_rref_t rhv ) : object( std::move( rhv ) )
            {
                _name = std::move( rhv._name ) ;
                _img = std::move( rhv._img ) ;
                _texture_type = rhv._texture_type ;

                memcpy( ( void_ptr_t ) _wrap_types, ( void_cptr_t ) rhv._wrap_types, sizeof( _wrap_types ) ) ;
                memcpy( ( void_ptr_t ) _filter_types, ( void_cptr_t ) rhv._filter_types, sizeof( _filter_types ) ) ;
            }

            virtual ~image_object( void_t ){}

            this_ref_t operator = ( this_cref_t rhv )
            {
                object::operator=( rhv ) ;

                _name = rhv._name ;
                _img = rhv._img ;
                _texture_type = rhv._texture_type ;

                memcpy( ( void_ptr_t ) _wrap_types, ( void_cptr_t )rhv._wrap_types, sizeof(_wrap_types) ) ;
                memcpy( ( void_ptr_t ) _filter_types, ( void_cptr_t )rhv._filter_types, sizeof(_filter_types) ) ;

                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv )
            {
                object::operator=( std::move( rhv ) ) ;

                _name = std::move( rhv._name ) ;
                _img = std::move( rhv._img ) ;
                _texture_type = rhv._texture_type ;

                memcpy( ( void_ptr_t ) _wrap_types, ( void_cptr_t ) rhv._wrap_types, sizeof( _wrap_types ) ) ;
                memcpy( ( void_ptr_t ) _filter_types, ( void_cptr_t ) rhv._filter_types, sizeof( _filter_types ) ) ;

                return *this ;
            }

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

        public:

            image_cref_t image( void_t ) const noexcept { return _img ; }
            image_ref_t image( void_t ) noexcept { return _img ; }

            motor::graphics::texture_wrap_type get_wrap( motor::graphics::texture_wrap_mode const wm ) const noexcept
            {
                return _wrap_types[ size_t( wm ) ] ;
            }

            motor::graphics::texture_filter_type get_filter( motor::graphics::texture_filter_mode const fm ) const noexcept
            {
                return _filter_types[ size_t( fm ) ] ;
            }

            this_ref_t set_filter( motor::graphics::texture_filter_mode const fm, motor::graphics::texture_filter_type const ft ) noexcept
            {
                _filter_types[ size_t( fm ) ] = ft ;
                return *this ;
            }

            this_ref_t set_wrap( motor::graphics::texture_wrap_mode const m, motor::graphics::texture_wrap_type const t ) noexcept
            {
                _wrap_types[ size_t( m ) ] = t ;
                return *this ;
            }

            this_ref_t set_type( motor::graphics::texture_type const tt ) noexcept 
            {
                _texture_type = tt ;
                return *this ;
            }

            motor::graphics::texture_type get_type( void_t ) const noexcept 
            {
                return _texture_type ;
            }
        };
        motor_typedef( image_object ) ;
    }
}
