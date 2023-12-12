#pragma once

#include "../gl_info.h"

#include "../gfx_context.h"

#include <natus/graphics/backend/gl/gl_context.h>
#include <natus/graphics/backend/gl/gl4.h>

#include <natus/memory/res.hpp>
#include <natus/math/vector/vector4.hpp>

#include <windows.h>

namespace natus
{
    namespace application
    {
        namespace wgl
        {
            class gl_context ;
            natus_class_proto_typedefs( gl_context ) ;

            class NATUS_APPLICATION_API context : public gfx_context
            {
                natus_this_typedefs( context ) ;

            private:

                HWND _hwnd = NULL ;
                HGLRC _hrc = NULL ;

                /// will only be used temporarily for making the
                /// context active. Will be released on deactivation.
                HDC _hdc = NULL ;

                gl_context_ptr_t _bend_ctx = nullptr ;

            public:

                /// initializes this context but does not create the context.
                /// @see create_context
                context( void_t ) ;
                context( HWND ) ;
                context( this_cref_t ) = delete ;

                /// allows to move-construct a context.
                context( this_rref_t ) ;
                ~context( void_t ) ;

            private:

                context( HWND hwnd, HGLRC ctx ) ;

            public: // operator =

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) ;

            public:

                virtual natus::application::result activate( void_t ) ;
                virtual natus::application::result deactivate( void_t ) ;
                virtual natus::application::result vsync( bool_t const on_off ) ;
                virtual natus::application::result swap( void_t ) ;

                virtual natus::graphics::backend_res_t create_backend( void_t ) noexcept ;

            public:

                /// @note a valid window handle must be passed.
                natus::application::result create_context( HWND hwnd ) ;

                /// Returns ok, if the extension is supported, otherwise, this function fails.
                /// @precondition Must be used after context has been created and made current.
                natus::application::result is_extension_supported( natus::ntd::string_cref_t extension_name ) ;

                /// This function fills the incoming list with all wgl extension strings.
                /// @precondition Must be used after context has been created and made current.
                natus::application::result get_wgl_extension( natus::ntd::vector< natus::ntd::string_t >& ext_list ) ;

                /// @precondition Context must be current.
                natus::application::result get_gl_extension( natus::ntd::vector< natus::ntd::string_t >& ext_list ) ;

                /// @precondition Must be used after context has been created and made current. 
                result get_gl_version( natus::application::gl_version& version ) const ;

                /// @precondition Context must be active. For debug purpose. Just clears the screen.
                void_t clear_now( natus::math::vec4f_t const& vec ) ;

            private:

                natus::application::result create_the_context( natus::application::gl_info_cref_t gli ) ;
            };
            natus_res_typedef( context ) ;

            // this is passed to the graphics backend at construction time, so the backend
            // can check for extensions or other context related topics.
            class NATUS_APPLICATION_API gl_context : public natus::graphics::gl_context
            {
                natus_this_typedefs( gl_context ) ;

                friend class natus::application::wgl::context ;

            private:

                // owner
                context_ptr_t _app_context = nullptr ;

                gl_context( context_ptr_t ctx ) noexcept : _app_context( ctx ) {}
                gl_context( this_cref_t ) = delete ;

            public:

                gl_context( this_rref_t rhv ) noexcept
                { 
                    natus_move_member_ptr( _app_context, rhv ) ;
                }

                void_t change_owner( context_ptr_t ctx ) noexcept { _app_context = ctx ; }

            public:

                virtual ~gl_context( void_t ) noexcept {}

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