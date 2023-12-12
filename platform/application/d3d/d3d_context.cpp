#include "d3d_context.h"

#include <natus/graphics/backend/d3d/d3d11.h>

#include <natus/core/assert.h>

#include <natus/ntd/string/split.hpp>

#include <natus/log/global.h>

using namespace natus::application ;
using namespace natus::application::d3d ;

//***********************************************************************
context::context( void_t ) noexcept
{
    _bend_ctx = natus::memory::global_t::alloc( natus::application::d3d::d3d11_context( this ),
        "[context] : backend gl_context" ) ;
}

//***********************************************************************
context::context( HWND hwnd ) noexcept
{
    _bend_ctx = natus::memory::global_t::alloc( natus::application::d3d::d3d11_context( this ),
        "[context] : backend gl_context" ) ;
    this_t::create_context( hwnd ) ;
}

//***********************************************************************
context::context( HWND hwnd, HGLRC ctx ) noexcept
{
    _bend_ctx = natus::memory::global_t::alloc( natus::application::d3d::d3d11_context( this ),
        "[context] : backend gl_context" ) ;

    _hwnd = hwnd ;
}

//***********************************************************************
context::context( this_rref_t rhv ) noexcept
{
    *this = std::move( rhv ) ;
    natus_move_member_ptr( _bend_ctx, rhv ) ;
    _bend_ctx->change_owner( this ) ;
}

//***********************************************************************
context::~context( void_t ) noexcept
{
    this_t::deactivate() ;

    if( _pImmediateContext ) _pImmediateContext->ClearState();

    if( _pRenderTargetView ) _pRenderTargetView->Release();
    if( _pSwapChain1 ) _pSwapChain1->Release();
    if( _pSwapChain ) _pSwapChain->Release();
    if( _pImmediateContext1 ) _pImmediateContext1->Release();
    if( _pImmediateContext ) _pImmediateContext->Release();
    if( _pd3dDevice1 ) _pd3dDevice1->Release();
    if( _pd3dDevice ) _pd3dDevice->Release();
    if( _pDebug ) _pDebug->Release() ;

    natus::memory::global_t::dealloc( _bend_ctx ) ;
}

//***********************************************************************
context::this_ref_t context::operator = ( this_rref_t rhv ) noexcept
{
    _hwnd = rhv._hwnd ;
    rhv._hwnd = NULL ;
    _vsync = rhv._vsync ;

    _driverType = rhv._driverType ;
    _featureLevel = rhv._featureLevel ;

    natus_move_member_ptr( _pd3dDevice, rhv ) ;
    natus_move_member_ptr( _pd3dDevice1, rhv ) ;
    natus_move_member_ptr( _pImmediateContext, rhv ) ;
    natus_move_member_ptr( _pImmediateContext1, rhv ) ;
    natus_move_member_ptr( _pSwapChain, rhv ) ;
    natus_move_member_ptr( _pSwapChain1, rhv ) ;
    natus_move_member_ptr( _pRenderTargetView, rhv ) ;
    natus_move_member_ptr( _pDepthStencil, rhv ) ;
    natus_move_member_ptr( _pDepthStencilView, rhv ) ;
    natus_move_member_ptr( _pDebug, rhv ) ;

    return *this ;
}

