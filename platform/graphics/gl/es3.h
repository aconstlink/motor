
#pragma once

#include "es_context.h"
#include <motor/graphics/backend/backend.h>

namespace motor
{
    namespace platform
    {
        class MOTOR_PLATFORM_API es3_backend : public motor::graphics::backend
        {
            motor_this_typedefs( es3_backend ) ;

        private:

            struct pimpl ;
            pimpl* _pimpl ;

            es_context_ptr_t _context = nullptr ;

        public:

            es3_backend( motor::graphics::es_context_ptr_t ) noexcept ;
            es3_backend( this_cref_t ) = delete ;
            es3_backend( this_rref_t ) noexcept ;
            virtual ~es3_backend( void_t ) ;

        public:

            virtual void_t set_window_info( window_info_cref_t ) noexcept ;

            virtual motor::graphics::result configure( motor::graphics::geometry_object_res_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::render_object_res_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::shader_object_res_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::image_object_res_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::framebuffer_object_res_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::state_object_res_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::array_object_res_t ) noexcept override ;
            virtual motor::graphics::result configure( motor::graphics::streamout_object_res_t ) noexcept ;

            virtual motor::graphics::result release( motor::graphics::geometry_object_res_t ) noexcept ;
            virtual motor::graphics::result release( motor::graphics::render_object_res_t ) noexcept ;
            virtual motor::graphics::result release( motor::graphics::shader_object_res_t ) noexcept ;
            virtual motor::graphics::result release( motor::graphics::image_object_res_t ) noexcept ;
            virtual motor::graphics::result release( motor::graphics::framebuffer_object_res_t ) noexcept ;
            virtual motor::graphics::result release( motor::graphics::state_object_res_t ) noexcept ;
            virtual motor::graphics::result release( motor::graphics::array_object_res_t ) noexcept ;
            virtual motor::graphics::result release( motor::graphics::streamout_object_res_t ) noexcept ;

            virtual motor::graphics::result connect( motor::graphics::render_object_res_t, motor::graphics::variable_set_res_t ) noexcept ;
            virtual motor::graphics::result update( motor::graphics::geometry_object_res_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::array_object_res_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::streamout_object_res_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::image_object_res_t ) noexcept override ;
            virtual motor::graphics::result update( motor::graphics::render_object_res_t, size_t const varset ) noexcept ;

            virtual motor::graphics::result use( motor::graphics::framebuffer_object_res_t ) noexcept override ;
            virtual motor::graphics::result use( motor::graphics::streamout_object_res_t ) noexcept ;
            virtual motor::graphics::result unuse( motor::graphics::backend::unuse_type const ) noexcept override ;
            virtual motor::graphics::result push( motor::graphics::state_object_res_t, size_t const, bool_t const ) noexcept override ;
            virtual motor::graphics::result pop( motor::graphics::backend::pop_type const ) noexcept ;

            virtual motor::graphics::result render( motor::graphics::render_object_res_t, motor::graphics::backend::render_detail_cref_t ) noexcept override ;

            virtual void_t render_begin( void_t ) noexcept ;
            virtual void_t render_end( void_t ) noexcept ;

        private:

            //
        };
        motor_typedef( es3_backend ) ;
    }
}
