

#pragma once 

#include "backend.h"

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API null_backend : public motor::graphics::gen4::backend
        {
            typedef motor::graphics::gen4::backend base_t ;
            motor_this_typedefs( null_backend ) ;

        public:

            null_backend( void_t ) noexcept ;
            null_backend( this_cref_t ) = delete ;
            null_backend( this_rref_t ) noexcept ;
            virtual ~null_backend( void_t ) noexcept ;

        public:

            virtual void_t set_window_info( window_info_cref_t ) noexcept ;

            virtual motor::graphics::result configure( motor::graphics::geometry_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::render_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::shader_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::image_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::framebuffer_object_mtr_t ) noexcept ;
            virtual motor::graphics::result configure( motor::graphics::state_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::array_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::streamout_object_mtr_t ) noexcept override ;

            virtual motor::graphics::result release( motor::graphics::geometry_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result release( motor::graphics::render_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result release( motor::graphics::shader_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result release( motor::graphics::image_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result release( motor::graphics::framebuffer_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result release( motor::graphics::state_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result release( motor::graphics::array_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result release( motor::graphics::streamout_object_mtr_t ) noexcept override ;

            virtual motor::graphics::result update( motor::graphics::geometry_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::array_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::streamout_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::image_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::render_object_mtr_t, size_t const varset ) noexcept ;

            virtual motor::graphics::result use( motor::graphics::framebuffer_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result use( motor::graphics::streamout_object_mtr_t ) noexcept override ;
            virtual motor::graphics::result unuse( motor::graphics::gen4::backend::unuse_type const ) noexcept override ;
            virtual motor::graphics::result push( motor::graphics::state_object_mtr_t, size_t const, bool_t const ) noexcept override;
            virtual motor::graphics::result pop( motor::graphics::gen4::backend::pop_type const ) noexcept override ;

            virtual motor::graphics::result render( motor::graphics::render_object_mtr_t, motor::graphics::gen4::backend::render_detail_cref_t ) noexcept override ;

            virtual void_t render_begin( void_t ) noexcept ;
            virtual void_t render_end( void_t ) noexcept ;
        };
        motor_typedef( null_backend ) ;
    }
}