//***********************************************************************
natus::application::result context::activate( void_t )
{
    // make current

    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::deactivate( void_t )
{
    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::vsync( bool_t const on_off )
{
    _vsync = on_off ? 1 : 0 ;
    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::swap( void_t )
{
    _pSwapChain->Present( _vsync, 0 );

    {
        RECT rc ;
        GetClientRect( _hwnd, &rc ) ;
        UINT const width = rc.right - rc.left ;
        UINT const height = rc.bottom - rc.top ;

        DXGI_SWAP_CHAIN_DESC desc ;
        _pSwapChain->GetDesc( &desc ) ;

        if( width == 0 || height == 0 ) 
            return natus::application::result::ok ;

        if( desc.BufferDesc.Width != width ||
            desc.BufferDesc.Height != height )
        {
            _pImmediateContext->OMSetRenderTargets( 0, 0, 0 );

            // Release all outstanding references to the swap chain's buffers.
            _pRenderTargetView->Release() ;
            _pDepthStencilView->Release() ;

            HRESULT hr;
            // Preserve the existing buffer count and format.
            // Automatically choose the width and height to match the client rect for HWNDs.
            hr = _pSwapChain->ResizeBuffers( 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0 );
            
            if( FAILED( hr ) )
            {
                natus::log::global_t::error( "D3D11 context resize failed" ) ;
                return natus::application::result::failed_d3d ;
            }

            {
                // Get buffer and create a render-target-view.
                ID3D11Texture2D* pBuffer;
                hr = _pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ),
                    ( void** ) &pBuffer );
                // Perform error handling here!

                hr = _pd3dDevice->CreateRenderTargetView( pBuffer, NULL,
                    &_pRenderTargetView );
                // Perform error handling here!
                pBuffer->Release();
            }

            {
                // Create depth stencil texture
                D3D11_TEXTURE2D_DESC descDepth = { };
                descDepth.Width = width;
                descDepth.Height = height;
                descDepth.MipLevels = 1;
                descDepth.ArraySize = 1;
                descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                descDepth.SampleDesc.Count = 1;
                descDepth.SampleDesc.Quality = 0;
                descDepth.Usage = D3D11_USAGE_DEFAULT;
                descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
                descDepth.CPUAccessFlags = 0;
                descDepth.MiscFlags = 0;
                hr = _pd3dDevice->CreateTexture2D( &descDepth, nullptr, &_pDepthStencil );
                if( FAILED( hr ) ) return natus::application::result::failed_d3d ;

                // Create the depth stencil view
                D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = { };
                descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                descDSV.Texture2D.MipSlice = 0;
                hr = _pd3dDevice->CreateDepthStencilView( _pDepthStencil, &descDSV, &_pDepthStencilView );
            }
            _pImmediateContext->OMSetRenderTargets( 1, &_pRenderTargetView, _pDepthStencilView );

            // Set up the viewport.
            D3D11_VIEWPORT vp;
            vp.Width = FLOAT( width ) ;
            vp.Height = FLOAT( height ) ;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            _pImmediateContext->RSSetViewports( 1, &vp );
        }
    }
    return natus::application::result::ok ;
}

//***********************************************************************
natus::graphics::backend_res_t context::create_backend( void_t ) noexcept 
{
    if( _pd3dDevice != nullptr )
    {
        return natus::graphics::d3d11_backend_res_t(
            natus::graphics::d3d11_backend_t( _bend_ctx ) ) ;
    }

    return natus::graphics::null_backend_res_t(
        natus::graphics::null_backend_t() ) ;
}

//***********************************************************************
natus::application::result context::create_context( HWND hwnd )
{
    _hwnd = hwnd ;

    if( natus::log::global::error( _hwnd == NULL,
        "[context::create_context] : Window handle is no win32 handle." ) )
        return natus::application::result::invalid_argument ;

    return this_t::create_the_context( natus::application::d3d_info_t() ) ;
}


//***********************************************************************
void_t context::clear_now( natus::math::vec4f_t const& vec )
{
    // old: DirectX::Colors::MidnightBlue
    FLOAT color[ 4 ] = { vec.x(), vec.y(), vec.z(), vec.w() } ;
    _pImmediateContext->ClearRenderTargetView( _pRenderTargetView, color ) ;
}

//***********************************************************************
void_t context::clear_depth_stencil( void_t ) 
{
    _pImmediateContext->ClearDepthStencilView( _pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 ) ;
}

//***********************************************************************
void_t context::activate_render_target( void_t ) 
{
    _pImmediateContext->OMSetRenderTargets( 1, &_pRenderTargetView, _pDepthStencilView );
}

//***********************************************************************
natus::application::result context::create_the_context( d3d_info_cref_t gli )
{
    typedef std::chrono::high_resolution_clock local_clock_t ;
    auto t1 = local_clock_t::now() ;

    // here we go...
    // code taken from the official sdk samples
    {
        HRESULT hr = S_OK ;

        RECT rc ;
        GetClientRect( _hwnd, &rc ) ;
        UINT const width = rc.right - rc.left ;
        UINT const height = rc.bottom - rc.top ;

        UINT createDeviceFlags = 0 ;

        #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        D3D_DRIVER_TYPE const driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT const numDriverTypes = ARRAYSIZE( driverTypes ) ;

        D3D_FEATURE_LEVEL const featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT const numFeatureLevels = ARRAYSIZE( featureLevels ) ;

        for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
        {
            _driverType = driverTypes[ driverTypeIndex ];
            hr = D3D11CreateDevice( nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, &_pd3dDevice, &_featureLevel, &_pImmediateContext );

            if( hr == E_INVALIDARG )
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice( nullptr, _driverType, nullptr, createDeviceFlags, &featureLevels[ 1 ], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, &_pd3dDevice, &_featureLevel, &_pImmediateContext );
            }

            if( SUCCEEDED( hr ) )
                break;
        }
        if( FAILED( hr ) ) return natus::application::result::failed_d3d ;

        
        #if _DEBUG
        // Obtain a debug interface
        {
            ID3D11Debug * debug = nullptr ;
            hr = _pd3dDevice->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast< void** >( &debug ) );
            if( FAILED( hr ) ) 
            {
                natus::log::global_t::error("[D3D11] : Can not query debug interface.") ;
            }
            _pDebug = debug ;
        }
        #endif

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        IDXGIFactory1* dxgiFactory = nullptr;
        {
            IDXGIDevice* dxgiDevice = nullptr;
            hr = _pd3dDevice->QueryInterface( __uuidof( IDXGIDevice ), reinterpret_cast< void** >( &dxgiDevice ) );
            if( SUCCEEDED( hr ) )
            {
                IDXGIAdapter* adapter = nullptr;
                hr = dxgiDevice->GetAdapter( &adapter );
                if( SUCCEEDED( hr ) )
                {
                    hr = adapter->GetParent( __uuidof( IDXGIFactory1 ), reinterpret_cast< void** >( &dxgiFactory ) );
                    adapter->Release();
                }
                dxgiDevice->Release();
            }
        }
        if( FAILED( hr ) ) return natus::application::result::failed_d3d ;

        // Create swap chain
        IDXGIFactory2* dxgiFactory2 = nullptr;
        hr = dxgiFactory->QueryInterface( __uuidof( IDXGIFactory2 ), reinterpret_cast< void** >( &dxgiFactory2 ) );
        if( dxgiFactory2 )
        {
            // DirectX 11.1 or later
            hr = _pd3dDevice->QueryInterface( __uuidof( ID3D11Device1 ), reinterpret_cast< void** >( &_pd3dDevice1 ) );
            if( SUCCEEDED( hr ) )
            {
                ( void ) _pImmediateContext->QueryInterface( __uuidof( ID3D11DeviceContext1 ), reinterpret_cast< void** >( &_pImmediateContext1 ) );
            }

            DXGI_SWAP_CHAIN_DESC1 sd = { };
            sd.Width = width;
            sd.Height = height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;

            hr = dxgiFactory2->CreateSwapChainForHwnd( _pd3dDevice, _hwnd, &sd, nullptr, nullptr, &_pSwapChain1 );
            if( SUCCEEDED( hr ) )
            {
                hr = _pSwapChain1->QueryInterface( __uuidof( IDXGISwapChain ), reinterpret_cast< void** >( &_pSwapChain ) );
            }

            dxgiFactory2->Release();
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = { };
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = _hwnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            hr = dxgiFactory->CreateSwapChain( _pd3dDevice, &sd, &_pSwapChain );
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation( _hwnd, DXGI_MWA_NO_ALT_ENTER );

        dxgiFactory->Release();

        if( FAILED( hr ) ) return natus::application::result::failed_d3d ;

        // Create a render target view
        ID3D11Texture2D* pBackBuffer = nullptr;
        hr = _pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &pBackBuffer ) );
        if( FAILED( hr ) )
            return natus::application::result::failed_d3d ;

        hr = _pd3dDevice->CreateRenderTargetView( pBackBuffer, nullptr, &_pRenderTargetView );
        pBackBuffer->Release();
        if( FAILED( hr ) )
            return natus::application::result::failed_d3d ;

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth = { };
        descDepth.Width = width;
        descDepth.Height = height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        hr = _pd3dDevice->CreateTexture2D( &descDepth, nullptr, &_pDepthStencil );
        if( FAILED( hr ) ) return natus::application::result::failed_d3d ;

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = { };
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = _pd3dDevice->CreateDepthStencilView( _pDepthStencil, &descDSV, &_pDepthStencilView );
        if( FAILED( hr ) ) return natus::application::result::failed_d3d ;

        _pImmediateContext->OMSetRenderTargets( 1, &_pRenderTargetView, _pDepthStencilView );

        // Setup the viewport
        D3D11_VIEWPORT vp;
        vp.Width = ( FLOAT ) width;
        vp.Height = ( FLOAT ) height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        _pImmediateContext->RSSetViewports( 1, &vp );
    }


    {
        this_t::activate() ;

        natus::log::global::warning( natus::application::no_success( this_t::vsync( gli.vsync_enabled ) ),
            "[context::create_the_context] : vsync setting failed." ) ;

        this_t::deactivate() ;
    }

    // timing end
    {
        size_t const milli = size_t( ::std::chrono::duration_cast< ::std::chrono::milliseconds >(
            local_clock_t::now() - t1 ).count() ) ;

        natus::log::global::status( natus_log_fn( "created [" + ::std::to_string( milli ) + " ms]" ) ) ;
    }

    return natus::application::result::ok ;
}
