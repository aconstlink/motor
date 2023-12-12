#pragma once

#include "../d3d_info.h"

#include "../gfx_context.h"

#include <natus/graphics/backend/d3d/d3d11_context.h>
#include <natus/graphics/backend/null/null.h>

#include <natus/memory/res.hpp>
#include <natus/math/vector/vector4.hpp>

#include <windows.h>
#include <d3d11_1.h>
#include <directxcolors.h>

namespace natus
{
    namespace application
    {
        namespace d3d
        {
            class d3d11_context ;
            natus_class_proto_typedefs( d3d11_context ) ;

            class NATUS_APPLICATION_API context : public gfx_context
            {
                friend class d3d11_context ;

                natus_this_typedefs( context ) ;

            private:

                HWND _hwnd = NULL ;
                UINT _vsync = 1 ;

                D3D_DRIVER_TYPE _driverType = D3D_DRIVER_TYPE_NULL;
                D3D_FEATURE_LEVEL _featureLevel = D3D_FEATURE_LEVEL_11_0;
                ID3D11Device * _pd3dDevice = nullptr;
                ID3D11Device1 * _pd3dDevice1 = nullptr;
                ID3D11DeviceContext * _pImmediateContext = nullptr;
                ID3D11DeviceContext1 * _pImmediateContext1 = nullptr;
                IDXGISwapChain* _pSwapChain = nullptr;
                IDXGISwapChain1* _pSwapChain1 = nullptr;
                ID3D11RenderTargetView* _pRenderTargetView = nullptr;
                ID3D11Texture2D* _pDepthStencil = nullptr;
                ID3D11DepthStencilView* _pDepthStencilView = nullptr;
                ID3D11Debug * _pDebug = nullptr ;

                d3d11_context_ptr_t _bend_ctx = nullptr ;

            public:

                /// initializes this context but does not create the context.
                /// @see create_context
                context( void_t ) noexcept ;
                context( HWND ) noexcept ;
                context( this_cref_t ) = delete ;
                context( this_rref_t ) noexcept ;
                ~context( void_t ) noexcept ;

            private:

                context( HWND hwnd, HGLRC ctx ) noexcept ;

            public: // operator =

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) noexcept ;

            public:

                virtual natus::application::result activate( void_t ) ;
                virtual natus::application::result deactivate( void_t ) ;
                virtual natus::application::result vsync( bool_t const on_off ) ;
                virtual natus::application::result swap( void_t ) ;

                virtual natus::graphics::backend_res_t create_backend( void_t ) noexcept ;

            public:

                /// @note a valid window handle must be passed.
                natus::application::result create_context( HWND hwnd ) ;

                /// @precondition Context must be active. For debug purpose. Just clears the screen.
                void_t clear_now( natus::math::vec4f_t const& vec ) ;
                void_t clear_depth_stencil( void_t ) ;
                void_t activate_render_target( void_t ) ;

            private:

                natus::application::result create_the_context( natus::application::d3d_info_cref_t gli ) ;
            };
            natus_typedef( context ) ;
            typedef natus::memory::res_t< context_t > context_res_t ;

            class NATUS_APPLICATION_API d3d11_context : public natus::graphics::d3d11_context
            {
                natus_this_typedefs( d3d11_context ) ;

                friend class natus::application::d3d::context ;

            private:

                // owner
                context_ptr_t _app_context = nullptr ;

                d3d11_context( context_ptr_t ctx ) noexcept : _app_context( ctx ) {}
                d3d11_context( this_cref_t ) = delete ;

            public:

                d3d11_context( this_rref_t rhv ) noexcept
                {
                    natus_move_member_ptr( _app_context, rhv ) ;
                }

                void_t change_owner( context_ptr_t ctx ) noexcept { _app_context = ctx ; }

            public:

                virtual ~d3d11_context( void_t ) noexcept {}

            public:


                virtual ID3D11DeviceContext * ctx( void_t ) noexcept 
                {
                    return _app_context->_pImmediateContext ;
                }

                virtual ID3D11Device * dev( void_t ) noexcept 
                {
                    return _app_context->_pd3dDevice ;
                }

                virtual ID3D11Debug * debug( void_t ) noexcept 
                {
                    return _app_context->_pDebug ;
                }

                virtual void_t activate_framebuffer( void_t ) noexcept 
                {
                    return _app_context->activate_render_target() ;
                }

                // clear the backbuffer
                virtual void_t clear_render_target_view( natus::math::vec4f_cref_t color ) noexcept
                {
                    _app_context->clear_now( color ) ;
                }

                virtual void_t clear_depth_stencil_view( void_t ) noexcept 
                {
                    _app_context->clear_depth_stencil() ;
                }
            };

        }
    }
}