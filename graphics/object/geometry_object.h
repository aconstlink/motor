
#pragma once

#include "../object.hpp"

#include "../buffer/vertex_buffer.hpp"
#include "../buffer/index_buffer.hpp"

#include <motor/geometry/mesh/polygon_mesh.h>
#include <motor/std/vector>

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API geometry_object : public object
        {
            motor_this_typedefs( geometry_object ) ;

        private:

            motor::graphics::primitive_type _pt ;
            motor::graphics::vertex_buffer_t _vb ;
            motor::graphics::index_buffer_t _ib ;

            motor::string_t _name ;

        public: 

            geometry_object( void_t ) noexcept {}

            geometry_object( motor::string_cref_t name ) noexcept : _name(name)
            {}

            geometry_object( motor::string_cref_t name, motor::graphics::primitive_type const pt, 
                motor::graphics::vertex_buffer_cref_t vb, motor::graphics::index_buffer_cref_t ib ) noexcept
            {
                _name = name ;
                _pt = pt ;
                _vb = vb ;
                _ib = ib ;
            }

            geometry_object( motor::string_cref_t name, motor::graphics::primitive_type const pt,
                motor::graphics::vertex_buffer_rref_t vb, motor::graphics::index_buffer_rref_t ib ) noexcept
            {
                _name = name ;
                _pt = pt ;
                _vb = std::move( vb ) ;
                _ib = std::move( ib ) ;
            }

            geometry_object( motor::string_cref_t name, motor::graphics::primitive_type const pt, 
                motor::graphics::vertex_buffer_cref_t vb ) noexcept
            {
                _name = name ;
                _pt = pt ;
                _vb = vb ;
            }

            geometry_object( motor::string_cref_t name, motor::graphics::primitive_type const pt,
                motor::graphics::vertex_buffer_rref_t vb  ) noexcept
            {
                _name = name ;
                _pt = pt ;
                _vb = std::move( vb ) ;
            }

            geometry_object( this_cref_t rhv ) noexcept : object( rhv ) 
            {
                _pt = rhv._pt ;
                _vb = rhv._vb ;
                _ib = rhv._ib ;
                _name = rhv._name ;
            }

            geometry_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) )
            {
                _pt = rhv._pt ;
                _vb = std::move( rhv._vb ) ;
                _ib = std::move( rhv._ib ) ;
                _name = std::move( rhv._name ) ;
            }

            ~geometry_object( void_t ) noexcept
            {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                object::operator=( rhv ) ;

                _pt = rhv._pt ;
                _vb = rhv._vb ;
                _ib = rhv._ib ;
                _name = rhv._name ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                object::operator=( std::move( rhv ) ) ;

                _pt = rhv._pt ;
                _vb = std::move( rhv._vb ) ;
                _ib = std::move( rhv._ib ) ;
                _name = std::move( rhv._name ) ;
                return *this ;
            }

        public:

            motor::graphics::primitive_type primitive_type( void_t ) const noexcept 
            {
                return _pt ;
            }

            motor::graphics::vertex_buffer_ref_t vertex_buffer( void_t ) noexcept
            {
                return _vb ;
            }

            motor::graphics::vertex_buffer_cref_t vertex_buffer( void_t ) const noexcept
            {
                return _vb ;
            }

            motor::graphics::index_buffer_ref_t index_buffer( void_t ) noexcept 
            {
                return _ib ;
            }

            motor::graphics::index_buffer_cref_t index_buffer( void_t ) const noexcept
            {
                return _ib ;
            }

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

        public:

            static this_t create( motor::string_in_t name, motor::geometry::polygon_mesh_in_t mesh ) noexcept ;

        };
        motor_typedef( geometry_object ) ;
    }
}
