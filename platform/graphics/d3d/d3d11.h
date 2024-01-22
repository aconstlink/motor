
#pragma once

#include "d3d11_context.h"
#include <motor/graphics/backend/gen4/backend.h>

namespace motor
{
    namespace platform
    {
        namespace gen4
        {
            class MOTOR_PLATFORM_API d3d11_backend : public motor::graphics::gen4::backend
            {
                motor_this_typedefs( d3d11_backend ) ;

            private:

                struct pimpl ;
                pimpl* _pimpl ;

                motor::platform::d3d11::rendering_context_ptr_t _context = nullptr ;

            public:

                d3d11_backend( motor::platform::d3d11::rendering_context_ptr_t ) noexcept ;
                d3d11_backend( this_cref_t ) = delete ;
                d3d11_backend( this_rref_t ) noexcept ;
                virtual ~d3d11_backend( void_t ) ;

            public:

                virtual void_t set_window_info( window_info_cref_t ) noexcept ;

                virtual motor::graphics::result configure( motor::graphics::msl_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::geometry_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::render_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::shader_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::image_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::framebuffer_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::state_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::array_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result configure( motor::graphics::streamout_object_mtr_t ) noexcept ;

                virtual motor::graphics::result release( motor::graphics::geometry_object_mtr_t ) noexcept ;
                virtual motor::graphics::result release( motor::graphics::render_object_mtr_t ) noexcept ;
                virtual motor::graphics::result release( motor::graphics::shader_object_mtr_t ) noexcept ;
                virtual motor::graphics::result release( motor::graphics::image_object_mtr_t ) noexcept ;
                virtual motor::graphics::result release( motor::graphics::framebuffer_object_mtr_t ) noexcept ;
                virtual motor::graphics::result release( motor::graphics::state_object_mtr_t ) noexcept ;
                virtual motor::graphics::result release( motor::graphics::array_object_mtr_t ) noexcept ;
                virtual motor::graphics::result release( motor::graphics::streamout_object_mtr_t ) noexcept ;

                virtual motor::graphics::result update( motor::graphics::geometry_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result update( motor::graphics::array_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result update( motor::graphics::streamout_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result update( motor::graphics::image_object_mtr_t ) noexcept override ;
                virtual motor::graphics::result update( motor::graphics::render_object_mtr_t, size_t const varset ) noexcept ;

                virtual motor::graphics::result use( motor::graphics::framebuffer_object_mtr_t ) noexcept ;
                virtual motor::graphics::result use( motor::graphics::streamout_object_mtr_t ) noexcept ;
                virtual motor::graphics::result unuse( motor::graphics::gen4::backend::unuse_type const ) noexcept ;
                virtual motor::graphics::result push( motor::graphics::state_object_mtr_t, size_t const, bool_t const ) noexcept ;
                virtual motor::graphics::result pop( motor::graphics::gen4::backend::pop_type const ) noexcept ;

                virtual motor::graphics::result render( motor::graphics::render_object_mtr_t, motor::graphics::gen4::backend::render_detail_cref_t ) noexcept override ;
                virtual motor::graphics::result render( motor::graphics::msl_object_mtr_t, motor::graphics::gen4::backend::render_detail_cref_t ) noexcept ;

                virtual void_t render_begin( void_t ) noexcept ;
                virtual void_t render_end( void_t ) noexcept ;

            private:

                //
                void_t report_live_device_objects( D3D11_RLDO_FLAGS const ) noexcept ;
            };
            motor_typedef( d3d11_backend ) ;
        }
    }
}