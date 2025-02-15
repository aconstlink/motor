#pragma once

#include "../../api.h"
#include "../../typedefs.h"
#include "../../result.h"

#include "../../graphics/gl/gl_context.h"
#include "../../graphics/gl/gl4.h"

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
            class MOTOR_PLATFORM_API wgl_context : public motor::platform::opengl::rendering_context
            {
                motor_this_typedefs( wgl_context ) ;

            private:

                HWND _hwnd = NULL ;
                HGLRC _hrc = NULL ;
               
                bool_t _active = false ;

                motor::platform::gen4::gl4_backend_mtr_t _backend = nullptr ;

                std::array< int, 20 > _attrib_list = {0} ;

                // all shared contexts derived from this one.
                motor::vector< HGLRC > _shared_contexts ;

            public:

                /// initializes this context but does not create the context.
                /// @see create_context
                wgl_context( void_t ) noexcept ;
                wgl_context( HWND ) noexcept ;
                wgl_context( this_cref_t ) = delete ;

                /// allows to move-construct a context.
                wgl_context( this_rref_t ) noexcept ;
                virtual ~wgl_context( void_t ) noexcept ;

            private:

                wgl_context( HWND hwnd, HGLRC ctx ) noexcept ;

                // for shared context creation
                wgl_context( HWND hwnd, HGLRC ctx,
                    motor::platform::gen4::gl4_backend_mtr_safe_t ) noexcept ;

            public: // operator =

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) noexcept ;

            public:

                virtual motor::platform::result activate( void_t ) noexcept ;
                virtual motor::platform::result deactivate( void_t ) noexcept ;
                virtual motor::platform::result vsync( bool_t const on_off ) noexcept ;
                virtual motor::platform::result swap( void_t ) noexcept ;

            public: // which interface ?

                motor::graphics::gen4::backend_mtr_safe_t backend( void_t ) noexcept ;
                motor::graphics::gen4::backend_borrow_t::mtr_t borrow_backend( void_t ) noexcept ;

            public:

                /// @note a valid window handle must be passed.
                motor::platform::result create_context( HWND hwnd ) noexcept ;

                /// Returns ok, if the extension is supported, otherwise, this function fails.
                /// @precondition Must be used after context has been created and made current.
                motor::platform::result is_extension_supported( motor::string_cref_t extension_name ) const noexcept ;

                /// This function fills the incoming list with all wgl extension strings.
                /// @precondition Must be used after context has been created and made current.
                motor::platform::result get_wgl_extension( motor::vector< motor::string_t >& ext_list ) const noexcept ;

                /// @precondition Context must be current.
                motor::platform::result get_gl_extension( motor::vector< motor::string_t >& ext_list ) noexcept ;

                /// @precondition Must be used after context has been created and made current. 
                result get_gl_version( motor::application::gl_version& version ) const noexcept ;

                /// @precondition Context must be active. For debug purpose. Just clears the screen.
                void_t clear_now( motor::math::vec4f_t const& vec ) noexcept ;

                // creates a shared context from this one
                this_t create_shared_context( void_t ) noexcept ;

            private:

                motor::platform::result create_the_context( motor::application::gl_info_cref_t gli ) noexcept ;

            private: // rendering_context interface

                virtual motor::platform::opengl::rendering_context_mtr_safe_t 
                    create_shared( void_t ) noexcept override ;
                

                virtual bool_t is_ext_supported( motor::string_cref_t ext ) const noexcept 
                {
                    auto const res = this_t::is_extension_supported( ext ) ;
                    return res == motor::platform::result::ok ;
                }

            };
            motor_typedef( wgl_context ) ;
        }
    }
}