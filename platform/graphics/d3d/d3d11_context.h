
#pragma once

#include "../../api.h"
#include "../../typedefs.h"
#include <motor/graphics/backend/icontext.h>

#include <motor/std/string>
#include <motor/math/vector/vector4.hpp>

#include <d3d11.h>
#include <d3d11_1.h>

namespace motor
{
    namespace platform
    {
        class MOTOR_PLATFORM_API d3d11_context : public motor::graphics::icontext
        {
        public:

            virtual ~d3d11_context( void_t ) noexcept {}

        public:

            virtual ID3D11DeviceContext * ctx( void_t ) noexcept = 0 ;
            virtual ID3D11Device * dev( void_t ) noexcept = 0 ;
            virtual ID3D11Debug * debug( void_t ) noexcept = 0 ;

            virtual void_t activate_framebuffer( void_t ) noexcept = 0 ;
            virtual void_t clear_render_target_view( motor::math::vec4f_cref_t ) noexcept = 0 ;
            virtual void_t clear_depth_stencil_view( void_t ) noexcept = 0 ;
        };
        motor_typedef( d3d11_context ) ;
    }
}