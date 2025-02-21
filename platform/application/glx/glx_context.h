#pragma once

#include "../../typedefs.h"
#include "../../result.h"

#include "../../graphics/gl/gl_context.h"
#include "../../graphics/gl/gl4.h"

#include <motor/application/gl_info.h>
#include <motor/math/vector/vector4.hpp>

#include <motor/std/string>
#include <motor/std/vector>

#include <X11/Xlib.h>


namespace motor
{
    namespace platform
    {
        namespace glx
        {
            //class gl_context ;
            //motor_class_proto_typedefs( gl_context ) ;

            class context : public motor::platform::opengl::rendering_context
            {
                motor_this_typedefs( context ) ;

            private:

                Display * _display = NULL ;
                Window _wnd ;

                struct pimpl ;
                pimpl * _pimpl ;

                motor::platform::gen4::gl4_backend_mtr_t _backend = nullptr ;

            public:

                motor_typedefs( motor::vector< motor::string_t >, strings ) ;

            public:

                context( void_t ) noexcept ;
                context( Window wnd, Display * disp ) noexcept ;
                context( this_cref_t ) = delete ;
                /// allows to move-construct a context.
                context( this_rref_t ) noexcept ;
                ~context( void_t ) noexcept ;

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) noexcept ;

            private:

                bool_t  determine_gl_version( motor::application::gl_version & ) const noexcept ;

            public:

                virtual motor::platform::result activate( void_t ) noexcept ;
                virtual motor::platform::result deactivate( void_t ) noexcept ;
                virtual motor::platform::result vsync( bool_t const on_off ) noexcept ;
                virtual motor::platform::result swap( void_t ) noexcept ;

                motor::graphics::gen4::backend_mtr_safe_t backend( void_t ) noexcept ;
                motor::graphics::gen4::backend_borrow_t::mtr_t borrow_backend( void_t ) noexcept ;
                
            public:

                virtual bool_t is_ext_supported( motor::string_cref_t ext ) const noexcept 
                {
                    auto const res = this_t::is_extension_supported( ext ) ;
                    return res == motor::platform::result::ok ;
                }

                motor::platform::result create_context( Window wnd, Display * disp ) noexcept ;
                virtual motor::platform::opengl::rendering_context_mtr_safe_t create_shared( void_t ) noexcept ;

            public:

                /// Returns ok, if the extension is supported, 
                /// otherwise, this function fails.
                /// @precondition Must be used after context has 
                /// been created and made current.
                motor::platform::result is_extension_supported( 
                    motor::string_cref_t extension_name ) const noexcept ;

                /// This function fills the incoming list with all 
                /// wgl extension strings.
                /// @precondition Must be used after context has 
                /// been created and made current.
                motor::platform::result get_glx_extension( 
                    strings_out_t ext_list ) const noexcept ;

                /// @precondition Context must be current.
                motor::platform::result get_gl_extension( 
                    strings_out_t ext_list ) const noexcept;

                /// @precondition Must be used after context has 
                /// been created and made current. 
                motor::platform::result get_gl_version( 
                    motor::application::gl_version & version ) const noexcept ;

                /// @precondition Context must be active. 
                /// For debug purpose. Just clears the screen.
                void_t clear_now( motor::math::vec4f_t const & vec ) noexcept ;

            private:

                motor::platform::result create_the_context( 
                    motor::application::gl_info_cref_t gli ) noexcept ;

                static void_t init_gl_context( void_t ) noexcept ;
            };
            motor_typedef( context ) ;
            motor_typedefs( context, glx_context ) ;
        }
    }
}
