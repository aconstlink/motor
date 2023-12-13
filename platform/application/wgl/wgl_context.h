#pragma once

#include "../../api.h"
#include "../../typedefs.h"
#include "../../result.h"

//#include "../gl_info.h"

//#include "../gfx_context.h"

//#include <natus/graphics/backend/gl/gl_context.h>
//#include <natus/graphics/backend/gl/gl4.h>
#include <motor/application/gl_info.h>

#include <motor/math/vector/vector4.hpp>

#include <motor/std/string>
#include <motor/std/vector>

#include <windows.h>

namespace motor
{
    namespace platform
    {
        namespace wgl
        {
            //class gl_context ;
            //motor_class_proto_typedefs( gl_context ) ;

            class MOTOR_PLATFORM_API wgl_context //: public gfx_context
            {
                motor_this_typedefs( wgl_context ) ;

            private:

                HWND _hwnd = NULL ;
                HGLRC _hrc = NULL ;

                /// will only be used temporarily for making the
                /// context active. Will be released on deactivation.
                HDC _hdc = NULL ;

                //gl_context_ptr_t _bend_ctx = nullptr ;

            public:

                /// initializes this context but does not create the context.
                /// @see create_context
                wgl_context( void_t ) noexcept ;
                wgl_context( HWND ) noexcept ;
                wgl_context( this_cref_t ) = delete ;

                /// allows to move-construct a context.
                wgl_context( this_rref_t ) noexcept ;
                ~wgl_context( void_t ) noexcept ;

            private:

                wgl_context( HWND hwnd, HGLRC ctx ) noexcept ;

            public: // operator =

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) noexcept ;

            public:

                virtual motor::platform::result activate( void_t ) noexcept ;
                virtual motor::platform::result deactivate( void_t ) noexcept ;
                virtual motor::platform::result vsync( bool_t const on_off ) noexcept ;
                virtual motor::platform::result swap( void_t ) noexcept ;

                //virtual motor::graphics::backend_res_t create_backend( void_t ) noexcept ;

            public:

                /// @note a valid window handle must be passed.
                motor::platform::result create_context( HWND hwnd ) noexcept ;

                /// Returns ok, if the extension is supported, otherwise, this function fails.
                /// @precondition Must be used after context has been created and made current.
                motor::platform::result is_extension_supported( motor::string_cref_t extension_name ) noexcept ;

                /// This function fills the incoming list with all wgl extension strings.
                /// @precondition Must be used after context has been created and made current.
                motor::platform::result get_wgl_extension( motor::vector< motor::string_t >& ext_list ) noexcept ;

                /// @precondition Context must be current.
                motor::platform::result get_gl_extension( motor::vector< motor::string_t >& ext_list ) noexcept ;

                /// @precondition Must be used after context has been created and made current. 
                result get_gl_version( motor::application::gl_version& version ) const noexcept ;

                /// @precondition Context must be active. For debug purpose. Just clears the screen.
                void_t clear_now( motor::math::vec4f_t const& vec ) noexcept ;

            private:

                motor::platform::result create_the_context( motor::application::gl_info_cref_t gli ) noexcept ;
            };
            motor_typedef( wgl_context ) ;

            #if 0
            // this is passed to the graphics backend at construction time, so the backend
            // can check for extensions or other context related topics.
            class NATUS_APPLICATION_API gl_context : public motor::graphics::gl_context
            {
                motor_this_typedefs( gl_context ) ;

                friend class motor::application::wgl::context ;

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
                    return res == motor::platform::result::ok ;
                }
            };
            #endif
        }
    }
}