
#pragma once

#include "es_context.h"
#include "../backend.h"

namespace natus
{
    namespace graphics
    {
        class NATUS_GRAPHICS_API es3_backend : public natus::graphics::backend
        {
            natus_this_typedefs( es3_backend ) ;

        private:

            struct pimpl ;
            pimpl* _pimpl ;

            es_context_ptr_t _context = nullptr ;

        public:

            es3_backend( natus::graphics::es_context_ptr_t ) noexcept ;
            es3_backend( this_cref_t ) = delete ;
            es3_backend( this_rref_t ) noexcept ;
            virtual ~es3_backend( void_t ) ;

        public:

            virtual void_t set_window_info( window_info_cref_t ) noexcept ;

            virtual natus::graphics::result configure( natus::graphics::geometry_object_res_t ) noexcept override ;
            virtual natus::graphics::result configure( natus::graphics::render_object_res_t ) noexcept override ;
            virtual natus::graphics::result configure( natus::graphics::shader_object_res_t ) noexcept override ;
            virtual natus::graphics::result configure( natus::graphics::image_object_res_t ) noexcept override ;
            virtual natus::graphics::result configure( natus::graphics::framebuffer_object_res_t ) noexcept override ;
            virtual natus::graphics::result configure( natus::graphics::state_object_res_t ) noexcept override ;
            virtual natus::graphics::result configure( natus::graphics::array_object_res_t ) noexcept override ;
            virtual natus::graphics::result configure( natus::graphics::streamout_object_res_t ) noexcept ;

            virtual natus::graphics::result release( natus::graphics::geometry_object_res_t ) noexcept ;
            virtual natus::graphics::result release( natus::graphics::render_object_res_t ) noexcept ;
            virtual natus::graphics::result release( natus::graphics::shader_object_res_t ) noexcept ;
            virtual natus::graphics::result release( natus::graphics::image_object_res_t ) noexcept ;
            virtual natus::graphics::result release( natus::graphics::framebuffer_object_res_t ) noexcept ;
            virtual natus::graphics::result release( natus::graphics::state_object_res_t ) noexcept ;
            virtual natus::graphics::result release( natus::graphics::array_object_res_t ) noexcept ;
            virtual natus::graphics::result release( natus::graphics::streamout_object_res_t ) noexcept ;

            virtual natus::graphics::result connect( natus::graphics::render_object_res_t, natus::graphics::variable_set_res_t ) noexcept ;
            virtual natus::graphics::result update( natus::graphics::geometry_object_res_t ) noexcept override ;
            virtual natus::graphics::result update( natus::graphics::array_object_res_t ) noexcept override ;
            virtual natus::graphics::result update( natus::graphics::streamout_object_res_t ) noexcept override ;
            virtual natus::graphics::result update( natus::graphics::image_object_res_t ) noexcept override ;
            virtual natus::graphics::result update( natus::graphics::render_object_res_t, size_t const varset ) noexcept ;

            virtual natus::graphics::result use( natus::graphics::framebuffer_object_res_t ) noexcept override ;
            virtual natus::graphics::result use( natus::graphics::streamout_object_res_t ) noexcept ;
            virtual natus::graphics::result unuse( natus::graphics::backend::unuse_type const ) noexcept override ;
            virtual natus::graphics::result push( natus::graphics::state_object_res_t, size_t const, bool_t const ) noexcept override ;
            virtual natus::graphics::result pop( natus::graphics::backend::pop_type const ) noexcept ;

            virtual natus::graphics::result render( natus::graphics::render_object_res_t, natus::graphics::backend::render_detail_cref_t ) noexcept override ;

            virtual void_t render_begin( void_t ) noexcept ;
            virtual void_t render_end( void_t ) noexcept ;

        private:

            //
        };
        natus_res_typedef( es3_backend ) ;
    }
}
