
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
    namespace application
    {
        namespace egl
        {
            class es_context ;
            motor_class_proto_typedefs( es_context ) ;

            class context : public gfx_context
            {
                motor_this_typedefs( context ) ;

            private:

                EGLNativeDisplayType _ndt = 0 ;
                EGLDisplay _display = 0 ;
                EGLNativeWindowType _wnd ;

                EGLContext _context ;
                EGLSurface _surface ;

                es_context_ptr_t _bend_ctx = nullptr ;

            public:

                motor_typedefs( 
                     natus::ntd::vector< natus::ntd::string >, strings ) ;

            public:

                context( void_t ) ;
                context( gl_info_in_t, EGLNativeWindowType wnd, EGLNativeDisplayType disp ) ;
                context( this_cref_t ) = delete ;
                /// allows to move-construct a context.
                context( this_rref_t ) ;
                ~context( void_t ) ;

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) ;

            private:

                
            public:

                virtual natus::application::result activate( void_t ) ;
                virtual natus::application::result deactivate( void_t ) ;
                virtual natus::application::result vsync( bool_t const on_off ) ;
                virtual natus::application::result swap( void_t ) ;

                virtual natus::graphics::backend_res_t create_backend( void_t ) noexcept ;

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

                natus::application::result create_the_context( 
                    gl_info_cref_t gli ) ;
            };
            motor_res_typedef( context ) ;

            // this is passed to the graphics backend at construction time, so the backend
            // can check for extensions or other context related topics.
            class NATUS_APPLICATION_API es_context : public natus::graphics::es_context
            {
                motor_this_typedefs( es_context ) ;

                friend class natus::application::egl::context ;

            private:

                // owner
                context_ptr_t _app_context = nullptr ;

                es_context( context_ptr_t ctx ) noexcept : _app_context( ctx ) {}
                es_context( this_cref_t ) = delete ;

            public:

                es_context( this_rref_t rhv ) noexcept
                {
                    motor_move_member_ptr( _app_context, rhv ) ;
                }

                void_t change_owner( context_ptr_t ctx ) noexcept { _app_context = ctx ; }

            public:

                virtual ~es_context( void_t ) noexcept {}

            public:

                virtual bool_t is_extension_supported( natus::ntd::string_cref_t ext ) const noexcept
                {
                    auto const res = _app_context->is_extension_supported( ext ) ;
                    return res == natus::application::result::ok ;
                }
            };
        }
    }
}
