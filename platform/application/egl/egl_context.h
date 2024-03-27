
#pragma once

#include "../../typedefs.h"
#include "../../result.h"

#include "../../graphics/gl/es_context.h"
//#include "../../graphics/gl/gl4.h"

#include <motor/application/gl_info.h>
#include <motor/math/vector/vector4.hpp>

#include <motor/std/string>
#include <motor/std/vector>

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace motor
{
    namespace platform
    {
        namespace egl
        {
            class egl_context : public motor::platform::opengl::rendering_context
            {
                motor_this_typedefs( egl_context ) ;

            private:

                EGLNativeDisplayType _ndt = 0 ;
                EGLDisplay _display = 0 ;
                EGLNativeWindowType _wnd ;

                EGLContext _context ;
                EGLSurface _surface ;

                //motor::platform::gen4::es3_backend_mtr_t _backend = nullptr ;

            public:

                motor_typedefs( motor::vector< motor::string_t >, strings ) ;

            public:

                egl_context( void_t ) noexcept ;
                egl_context( EGLNativeWindowType wnd, EGLNativeDisplayType disp ) noexcept ;
                egl_context( this_cref_t ) = delete ;
                egl_context( this_rref_t ) noexcept ;
                ~egl_context( void_t ) noexcept ;

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) noexcept ;

            private:

                bool_t  determine_gl_version( motor::application::gl_version & ) const noexcept ;
                
            public:

                virtual motor::platform::result activate( void_t ) ;
                virtual motor::platform::result deactivate( void_t ) ;
                virtual motor::platform::result vsync( bool_t const on_off ) ;
                virtual motor::platform::result swap( void_t ) ;

                motor::graphics::gen4::backend_mtr_safe_t backend( void_t ) noexcept ;
                motor::graphics::gen4::backend_borrow_t::mtr_t borrow_backend( void_t ) noexcept ;

                virtual bool_t is_ext_supported( motor::string_cref_t ext ) const noexcept 
                {
                    auto const res = this_t::is_extension_supported( ext ) ;
                    return res == motor::platform::result::ok ;
                }
                
            public:

                motor::platform::result create_egl( void_t ) ;

            public:

                /// @note a valid window handle must be passed.
                //motor::platform::result create_context( 
                  //  EGLNativDisplayType, EGLNativWindowType, ESContext ) ;

                /// Returns ok, if the extension is supported, 
                /// otherwise, this function fails.
                /// @precondition Must be used after context has 
                /// been created and made current.
                motor::platform::result is_extension_supported( 
                    motor::string_cref_t extension_name ) const noexcept;

                /// This function fills the incoming list with all 
                /// wgl extension strings.
                /// @precondition Must be used after context has 
                /// been created and made current.
                motor::platform::result get_egl_extension( 
                    strings_out_t ext_list ) ;

                /// @precondition Context must be current.
                motor::platform::result get_es_extension( 
                    strings_out_t ext_list ) ;

                motor::platform::result get_es_version( 
                   motor::application::gl_version & version ) const ;

                /// @precondition Context must be active. 
                /// For debug purpose. Just clears the screen.
                void_t clear_now( motor::math::vec4f_t const & vec ) ;

            private:

                motor::platform::result create_the_context( 
                    motor::application::gl_info_cref_t gli ) ;
            };
            motor_typedef( egl_context ) ;
        }
    }
}
