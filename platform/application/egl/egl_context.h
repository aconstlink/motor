
#pragma once

#include "../gl_info.h"

#include "../gfx_context.h"

#include <natus/graphics/backend/null/null.h>
#include <natus/graphics/backend/gl/es_context.h>

#include <natus/memory/res.hpp>
#include <natus/math/vector/vector4.hpp>

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace motor
{
    namespace platform
    {
        namespace egl
        {
            class context : public motor::platform::opengl::rendering_context
            {
                motor_this_typedefs( context ) ;

            private:

                EGLNativeDisplayType _ndt = 0 ;
                EGLDisplay _display = 0 ;
                EGLNativeWindowType _wnd ;

                EGLContext _context ;
                EGLSurface _surface ;

                //motor::platform::gen4::es3_backend_mtr_t _backend = nullptr ;

            public:

                motor_typedefs( motor::vector< natus::ntd::string >, strings ) ;

            public:

                context( void_t ) ;
                context( gl_info_in_t, EGLNativeWindowType wnd, EGLNativeDisplayType disp ) ;
                context( this_cref_t ) = delete ;
                context( this_rref_t ) ;
                ~context( void_t ) ;

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) ;

            private:

                bool_t  determine_gl_version( motor::application::gl_version & ) const noexcept ;
                
            public:

                virtual natus::application::result activate( void_t ) ;
                virtual natus::application::result deactivate( void_t ) ;
                virtual natus::application::result vsync( bool_t const on_off ) ;
                virtual natus::application::result swap( void_t ) ;

                motor::graphics::gen4::backend_mtr_safe_t backend( void_t ) noexcept ;
                motor::graphics::gen4::backend_borrow_t::mtr_t borrow_backend( void_t ) noexcept ;

            public:

                natus::application::result create_egl( void_t ) ;

            public:

                /// @note a valid window handle must be passed.
                //natus::application::result create_context( 
                  //  EGLNativDisplayType, EGLNativWindowType, ESContext ) ;

                /// Returns ok, if the extension is supported, 
                /// otherwise, this function fails.
                /// @precondition Must be used after context has 
                /// been created and made current.
                natus::application::result is_extension_supported( 
                    natus::ntd::string_cref_t extension_name ) ;

                /// This function fills the incoming list with all 
                /// wgl extension strings.
                /// @precondition Must be used after context has 
                /// been created and made current.
                natus::application::result get_egl_extension( 
                    strings_out_t ext_list ) ;

                /// @precondition Context must be current.
                natus::application::result get_es_extension( 
                    strings_out_t ext_list ) ;

                natus::application::result get_es_version( 
                   natus::application::gl_version & version ) const ;

                /// @precondition Context must be active. 
                /// For debug purpose. Just clears the screen.
                void_t clear_now( natus::math::vec4f_t const & vec ) ;

            private:

                natus::application::result create_the_context( gl_info_cref_t gli ) ;
            };
            motor_res_typedef( context ) ;
        }
    }
}
