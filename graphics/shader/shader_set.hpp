#pragma once

#include "../api.h"
#include "../protos.h"
#include "../typedefs.h"

#include "shader.hpp"

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API shader_set
        {
            motor_this_typedefs( shader_set ) ;

        private:

            motor::graphics::shader_t _vs ;
            motor::graphics::shader_t _ps ;
            motor::graphics::shader_t _gs ;

        public:

            shader_set( void_t ) 
            {}

            shader_set( this_cref_t rhv ) noexcept
            {
                *this = rhv ;
            }

            shader_set( this_rref_t rhv ) noexcept
            {
                *this = std::move( rhv ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _vs = rhv._vs ;
                _ps = rhv._ps ;
                _gs = rhv._gs ;

                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _vs = std::move( rhv._vs ) ;
                _ps = std::move( rhv._ps ) ;
                _gs = std::move( rhv._gs ) ;

                return *this ;
            }

        public:

            this_ref_t set_vertex_shader( motor::graphics::shader_in_t s ) noexcept
            {
                _vs = s ;
                return *this ;
            }

            this_ref_t set_pixel_shader( motor::graphics::shader_t s ) noexcept
            {
                _ps = s ;
                return *this ;
            }

            this_ref_t set_geometry_shader( motor::graphics::shader_t s ) noexcept
            {
                _gs = s ;
                return *this ;
            }

            motor::graphics::shader_cref_t vertex_shader( void_t ) const noexcept
            {
                return _vs ;
            }

            motor::graphics::shader_cref_t pixel_shader( void_t ) const noexcept
            {
                return _ps ;
            }

            motor::graphics::shader_cref_t geometry_shader( void_t ) const noexcept
            {
                return _gs ;
            }

            bool_t has_vertex_shader( void_t ) const noexcept
            {
                return !_vs.code().empty() ;
            }

            bool_t has_geometry_shader( void_t ) const noexcept
            {
                return !_gs.code().empty() ;
            }

            bool_t has_pixel_shader( void_t ) const noexcept
            {
                return !_ps.code().empty() ;
            }
        };
        motor_typedef( shader_set ) ;
    }
}