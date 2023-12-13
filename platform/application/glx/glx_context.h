#pragma once

//#include "../gl_info.h"

//#include "../gfx_context.h"

//#include <motor/graphics/backend/null/null.h>
//#include <motor/graphics/backend/gl/gl_context.h>
#include <motor/application/gl_info.h>
#include <motor/math/vector/vector4.hpp>

#include <GL/glcorearb.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/glcorearb.h>

namespace motor
{
    namespace platform
    {
        namespace glx
        {
            //class gl_context ;
            //motor_class_proto_typedefs( gl_context ) ;

            class context //: public gfx_context
            {
                motor_this_typedefs( context ) ;

            private:

                Display * _display = NULL ;
                Window _wnd ;

                GLXContext _context ;

                //gl_context_ptr_t _bend_ctx = nullptr ;

            public:

                motor_typedefs( motor::vector< motor::string_t >, strings ) ;

            public:

                context( void_t ) noexcept ;
                context( motor::application::gl_info_in_t, Window wnd, Display * disp ) noexcept ;
                context( this_cref_t ) = delete ;
                /// allows to move-construct a context.
                context( this_rref_t ) noexcept ;
                ~context( void_t ) noexcept ;

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) noexcept ;

            private:

                bool_t  determine_gl_version( motor::application::gl_version & ) const noexcept ;

            public:

                virtual motor::application::result activate( void_t ) noexcept ;
                virtual motor::application::result deactivate( void_t ) noexcept ;
                virtual motor::application::result vsync( bool_t const on_off ) noexcept ;
                virtual motor::application::result swap( void_t ) noexcept ;

                //virtual motor::graphics::backend_res_t create_backend( void_t ) noexcept ;

            public:

                motor::application::result create_glx( void_t ) noexcept ;

            public:

                /// @note a valid window handle must be passed.
                motor::application::result create_context( 
                    Display*, Window, GLXContext ) noexcept ;

                /// Returns ok, if the extension is supported, 
                /// otherwise, this function fails.
                /// @precondition Must be used after context has 
                /// been created and made current.
                motor::application::result is_extension_supported( 
                    motor::string_cref_t extension_name ) noexcept ;

                /// This function fills the incoming list with all 
                /// wgl extension strings.
                /// @precondition Must be used after context has 
                /// been created and made current.
                motor::application::result get_glx_extension( 
                    strings_out_t ext_list ) noexcept ;

                /// @precondition Context must be current.
                motor::application::result get_gl_extension( 
                    strings_out_t ext_list )  noexcept;

                /// @precondition Must be used after context has 
                /// been created and made current. 
                motor::application::result get_gl_version( 
                    motor::application::gl_version & version ) const noexcept ;

                /// @precondition Context must be active. 
                /// For debug purpose. Just clears the screen.
                void_t clear_now( motor::math::vec4f_t const & vec ) noexcept ;

            private:

                motor::application::result create_the_context( 
                    motor::application::gl_info_cref_t gli ) noexcept ;
            };
            motor_typedef( context ) ;

#if 0
            // this is passed to the graphics backend at construction time, so the backend
            // can check for extensions or other context related topics.
            class NATUS_APPLICATION_API gl_context : public motor::graphics::gl_context
            {
                motor_this_typedefs( gl_context ) ;

                friend class motor::application::glx::context ;

            private:

                // owner
                context_ptr_t _app_context = nullptr ;

                gl_context( context_ptr_t ctx ) noexcept : _app_context( ctx ) {}
                gl_context( this_cref_t ) = delete ;
            
            public:

                gl_context( this_rref_t rhv ) noexcept
                {
                    motor_move_member_ptr( _app_context, rhv ) ;
                }

                void_t change_owner( context_ptr_t ctx ) noexcept { _app_context = ctx ; }

            public:

                virtual ~gl_context( void_t ) noexcept {}

            public:

                virtual bool_t is_extension_supported( motor::string_cref_t ext ) const noexcept
                {
                    auto const res = _app_context->is_extension_supported( ext ) ;
                    return res == motor::application::result::ok ;
                }
            };
            #endif
        }
    }
}
