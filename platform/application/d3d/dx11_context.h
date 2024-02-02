#pragma once

#include "../../api.h"
#include "../../typedefs.h"
#include "../../result.h"

#include "../../graphics/d3d/d3d11_context.h"
#include "../../graphics/d3d/d3d11.h"

#include <motor/application/d3d_info.h>

#include <motor/math/vector/vector4.hpp>

#include <windows.h>
#include <d3d11_1.h>
#include <directxcolors.h>

namespace motor
{
    namespace platform
    {
        namespace directx
        {
            // is the context constructed for the window
            class dx11_context : public motor::platform::d3d11::rendering_context
            {
                motor_this_typedefs( dx11_context ) ;

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

                motor::platform::gen4::d3d11_backend_mtr_t _backend = nullptr ;

            public:

                /// initializes this context but does not create the context.
                /// @see create_context
                dx11_context( void_t ) noexcept ;
                dx11_context( HWND ) noexcept ;
                dx11_context( this_cref_t ) = delete ;
                dx11_context( this_rref_t ) noexcept ;
                ~dx11_context( void_t ) noexcept ;

            public: // operator =

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t ) noexcept ;

            public:

                virtual motor::platform::result activate( void_t ) noexcept ;
                virtual motor::platform::result deactivate( void_t ) noexcept ;
                virtual motor::platform::result vsync( bool_t const on_off ) noexcept ;
                virtual motor::platform::result swap( void_t ) noexcept ;

                motor::graphics::gen4::backend_mtr_safe_t backend( void_t ) noexcept ;
                motor::graphics::gen4::backend_borrow_t::mtr_t borrow_backend( void_t ) noexcept ;

            public:

                /// @note a valid window handle must be passed.
                motor::platform::result create_context( HWND hwnd ) noexcept ;

                /// @precondition Context must be active. For debug purpose. Just clears the screen.
                void_t clear_now( motor::math::vec4f_cref_t vec ) noexcept ;
                void_t clear_depth_stencil( void_t ) noexcept ;
                void_t activate_render_target( void_t ) noexcept ;

            private:

                motor::platform::result create_the_context( motor::application::d3d_info_cref_t gli ) noexcept ;

            private: // rendering context interface.

                virtual ID3D11DeviceContext * ctx( void_t ) noexcept 
                {
                    return _pImmediateContext ;
                }
                virtual ID3D11Device * dev( void_t ) noexcept 
                {
                    return _pd3dDevice ;
                }

                virtual ID3D11Debug * debug( void_t ) noexcept 
                {
                    return _pDebug ;
                }

                virtual void_t activate_framebuffer( void_t ) noexcept 
                {
                    this_t::activate_render_target() ;
                }

                virtual void_t clear_render_target_view( motor::math::vec4f_cref_t color ) noexcept 
                {
                    this_t::clear_now( color ) ;
                }

                virtual void_t clear_depth_stencil_view( void_t ) noexcept 
                {
                    this_t::clear_depth_stencil() ;
                }
            };
            motor_typedef( dx11_context ) ;
        }
    }
}