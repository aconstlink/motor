// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "d3d11.h"
#include "d3d11_convert.h"
#include "d3d11_independent.h"

#include "../datas.hpp"

#include <motor/msl/symbol.hpp>
#include <motor/msl/database.hpp>
#include <motor/msl/parser.h>
#include <motor/msl/dependency_resolver.hpp>
#include <motor/msl/generators/generator.h>
#include <motor/msl/generators/hlsl5_generator.h>

#include <motor/graphics/buffer/vertex_buffer.hpp>
#include <motor/graphics/buffer/index_buffer.hpp>
#include <motor/graphics/texture/image.hpp>
#include <motor/graphics/shader/msl_bridge.hpp>

#include <motor/profiling/probe_guard.hpp>

#include <motor/concurrent/mrsw.hpp>
#include <motor/concurrent/task/task.hpp>
#include <motor/concurrent/global.h>

#include <motor/memory/global.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/std/vector>
#include <motor/std/string>
#include <motor/std/string_split.hpp>
#include <motor/std/stack>
#include <motor/log/global.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <directxcolors.h>
#include <cstdio>

#include "d3d11_pimpl_data.h"
#include "d3d11_support_thread.hpp"

using namespace motor::platform;
using namespace motor::platform::gen4;

struct d3d11_backend::pimpl
{
    motor_this_typedefs( d3d11_backend::pimpl );

    //*******************************************************************************************
    template < typename T >
    using datas = motor::platform::datas< T >;

    using streamout_datas_t = datas< d3d11_pimpl_data::so_data_t >;
    streamout_datas_t _streamouts;
    size_t _cur_streamout_active = size_t( -1 );

    using geometry_datas_t = datas< d3d11_pimpl_data::geo_data_t >;
    geometry_datas_t _geos;

    using shader_datas_t = datas< d3d11_pimpl_data::shader_data_t >;
    shader_datas_t _shaders;

    using image_datas_t = datas< d3d11_pimpl_data::image_data_t >;
    image_datas_t _images;

    using array_datas_t = datas< d3d11_pimpl_data::array_data_t >;
    array_datas_t _arrays;

    using render_datas_t = datas< d3d11_pimpl_data::render_data_t >;
    render_datas_t _renders;

    using msl_datas_t = datas< d3d11_pimpl_data::msl_data_t >;
    msl_datas_t _msls;

    motor::msl::database_t _mdb;

  public: // framebuffer

    using framebuffers_t = datas< d3d11_pimpl_data::framebuffer_data_t >;
    framebuffers_t _framebuffers;
    size_t _cur_fb_active = size_t( -1 );

  public: // other variables

    motor::graphics::gen4::backend_type const bt = motor::graphics::gen4::backend_type::d3d11;
    motor::graphics::shader_api_type const sapi = motor::graphics::shader_api_type::hlsl_5_0;
    motor::platform::d3d11::rendering_context_mtr_t _ctx;

    using states_t = datas< d3d11_pimpl_data::state_data_t >;
    states_t _states;

    motor::stack< d3d11_pimpl_data::render_state_sets, 10 > _state_stack;

    FLOAT vp_width = FLOAT( 0 );
    FLOAT vp_height = FLOAT( 0 );

    // set by the backend using this pimpl
    size_t _bid = size_t( -1 );

  public: // support thread

    using shared_data_t = d3d11_detail::support_thread< this_t >::shared_data_t;
    shared_data_t * _ctsd = nullptr;

    d3d11_detail::support_thread< this_t > _support_thread;

  public: // functions

    //************************************************************************************************************
    pimpl( size_t const bid, motor::platform::d3d11::rendering_context_mtr_t ctx ) noexcept
        : _bid( bid )
    {
        _ctx = ctx;

        {
            motor::graphics::state_object_t obj( "d3d11_default_states" );

            auto new_states = motor::graphics::gen4::backend_t::default_render_states();

            new_states.view_s.do_change = true;
            new_states.view_s.ss.do_activate = true;
            new_states.scissor_s.do_change = true;
            new_states.scissor_s.ss.do_activate = false;
            new_states.blend_s.do_change = true;
            new_states.blend_s.ss.do_activate = false;
            new_states.polygon_s.do_change = true;
            new_states.polygon_s.ss.do_activate = true;
            new_states.depth_s.do_change = true;

            obj.add_render_state_set( new_states );

            bool_t const res = this->construct_state( obj );
            assert( res );
        }

        _ctsd = _support_thread.start_support_thread( this, ctx );
    }

    //************************************************************************************************************
    // @todo is the move required? pimpl is only used via
    // pointer movement.
    pimpl( pimpl && rhv ) noexcept : _support_thread( std::move( rhv._support_thread ) )
    {
        _ctsd = motor::move( rhv._ctsd );
        _ctsd->owner = this;

        motor_move_member_ptr( _ctx, rhv );
        _geos = std::move( rhv._geos );
        _images = std::move( rhv._images );
        _shaders = std::move( rhv._shaders );
        _arrays = std::move( rhv._arrays );
        _renders = std::move( rhv._renders );

        _msls = std::move( rhv._msls );
        _state_stack = std::move( rhv._state_stack );
        _states = std::move( rhv._states );
        _cur_fb_active = rhv._cur_fb_active;

        _bid = rhv._bid;
    }

    //************************************************************************************************************
    ~pimpl( void_t )
    {
        _support_thread.stop_support_thread();

        _geos.invalidate_and_clear();
        _shaders.invalidate_and_clear();
        _renders.invalidate_and_clear();
        _images.invalidate_and_clear();
        _arrays.invalidate_and_clear();
        _framebuffers.invalidate_and_clear();
        _streamouts.invalidate_and_clear();
        _msls.invalidate_and_clear();

        _states.invalidate_and_clear();

        _cur_fb_active = size_t( -1 );
    }

    //************************************************************************************************************
    bool_t construct_state( motor::graphics::state_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid );
        auto const res =
            _states.access( oid, obj.name(), [ & ]( d3d11_pimpl_data::state_data_ref_t states )
        {
            states.states.resize( obj.size() );

            // @note
            // set all enables. Enable/Disable only possible during construction
            // values are assigned in the update function for the render states
            obj.for_each( [ & ]( size_t const i, motor::graphics::render_state_sets_cref_t rs )
            { states.states[ i ] = rs; } );

            return true;
        } );

        if( !res ) return false;

        obj.set_oid( _bid, oid );
        return true;
    }

    //************************************************************************************************************
    void_t update_state( size_t const oid, motor::graphics::state_object_ref_t obj ) noexcept
    {
        assert( oid < _states.size() );

        if( !obj.check_and_reset_changed( _bid ) ) return;

        auto const res = _states.access( oid, [ & ]( d3d11_pimpl_data::state_data_ref_t sd )
        {
            obj.for_each( [ & ]( size_t const i, motor::graphics::render_state_sets_cref_t rs )
            {
                if( i >= sd.states.size() ) return;
                sd.states[ i ] = rs;
            } );
            return true;
        } );
    }

    //************************************************************************************************************
    void_t release_state( size_t const oid ) noexcept {}

    //************************************************************************************************************
    void_t handle_render_state(
        d3d11_pimpl_data::render_state_sets & incoming_states, bool_t const popped )
    {
        //  viewport
        if( incoming_states.rss.view_s.do_change )
        {
            if( incoming_states.rss.view_s.ss.do_activate )
            {
                auto const & vp_ = incoming_states.rss.view_s.ss.vp;

                // Setup the viewport
                D3D11_VIEWPORT vp;
                vp.Width = FLOAT( vp_.z() );
                vp.Height = FLOAT( vp_.w() );
                vp.MinDepth = 0.0f;
                vp.MaxDepth = 1.0f;
                vp.TopLeftX = FLOAT( vp_.x() );
                vp.TopLeftY = FLOAT( vp_.y() );

                // this solution here only works because
                // - framebuffers do not stack. They must be used and unused.
                // - render states targeting a framebuffer must be activated
                //   within the use/unuse functions by the user.
                if( _cur_fb_active == size_t( -1 ) || popped )
                    vp.TopLeftY = vp_height - vp.Height - vp.TopLeftY;
                else
                {
                    _framebuffers.access( _cur_fb_active,
                        [ & ]( d3d11_pimpl_data::framebuffer_data_ref_t fbd )
                    { vp.TopLeftY = fbd.height - vp.Height - vp.TopLeftY; } );
                }

                _ctx->ctx()->RSSetViewports( 1, &vp );
            }
        }

        if( incoming_states.rss.clear_s.do_change && incoming_states.rss.clear_s.ss.do_activate &&
            !popped )
        {
            if( _cur_fb_active == size_t( -1 ) )
            {
                bool_t const clear_color = incoming_states.rss.clear_s.ss.do_color_clear;
                bool_t const clear_depth = incoming_states.rss.clear_s.ss.do_depth_clear;

                if( clear_color )
                {
                    motor::math::vec4f_t const color = incoming_states.rss.clear_s.ss.clear_color;
                    _ctx->clear_render_target_view( color );
                }

                if( clear_depth )
                {
                    _ctx->clear_depth_stencil_view();
                }
            }
            else
            {
                _framebuffers.access( _cur_fb_active,
                    [ & ]( d3d11_pimpl_data::framebuffer_data_ref_t fb )
                {
                    bool_t const clear_color = incoming_states.rss.clear_s.ss.do_color_clear;
                    bool_t const clear_depth = incoming_states.rss.clear_s.ss.do_depth_clear;

                    if( clear_color )
                    {
                        motor::math::vec4f_t const color =
                            incoming_states.rss.clear_s.ss.clear_color;
                        FLOAT const dxcolor[ 4 ] = { color.x(), color.y(), color.z(), color.w() };
                        for( size_t i = 0; i < fb.num_color; ++i )
                            _ctx->ctx()->ClearRenderTargetView( fb.rt_view[ i ], dxcolor );
                    }

                    if( clear_depth && fb.ds_view != nullptr )
                    {
                        _ctx->ctx()->ClearDepthStencilView(
                            fb.ds_view, D3D11_CLEAR_DEPTH, 1.0f, 0 );
                    }
                } );
            }
        }

        if( incoming_states.rss.depth_s.do_change )
        {
            if( popped )
            {
                assert( incoming_states.depth_stencil_state != nullptr );

                _ctx->ctx()->OMSetDepthStencilState( incoming_states.depth_stencil_state, 0 );

                incoming_states.depth_stencil_state->Release();
                incoming_states.depth_stencil_state = nullptr;
            }
            else
            {
                assert( incoming_states.depth_stencil_state == nullptr );

                D3D11_DEPTH_STENCIL_DESC desc = {};
                desc.DepthEnable = incoming_states.rss.depth_s.ss.do_activate ? TRUE : FALSE;
                desc.DepthFunc = motor::platform::d3d11::convert_depth_func(
                    incoming_states.rss.depth_s.ss.compare_funk );
                desc.DepthWriteMask = incoming_states.rss.depth_s.ss.do_depth_write
                                          ? D3D11_DEPTH_WRITE_MASK_ALL
                                          : D3D11_DEPTH_WRITE_MASK_ZERO;

                auto const res = _ctx->dev()->CreateDepthStencilState(
                    &desc, &incoming_states.depth_stencil_state );
                if( FAILED( res ) )
                {
                    motor::log::global_t::error( "CreateDepthStencilState" );
                }

                _ctx->ctx()->OMSetDepthStencilState( incoming_states.depth_stencil_state, 0 );
            }
        }

        // blend
        {
            if( popped )
            {
                assert( incoming_states.blend_state != nullptr );

                _ctx->ctx()->OMSetBlendState( incoming_states.blend_state, 0, 0xffffffff );

                incoming_states.blend_state->Release();
                incoming_states.blend_state = nullptr;
            }
            else
            {
                assert( incoming_states.blend_state == nullptr );

                D3D11_BLEND_DESC desc = {};

                desc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
                desc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                desc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;
                desc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
                desc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

                if( incoming_states.rss.blend_s.do_change )
                {
                    desc.RenderTarget[ 0 ].BlendEnable = incoming_states.rss.blend_s.ss.do_activate;

                    desc.RenderTarget[ 0 ].SrcBlend = motor::platform::d3d11::convert(
                        incoming_states.rss.blend_s.ss.src_blend_factor );
                    desc.RenderTarget[ 0 ].DestBlend = motor::platform::d3d11::convert(
                        incoming_states.rss.blend_s.ss.dst_blend_factor );
                    desc.RenderTarget[ 0 ].BlendOp = motor::platform::d3d11::convert(
                        incoming_states.rss.blend_s.ss.blend_func );

                    desc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;
                    desc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
                    desc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;

                    auto const res =
                        _ctx->dev()->CreateBlendState( &desc, &incoming_states.blend_state );
                    if( SUCCEEDED( res ) )
                    {
                        _ctx->ctx()->OMSetBlendState( incoming_states.blend_state, 0, 0xffffffff );
                    }
                }
            }
        }
        // rasterizer state conglomerate
        {
            if( popped )
            {
                assert( incoming_states.raster_state != nullptr );

                _ctx->ctx()->RSSetState( incoming_states.raster_state );

                incoming_states.raster_state->Release();
                incoming_states.raster_state = nullptr;
            }
            else
            {
                assert( incoming_states.raster_state == nullptr );

                D3D11_RASTERIZER_DESC raster_desc = {};

                if( incoming_states.rss.polygon_s.do_change )
                {
                    if( incoming_states.rss.polygon_s.ss.do_activate )
                        raster_desc.CullMode =
                            motor::platform::d3d11::convert( incoming_states.rss.polygon_s.ss.cm );
                    else
                        raster_desc.CullMode = D3D11_CULL_NONE;

                    raster_desc.FillMode =
                        motor::platform::d3d11::convert( incoming_states.rss.polygon_s.ss.fm );

                    raster_desc.FrontCounterClockwise =
                        incoming_states.rss.polygon_s.ss.ff ==
                        motor::graphics::front_face::counter_clock_wise;
                }

                if( incoming_states.rss.scissor_s.do_change )
                {
                    raster_desc.ScissorEnable = incoming_states.rss.scissor_s.ss.do_activate;
                    if( incoming_states.rss.scissor_s.ss.do_activate )
                    {
                        LONG const h = _state_stack.top().rss.view_s.ss.vp.w();

                        D3D11_RECT rect;
                        rect.left = incoming_states.rss.scissor_s.ss.rect.x();
                        rect.right = incoming_states.rss.scissor_s.ss.rect.x() +
                                     incoming_states.rss.scissor_s.ss.rect.z();
                        rect.top = h - ( incoming_states.rss.scissor_s.ss.rect.y() +
                                           incoming_states.rss.scissor_s.ss.rect.w() );
                        rect.bottom = rect.top + incoming_states.rss.scissor_s.ss.rect.w();

                        _ctx->ctx()->RSSetScissorRects( 1, &rect );
                    }
                }
                if( incoming_states.raster_state != nullptr )
                {
                    incoming_states.raster_state->Release();
                    incoming_states.raster_state = nullptr;
                }

                auto const res = _ctx->dev()->CreateRasterizerState(
                    &raster_desc, &incoming_states.raster_state );
                if( FAILED( res ) )
                {
                    motor::log::global_t::error( "CreateRasterizerState" );
                }

                _ctx->ctx()->RSSetState( incoming_states.raster_state );
            }
        }
    }

    //************************************************************************************************************
    // if oid == -1, the state is popped.
    void_t handle_render_state( size_t const oid, size_t const rs_id ) noexcept
    {
        auto new_id = std::make_pair( oid, rs_id );

        // pop state
        if( oid == size_t( -1 ) )
        {
            if( _state_stack.size() == 1 )
            {
                motor::log::global_t::error( d3d11_backend_log( "no more render states to pop" ) );
                return;
            }
            auto old = _state_stack.pop();

            // undo render state effects
            {
                old.rss = old.rss - _state_stack.top().rss;
                this->handle_render_state( old, true );
            }
        }
        else
        {
            d3d11_pimpl_data::render_state_sets rss;
            _states.access( new_id.first, [ & ]( d3d11_pimpl_data::state_data_ref_t sd )
            { rss.rss = _state_stack.top().rss + sd.states[ new_id.second ]; } );
            this->handle_render_state( rss, false );
            _state_stack.push( rss );
        }
    }

    //************************************************************************************************************
    bool_t construct_framebuffer( motor::graphics::framebuffer_object_ref_t obj ) noexcept
    {
        auto fbd = _framebuffers.access( obj.get_oid( _bid ), obj.name() );
        if( !fbd )
        {
            motor::log::global_t::error< 1024 >(
                "[d3d11] : invalid framebuffer %s", obj.name().c_str() );
            return false;
        }

        ID3D11Device * dev = _ctx->dev();

        // release all framebuffer resources
        {
            for( size_t i = 0; i < d3d11_pimpl_data::framebuffer_data_t::max_rt_views; ++i )
            {
                auto *& view = fbd->rt_view[ i ];
                if( view != nullptr )
                {
                    view->Release();
                    view = nullptr;
                }
            }

            // release images
            for( size_t i = 0; i < d3d11_pimpl_data::framebuffer_data_t::max_images; ++i )
            {
                if( auto imgd = _images.access( fbd->image_ids[ i ] ); imgd )
                {
                    //
                    // could just call img.invalidate() i guess
                    //
                    imgd->view->Release();
                    imgd->texture->Release();
                    imgd->sampler->Release();

                    imgd->view = nullptr;
                    imgd->texture = nullptr;
                    imgd->sampler = nullptr;
                }
            }
        }

        // colors
        {
#if 0
            for( size_t i = 0; i < fbd->num_color; ++i )
            {
                auto *& view = fbd->rt_view[ i ];
                if( view != nullptr )
                {
                    view->Release();
                    view = nullptr;
                }
            }

            // release images
            for( size_t i = 0; i < fbd->num_color; ++i )
            {
                if( fbd->image_ids[ i ] != size_t( -1 ) )
                {
                    if( auto imgd = _images.access( fbd->image_ids[ i ] ); imgd )
                    {
                        //
                        // could just call img.invalidate() i guess
                        //
                        imgd->view->Release();
                        imgd->texture->Release();
                        imgd->sampler->Release();

                        imgd->view = nullptr;
                        imgd->texture = nullptr;
                        imgd->sampler = nullptr;
                    }
                }
            }
#endif

            size_t const nt = obj.get_num_color_targets();
            auto const ctt = obj.get_color_target();
            auto const dst = obj.get_depth_target();

            for( size_t i = 0; i < nt; ++i )
            {
                // sampler
                d3d11_pimpl_data::guard< ID3D11SamplerState > smp;
                {
                    D3D11_SAMPLER_DESC sampDesc = {};
                    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
                    sampDesc.MinLOD = 0;
                    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

                    auto const hr = dev->CreateSamplerState( &sampDesc, smp );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "CreateSamplerState" ) );
                        continue;
                    }
                }

                // texture
                d3d11_pimpl_data::guard< ID3D11Texture2D > tex;
                {
                    auto const dims = obj.get_dims();
                    size_t const width = UINT( dims.x() );
                    size_t const height = UINT( dims.y() );

                    D3D11_TEXTURE2D_DESC desc = {};
                    desc.Width = static_cast< UINT >( width );
                    desc.Height = static_cast< UINT >( height );
                    desc.MipLevels = static_cast< UINT >( 1 );
                    desc.ArraySize = static_cast< UINT >( 1 );
                    desc.Format = motor::platform::d3d11::convert( ctt );
                    desc.SampleDesc.Count = 1;
                    desc.SampleDesc.Quality = 0;
                    desc.Usage = D3D11_USAGE_DEFAULT;
                    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
                    desc.CPUAccessFlags = 0;
                    desc.MiscFlags = 0;

                    // create the texture object
                    {
                        auto const hr = dev->CreateTexture2D( &desc, nullptr, tex );
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateTexture2D" ) );
                            continue;
                        }
                    }
                }

                // shader resource view
                d3d11_pimpl_data::guard< ID3D11ShaderResourceView > srv;
                {
                    D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
                    res_desc.Format = motor::platform::d3d11::convert( ctt );
                    res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    res_desc.Texture2D.MostDetailedMip = 0;
                    res_desc.Texture2D.MipLevels = UINT( 1 );

                    auto const hr = dev->CreateShaderResourceView( tex, &res_desc, srv );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log(
                            "CreateShaderResourceView for texture : [" + obj.name() + "]" ) );
                        continue;
                    }
                }

                // render target view
                d3d11_pimpl_data::guard< ID3D11RenderTargetView > view;
                {
                    D3D11_RENDER_TARGET_VIEW_DESC desc = {};
                    desc.Format = motor::platform::d3d11::convert( ctt );
                    desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = 0;

                    auto const res = _ctx->dev()->CreateRenderTargetView( tex, &desc, view );
                    if( FAILED( res ) )
                    {
                        motor::log::global_t::error(
                            d3d11_backend_log( "CreateRenderTargetView" ) );
                        continue;
                    }
                }

                // store data
                {
                    motor::string_t const name = obj.name() + "." + motor::to_string( i );

                    if( auto imgd = _images.access( fbd->image_ids[ i ], name ); imgd )
                    {
                        imgd->sampler = smp.move_out();
                        imgd->texture = tex.move_out();
                        imgd->view = srv.move_out();
                        imgd->requires_y_flip = 1.0f;

                        fbd->image_ids[ i ] = imgd.oid();
                    }

                    fbd->rt_view[ i ] = view.move_out();
                }
            }

            {
                auto const dims = obj.get_dims();
                fbd->width = FLOAT( dims.x() );
                fbd->height = FLOAT( dims.y() );
            }
            fbd->num_color = nt;
        }

        // depth stencil
        {
            if( fbd->ds_view != nullptr )
            {
                fbd->ds_view->Release();
                fbd->ds_view = nullptr;
            }
            #if 0
            // release images
            {
                size_t img_id = fbd->image_ids[ fbd->num_color ];
                if( auto imgd = image_datas_t::handle_t( img_id, _images ); imgd )
                {
                    //
                    // could just call img.invalidate() i guess
                    //
                    imgd->view->Release();
                    imgd->texture->Release();
                    imgd->sampler->Release();

                    imgd->view = nullptr;
                    imgd->texture = nullptr;
                    imgd->sampler = nullptr;
                }
            }
            #endif
            auto const dst = obj.get_depth_target();

            if( dst != motor::graphics::depth_stencil_target_type::unknown )
            {
                // sampler
                d3d11_pimpl_data::guard< ID3D11SamplerState > smp;
                {
                    D3D11_SAMPLER_DESC sampDesc = {};
                    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
                    sampDesc.MinLOD = 0;
                    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

                    auto const hr = dev->CreateSamplerState( &sampDesc, smp );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "CreateSamplerState" ) );
                        return false;
                    }
                }

                // texture
                d3d11_pimpl_data::guard< ID3D11Texture2D > tex;
                {
                    auto const dims = obj.get_dims();
                    size_t const width = UINT( dims.x() );
                    size_t const height = UINT( dims.y() );

                    D3D11_TEXTURE2D_DESC desc = {};
                    desc.Width = static_cast< UINT >( width );
                    desc.Height = static_cast< UINT >( height );
                    desc.MipLevels = static_cast< UINT >( 1 );
                    desc.ArraySize = static_cast< UINT >( 1 );
                    desc.Format = motor::platform::d3d11::convert_texture( dst );
                    desc.SampleDesc.Count = 1;
                    desc.SampleDesc.Quality = 0;
                    desc.Usage = D3D11_USAGE_DEFAULT;
                    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
                    desc.CPUAccessFlags = 0;
                    desc.MiscFlags = 0;

                    // create the texture object
                    {
                        auto const hr = dev->CreateTexture2D( &desc, nullptr, tex );
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateTexture2D" ) );
                            return false;
                        }
                    }
                }

                // shader resource view
                d3d11_pimpl_data::guard< ID3D11ShaderResourceView > srv;
                {
                    D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
                    res_desc.Format = motor::platform::d3d11::convert_shader_resource( dst );
                    res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    res_desc.Texture2D.MostDetailedMip = 0;
                    res_desc.Texture2D.MipLevels = UINT( 1 );

                    auto const hr = dev->CreateShaderResourceView( tex, &res_desc, srv );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log(
                            "CreateShaderResourceView for texture : [" + obj.name() + "]" ) );
                        return false;
                    }
                }

                // render target view
                d3d11_pimpl_data::guard< ID3D11DepthStencilView > view;
                {
                    D3D11_DEPTH_STENCIL_VIEW_DESC desc = {};
                    desc.Format = motor::platform::d3d11::convert_depth_stencil_view( dst );
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                    desc.Texture2D.MipSlice = 0;

                    auto const res = _ctx->dev()->CreateDepthStencilView( tex, &desc, view );
                    if( FAILED( res ) )
                    {
                        motor::log::global_t::error(
                            d3d11_backend_log( "CreateDepthStencilView" ) );
                        return false;
                    }
                }

                // store data
                {
                    size_t const i = obj.get_num_color_targets();
                    motor::string_t const name = obj.name() + ".depth";

                    if( auto imgd = _images.access( fbd->image_ids[ i ], name ); imgd )
                    {
                        imgd->sampler = smp.move_out();
                        imgd->texture = tex.move_out();
                        imgd->view = srv.move_out();
                        imgd->requires_y_flip = 1.0f;

                        fbd->image_ids[ i ] = imgd.oid();
                    }

                    {
                        auto const dims = obj.get_dims();
                        fbd->width = FLOAT( dims.x() );
                        fbd->height = FLOAT( dims.y() );
                    }

                    fbd->ds_view = view.move_out();
                }
            }
        }

        obj.set_oid( _bid, fbd.oid() );

        return true;
    }

    //************************************************************************************************************
    void_t release_framebuffer( size_t const oid ) noexcept
    {
        // auto & fb = _framebuffers[ oid ] ;
        // fb.invalidate() ;

        _framebuffers.invalidate( oid );
    }

    //************************************************************************************************************
    bool_t activate_framebuffer( size_t const oid ) noexcept
    {
        if( auto fbd = framebuffers_t::handle_t( oid, _framebuffers ); fbd )
        {
            _ctx->ctx()->OMSetRenderTargets( UINT( fbd->num_color ), fbd->rt_view, fbd->ds_view );
        }

        _cur_fb_active = oid;

        return true;
    }

    //************************************************************************************************************
    void_t deactivate_framebuffer( void_t )
    {
        _ctx->ctx()->OMSetRenderTargets( 0, nullptr, nullptr );
        _ctx->activate_framebuffer();

        // done by the render states

#if 0
        // Setup the viewport
        D3D11_VIEWPORT vp ;
        vp.Width = vp_width ;
        vp.Height = vp_height ;
        vp.MinDepth = 0.0f ;
        vp.MaxDepth = 1.0f ;
        vp.TopLeftX = FLOAT( 0 ) ;
        vp.TopLeftY = FLOAT( 0 ) ;
        _ctx->ctx()->RSSetViewports( 1, &vp );
#endif

        _cur_fb_active = size_t( -1 );
    }

    //************************************************************************************************************
    size_t construct_streamout( size_t oid, motor::graphics::streamout_object_ref_t obj ) noexcept
    {
        auto sod = _streamouts.access( oid, obj.name() );

        // capture the vertex layout for stream out
        if( obj.num_buffers() > 0 )
        {
            obj.get_buffer( 0 ).for_each_layout_element(
                [ & ]( motor::graphics::vertex_buffer_t::data_cref_t d )
            {
                d3d11_pimpl_data::so_data_t::layout_element le;
                le.va = d.va;
                le.type = d.type;
                le.type_struct = d.type_struct;
                sod->elements.push_back( le );
            } );

            sod->stride = UINT( obj.get_buffer( 0 ).get_layout_sib() );
        }

        // create buffers
        {
            obj.for_each( [ & ]( size_t const i, motor::graphics::vertex_buffer_cref_t vb )
            {
                if( i >= d3d11_pimpl_data::so_data_t::max_buffers ) return;

                size_t const byte_width = obj.size() * obj.get_buffer( i ).get_layout_sib();

                // create buffer
                {
                    D3D11_BUFFER_DESC bd = {};
                    bd.ByteWidth = UINT( byte_width );
                    bd.Usage = D3D11_USAGE_DEFAULT;
                    bd.BindFlags = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER |
                                   D3D10_BIND_SHADER_RESOURCE;

                    for( size_t j = 0; j < 2; ++j )
                    {
                        auto & b = sod->_ping_pong[ j ].buffers[ i ].invalidate();

                        HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, NULL, b );
                        motor::log::global_t::error( FAILED( hr ),
                            d3d11_backend_log( "CreateBuffer - D3D11_BIND_STREAM_OUTPUT" ) );
                    }
                }

                // create the resource view
                {
                    auto const le = obj.get_buffer( i ).get_layout_element_zero();
                    D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
                    res_desc.Format = motor::platform::d3d11::convert_type_to_vec_format(
                        le.type, le.type_struct );
                    res_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

                    res_desc.Buffer.FirstElement = 0;

                    // somehow, this can not be 0
                    // otherwise, hr will be in valid.
                    res_desc.Buffer.NumElements =
                        UINT( obj.size() * obj.get_buffer( i ).get_num_layout_elements() );

                    for( size_t j = 0; j < 2; ++j )
                    {
                        auto & b = sod->_ping_pong[ j ].buffers[ i ];
                        auto & v = sod->_ping_pong[ j ].views[ i ].invalidate();
                        auto const hr = _ctx->dev()->CreateShaderResourceView( b, &res_desc, v );
                        if( FAILED( hr ) )
                        {
                            d3d11_log_errorv(
                                "CreateShaderResourceView for buffer : [%s]", sod.name().c_str() );
                        }
                    }
                }
            } );

            sod->buffers_used =
                std::min( obj.num_buffers(), d3d11_pimpl_data::so_data_t::max_buffers );
        }

        // create query objects
        {
            D3D11_QUERY_DESC qd;
            qd.Query = D3D11_QUERY_SO_STATISTICS;
            qd.MiscFlags = 0;

            for( size_t j = 0; j < 2; ++j )
            {
                HRESULT const hr = _ctx->dev()->CreateQuery( &qd, sod->_ping_pong[ j ].query );
                motor::log::global_t::error(
                    FAILED( hr ), d3d11_backend_log( "CreateQuery - D3D11_QUERY_SO_STATISTICS" ) );
            }
        }

        return sod.oid();
    }

    //************************************************************************************************************
    void_t release_streamout( size_t const oid ) noexcept
    {
        _renders.for_each(
            [ & ]( d3d11_pimpl_data::render_data_ref_t rd ) { rd.unlink_streamout( oid ); } );

        // auto & d = _streamouts[ oid ] ;
        _streamouts.invalidate( oid );
    }

    //************************************************************************************************************
    bool_t update(
        size_t const oid, motor::graphics::streamout_object_ref_t obj, bool_t const is_config )
    {
        if( auto sod = _streamouts.access( oid ); sod )
        {
            if( sod->buffers_used != obj.num_buffers() )
            {
                this_t::construct_streamout( oid, obj );
                return true;
            }

            obj.for_each( [ & ]( size_t const i, motor::graphics::vertex_buffer_cref_t vb )
            {
                if( i >= d3d11_pimpl_data::so_data_t::max_buffers ) return;

                size_t const byte_width = obj.size() * obj.get_buffer( i ).get_layout_sib();

                for( size_t pp = 0; pp < 2; ++pp )
                {
                    D3D11_BUFFER_DESC bd;
                    sod->_ping_pong[ pp ].buffers[ i ]->GetDesc( &bd );

                    if( byte_width <= bd.ByteWidth ) continue;
                    bd.ByteWidth = UINT( byte_width );

                    auto & b = sod->_ping_pong[ pp ].buffers[ i ].invalidate();
                    HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, NULL, b );
                    motor::log::global_t::error( FAILED( hr ),
                        d3d11_backend_log( "CreateBuffer - D3D11_BIND_STREAM_OUTPUT" ) );
                }
            } );
            return true;
        }
        return false;
    }

    //************************************************************************************************************
    bool_t activate_streamout( size_t const oid ) noexcept
    {
        _cur_streamout_active = oid;

        if( _cur_streamout_active == size_t( -1 ) ) return true;

        if( auto sod = streamout_datas_t::handle_t( _cur_streamout_active, _streamouts ); sod )
        {
            auto & wrtb = sod->write_buffer();
            _ctx->ctx()->Begin( wrtb.query );

            {
                UINT offsets[ d3d11_pimpl_data::so_data_t::max_buffers ] = { 0, 0, 0, 0 };
                ID3D11Buffer * buffers[ d3d11_pimpl_data::so_data_t::max_buffers ];
                for( size_t i = 0; i < d3d11_pimpl_data::so_data_t::max_buffers; ++i )
                {
                    if( wrtb.buffers[ i ] == nullptr ) break;
                    buffers[ i ] = wrtb.buffers[ i ];
                }
                _ctx->ctx()->SOSetTargets( UINT( sod->buffers_used ), buffers, offsets );
            }
        }

        return true;
    }

    //************************************************************************************************************
    void_t deactivate_streamout( void_t )
    {
        if( _cur_streamout_active == size_t( -1 ) ) return;

        // auto & d = _streamouts[_cur_streamout_active] ;
        //_streamouts.access( _cur_streamout_active, [ & ]( d3d11_pimpl_data::so_data_ref_t d )

        if( auto sod = streamout_datas_t::handle_t( _cur_streamout_active, _streamouts ); sod )
        {
            sod->swap();
            _ctx->ctx()->End( sod->read_buffer().query );
            _ctx->ctx()->SOSetTargets( 0, nullptr, nullptr );
        }

        _cur_streamout_active = size_t( -1 );
    }

    //****************************************************************************************
    void_t send_configure_to_st( motor::graphics::msl_object_mtr_t obj ) noexcept
    {
        _ctsd->add_forward_item( d3d11_detail::work_item::make_item(
            d3d11_detail::work_item::work_type::compile, motor::share( obj ) ) );
    }

    void_t inject_default_shader_variable_values( motor::graphics::msl_object_ref_t obj ) {}

    //****************************************************************************************
    // call from support thread
    bool_t construct_msl_data_st( motor::graphics::msl_object_ptr_t obj_in ) noexcept
    {
        auto & obj = *obj_in;

        motor::vector< motor::msl::symbol_t > config_symbols;

        obj.for_each_msl( motor::graphics::msl_api_type::msl_4_0, [ & ]( motor::string_in_t code )
        {
            motor::msl::post_parse::document_t doc =
                motor::msl::parser_t( "d3d11" ).process( code );

            _mdb.insert( std::move( doc ), config_symbols );
        } );

        // cases:
        // obj.oid == -1 && obj.name.empty() : library/config shader
        // obj.oid == -1 && !obj.name.empty() : initial configuration

        // obj.oid != -1 && obj.name.empty() : invalid
        // obj.oid != -1 && !obj.name.empty() : was already configured

        // if the incoming msl shader is a library shader for example,
        // it does not need to have a associated background object
        size_t oid = obj_in->get_oid( _bid );
        _msls.access( oid, obj.name(), []( d3d11_pimpl_data::msl_data_ref_t ) { return true; } );

        // if -1, it is probably a library shader or some tmp
        // msl object. So do not return any valid is below.
        bool_t const is_valid_msl = oid != size_t( -1 );

        bool_t success = true;

        for( auto const & c : config_symbols )
        {
            auto const c_exp = c.expand();

// this most likely came from a library dependency.
// need to figure out the msl object associated
// with this render configuration/object.
// the msl object is required in order to reconstruct
// the render_object and the shader_object.

// 1. find the msl object associated to c
// 2. use the found oid for further processing
#if 0
            if ( oid == size_t( -1 ) )
            {
                auto [i, o] = this_t::find_pair_by_ro_name( c_exp, _msls ) ;
                oid = i ;
                obj = o ;
            }
#endif

            // msl database contains render configuration
            // which has not been configured by the user...
            if( oid == size_t( -1 ) )
            {
                motor::log::global::warning< 1024 >( "[d3d11::construct_msl_data] : "
                                                     "render configuration not found : %s",
                    c_exp.c_str() );
                continue;
            }

            motor::msl::generatable_t res = motor::msl::dependency_resolver_t().resolve( &_mdb, c );
            if( res.missing.size() != 0 )
            {
                motor::log::global_t::warning(
                    "[d3d11] : We have missing symbols for " + c_exp + " :" );
                for( auto const & s : res.missing )
                {
                    motor::log::global_t::status( s.expand() );
                }
                continue;
            }

            motor::graphics::variable_set_t default_values;

            // inject default variable values into the
            // variable sets
            for( auto & shd_ : res.config.shaders )
            {
                for( auto & var_ : shd_.variables )
                {
                    if( var_.def_val == size_t( -1 ) ) continue;

                    auto * df = res.config.def_values[ var_.def_val ];
                    if( dynamic_cast< motor::msl::generic_default_value< bool_t > * >( df ) !=
                        nullptr )
                    {
                        using ptr_t = motor::msl::generic_default_value< bool_t > *;
                        ptr_t gdv = static_cast< ptr_t >( df );
                        default_values.data_variable< bool_t >( var_.name )->set( gdv->get() );
                    }
                    else if( dynamic_cast< motor::msl::generic_default_value< float_t > * >( df ) !=
                             nullptr )
                    {
                        using ptr_t = motor::msl::generic_default_value< float_t > *;
                        ptr_t gdv = static_cast< ptr_t >( df );
                        default_values.data_variable< float_t >( var_.name )->set( gdv->get() );
                    }
                    else if( dynamic_cast<
                                 motor::msl::generic_default_value< motor::math::vec3f_t > * >(
                                 df ) != nullptr )
                    {
                        using ptr_t = motor::msl::generic_default_value< motor::math::vec3f_t > *;
                        ptr_t gdv = static_cast< ptr_t >( df );
                        default_values.data_variable< motor::math::vec3f_t >( var_.name )
                            ->set( gdv->get() );
                    }
                    else if( dynamic_cast<
                                 motor::msl::generic_default_value< motor::math::vec4f_t > * >(
                                 df ) != nullptr )
                    {
                        using ptr_t = motor::msl::generic_default_value< motor::math::vec4f_t > *;
                        ptr_t gdv = static_cast< ptr_t >( df );
                        default_values.data_variable< motor::math::vec4f_t >( var_.name )
                            ->set( gdv->get() );
                    }
                    else if( dynamic_cast< motor::msl::texture_dv_ptr_t >( df ) != nullptr )
                    {
                        using ptr_t = motor::msl::texture_dv_ptr_t;
                        ptr_t gdv = static_cast< ptr_t >( df );
                        default_values.texture_variable( var_.name )->set( gdv->get().name );
                    }
                }
            }

            auto * ro = obj.borrow_render_object();

            motor::graphics::shader_object_t so( c_exp );

            // generate code
            {
                auto tp_begin = std::chrono::high_resolution_clock::now();

                motor::msl::generator_t gen( std::move( res ) );
                auto const code = gen.generate< motor::msl::hlsl::hlsl5_generator_t >();
                motor::graphics::msl_bridge::create_by_api_type(
                    motor::graphics::shader_api_type::hlsl_5_0, code, so );

                {
                    size_t const milli = std::chrono::duration_cast< std::chrono::milliseconds >(
                        std::chrono::high_resolution_clock::now() - tp_begin )
                                             .count();

                    char buffer[ 2048 ];
                    std::snprintf(
                        buffer, 2048, "[d3d11] : generating hlsl shader took %zu ms", milli );
                    motor::log::global_t::status( buffer );
                }
            }

            {
                ro->link_shader( c_exp );
            }

            auto const access_res =
                _msls.access( oid, obj.name(), [ & ]( d3d11_pimpl_data::msl_data_ref_t msl )
            {
                {
                    // so.set_oid( bid, this_t::construct_shader_config( so.get_oid( bid ), so ) ) ;
                    if( !this_t::construct_shader_config( so ) )
                    {
                        // construction/compilation failed
                        // @todo return here.
                        return false;
                    }

                    _shaders.access( so.get_oid( _bid ),
                        [ & ]( d3d11_pimpl_data::shader_data_ref_t shd )
                    { shd.default_values = std::move( default_values ); } );

                    if( !this_t::construct_render_data( *ro ) )
                    {
                        return false;
                    }
                }

                // render object
                {
                    msl.ro_id = ro->get_oid( this_t::_bid );
                }

                // shader object
                {
                    // reflect compilation result to the user
                    _shaders.access( so.get_oid( _bid ),
                        [ & ]( d3d11_pimpl_data::shader_data_ref_t shd )
                    {
                        obj.for_each( [ & ]( motor::graphics::compilation_listener_mtr_t lst )
                        {
                            auto const s =
                                shd.compiled
                                    ? motor::graphics::compilation_listener::state::successful
                                    : motor::graphics::compilation_listener::state::failed;

                            lst->set( s, so.shader_bindings() );
                        } );
                    } );

                    {
                        msl.so = std::move( so );
                    }
                }

                return true;
            } );

            if( !access_res )
            {
                motor::log::global_t::warning( "[d3d11] : msl failed. msl will not be set." );
                success = false;
            }
        }

        if( success )
        {
            motor::graphics::object_t::data_manipulator mani( obj_in, this_t::_bid );
            mani.reset_in_transit();
            mani.change_to_ready( motor::graphics::result::ok );
        }
        else
        {
            motor::graphics::object_t::data_manipulator mani( obj_in, this_t::_bid );
            mani.reset_in_transit();
            mani.change_to_ready( motor::graphics::result::failed );
        }

        // true: was msl object. so the id
        // needs to go back to the caller
        if( is_valid_msl )
        {
            obj_in->set_oid( _bid, oid );
        }
        else
        {
            obj_in->set_oid( _bid, size_t( -1 ) );
        }

        return is_valid_msl;
    }

    //************************************************************************************************************
    size_t construct_geo( size_t oid, motor::graphics::geometry_object_ref_t obj ) noexcept
    {
        auto geod = _geos.access( oid, obj.name() );

        this_t::construct_geo( obj, *geod );

        return geod.oid();
    }

    //************************************************************************************************************
    bool_t construct_geo( motor::graphics::geometry_object_ref_t obj,
        d3d11_pimpl_data::geo_data_ref_t config ) noexcept
    {
        bool_t res = true;

        config.pt = obj.primitive_type();

        // vertex buffer object
        {
            config.elements.clear();

            obj.vertex_buffer().for_each_layout_element(
                [ & ]( motor::graphics::vertex_buffer_t::data_cref_t d )
            {
                d3d11_pimpl_data::geo_data_t::layout_element le;
                le.va = d.va;
                le.type = d.type;
                le.type_struct = d.type_struct;
                config.elements.push_back( le );
            } );

            config.stride = UINT( obj.vertex_buffer().get_layout_sib() );

            // = number of vertices * sizeof( vertex )
            // if there are vertices already setup, take the numbers
            size_t const byte_width = obj.vertex_buffer().get_sib();

            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DYNAMIC;
            // using max of 1 sib so empty vbs will create and will
            // be resized in the update function. Important for now
            // is that the buffer will be NOT nullptr.
            bd.ByteWidth = UINT( std::max( byte_width, size_t( 1 ) ) );
            bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            motor::memory::global_t::dealloc_raw( config.vb_mem );
            config.vb_mem = motor::memory::global_t::alloc_raw< uint8_t >(
                byte_width, "[d3d11] : vertex buffer memory" );

            if( config.vb != nullptr )
            {
                config.vb->Release();
                config.vb = nullptr;
            }

            if( byte_width != 0 )
            {
                D3D11_SUBRESOURCE_DATA init_data = {};
                init_data.pSysMem = (void_cptr_t)config.vb_mem;

                std::memcpy( config.vb_mem, obj.vertex_buffer().data(), byte_width );

                HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, config.vb );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_VERTEX_BUFFER" ) );
                    res = false;
                }
            }
            else
            {
                HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, nullptr, config.vb );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_VERTEX_BUFFER" ) );
                    res = false;
                }
            }
        }

        // index buffer object
        if( obj.index_buffer().get_num_elements() != 0 )
        {
            // = number of vertices * sizeof( index_t )
            size_t const byte_width = obj.index_buffer().get_sib();

            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DYNAMIC;
            // using max of 1 sib so empty ibs will create and will
            // be resized in the update function. Important for now
            // is that the buffer will be NOT nullptr.
            bd.ByteWidth = uint_t( std::max( byte_width, size_t( 1 ) ) );
            bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
            bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            motor::memory::global_t::dealloc_raw( config.ib_mem );
            config.ib_mem = motor::memory::global_t::alloc_raw< uint8_t >(
                byte_width, "[d3d11] : index buffer memory" );

            if( config.ib != nullptr )
            {
                config.ib->Release();
                config.ib = nullptr;
            }

            if( byte_width != 0 )
            {
                D3D11_SUBRESOURCE_DATA init_data = {};
                init_data.pSysMem = (void_cptr_t)config.ib_mem;

                std::memcpy( config.ib_mem, obj.index_buffer().data(), byte_width );

                HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, config.ib );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_INDEX_BUFFER" ) );
                    res = false;
                }
            }
            else
            {
                HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, nullptr, config.ib );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_INDEX_BUFFER" ) );
                    res = false;
                }
            }
        }

        return res;
    }

    //************************************************************************************************************
    void_t release_geometry( size_t const oid ) noexcept
    {
        _renders.for_each(
            [ = ]( render_datas_t::checked_handle_t rd ) { rd->unlink_geometry( oid ); } );
        _geos.invalidate( oid );
    }

    //************************************************************************************************************
    bool_t update( size_t const id, motor::graphics::geometry_object_mtr_t geo )
    {
        auto geod = geometry_datas_t::handle_t( id, _geos );
        if( !geod ) return false;

        if( geo->index_buffer().get_num_elements() != 0 && geod->ib == nullptr )
        {
            this_t::construct_geo( *geo, *geod );
        }

        // vb: check memory space
        {
            D3D11_BUFFER_DESC bd;
            geod->vb->GetDesc( &bd );

            size_t const sib = geo->vertex_buffer().get_sib();
            if( sib > bd.ByteWidth )
            {
                bd.ByteWidth = uint_t( sib );

                motor::memory::global_t::dealloc_raw( geod->vb_mem );
                geod->vb_mem = motor::memory::global_t::alloc_raw< uint8_t >(
                    sib, "[d3d11] : vertex buffer memory" );

                D3D11_SUBRESOURCE_DATA init_data = {};
                init_data.pSysMem = (void_ptr_t)geod->vb_mem;

                geod->vb->Release();
                {
                    auto const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, geod->vb );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error(
                            d3d11_backend_log( "CreateBuffer( vertex_buffer )" ) );
                        return false;
                    }
                }
            }

            geod->num_elements_vb = geo->vertex_buffer().get_num_elements();

            // copy data
            {
                // exactly the size in bytes of the 1d buffer
                LONG const lsib = LONG( geo->vertex_buffer().get_layout_sib() );
                LONG const ne = LONG( geo->vertex_buffer().get_num_elements() );
                void_cptr_t data = geo->vertex_buffer().data();
                CD3D11_BOX const box( 0, 0, 0, ne, 1, 1 );

                //_ctx->ctx()->UpdateSubresource( geod->vb, 0, nullptr /*&box*/, data, lsib, 0
                //) ;
                D3D11_MAPPED_SUBRESOURCE resource;
                _ctx->ctx()->Map( geod->vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
                std::memcpy( resource.pData, data, lsib * ne );
                _ctx->ctx()->Unmap( geod->vb, 0 );
            }
        }

        // ib: check memory space
        if( geod->ib != nullptr )
        {
            D3D11_BUFFER_DESC bd;
            geod->ib->GetDesc( &bd );

            size_t const sib = geo->index_buffer().get_sib();
            if( sib > bd.ByteWidth )
            {
                bd.ByteWidth = uint_t( sib );

                motor::memory::global_t::dealloc_raw( geod->ib_mem );
                geod->ib_mem = motor::memory::global_t::alloc_raw< uint8_t >(
                    sib, "[d3d11] : index buffer memory" );

                D3D11_SUBRESOURCE_DATA init_data = {};
                init_data.pSysMem = (void_ptr_t)geod->ib_mem;

                geod->ib->Release();
                {
                    auto const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, geod->ib );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error(
                            d3d11_backend_log( "CreateBuffer( index_buffer )" ) );
                        return false;
                    }
                }
            }

            geod->num_elements_ib = geo->index_buffer().get_num_elements();

            // copy data
            {
                // exactly the size in bytes of the 1d buffer
                LONG const lsib = LONG( sizeof( uint32_t ) );
                LONG const ne = LONG( geo->index_buffer().get_num_elements() );
                void_cptr_t data = geo->index_buffer().data();
                CD3D11_BOX const box( 0, 0, 0, ne, 1, 1 );

                //_ctx->ctx()->UpdateSubresource( geod->ib, 0, &box, data, lsib, ne ) ;
                D3D11_MAPPED_SUBRESOURCE resource;
                _ctx->ctx()->Map( geod->ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
                std::memcpy( resource.pData, data, lsib * ne );
                _ctx->ctx()->Unmap( geod->ib, 0 );
            }
        }

        return true;
    }

    //****************************************************************************************
    bool_t construct_msl_data( d3d11_independent::compilation_result && cr ) noexcept
    {
        motor::graphics::object_t::data_manipulator mani( cr.object, this_t::_bid );

        if( !cr.success )
        {
            mani.reset_in_transit();
            mani.change_to_raw( motor::graphics::result::failed );
            cr.object->for_each( [ & ]( motor::graphics::compilation_listener_mtr_t lst )
            {
                lst->set( motor::graphics::compilation_listener::state::failed,
                    motor::graphics::shader_bindings() );
            } );

            motor::release( motor::move( cr.object ) );
            return false;
        }

        auto & obj = *cr.object;
        auto so = std::move( cr.shader );
        auto sd = std::move( cr.data );

        auto * ro = obj.borrow_render_object();
        {
            ro->link_shader( so.name() );
        }

        size_t oid = obj.get_oid( _bid );

        auto const success =
            _msls.access( oid, obj.name(), [ & ]( d3d11_pimpl_data::msl_data_ref_t msl )
        {
            if( !this_t::install_shader_data( so, sd ) ) return false;
            if( !this_t::construct_render_data( *ro ) ) return false;

            // render object
            {
                msl.ro_id = ro->get_oid( this_t::_bid );
            }

            // shader object
            {
                // reflect compilation result to the user
                _shaders.access( so.get_oid( _bid ),
                    [ & ]( d3d11_pimpl_data::shader_data_ref_t shd )
                {
                    obj.for_each( [ & ]( motor::graphics::compilation_listener_mtr_t lst )
                    {
                        auto const s =
                            shd.compiled ? motor::graphics::compilation_listener::state::successful
                                         : motor::graphics::compilation_listener::state::failed;

                        lst->set( s, so.shader_bindings() );
                    } );
                } );

                {
                    msl.so = std::move( so );
                }
            }

            return true;
        } );

        if( success )
        {
            mani.reset_in_transit();
            mani.change_to_ready( motor::graphics::result::ok );
        }
        else
        {
            mani.reset_in_transit();
            mani.change_to_ready( motor::graphics::result::failed );

            cr.object->for_each( [ & ]( motor::graphics::compilation_listener_mtr_t lst )
            {
                lst->set( motor::graphics::compilation_listener::state::failed,
                    motor::graphics::shader_bindings() );
            } );
        }

        cr.object->set_oid( _bid, oid );
        motor::release( motor::move( cr.object ) );

        return success;
    }

    //************************************************************************************************************
    bool_t install_shader_data( motor::graphics::shader_object_ref_t obj,
        d3d11_pimpl_data::shader_data_ref_t new_shader_data ) noexcept
    {
        auto shd = _shaders.access( obj.get_oid( _bid ), obj.name() );
        {
            shd->invalidate( std::nullopt );
            *shd = std::move( new_shader_data );
        }

        obj.set_oid( _bid, shd.oid() );

        return true;
    }

    //************************************************************************************************************
    // this function compiles the shader.
    bool_t construct_shader_data( motor::graphics::shader_object_ref_t obj ) noexcept
    {
        d3d11_pimpl_data::shader_data_t new_shader_data;
        if( !d3d11_independent::construct_shader_data( obj, new_shader_data, _ctx ) )
        {
            new_shader_data.invalidate( std::nullopt );
            return false;
        }

        return this_t::install_shader_data( obj, new_shader_data );
    }

    //************************************************************************************************************
    // this one compiles and installs the shader when called.
    // @note do not use this function at run-time. shaders should be
    // compiled through the support thread.
    bool_t construct_shader_config( motor::graphics::shader_object_ref_t obj )
    {
        size_t oid = obj.get_oid( _bid );

        d3d11_pimpl_data::shader_data_t new_shader;
        if( d3d11_independent::construct_shader_data( obj, new_shader, _ctx ) )
        {
            return this_t::install_shader_data( obj, new_shader );
        }

        // compilation failed.
        return false;
    }

    //************************************************************************************************************
    void_t release_shader_data( size_t const oid ) noexcept
    {
        _renders.for_each( [ & ]( d3d11_pimpl_data::render_data_t & rd )
        {
            if( rd.shd_id == oid ) rd.shd_id = size_t( -1 );
        } );

        _shaders.invalidate( oid );
    }

    //************************************************************************************************************
    // @param sid shader oid if known
    bool_t construct_render_data( motor::graphics::render_object_ref_t obj )
    {
        auto rd = _renders.access( obj.get_oid( _bid ), obj.name() );
        {
            if( rd->vertex_layout != nullptr )
            {
                rd->vertex_layout->Release();
                rd->vertex_layout = nullptr;
            }

            if( rd->vertex_layout_so != nullptr )
            {
                rd->vertex_layout_so->Release();
                rd->vertex_layout_so = nullptr;
            }

            std::memset( rd->layout, 0, ARRAYSIZE( rd->layout ) );

            if( !this_t::update( render_datas_t::checked_handle_t( rd ), obj ) )
            {
                motor::log::global_t::error< 1024 >(
                    "[d3d11] : update from construct_render_data for %s", rd.name().c_str() );
                return false;
            }
        }
        obj.set_oid( _bid, rd.oid() );
        return true;
    }

    //************************************************************************************************************
    void_t release_render_data( size_t const oid ) noexcept
    {
        _renders.invalidate( oid );
    }

    //************************************************************************************************************
    void_t release_msl_data( size_t const oid ) noexcept
    {
        _msls.access( oid, [ & ]( d3d11_pimpl_data::msl_data_t & data )
        {
            this_t::release_render_data( data.ro_id );
            _shaders.invalidate( data.so.get_oid( _bid ) );
        } );
        _msls.invalidate( oid );
    }

    //************************************************************************************************************
    // this function prepares the vertex input layout
    // it will also remove the old input layout and recreate the new one.
    // @param geo_idx by default is 0. we assume for now, all vertex input layouts are the same.
    // but we need it for dynamic geometry links. having a separate vertex input layout for every
    // geometry link is already issued. @see #159
    bool_t bind_vertex_inputs(
        render_datas_t::checked_handle_t rd, size_t const geo_idx = 0 ) noexcept
    {
        auto shd = _shaders.access( rd->shd_id );
        if( !shd ) return false;

        if( rd->geo_ids.size() != 0 )
        {
            d3d11_pimpl_data::geo_data_t::elements_t elems;
            {
                bool_t const valid_obj = _geos.access( rd->geo_ids[ geo_idx ].id,
                    [ & ]( d3d11_pimpl_data::geo_data_ref_t geo ) { elems = geo.elements; } );
                assert( valid_obj );
            }

            ID3DBlob * blob_ptr = nullptr;
            d3d11_pimpl_data::shader_data_t::vibs_t vibs;
            {
#if 1
                vibs = shd->vertex_inputs;
                blob_ptr = shd->vs_blob;
#else
                bool_t const valid_obj =
                    _shaders.access( rd.shd_id, [ & ]( d3d11_pimpl_data::shader_data_ref_t shd )
                {
                    vibs = shd.vertex_inputs;
                    blob_ptr = shd.vs_blob;
                } );
                assert( valid_obj );
#endif
            }

            // sort shader input attributes based on geometry layout.
            // in hlsl the order seems to be required to match for the
            // used geometry layout (next section) and the shader input
            // attribute order.
            {
                size_t const num_elem = std::min( elems.size(), vibs.size() );
                for( size_t i = 0; i < num_elem; ++i )
                {
                    size_t j = i;

                    while( ( j < num_elem ) && ( elems[ i ].va != vibs[ j ].va ) ) ++j;

                    if( j == i ) continue;
                    // input variable declared in shader not found in geometry, remove ...
                    else if( j == num_elem )
                    {
                        vibs.erase( vibs.begin() + i );
                        motor::log::global_t::warning(
                            d3d11_backend_log( "removed shader input attribute in [" + rd.name() +
                                               "]. "
                                               "Attribute not found in geometry layout." ) );
                    }
                    // .. or entries are not at the same spot. exchange.
                    else if( j != i )
                    {
                        auto const tmp = vibs[ i ];
                        vibs[ i ] = vibs[ j ];
                        vibs[ j ] = tmp;
                    }
                }
            }

            // for binding attributes, the shader and the geometry is required.
            {
                size_t i = 0;
                UINT offset = 0;
                for( auto const & b : vibs )
                {
                    char_cptr_t name = motor::platform::d3d11::vertex_binding_to_semantic( b.va );
                    UINT const semantic_index =
                        motor::platform::d3d11::vertex_output_binding_to_semantic_index( b.va );
                    DXGI_FORMAT const fmt =
                        d3d11_pimpl_data::geo_data_t::get_format_from_element( b.va, elems );
                    UINT input_slot = 0;
                    UINT aligned_byte_offset = offset;
                    D3D11_INPUT_CLASSIFICATION const iclass = D3D11_INPUT_PER_VERTEX_DATA;
                    UINT instance_data_step_rate = 0;

                    rd->layout[ i++ ] = { name, semantic_index, fmt, input_slot,
                        aligned_byte_offset, iclass, instance_data_step_rate };

                    offset += d3d11_pimpl_data::geo_data_t::get_sib( b.va, elems );
                }

                UINT const num_elements = UINT( i );

                if( rd->vertex_layout != nullptr )
                {
                    rd->vertex_layout->Release();
                    rd->vertex_layout = nullptr;
                }

                bool_t failed = false;
                {
                    auto const hr = _ctx->dev()->CreateInputLayout( rd->layout, num_elements,
                        shd->vs_blob->GetBufferPointer(), shd->vs_blob->GetBufferSize(),
                        &rd->vertex_layout );

                    if( FAILED( hr ) )
                    {
                        d3d11_log_warningv(
                            "CreateInputLayout for shader [%s] and render object[%s]",
                            shd.name().c_str(), rd.name().c_str() );
                        d3d11_log_warningv(
                            "This is probably due to a missmatch of the shader input attributes \
                                            and the vertex buffer layout. They do not match." );

                        failed = true;
                    }
                }

                if( failed ) return false;
            }
        }

        if( rd->so_ids.size() != 0 )
        {
            d3d11_pimpl_data::shader_data_t::vibs_t vibs;
            d3d11_pimpl_data::so_data_t::elements_t elems;

#if 1
            vibs = shd->vertex_inputs;
#else

            _shaders.access( rd.shd_id,
                [ & ]( d3d11_pimpl_data::shader_data_ref_t shd ) { vibs = shd.vertex_inputs; } );
#endif
            // sort shader input attributes based on geometry layout.
            // in hlsl the order seems to be required to match for the
            // used geometry layout (next section) and the shader input
            // attribute order.
            {
                size_t n = 0;
                //_streamouts.access( rd.so_ids[ 0 ], [ & ]( d3d11_pimpl_data::so_data_ref_t sod )
                if( auto sod = streamout_datas_t::handle_t( rd->so_ids[ 0 ], _streamouts ); sod )
                {
                    elems = sod->elements;
                    n = std::min( sod->elements.size(), vibs.size() );
                }

                // d3d11_pimpl_data::so_data_ref_t sod = _streamouts[ rd.so_ids[0] ] ;
                // size_t const n = std::min( sod.elements.size(), vibs.size() ) ;

                if( vibs.size() != elems.size() )
                {
                    motor::log::global_t::error( "[d3d11] : number of shader input attributes(" +
                                                 motor::to_string( vibs.size() ) +
                                                 ") do not match the number of streamout "
                                                 "vertex attributes(" +
                                                 motor::to_string( elems.size() ) +
                                                 ") for render object[" + rd.name() + "] " );
                }

                for( size_t i = 0; i < n; ++i )
                {
                    size_t j = i;

                    while( j < vibs.size() && elems[ i ].va != vibs[ j ].va ) ++j;

                    if( j == i ) continue;

                    // input variable declared in shader not found in geometry, remove ...
                    else if( j == vibs.size() )
                    {
                        vibs.erase( vibs.begin() + i );
#if 1
                        {
                            motor::log::global_t::warning( d3d11_backend_log(
                                "removed shader input attribute in [" + shd.name() +
                                "]. "
                                "Attribute not found in geometry layout." ) );
                        }
#else
                        _shaders.access( rd.shd_id, [ & ]( motor::string_in_t shd_name,
                                                        d3d11_pimpl_data::shader_data_ref_t shd )
                        {
                            motor::log::global_t::warning( d3d11_backend_log(
                                "removed shader input attribute in [" + shd_name +
                                "]. "
                                "Attribute not found in geometry layout." ) );
                        } );
#endif
                    }
                    // .. or entries are not at the same spot. exchange.
                    else if( j != i )
                    {
                        auto const tmp = vibs[ i ];
                        vibs[ i ] = vibs[ j ];
                        vibs[ j ] = tmp;
                    }
                }
            }

            // for binding attributes, the shader and the geometry is required.
            // in this case it is the streamout buffer.
            {
                size_t i = 0;
                UINT offset = 0;
                for( auto const & b : vibs )
                {
                    char_cptr_t name = motor::platform::d3d11::vertex_binding_to_semantic( b.va );
                    UINT const semantic_index =
                        motor::platform::d3d11::vertex_output_binding_to_semantic_index( b.va );
                    DXGI_FORMAT const fmt =
                        d3d11_pimpl_data::so_data_t::get_format_from_element( b.va, elems );
                    UINT input_slot = 0;
                    UINT aligned_byte_offset = offset;
                    D3D11_INPUT_CLASSIFICATION const iclass = D3D11_INPUT_PER_VERTEX_DATA;
                    UINT instance_data_step_rate = 0;

                    rd->layout_so[ i++ ] = { name, semantic_index, fmt, input_slot,
                        aligned_byte_offset, iclass, instance_data_step_rate };

                    offset += d3d11_pimpl_data::so_data_t::get_sib( b.va, elems );
                }

                UINT const num_elements = UINT( i );

                if( rd->vertex_layout_so != nullptr )
                {
                    rd->vertex_layout_so->Release();
                    rd->vertex_layout_so = nullptr;
                }
#if 1
                auto res = true;
                {
                    HRESULT const hr = _ctx->dev()->CreateInputLayout( rd->layout_so, num_elements,
                        shd->vs_blob->GetBufferPointer(), shd->vs_blob->GetBufferSize(),
                        &rd->vertex_layout_so );

                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::warning(
                            d3d11_backend_log( "CreateInputLayout for shader [" + shd.name() +
                                               "] and "
                                               "render object[" +
                                               rd.name() + "]" ) );
                        res = false;
                    }
                }

#else
                auto const res = _shaders.access( rd.shd_id,
                    [ & ]( motor::string_in_t shd_name, d3d11_pimpl_data::shader_data_ref_t shd )
                {
                    HRESULT const hr = _ctx->dev()->CreateInputLayout( rd.layout_so, num_elements,
                        shd.vs_blob->GetBufferPointer(), shd.vs_blob->GetBufferSize(),
                        &rd.vertex_layout_so );

                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::warning(
                            d3d11_backend_log( "CreateInputLayout for shader [" + shd_name +
                                               "] and "
                                               "render object[" +
                                               rd.name() + "]" ) );
                        return false;
                    }
                    return true;
                } );
#endif
                if( !res ) return false;
            }
        }

        return true;
    }

    //****************************************************************************************
    // helper function requires context:
    // you need to know if a variable set is reconnecting.
    // at the moment, just really call if you know if a reconnect is happening.
    // a reconnection could happen, if a variable set is still there, but on the user side
    // the variable set has been replaced so the hash values differ.
    void_t reconnect_data_for_variable_set( render_datas_t::checked_handle_t rd,
        shader_datas_t::checked_handle_t shd, motor::graphics::render_object_ref_t rc,
        size_t const vs_idx, size_t const hash ) noexcept
    {
        // enforce description - see function desc
        assert( rd->varset_to_idx.size() > vs_idx );
        assert( rd->varset_to_idx[ vs_idx ].idx != size_t( -1 ) );
        assert( rd->var_sets[ rd->varset_to_idx[ vs_idx ].idx ].hash != hash );
        assert( rd->var_sets[ rd->varset_to_idx[ vs_idx ].idx ].vs != nullptr );

        // borrowed
        motor::graphics::variable_set_mtr_t default_values = &( shd->default_values );

        auto & old_vs_entry = rd->via_indirection( vs_idx );
        auto vs_entry = rc.borrow_variable_set( vs_idx );

        if( default_values != nullptr )
        {
            vs_entry.vs->clone_from_if_not_exist( default_values );
        }

        // exchange data variables
        {
            auto funk = [ &, this ](
                            d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices const it,
                            d3d11_pimpl_data::render_data_t::cbuffers_ref_t buffer )
            {
                auto const s = rd->varset_to_idx[ vs_idx ].cbuffer_begin[ it ];
                auto const e = rd->varset_to_idx[ vs_idx ].cbuffer_end[ it ];

                for( size_t i = s; i < e; ++i )
                {
                    for( auto & entry : buffer[ i ].data_variables )
                    {
                        motor::graphics::variable_set_t::data_variable_info_t dvi;
                        if( !old_vs_entry.vs->data_variable_name( entry.ivar, dvi ) )
                        {
                            motor::log::global::error< 1024 >(
                                "[reconnect] : count not find variable %s", dvi.name.data() );
                            entry.ivar = nullptr;
                            continue;
                        }

                        entry.ivar = vs_entry.vs->data_variable(
                            motor::string_t( dvi.name ), dvi.type, dvi.type_struct );
                    }
                }
            };

            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                funk( t, rd->_cbuffers_vs );
            }
            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
                funk( t, rd->_cbuffers_gs );
            }
            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;
                funk( t, rd->_cbuffers_ps );
            }
        }

        // exchange texture variables
        {
            auto funk = [ &, this ](
                            d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices const it,
                            d3d11_pimpl_data::render_data_t::image_variables_ref_t buffer )
            {
                auto const s = rd->varset_to_idx[ vs_idx ].imgs_begin[ it ];
                auto const e = rd->varset_to_idx[ vs_idx ].imgs_end[ it ];

                for( size_t i = s; i < e; ++i )
                {
                    auto & entry = buffer[ i ];

                    auto * tx_var = vs_entry.vs->find_texture_variable( entry.name.c_str() );
                    if( tx_var == nullptr )
                    {
                        motor::log::global::error< 1024 >(
                            "[reconnect] : count not find variable %s", entry.name.c_str() );
                        continue;
                    }
                    entry.id = _images.find_by_name( tx_var->get().name() );
                    entry.value_hash = tx_var->get().hash();

                    // y-flip reentry.
                    {
                        motor::string_t const name = "sys_flipv_" + entry.name;
                        auto * ptr = old_vs_entry.vs->find_data_variable< float_t >( name.c_str() );
                        if( ptr != nullptr )
                        {
                            vs_entry.vs->data_variable< float_t >( name.c_str() )
                                ->set( ptr->get() );
                        }
                    }
                }
            };

            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                funk( t, rd->var_sets_imgs_vs );
            }
#if 0 // dont have it yet
            {
                auto const t = d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader ;
                funk( t, rd->var_sets_imgs_gs ) ;
            }
#endif
            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;
                funk( t, rd->var_sets_imgs_ps );
            }
        }

        // exchage array buffers
        {
            auto funk = [ &, this ](
                            d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices const it,
                            d3d11_pimpl_data::render_data_t::buffer_variables_ref_t buffer )
            {
                auto const s = rd->varset_to_idx[ vs_idx ].buffers_begin[ it ];
                auto const e = rd->varset_to_idx[ vs_idx ].buffers_end[ it ];

                for( size_t i = s; i < e; ++i )
                {
                    auto & entry = buffer[ i ];

                    auto * var = vs_entry.vs->find_array_variable( entry.name.c_str() );
                    if( var == nullptr )
                    {
                        motor::log::global::error< 1024 >(
                            "[reconnect] : count not find variable %s", entry.name.c_str() );
                        continue;
                    }
                    entry.id = _arrays.find_by_name( var->get().name() );
                }
            };

            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                funk( t, rd->var_sets_buffers_vs );
            }
            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
                funk( t, rd->var_sets_buffers_gs );
            }
            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;
                funk( t, rd->var_sets_buffers_ps );
            }
        }

        // exchage streamout buffers
        {
            auto funk = [ &, this ](
                            d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices const it,
                            d3d11_pimpl_data::render_data_t::buffer_variables_ref_t buffer )
            {
                auto const s = rd->varset_to_idx[ vs_idx ].buffers_so_begin[ it ];
                auto const e = rd->varset_to_idx[ vs_idx ].buffers_so_end[ it ];

                for( size_t i = s; i < e; ++i )
                {
                    auto & entry = buffer[ i ];

                    auto * var = vs_entry.vs->find_streamout_variable( entry.name.c_str() );
                    if( var == nullptr )
                    {
                        motor::log::global::error< 1024 >(
                            "[reconnect] : count not find variable %s", entry.name.c_str() );
                        continue;
                    }
                    entry.id = _streamouts.find_by_name( var->get().name() );
                }
            };

            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                funk( t, rd->var_sets_buffers_so_vs );
            }
            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
                funk( t, rd->var_sets_buffers_so_gs );
            }
            {
                auto const t =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;
                funk( t, rd->var_sets_buffers_so_ps );
            }
        }

        // fix variable set reference
        {
            motor::release( motor::move( old_vs_entry.vs ) );
            old_vs_entry.vs = motor::share( vs_entry.vs );
            old_vs_entry.hash = hash;
        }
    }

    // if vs_idx == size_t(-1) -> create all
    // otherwise, only create for specific idx
    void_t check_and_create_data_for_variable_set( render_datas_t::checked_handle_t rd,
        shader_datas_t::checked_handle_t shd, motor::graphics::render_object_ref_t rc,
        size_t const vs_idx = size_t( -1 ) ) noexcept
    {
        if( rd->has_variable_set( vs_idx ) ) return;

        // borrowed
        motor::graphics::variable_set_mtr_t default_values = &( shd->default_values );

        if( vs_idx == size_t( -1 ) )
        {
            // track ref count for variable set
            {
                rd->varset_to_idx.resize( rc.get_num_variable_sets() );

                rc.for_each( [ & ]( size_t const i,
                                 motor::graphics::render_object_t::variable_set_cref_t vs )
                {
                    if( default_values != nullptr )
                    {
                        vs.vs->clone_from_if_not_exist( default_values );
                    }
                    {
                        d3d11_pimpl_data::render_data_t::varset_to_idx_data entry;
                        entry.idx = rd->var_sets.size();
                        rd->varset_to_idx[ i ] = std::move( entry );
                    }
                    {
                        d3d11_pimpl_data::render_data_t::variable_set vs_;
                        vs_.vs = motor::share( vs.vs );
                        vs_.hash = vs.hash;
                        rd->var_sets.emplace_back( std::move( vs_ ) );
                    }
                } );
            }
        }
        else
        {
            // make indirection entry
            {
                d3d11_pimpl_data::render_data_t::varset_to_idx_data entry;
                entry.idx = rd->var_sets.size();

                if( rd->varset_to_idx.size() <= vs_idx ) rd->varset_to_idx.resize( vs_idx + 1 );
                rd->varset_to_idx[ vs_idx ] = std::move( entry );
            }

            // store variable set itself
            {
                auto entry = rc.borrow_variable_set( vs_idx );
                d3d11_pimpl_data::render_data_t::variable_set vs_;
                vs_.vs = motor::share( entry.vs );
                vs_.hash = entry.hash;
                rd->var_sets.emplace_back( std::move( vs_ ) );
            }

            // assign default values
            {
                auto vs = rc.borrow_variable_set( vs_idx );
                if( default_values != nullptr )
                {
                    vs.vs->clone_from_if_not_exist( default_values );
                }
            }
        }

        // constant buffer mapping
        {
            auto var_funk = []( ID3D11Device * dev, size_t const var_set_idx,
                                motor::graphics::variable_set_mtr_t vs,
                                d3d11_pimpl_data::shader_data_t::cbuffers_ref_t cbuffers,
                                d3d11_pimpl_data::render_data_t::cbuffers_inout_t cbs )
            {
                for( auto & c : cbuffers )
                {
                    d3d11_pimpl_data::render_data_t::cbuffer_t cb;
                    cb.mem = motor::memory::global_t::alloc_raw< uint8_t >(
                        c.sib, "[d3d11] : vertex shader cbuffer variable" );
                    cb.slot = c.slot;

                    D3D11_BUFFER_DESC bd = {};
                    bd.Usage = D3D11_USAGE_DEFAULT;
                    bd.ByteWidth = UINT( c.sib );
                    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                    bd.CPUAccessFlags = 0;

                    D3D11_SUBRESOURCE_DATA InitData = {};
                    InitData.pSysMem = cb.mem;
                    auto const hr = dev->CreateBuffer( &bd, &InitData, cb.ptr );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error(
                            d3d11_backend_log( "D3D11_BIND_CONSTANT_BUFFER" ) );
                    }

                    for( auto & var : c.datas )
                    {
                        d3d11_pimpl_data::render_data_t::data_variable_t dv;
                        dv.ivar = vs->data_variable( var.name, var.t, var.ts );
                        dv.sib = ( var.t == motor::graphics::type::tbool
                                         ? sizeof( uint32_t )
                                         : motor::graphics::size_of( var.t ) ) *
                                 motor::graphics::size_of( var.ts );
                        dv.name = var.name;
                        dv.t = var.t;
                        dv.ts = var.ts;
                        dv.offset = var.offset;
                        cb.data_variables.emplace_back( dv );
                    }
                    cbs.emplace_back( std::move( cb ) );
                }
            };

            // d3d11_pimpl_data::shader_data_ref_t shd = _shaders[ rd->shd_id ] ;

            {
                size_t const a0 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                size_t const a1 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
                size_t const a2 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;

                if( vs_idx == size_t( -1 ) )
                {
                    rc.for_each( [ & ]( size_t const i,
                                     motor::graphics::render_object_t::variable_set_cref_t vs )
                    {
                        auto & mapping_entry = rd->varset_to_idx[ i ];

                        mapping_entry.cbuffer_begin[ a0 ] = rd->_cbuffers_vs.size();
                        mapping_entry.cbuffer_begin[ a1 ] = rd->_cbuffers_gs.size();
                        mapping_entry.cbuffer_begin[ a2 ] = rd->_cbuffers_ps.size();

                        var_funk( _ctx->dev(), i, vs.vs, shd->vs_cbuffers, rd->_cbuffers_vs );
                        var_funk( _ctx->dev(), i, vs.vs, shd->gs_cbuffers, rd->_cbuffers_gs );
                        var_funk( _ctx->dev(), i, vs.vs, shd->ps_cbuffers, rd->_cbuffers_ps );

                        mapping_entry.cbuffer_end[ a0 ] = rd->_cbuffers_vs.size();
                        mapping_entry.cbuffer_end[ a1 ] = rd->_cbuffers_gs.size();
                        mapping_entry.cbuffer_end[ a2 ] = rd->_cbuffers_ps.size();
                    } );
                }
                else
                {
                    auto & mapping_entry = rd->varset_to_idx[ vs_idx ];

                    mapping_entry.cbuffer_begin[ a0 ] = rd->_cbuffers_vs.size();
                    mapping_entry.cbuffer_begin[ a1 ] = rd->_cbuffers_gs.size();
                    mapping_entry.cbuffer_begin[ a2 ] = rd->_cbuffers_ps.size();

                    auto vs = rc.borrow_variable_set( vs_idx );
                    var_funk( _ctx->dev(), vs_idx, vs.vs, shd->vs_cbuffers, rd->_cbuffers_vs );
                    var_funk( _ctx->dev(), vs_idx, vs.vs, shd->gs_cbuffers, rd->_cbuffers_gs );
                    var_funk( _ctx->dev(), vs_idx, vs.vs, shd->ps_cbuffers, rd->_cbuffers_ps );

                    mapping_entry.cbuffer_end[ a0 ] = rd->_cbuffers_vs.size();
                    mapping_entry.cbuffer_end[ a1 ] = rd->_cbuffers_gs.size();
                    mapping_entry.cbuffer_end[ a2 ] = rd->_cbuffers_ps.size();
                }
            }
        }

        // texture variable mapping
        {
            auto var_funk =
                []( ID3D11Device * dev, motor::graphics::variable_set_mtr_t vs,
                    image_datas_t & images,
                    d3d11_pimpl_data::shader_data_t::image_variables_ref_t ps_textures,
                    d3d11_pimpl_data::render_data_t::image_variables_inout_t var_sets_imgs_ps )
            {
                for( auto & t : ps_textures )
                {
                    auto * dv = vs->texture_variable( t.name );
                    motor::string_t const img_name = dv->get().name();

                    size_t const i = images.find_by_name( img_name );
                    if( i == size_t( -1 ) ) continue;

                    d3d11_pimpl_data::render_data_t::image_variable_t iv;
                    iv.value_hash = dv->get().hash();
                    iv.id = i;
                    iv.name = t.name;
                    iv.slot = t.slot;
                    var_sets_imgs_ps.emplace_back( std::move( iv ) );

                    // set y flip for the current texture in the current variable set
                    images.access( i, [ & ]( d3d11_pimpl_data::image_data_ref_t d )
                    {
                        auto * var = vs->data_variable< float_t >( "sys_flipv_" + t.name );
                        var->set( d.requires_y_flip );
                    } );
                }
            };

            {
                size_t const a0 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                size_t const a1 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
                size_t const a2 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;

                if( vs_idx == size_t( -1 ) )
                {
                    rc.for_each( [ & ]( size_t const i,
                                     motor::graphics::render_object_t::variable_set_cref_t vs )
                    {
                        auto & mapping_entry = rd->varset_to_idx[ i ];

                        mapping_entry.imgs_begin[ a0 ] = rd->var_sets_imgs_vs.size();
                        var_funk(
                            _ctx->dev(), vs.vs, _images, shd->vs_textures, rd->var_sets_imgs_vs );
                        mapping_entry.imgs_end[ a0 ] = rd->var_sets_imgs_vs.size();

#if 0 // vs and gs not used right now                        
                        mapping_entry.imgs_begin[a1] = rd->var_sets_imgs_gs.size() ;
                        var_funk( _ctx->dev(), vs.vs, _images, shd->ps_textures, rd->var_sets_imgs_gs ) ;
                        mapping_entry.imgs_end[a1] = rd->var_sets_imgs_gs.size() ;
#endif

                        mapping_entry.imgs_begin[ a2 ] = rd->var_sets_imgs_ps.size();
                        var_funk(
                            _ctx->dev(), vs.vs, _images, shd->ps_textures, rd->var_sets_imgs_ps );
                        mapping_entry.imgs_end[ a2 ] = rd->var_sets_imgs_ps.size();
                    } );
                }
                else
                {
                    auto & mapping_entry = rd->varset_to_idx[ vs_idx ];
                    auto vs = rc.borrow_variable_set( vs_idx );

                    // vs
                    {
                        mapping_entry.imgs_begin[ a0 ] = rd->var_sets_imgs_vs.size();
                        var_funk(
                            _ctx->dev(), vs.vs, _images, shd->vs_textures, rd->var_sets_imgs_vs );
                        mapping_entry.imgs_end[ a0 ] = rd->var_sets_imgs_vs.size();
                    }

                    // gs here
                    {
                    }

                    // ps
                    {
                        mapping_entry.imgs_begin[ a2 ] = rd->var_sets_imgs_ps.size();
                        var_funk(
                            _ctx->dev(), vs.vs, _images, shd->ps_textures, rd->var_sets_imgs_ps );
                        mapping_entry.imgs_end[ a2 ] = rd->var_sets_imgs_ps.size();
                    }
                }
            }
        }

        auto array_variable_mapping =
            [ & ]( // motor::graphics::render_object_ref_t rc_,
                motor::graphics::variable_set_mtr_t vs, pimpl::streamout_datas_t & streamouts,
                d3d11_pimpl_data::render_data_t::buffer_variables_t & buffer_vars,
                d3d11_pimpl_data::render_data_t::buffer_variables_t & buffer_so_vars,
                d3d11_pimpl_data::shader_data_t::buffer_variables_ref_t the_buffer )
        {
            for( auto & t : the_buffer )
            {
                // first try data_buffers...
                motor::string_t const name = vs->array_variable( t.name )->get().name();
                size_t const i = _arrays.find_by_name( name );

                // ... if the stored variable name is found in the data_buffers array, it is used
                // ...
                if( i < _arrays.size() )
                {
                    d3d11_pimpl_data::render_data_t::buffer_variable_t bv;
                    bv.id = i;
                    bv.name = t.name;
                    bv.slot = t.slot;
                    buffer_vars.emplace_back( std::move( bv ) );
                }
                // ... otherwise we default to the streamout objects
                else
                {
                    motor::string_t const name2 =
                        vs->array_variable_streamout( t.name )->get().name();
                    size_t const i2 = streamouts.find_by_name( name2 );

                    if( i2 < streamouts.size() )
                    {
                        d3d11_pimpl_data::render_data_t::buffer_variable_t bv;
                        bv.id = i2;
                        bv.name = t.name;
                        bv.slot = t.slot;
                        buffer_so_vars.emplace_back( std::move( bv ) );
                    }
                }
            }
        };

        {
            size_t const a0 =
                d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
            size_t const a1 =
                d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
            size_t const a2 =
                d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;

            if( vs_idx == size_t( -1 ) )
            {
                rc.for_each( [ & ]( size_t const i,
                                 motor::graphics::render_object_t::variable_set_cref_t vs )
                {
                    auto & mapping_entry = rd->varset_to_idx[ i ];

                    {
                        mapping_entry.buffers_begin[ a0 ] = rd->var_sets_buffers_vs.size();
                        mapping_entry.buffers_so_begin[ a0 ] = rd->var_sets_buffers_so_vs.size();
                        array_variable_mapping( vs.vs, _streamouts, rd->var_sets_buffers_vs,
                            rd->var_sets_buffers_so_vs, shd->vs_buffers );
                        mapping_entry.buffers_so_end[ a0 ] = rd->var_sets_buffers_so_vs.size();
                        mapping_entry.buffers_end[ a0 ] = rd->var_sets_buffers_vs.size();
                    }

                    {
                        mapping_entry.buffers_begin[ a1 ] = rd->var_sets_buffers_gs.size();
                        mapping_entry.buffers_so_begin[ a1 ] = rd->var_sets_buffers_so_gs.size();
                        array_variable_mapping( vs.vs, _streamouts, rd->var_sets_buffers_gs,
                            rd->var_sets_buffers_so_gs, shd->gs_buffers );
                        mapping_entry.buffers_so_end[ a1 ] = rd->var_sets_buffers_so_gs.size();
                        mapping_entry.buffers_end[ a1 ] = rd->var_sets_buffers_gs.size();
                    }

                    {
                        mapping_entry.buffers_begin[ a2 ] = rd->var_sets_buffers_ps.size();
                        mapping_entry.buffers_so_begin[ a2 ] = rd->var_sets_buffers_so_ps.size();
                        array_variable_mapping( vs.vs, _streamouts, rd->var_sets_buffers_ps,
                            rd->var_sets_buffers_so_ps, shd->ps_buffers );
                        mapping_entry.buffers_so_end[ a2 ] = rd->var_sets_buffers_so_ps.size();
                        mapping_entry.buffers_end[ a2 ] = rd->var_sets_buffers_ps.size();
                    }
                } );
            }
            else
            {
                auto & mapping_entry = rd->varset_to_idx[ vs_idx ];

                auto vs = rc.borrow_variable_set( vs_idx );

                {
                    mapping_entry.buffers_begin[ a0 ] = rd->var_sets_buffers_vs.size();
                    mapping_entry.buffers_so_begin[ a0 ] = rd->var_sets_buffers_so_vs.size();
                    array_variable_mapping( vs.vs, _streamouts, rd->var_sets_buffers_vs,
                        rd->var_sets_buffers_so_vs, shd->vs_buffers );
                    mapping_entry.buffers_so_end[ a0 ] = rd->var_sets_buffers_so_vs.size();
                    mapping_entry.buffers_end[ a0 ] = rd->var_sets_buffers_vs.size();
                }

                {
                    mapping_entry.buffers_begin[ a1 ] = rd->var_sets_buffers_gs.size();
                    mapping_entry.buffers_so_begin[ a1 ] = rd->var_sets_buffers_so_gs.size();
                    array_variable_mapping( vs.vs, _streamouts, rd->var_sets_buffers_gs,
                        rd->var_sets_buffers_so_gs, shd->gs_buffers );
                    mapping_entry.buffers_so_end[ a1 ] = rd->var_sets_buffers_so_gs.size();
                    mapping_entry.buffers_end[ a1 ] = rd->var_sets_buffers_gs.size();
                }

                {
                    mapping_entry.buffers_begin[ a2 ] = rd->var_sets_buffers_ps.size();
                    mapping_entry.buffers_so_begin[ a2 ] = rd->var_sets_buffers_so_ps.size();
                    array_variable_mapping( vs.vs, _streamouts, rd->var_sets_buffers_ps,
                        rd->var_sets_buffers_so_ps, shd->ps_buffers );
                    mapping_entry.buffers_so_end[ a2 ] = rd->var_sets_buffers_so_ps.size();
                    mapping_entry.buffers_end[ a2 ] = rd->var_sets_buffers_ps.size();
                }
            }
        }
    }

    //************************************************************************************************************
    // must be called from within a safe area(i.e. through an access into _renders)
    bool_t update( render_datas_t::checked_handle_t rd, motor::graphics::render_object_ref_t rc )
    {
        rd->shd_id = size_t( -1 );

        // find geometry
        {
            rd->geo_ids.clear();
            for( size_t i = 0; i < rc.get_num_geometry(); ++i )
            {
                auto const id_ = _geos.find_by_name( rc.get_geometry_link( i ).name );

                if( id_ == size_t( -1 ) )
                {
                    motor::log::global_t::warning( d3d11_backend_log(
                        "no geometry with name [" + rc.get_geometry_link( i ).name +
                        "] for render_config [" + rc.name() + "]" ) );
                    continue;
                }

                rd->geo_ids.emplace_back( id_ );
            }
        }

        // handle stream out links
        {
            rd->so_ids.clear();
            for( size_t i = 0; i < rc.get_num_streamout(); ++i )
            {
                auto const id_ = _streamouts.find_by_name( rc.get_streamout( i ) );

                if( id_ == size_t( -1 ) )
                {
                    motor::log::global_t::warning(
                        d3d11_backend_log( "no streamout with name [" + rc.get_streamout( i ) +
                                           "] for render_config [" + rc.name() + "]" ) );
                    continue;
                }

                rd->so_ids.emplace_back( id_ );
            }
        }

        // find shader
        if( rd->shd_id == size_t( -1 ) )
        {
            auto const id_ = _shaders.find_by_name( rc.get_shader() );
            if( id_ == size_t( -1 ) )
            {
                motor::log::global_t::warning(
                    d3d11_backend_log( "no streamout with name [" + rc.get_shader() +
                                       "] for render_config [" + rc.name() + "]" ) );
                return false;
            }
            rd->shd_id = id_;
        }

        // may happen if shaders did not compile properly the first time.
        if( rd->shd_id == size_t( -1 ) /*|| _shaders[ rd->shd_id ].vs_blob == nullptr*/ )
        {
            motor::log::global_t::warning( d3d11_backend_log(
                "something strange happened to render_config [" + rc.name() + "]" ) );
            return false;
        }

        {
            if( !this_t::bind_vertex_inputs( rd ) )
            {
                return false;
            }
        }

        // release placeholder/ref count manager variable sets
        {
            rd->varset_to_idx.clear();
            // rd->var_set_to_idx_map.clear() ;

            for( auto & vs : rd->var_sets ) motor::memory::release_ptr( vs.vs );
            rd->var_sets.clear();
        }

        {
            auto release_funk = [ & ]( d3d11_pimpl_data::render_data_t::cbuffers_ref_t datum )
            {
                for( auto & b : datum )
                {
                    if( b.mem != nullptr )
                    {
                        motor::memory::global_t::dealloc_raw( b.mem );
                        b.mem = nullptr;
                    }
                    b.ptr = d3d11_pimpl_data::guard< ID3D11Buffer >();
                }
                datum.clear();
            };
            release_funk( rd->_cbuffers_vs );
            release_funk( rd->_cbuffers_gs );
            release_funk( rd->_cbuffers_ps );
        }

        {
            rd->var_sets_imgs_vs.clear();
            rd->var_sets_imgs_ps.clear();

            rd->_cbuffers_vs.clear();
            rd->_cbuffers_gs.clear();
            rd->_cbuffers_ps.clear();

            rd->var_sets_buffers_vs.clear();
            rd->var_sets_buffers_gs.clear();
            rd->var_sets_buffers_ps.clear();

            rd->var_sets_buffers_so_vs.clear();
            rd->var_sets_buffers_so_gs.clear();
            rd->var_sets_buffers_so_ps.clear();
        }

        if( auto shd = shader_datas_t::handle_t( rd->shd_id, _shaders ); shd )
        {
            this_t::check_and_create_data_for_variable_set(
                rd, shader_datas_t::checked( shd ), rc );
        }

        return true;
    }

    //************************************************************************************************************
    bool_t update_geometry_link(
        motor::graphics::render_object_ref_t ro, size_t const geo_idx ) noexcept
    {
        auto rd = _renders.access( ro.get_oid( _bid ), ro.name() );
        if( !rd ) return false;

        // do full binding
        if( rd->geo_ids.size() <= geo_idx )
        {
            auto const gid = _geos.find_by_name( ro.get_geometry_link( geo_idx ).name );
            if( gid == size_t( -1 ) )
            {
                motor::log::global_t::warning< 1024 >(
                    "[d3d11:update_geometry_link] : no geometry with name [%s] for render_data "
                    "[%s]",
                    ro.get_geometry_link( geo_idx ).name.c_str(), ro.name().c_str() );
                return false;
            }
            rd->geo_ids.emplace_back( d3d11_pimpl_data::render_data_t::geo_id_data{
                gid, ro.get_geometry_link( geo_idx ).hash } );
            //_geos.access( gid, [&]( d3d11_pimpl_data::geo_data_ref_t d ){
            // d.add_render_data_id( new_id ) ; } ) ;

            auto const res = this_t::bind_vertex_inputs( render_datas_t::checked_handle_t( rd ) );
        }
        else if( rd->geo_ids[ geo_idx ].hash != ro.get_geometry_link( geo_idx ).hash )
        {
            auto const hash = ro.get_geometry_link( geo_idx ).hash;
            if( ro.get_geometry_link( geo_idx ).ref_count == 0 )
            {
                rd->invalidate_geometry_index( geo_idx, hash );
            }
            else if( rd->geo_ids[ geo_idx ].id != size_t( -1 ) )
            {
                // config geometry entry is valid.
                // maybe the data changed lets check it
                {
                    auto const gid = _geos.find_by_name( ro.get_geometry_link( geo_idx ).name );
                    if( gid != rd->geo_ids[ geo_idx ].id )
                    {
                        // ok, geometry changed
                        rd->geo_ids[ geo_idx ].id = gid;

                        // at this point, the geometry needs to be rebound
                        // to the shader inputs.
                        auto const res = this_t::bind_vertex_inputs(
                            render_datas_t::checked_handle_t( rd ), geo_idx );
                    }
                }

                rd->geo_ids[ geo_idx ].hash = hash;
            }
            else
            {
                auto const gid = _geos.find_by_name( ro.get_geometry_link( geo_idx ).name );
                rd->geo_ids[ geo_idx ].id = gid;
                rd->geo_ids[ geo_idx ].hash = hash;
            }
        }
        else
        {
            // nothing to do
            // everything is valid.
        }

        return true;
    }

    //************************************************************************************************************
    bool_t update_render_object( motor::graphics::render_object_ref_t obj, size_t const vs_id )
    {

        size_t const ro_oid = obj.get_oid( _bid );
        auto rd = render_datas_t::handle_t( ro_oid, _renders );
        if( rd )
        {
            return this_t::update_render_data( render_datas_t::checked_handle_t( rd ), obj, vs_id );
        }

        return false;
    }

    //************************************************************************************************************
    bool_t update_msl( motor::graphics::msl_object_ref_t obj, size_t const vs_id )
    {
        auto msld = msl_datas_t::handle_t( obj.get_oid( _bid ), _msls );
        if( msld )
        {
            return this_t::update_render_object( *obj.borrow_render_object(), vs_id );
        }
        return false;
    }

    //************************************************************************************************************
    bool_t construct_image_config( motor::graphics::image_object_ref_t obj )
    {
        size_t oid = obj.get_oid( _bid );
        auto const res = _images.access( oid, obj.name(),
            [ & ]( motor::string_in_t img_name, d3d11_pimpl_data::image_data_ref_t img )
        {
            if( img.texture != nullptr )
            {
                img.texture->Release();
                img.sampler->Release();
                img.view->Release();

                img.texture = nullptr;
                img.sampler = nullptr;
                img.view = nullptr;
            }

            auto * dev = _ctx->dev();

            // sampler
            {
                D3D11_SAMPLER_DESC sampDesc = {};
                sampDesc.Filter = motor::platform::d3d11::convert(
                    obj.get_filter( motor::graphics::texture_filter_mode::min_filter ),
                    obj.get_filter( motor::graphics::texture_filter_mode::mag_filter ) );

                sampDesc.AddressU = motor::platform::d3d11::convert(
                    obj.get_wrap( motor::graphics::texture_wrap_mode::wrap_s ) );

                sampDesc.AddressV = motor::platform::d3d11::convert(
                    obj.get_wrap( motor::graphics::texture_wrap_mode::wrap_t ) );

                sampDesc.AddressW = motor::platform::d3d11::convert(
                    obj.get_wrap( motor::graphics::texture_wrap_mode::wrap_r ) );

                sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
                sampDesc.MinLOD = 0;
                sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

                auto const hr = dev->CreateSamplerState( &sampDesc, img.sampler );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "CreateSamplerState" ) );
                    return false;
                }
            }

            // texture
            {
                auto & iref = obj.image();

                size_t const width = iref.get_dims().x();
                size_t const height = iref.get_dims().y();
                size_t const depth = iref.get_dims().z();

                D3D11_TEXTURE2D_DESC desc = {};
                desc.Width = static_cast< UINT >( width );
                desc.Height = static_cast< UINT >( height );
                desc.ArraySize = static_cast< UINT >( depth );
                desc.MipLevels = static_cast< UINT >( 1 );
                desc.Format = motor::platform::d3d11::convert(
                    iref.get_image_format(), iref.get_image_element_type() );
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags = 0;

                size_t const bpp = motor::graphics::size_of( iref.get_image_format() ) *
                                   motor::graphics::size_of( iref.get_image_element_type() );

                // uint8_ptr_t mem = motor::memory::global_t::alloc_raw<uint8_t>( width * height * 4
                // ) ; for( size_t p = 0; p < width * height * 4; ++p ) mem[ p ] = uint8_t( 255 ) ;

                // create the texture object
                {
                    motor::memory::malloc_guard< D3D11_SUBRESOURCE_DATA > init_datas( depth );

                    for( size_t i = 0; i < depth; ++i )
                    {
                        init_datas[ i ].pSysMem = iref.get_image_ptr( i );
                        init_datas[ i ].SysMemPitch = UINT( width * bpp );
                        init_datas[ i ].SysMemSlicePitch = UINT( width * height * bpp );
                    }

                    auto const hr = dev->CreateTexture2D( &desc, init_datas.get(), img.texture );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "CreateTexture2D" ) );
                        return false;
                    }
                }

                // create the resource view for the texture in order to be
                // shader variable bindable.
                if( img.texture != nullptr )
                {
                    D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
                    res_desc.Format = desc.Format;
                    if( obj.get_type() == motor::graphics::texture_type::texture_2d )
                    {
                        res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                        res_desc.Texture2D.MostDetailedMip = 0;
                        res_desc.Texture2D.MipLevels = UINT( 1 );
                    }
                    else if( obj.get_type() == motor::graphics::texture_type::texture_2d_array )
                    {
                        res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                        res_desc.Texture2DArray.ArraySize = UINT( depth );
                        res_desc.Texture2DArray.MostDetailedMip = 0;
                        res_desc.Texture2DArray.MipLevels = UINT( 1 );
                        res_desc.Texture2DArray.FirstArraySlice = UINT( 0 );
                    }

                    auto const hr =
                        dev->CreateShaderResourceView( img.texture, &res_desc, img.view );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log(
                            "CreateShaderResourceView for texture : [" + img_name + "]" ) );
                        return false;
                    }
                }
            }

            return true;
        } );

        if( res ) obj.set_oid( _bid, oid );
        return res;
    }

    //************************************************************************************************************
    void_t release_image_data( size_t const oid ) noexcept
    {
        _images.invalidate( oid );
    }

    //************************************************************************************************************
    size_t construct_array_data( size_t oid, motor::graphics::array_object_ref_t obj ) noexcept
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : construct array object" );

        // only vec4 float allowed
        {
            bool_t valid = true;
            obj.data_buffer().for_each_layout_element(
                [ & ]( motor::graphics::data_buffer::layout_element_cref_t le )
            {
                if( le.type != motor::graphics::type::tfloat &&
                    le.type_struct != motor::graphics::type_struct::vec4 )
                    valid = false;
            } );

            if( !valid )
            {
                motor::log::global_t::error(
                    "[d3d11] : data buffer must only consist of vec4f layout elements." );
                return oid;
            }
        }

        // oid = this_t::determine_oid( oid, obj.name(), _arrays ) ;
        // d3d11_pimpl_data::array_data_ref_t data = _arrays[ oid ] ;
        auto const res =
            _arrays.access( oid, obj.name(), [ & ]( d3d11_pimpl_data::array_data_ref_t data )
        {
            ID3D11Device * dev = _ctx->dev();

            // = number of vertices * sizeof( vertex )
            // if there are vertices already setup, take the numbers
            size_t const byte_width = obj.data_buffer().get_sib();

            if( data.buffer != nullptr )
            {
                data.buffer->Release();
                data.buffer = nullptr;
            }

            // this release is important for releasing the
            // buffer reference too.
            if( data.view != nullptr )
            {
                data.view->Release();
                data.view = nullptr;
            }

            // must be 16 in order to have access to all data.
            // this is independent from the number of layout elements.
            // layout elements may only have vec4f inserted!
            size_t const elem_sib = 16; // sizeof( vec4f ) ;

            // create the buffer
            {
                D3D11_BUFFER_DESC bd = {};
                bd.Usage = D3D11_USAGE_DYNAMIC;
                // using max of 1 sib so empty vbs will create and will
                // be resized in the update function. Important for now
                // is that the buffer will be NOT nullptr.
                bd.ByteWidth = UINT( std::max( byte_width, elem_sib ) );
                bd.BindFlags =
                    UINT( D3D11_BIND_VERTEX_BUFFER ) | UINT( D3D10_BIND_SHADER_RESOURCE );
                bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

                HRESULT const hr = dev->CreateBuffer( &bd, nullptr, data.buffer );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_DATA_BUFFER" ) );
                }
            }

            // create the resource view
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {};
                res_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                res_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

                res_desc.Buffer.FirstElement = 0;

                // somehow, this can not be 0
                // otherwise, hr will be in valid.
                res_desc.Buffer.NumElements = std::max( UINT( byte_width ) / UINT( elem_sib ), 1u );

                auto const hr = dev->CreateShaderResourceView( data.buffer, &res_desc, data.view );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log(
                        "CreateShaderResourceView for buffer : [" + obj.name() + "]" ) );
                }
            }

            return true;
        } );

        return oid;
    }

    //************************************************************************************************************
    void_t release_array_data( size_t const oid ) noexcept
    {
        _arrays.invalidate( oid );
    }

    //************************************************************************************************************
    bool_t update(
        size_t const id, motor::graphics::array_object_ref_t obj, bool_t const is_config )
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : update array object" );

        // auto & data = _arrays[ id ] ;

        _arrays.access( id, [ & ]( d3d11_pimpl_data::array_data_ref_t data )
        {
            D3D11_BUFFER_DESC bd;
            data.buffer->GetDesc( &bd );

            size_t const sib = obj.data_buffer().get_sib();
            if( sib > bd.ByteWidth )
            {
                this->construct_array_data( id, obj );
            }

            // copy data
            {
                // exactly the size in bytes of the 1d buffer
                LONG const lsib = LONG( obj.data_buffer().get_layout_sib() );
                LONG const ne = LONG( obj.data_buffer().get_num_elements() );
                void_cptr_t data_ptr = obj.data_buffer().data();

                D3D11_MAPPED_SUBRESOURCE resource;
                _ctx->ctx()->Map( data.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
                std::memcpy( resource.pData, data_ptr, lsib * ne );
                _ctx->ctx()->Unmap( data.buffer, 0 );
            }
        } );

        return true;
    }

    //************************************************************************************************************
    bool_t update(
        size_t const id, motor::graphics::image_object_ref_t obj, bool_t const is_config ) noexcept
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : update image object" );

        auto & iref = obj.image();

        return _images.access( id, [ & ]( d3d11_pimpl_data::image_data_ref_t data )
        {
            // check if image has changed
            {
                D3D11_TEXTURE2D_DESC bd;
                data.texture->GetDesc( &bd );

                size_t const width = iref.get_dims().x();
                size_t const height = iref.get_dims().y();
                size_t const depth = iref.get_dims().z();

                bool_t const b1 = bd.Width != width;
                bool_t const b2 = bd.Height != height;
                bool_t const b3 = bd.ArraySize != depth;
                bool_t const b4 =
                    bd.Format != motor::platform::d3d11::convert(
                                     iref.get_image_format(), iref.get_image_element_type() );

                if( motor::math::vec4b_t( b1, b2, b3, b4 ).any() )
                {
                    return this->construct_image_config( obj );
                }
            }

            // copy data if only the content has changed.
            {
                size_t const width = iref.get_dims().x();
                size_t const height = iref.get_dims().y();
                size_t const depth = iref.get_dims().z();

                D3D11_BOX region;
                region.left = 0u;
                region.right = (UINT)width;
                region.top = 0u;
                region.bottom = (UINT)height;
                region.front = 0u;
                region.back = (UINT)depth;

                UINT const row_pitch = (UINT)iref.row_sib();
                UINT const depth_pitch = (UINT)iref.layer_sib();

                _ctx->ctx()->UpdateSubresource(
                    data.texture, 0, &region, iref.get_image_ptr(), row_pitch, depth_pitch );
            }

            return true;
        } );
    }

    //************************************************************************************************************
    // update the variable data in the api, i.e. it copies the data from engine to api memory
    bool_t update_render_data( render_datas_t::checked_handle_t rd,
        motor::graphics::render_object_ref_t obj, size_t const varset_id )
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : update render object" );

        auto shd = shader_datas_t::handle_t( rd->shd_id, _shaders );
        if( !shd )
        {
            motor::log::global_t::error< 1024 >(
                "[d3d11::update_render_data] : invalid shader data with %zu", rd->shd_id );
            return false;
        }

        auto shdc = shader_datas_t::checked( shd );

        // search if variable set exists
        {
            auto entry = obj.borrow_variable_set( varset_id );
            if( rd->has_not_variable_set( varset_id ) )
            {
                this_t::check_and_create_data_for_variable_set( rd, shdc, obj, varset_id );
            }
            else if( rd->has_variable_set_changed( varset_id, entry.hash ) )
            {
                this_t::reconnect_data_for_variable_set( rd, shdc, obj, varset_id, entry.hash );
            }
        }

        // data variables
        {
            auto update_funk =
                [ & ]( ID3D11DeviceContext * ctx_, d3d11_pimpl_data::render_data_t::cbuffer_t & cb )
            {
                for( auto & dv : cb.data_variables )
                {
                    if( dv.ivar == nullptr ) continue;
                    dv.do_copy_funk_from_origin( cb.mem );
                }
                ctx_->UpdateSubresource( cb.ptr, 0, nullptr, cb.mem, 0, 0 );
            };

            if( rd->has_variable_set( varset_id ) )
            {
                auto const & entry = rd->varset_to_idx[ varset_id ];
                size_t const a0 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                size_t const a1 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
                size_t const a2 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;

                for( size_t i = entry.cbuffer_begin[ a0 ]; i < entry.cbuffer_end[ a0 ]; ++i )
                {
                    update_funk( _ctx->ctx(), rd->_cbuffers_vs[ i ] );
                }
                for( size_t i = entry.cbuffer_begin[ a1 ]; i < entry.cbuffer_end[ a1 ]; ++i )
                {
                    update_funk( _ctx->ctx(), rd->_cbuffers_gs[ i ] );
                }
                for( size_t i = entry.cbuffer_begin[ a2 ]; i < entry.cbuffer_end[ a2 ]; ++i )
                {
                    update_funk( _ctx->ctx(), rd->_cbuffers_ps[ i ] );
                }
            }
        }

        // textures
        // all we need to do is to figure out if a texture variable has
        // changed its name. If so, assign correct image id.
        {
            auto update_funk =
                [ & ]( size_t const vsid, d3d11_pimpl_data::render_data_t::image_variable_t & iv )
            {
                auto * vs = rd->var_sets[ rd->varset_to_idx[ vsid ].idx ].vs;

                {
                    auto * tx_var = vs->find_texture_variable( iv.name.c_str() );

                    // if nullptr, variable does not exist anymore
                    // what to do then?
                    if( tx_var == nullptr ) return;

                    if( tx_var->get().hash() == iv.value_hash ) return;
                    iv.value_hash = tx_var->get().hash();

                    size_t const idx = _images.find_by_name( tx_var->get().name() );
                    iv.id = idx;
                }
            };

            if( rd->has_variable_set( varset_id ) )
            {
                auto const & entry = rd->varset_to_idx[ varset_id ];
                size_t const a0 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;
                size_t const a1 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;
                size_t const a2 =
                    d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;

                for( size_t i = entry.imgs_begin[ a0 ]; i < entry.imgs_end[ a0 ]; ++i )
                {
                    update_funk( varset_id, rd->var_sets_imgs_vs[ i ] );
                }
#if 0 // currently not implemented
                    for( size_t i=entry.imgs_begin[a1]; i<entry.imgs_end[a1]; ++i )
                    {
                        update_funk( varset_id, rd->var_sets_imgs_gs[i] ) ;
                    }
#endif
                for( size_t i = entry.imgs_begin[ a2 ]; i < entry.imgs_end[ a2 ]; ++i )
                {
                    update_funk( varset_id, rd->var_sets_imgs_ps[ i ] );
                }
            }
        }

        return true;
    }

    //************************************************************************************************************
    bool_t render( size_t const id, size_t const geo_idx = 0, bool_t feed_from_so = false,
        bool_t use_streamout_count = false, size_t const varset_id = size_t( 0 ),
        UINT const start_element = UINT( 0 ), UINT const num_elements = UINT( -1 ) )
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : render" );

        // d3d11_pimpl_data::render_data_ref_t rnd = _renders[ id ] ;

        auto rd = render_datas_t::handle_t( id, _renders );
        if( !rd )
        {
            motor::log::global_t::error< 1024 >(
                "[d3d11::render] : invalid render data with %zu", id );
            return false;
        }

        auto shd = shader_datas_t::handle_t( rd->shd_id, _shaders );
        if( !shd )
        {
            motor::log::global_t::error< 1024 >(
                "[d3d11::render] : invalid shader data with %zu", rd->shd_id );
            return false;
        }

        if( rd->has_not_variable_set( varset_id ) ) return false;

        if( geo_idx >= rd->geo_ids.size() && !feed_from_so )
        {
            motor::log::global_t::error( "[d3d11::render] : used geometry idx invalid "
                                         "because exceeds array size for render object : " +
                                         rd.name() );
            return false;
        }

        ID3D11DeviceContext * ctx = _ctx->ctx();

        {
            if( shd->vs == nullptr )
            {
                motor::log::global_t::error< 1024 >(
                    "[d3d11::render] : shader missing for %s", shd.name().c_str() );
                return false;
            }

            // SECTION: Set all shaders
            {
                ctx->VSSetShader( shd->vs, nullptr, 0 );
                ctx->GSSetShader( shd->gs, nullptr, 0 );
                ctx->PSSetShader( shd->ps, nullptr, 0 );
            }
        }

        // SECTION: vertex shader variables
        if( rd->has_variable_set( varset_id ) )
        {
            auto const access =
                d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::vertex_shader;

            auto const idx = rd->varset_to_idx[ varset_id ].idx;

            // update constant buffers/variables
            {
                auto const begin = rd->varset_to_idx[ varset_id ].cbuffer_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].cbuffer_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & cb = rd->_cbuffers_vs[ i ];
                    ctx->VSSetConstantBuffers( cb.slot, 1, cb.ptr );
                }
            }

            // update buffer variables
            {
                auto const begin = rd->varset_to_idx[ varset_id ].buffers_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].buffers_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & buf = rd->var_sets_buffers_vs[ i ];
                    if( auto hnd = array_datas_t::handle_t( buf.id, _arrays ); hnd )
                        ctx->VSSetShaderResources( buf.slot, 1, hnd->view );
                }
            }

            // update streamout buffers
            {
                auto const begin = rd->varset_to_idx[ varset_id ].buffers_so_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].buffers_so_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & buf = rd->var_sets_buffers_so_vs[ i ];

                    if( auto hnd = streamout_datas_t::handle_t( buf.id, _streamouts ); hnd )
                    {
                        auto & ppb = hnd->read_buffer();
                        ctx->VSSetShaderResources( buf.slot, 1, ppb.views[ 0 ] );
                    }
                }
            }
        }

        // SECTION: geometry shader variables
        if( rd->has_variable_set( varset_id ) )
        {
            auto const access =
                d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::geometry_shader;

            auto const idx = rd->varset_to_idx[ varset_id ].idx;

            // update data variables
            {
                auto const begin = rd->varset_to_idx[ varset_id ].cbuffer_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].cbuffer_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & cb = rd->_cbuffers_gs[ i ];
                    ctx->GSSetConstantBuffers( cb.slot, 1, cb.ptr );
                }
            }

            // update buffer variables
            {
                auto const begin = rd->varset_to_idx[ varset_id ].buffers_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].buffers_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & buf = rd->var_sets_buffers_gs[ i ];
                    if( auto hnd = array_datas_t::handle_t( buf.id, _arrays ); hnd )
                        ctx->GSSetShaderResources( buf.slot, 1, hnd->view );
                }
            }

            // update streamout buffers
            {
                auto const begin = rd->varset_to_idx[ varset_id ].buffers_so_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].buffers_so_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & buf = rd->var_sets_buffers_so_gs[ i ];

                    if( auto hnd = streamout_datas_t::handle_t( buf.id, _streamouts ); hnd )
                    {
                        auto & ppb = hnd->read_buffer();
                        ctx->GSSetShaderResources( buf.slot, 1, ppb.views[ 0 ] );
                    }
                }
            }
        }

        // SECTION: tesselation shader variables
        {
        }

        // SECTION: pixel shader variables
        if( rd->has_variable_set( varset_id ) )
        {
            auto const access =
                d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;
            auto const idx = rd->varset_to_idx[ varset_id ].idx;

            {
                auto const begin = rd->varset_to_idx[ varset_id ].cbuffer_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].cbuffer_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & cb = rd->_cbuffers_ps[ i ];
                    ctx->PSSetConstantBuffers( cb.slot, 1, cb.ptr );
                }
            }

            {
                auto const begin = rd->varset_to_idx[ varset_id ].imgs_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].imgs_end[ access ];
                for( size_t i = begin; i < end; ++i )
                {
                    auto & img = rd->var_sets_imgs_ps[ i ];

                    _images.access( img.id, [ & ]( d3d11_pimpl_data::image_data_ref_t imgd )
                    {
                        ctx->PSSetShaderResources( img.slot, 1, imgd.view );
                        ctx->PSSetSamplers( img.slot, 1, imgd.sampler );
                    } );
                }
            }

            {
                auto const begin = rd->varset_to_idx[ varset_id ].buffers_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].buffers_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & buf = rd->var_sets_buffers_ps[ i ];

                    _arrays.access( buf.id, [ & ]( d3d11_pimpl_data::array_data_ref_t ad )
                    { ctx->PSSetShaderResources( buf.slot, 1, ad.view ); } );
                }
            }
        }

        // feed from streamout path
        if( ( feed_from_so || use_streamout_count ) && rd->so_ids.size() != 0 )
        {
            D3D11_QUERY_DATA_SO_STATISTICS so_stats = { 0 };

            // feedback data path
            if( !feed_from_so )
            {
#if 1
                if( auto sod = streamout_datas_t::handle_t( rd->so_ids[ 0 ], _streamouts ); sod )
                {
                    size_t const ridx = sod->read_index();
                    auto & rbuffer = sod->read_buffer();

                    HRESULT hr = S_FALSE;
                    while( hr != S_OK )
                    {
                        hr = _ctx->ctx()->GetData( sod->read_buffer().query, &so_stats,
                            sizeof( D3D11_QUERY_DATA_SO_STATISTICS ), 0 );
                    }
                }

#else
                _streamouts.access( rd->so_ids[ 0 ], [ & ]( d3d11_pimpl_data::so_data_ref_t so )
                {
                    size_t const ridx = so.read_index();
                    auto & rbuffer = so.read_buffer();

                    HRESULT hr = S_FALSE;
                    while( hr != S_OK )
                    {
                        hr = _ctx->ctx()->GetData( so.read_buffer().query, &so_stats,
                            sizeof( D3D11_QUERY_DATA_SO_STATISTICS ), 0 );
                    }
                    return true;
                } );
#endif
            }

            // feedback draw path
            // this path uses the streamout primitives as input data and the number of captured
            // primitives using DrawAuto
            if( feed_from_so )
            {
#if 1
                if( auto sod = streamout_datas_t::handle_t( rd->so_ids[ 0 ], _streamouts ); sod )
                {
                    auto & rbuffer = sod->read_buffer();

                    UINT const stride = sod->stride;
                    UINT const offset = 0;
                    ctx->IASetVertexBuffers( 0, 1, rbuffer.buffers[ 0 ], &stride, &offset );
                    ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( sod->pt ) );
                    ctx->IASetInputLayout( rd->vertex_layout_so );

                    ctx->DrawAuto();
                }
#else
                // d3d11_pimpl_data::so_data_ref_t so = _streamouts[ rd->so_ids[ 0 ] ] ;
                _streamouts.access( rd->so_ids[ 0 ], [ & ]( d3d11_pimpl_data::so_data_ref_t so )
                {
                    auto & rbuffer = so.read_buffer();

                    UINT const stride = so.stride;
                    UINT const offset = 0;
                    ctx->IASetVertexBuffers( 0, 1, rbuffer.buffers[ 0 ], &stride, &offset );
                    ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( so.pt ) );
                    ctx->IASetInputLayout( rd->vertex_layout_so );

                    ctx->DrawAuto();
                    return true;
                } );
#endif
            }

            // this path uses the geometry but should render just the number of captured
            // primitives using DrawAuto
            else
            {
#if 1
                if( auto geod = geometry_datas_t::handle_t( rd->geo_ids[ geo_idx ].id, _geos );
                    geod )
                {
                    UINT const stride = geod->stride;
                    UINT const offset = 0;
                    ctx->IASetVertexBuffers( 0, 1, geod->vb, &stride, &offset );
                    ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( geod->pt ) );
                    ctx->IASetInputLayout( rd->vertex_layout );

                    // AutoDraw does not seem to work when binding a different vertex buffer
                    // then the used streamout buffer. If the other geometry uses index or
                    // instanced data, AutoDraw doesn't work anyways according to doc.
                    // @todo no index geo implemented, see remarks in geo path below.
                    ctx->Draw( UINT( so_stats.NumPrimitivesWritten ), UINT( start_element ) );
                }
#else
                // d3d11_pimpl_data::geo_data_ref_t geo = _geos[ rd->geo_ids[ geo_idx ] ] ;

                _geos.access( rd->geo_ids[ geo_idx ].id,
                    [ & ]( d3d11_pimpl_data::geo_data_ref_t geo )

                {
                    UINT const stride = geo.stride;
                    UINT const offset = 0;
                    ctx->IASetVertexBuffers( 0, 1, geo.vb, &stride, &offset );
                    ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( geo.pt ) );
                    ctx->IASetInputLayout( rd->vertex_layout );

                    // AutoDraw does not seem to work when binding a different vertex buffer
                    // then the used streamout buffer. If the other geometry uses index or
                    // instanced data, AutoDraw doesn't work anyways according to doc.
                    // @todo no index geo implemented, see remarks in geo path below.
                    ctx->Draw( UINT( so_stats.NumPrimitivesWritten ), UINT( start_element ) );

                    return true;
                } );
#endif
            }
        }

        // @todo
        // if use_streamout_count && !feed_from_so
        // this path must be used with the queried number of prims written in so_stats
        else // feed from geometry path
        {
#if 1
            if( auto geod = geometry_datas_t::handle_t( rd->geo_ids[ geo_idx ].id, _geos ); geod )
            {
#if 1
                if( auto sod = streamout_datas_t::handle_t( _cur_streamout_active, _streamouts );
                    sod )
                {
                    sod->pt = geod->pt;
                }
#else
                if( _cur_streamout_active != size_t( -1 ) )
                {
                    _streamouts.access( _cur_streamout_active,
                        [ & ]( d3d11_pimpl_data::so_data_ref_t so ) { so.pt = geod->pt; } );
                }
#endif
                {
                    UINT const stride = geod->stride;
                    UINT const offset = 0;
                    ctx->IASetVertexBuffers( 0, 1, geod->vb, &stride, &offset );
                    ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( geod->pt ) );
                }

                ctx->IASetInputLayout( rd->vertex_layout );

                if( geod->num_elements_ib != 0 )
                {
                    ctx->IASetIndexBuffer( geod->ib, DXGI_FORMAT_R32_UINT, 0 );

                    UINT const max_elems =
                        num_elements == UINT( -1 ) ? UINT( geod->num_elements_ib ) : num_elements;
                    ctx->DrawIndexed( max_elems, start_element, 0 );
                }
                else
                {
                    UINT const max_elems =
                        num_elements == UINT( -1 ) ? UINT( geod->num_elements_vb ) : num_elements;
                    ctx->Draw( max_elems, UINT( start_element ) );
                }
            }
#else
            // d3d11_pimpl_data::geo_data_ref_t geo = _geos[ rd->geo_ids[ geo_idx ] ] ;
            _geos.access( rd->geo_ids[ geo_idx ].id, [ & ]( d3d11_pimpl_data::geo_data_ref_t geo )
            {
                if( _cur_streamout_active != size_t( -1 ) )
                {
                    _streamouts.access( _cur_streamout_active,
                        [ & ]( d3d11_pimpl_data::so_data_ref_t so ) { so.pt = geo.pt; } );
                }

                {
                    UINT const stride = geo.stride;
                    UINT const offset = 0;
                    ctx->IASetVertexBuffers( 0, 1, geo.vb, &stride, &offset );
                    ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( geo.pt ) );
                }

                ctx->IASetInputLayout( rd->vertex_layout );

                if( geo.num_elements_ib != 0 )
                {
                    ctx->IASetIndexBuffer( geo.ib, DXGI_FORMAT_R32_UINT, 0 );

                    UINT const max_elems =
                        num_elements == UINT( -1 ) ? UINT( geo.num_elements_ib ) : num_elements;
                    ctx->DrawIndexed( max_elems, start_element, 0 );
                }
                else
                {
                    UINT const max_elems =
                        num_elements == UINT( -1 ) ? UINT( geo.num_elements_vb ) : num_elements;
                    ctx->Draw( max_elems, UINT( start_element ) );
                }
            } );
#endif
        }

        // SECTION: UNBIND pixel shader variables
        if( rd->has_variable_set( varset_id ) )
        {
            auto const access =
                d3d11_pimpl_data::render_data_t::varset_to_idx_data::indices::pixel_shader;
            auto const idx = rd->varset_to_idx[ varset_id ].idx;

            {
                auto const begin = rd->varset_to_idx[ varset_id ].cbuffer_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].cbuffer_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & cb = rd->_cbuffers_ps[ i ];

                    ID3D11Buffer * const null_buffer[ 1 ] = { nullptr };
                    ctx->PSSetConstantBuffers( cb.slot, 1, null_buffer );
                }
            }

            {
                auto const begin = rd->varset_to_idx[ varset_id ].imgs_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].imgs_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & img = rd->var_sets_imgs_ps[ i ];
                    ID3D11ShaderResourceView * const null_view[ 1 ] = { nullptr };
                    ctx->PSSetShaderResources( img.slot, 1, null_view );
                }
            }

            {
                auto const begin = rd->varset_to_idx[ varset_id ].buffers_begin[ access ];
                auto const end = rd->varset_to_idx[ varset_id ].buffers_end[ access ];

                for( size_t i = begin; i < end; ++i )
                {
                    auto & buf = rd->var_sets_buffers_ps[ i ];

                    ID3D11ShaderResourceView * const null_view[ 1 ] = { nullptr };
                    ctx->PSSetShaderResources( buf.slot, 1, null_view );
                }
            }
        }

        return true;
    }

    //************************************************************************************************************
    void_t begin_frame( void_t )
    {
        // set the viewport to the default new state,
        // so the correct viewport is set automatically.
        {
            auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) );

            //_states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp
            _states.access( ids_new.first, [ & ]( d3d11_pimpl_data::state_data_ref_t sd )
            {
                motor::math::vec4ui_t vp = sd.states[ ids_new.second ].view_s.ss.vp;
                vp.z( uint_t( vp_width ) );
                vp.w( uint_t( vp_height ) );
                sd.states[ ids_new.second ].view_s.ss.vp = vp;

                {
                    d3d11_pimpl_data::render_state_sets rss;
                    rss.rss = sd.states[ ids_new.second ];
                    _state_stack.push( rss );
                    this->handle_render_state( rss, false );

                    // rss is filled in handle_render_state, so repush that one
                    _state_stack.pop();
                    _state_stack.push( rss );
                }
            } );
            //_states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp = vp ;
        }

        {
            motor::math::vec4f_t const color = _state_stack.top().rss.clear_s.ss.clear_color;

            _ctx->clear_render_target_view( color );
            _ctx->clear_depth_stencil_view();
        }
    }

    //************************************************************************************************************
    void_t end_frame( void_t )
    {
        _ctsd->for_each( [ & ]( d3d11_independent::backward_message & item )
        {
            std::visit( [ this ]( auto && arg )
            {
                using T = std::decay_t< decltype( arg ) >;
                if constexpr( std::is_same_v< T, d3d11_independent::compilation_result > )
                {
                    this_t::construct_msl_data( std::move( arg ) );
                }
                else if constexpr( std::is_same_v< T, d3d11_independent::release_result > )
                {
                }
                else
                    static_assert( false, "non-exhaustive visitor!" );
            }, item );
        } );

        auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) );

        assert( _state_stack.size() == 1 );

        // simply pop the begin frame render states
        // and also release all state objects.
        auto rss = _state_stack.pop();
        this->handle_render_state( rss, true );

        _ctsd->notify_thread();
    }
};

//************************************************************************************************
//
//
//************************************************************************************************

//************************************************************************************************************
d3d11_backend::d3d11_backend( motor::platform::d3d11::rendering_context_ptr_t ctx ) noexcept
{
    _pimpl = motor::memory::global_t::alloc(
        this_t::pimpl( this_t::get_bid(), ctx ), "d3d11_backend::pimpl" );
    _context = ctx;
}

//************************************************************************************************************
d3d11_backend::d3d11_backend( this_rref_t rhv ) noexcept : backend( std::move( rhv ) )
{
    _pimpl = motor::move( rhv._pimpl );
    _context = motor::move( rhv._context );
}

//************************************************************************************************************
d3d11_backend::~d3d11_backend( void_t )
{
    this_t::report_live_device_objects( D3D11_RLDO_SUMMARY );
    motor::memory::global_t::dealloc( _pimpl );
}

//************************************************************************************************************
void_t d3d11_backend::set_window_info( window_info_cref_t wi ) noexcept
{
    {
        // bool_t change = false ;
        if( wi.width != 0 )
        {
            _pimpl->vp_width = FLOAT( wi.width );
        }
        if( wi.height != 0 )
        {
            _pimpl->vp_height = FLOAT( wi.height );
        }
    }
}

//******************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::msl_object_mtr_t obj ) noexcept
{
    if( obj == nullptr )
    {
        d3d11_log_errorv( "msl object is nullptr" );
        return motor::graphics::result::invalid_argument;
    }

    auto const oid = obj->get_oid( this_t::get_bid() );

    // is it in transit
    if( oid == size_t( -2 ) )
    {
        return motor::graphics::result::in_transit;
    }

    _pimpl->send_configure_to_st( obj );
    return motor::graphics::result::in_transit;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure(
    motor::graphics::geometry_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "geometry object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    size_t const oid = obj->set_oid(
        this_t::get_bid(), _pimpl->construct_geo( obj->get_oid( this_t::get_bid() ), *obj ) );

    {
        auto const res = _pimpl->update( oid, obj );
        if( !res )
        {
            return motor::graphics::result::failed;
        }
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure(
    motor::graphics::render_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "render object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    if( _pimpl->construct_render_data( *obj ) )
    {
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure(
    motor::graphics::shader_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "shader object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->construct_shader_config( *obj );
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::image_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "msl object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    auto const res = _pimpl->construct_image_config( *obj );
    return res ? motor::graphics::result::ok : motor::graphics::result::failed;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure(
    motor::graphics::framebuffer_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "framebuffer object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    auto const res = _pimpl->construct_framebuffer( *obj );

    return res ? motor::graphics::result::ok : motor::graphics::result::failed;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "state object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    auto const res = _pimpl->construct_state( *obj );

    return res ? motor::graphics::result::ok : motor::graphics::result::failed;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::array_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "array object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    size_t const oid = obj->set_oid( this_t::get_bid(),
        _pimpl->construct_array_data( obj->get_oid( this_t::get_bid() ), *obj ) );

    {
        auto const res = _pimpl->update( oid, *obj, true );
        if( !res ) return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure(
    motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "streamout object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    size_t const oid = obj->set_oid(
        this_t::get_bid(), _pimpl->construct_streamout( obj->get_oid( this_t::get_bid() ), *obj ) );

    {
        auto const res = _pimpl->update( oid, *obj, true );
        if( !res ) return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::msl_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "msl object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_msl_data( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    obj->borrow_render_object()->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release(
    motor::graphics::geometry_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "geometry object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_geometry( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::render_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "render object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_render_data( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "shader object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_shader_data( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::image_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "image object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_image_data( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release(
    motor::graphics::framebuffer_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "framebuffer object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_framebuffer( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "state object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_state( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::array_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "array object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_array_data( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release(
    motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "streamout object is nullptr or has no name" );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_streamout( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::geometry_object_mtr_t obj ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        motor::log::global_t::error( d3d11_backend_log( "invalid geometry object id" ) );
        return motor::graphics::result::failed;
    }

    auto const res = _pimpl->update( oid, obj );
    motor::log::global_t::error( !res, d3d11_backend_log( "update geometry" ) );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::array_object_mtr_t obj ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() );

    {
        auto const res = _pimpl->update( oid, *obj, false );
        if( !res ) return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update(
    motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() );

    {
        auto const res = _pimpl->update( oid, *obj, false );
        if( !res ) return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::image_object_mtr_t obj ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( !_pimpl->update( oid, *obj, false ) )
    {
        return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::use( motor::graphics::framebuffer_object_mtr_t obj ) noexcept
{
    if( obj == nullptr )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
    }

    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
    }

    auto const res = _pimpl->activate_framebuffer( oid );
    if( !res ) return motor::graphics::result::failed;

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::use( motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    if( obj == nullptr )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::streamout );
    }

    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::streamout );
    }

    auto const res = _pimpl->activate_streamout( oid );
    if( !res ) return motor::graphics::result::failed;

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::unuse(
    motor::graphics::gen4::backend::unuse_type const t ) noexcept
{
    switch( t )
    {
    case motor::graphics::gen4::backend::unuse_type::framebuffer:
        _pimpl->deactivate_framebuffer();
        break;
    case motor::graphics::gen4::backend::unuse_type::streamout:
        _pimpl->deactivate_streamout();
        break;
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::push(
    motor::graphics::state_object_mtr_t obj, size_t const sid, bool_t const ) noexcept
{
    if( obj == nullptr )
    {
        return this_t::pop( motor::graphics::gen4::backend::pop_type::render_state );
    }

    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        return this_t::pop( motor::graphics::gen4::backend::pop_type::render_state );
    }

    _pimpl->update_state( oid, *obj );
    _pimpl->handle_render_state( oid, sid );

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::pop(
    motor::graphics::gen4::backend::pop_type const ) noexcept
{
    _pimpl->handle_render_state( size_t( -1 ), size_t( -1 ) );
    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::prepare_render( motor::graphics::render_object_mtr_t,
    motor::graphics::gen4::backend::render_detail_cref_t ) noexcept
{
    return motor::graphics::result::invalid;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::prepare_render( motor::graphics::msl_object_mtr_t,
    motor::graphics::gen4::backend::render_detail_cref_t ) noexcept
{
    return motor::graphics::result::invalid;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::render( motor::graphics::render_object_mtr_t obj,
    motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept
{
    if( obj == nullptr ) return motor::graphics::result::invalid_argument;

    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        d3d11_log_errorv( "[render|render_object] : invalid id for %s", obj->name().c_str() );
        return motor::graphics::result::failed;
    }

    {
        auto const res = _pimpl->update_geometry_link( *obj, detail.geo );
        if( !res ) return motor::graphics::result::failed;
    }

    // update variables
    {
        _pimpl->update_render_object( *obj, detail.varset );
    }

    // render
    {
        _pimpl->render( oid, detail.geo, detail.feed_from_streamout, detail.use_streamout_count,
            detail.varset, (UINT)detail.start, (UINT)detail.num_elems );
    }

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::render( motor::graphics::msl_object_mtr_t obj,
    motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept
{
    if( obj == nullptr ) return motor::graphics::result::invalid_argument;

    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        d3d11_log_errorv( "[render|render_object] : invalid id for %s", obj->name().c_str() );
        return motor::graphics::result::failed;
    }

    if( oid == size_t( -2 ) )
    {
        return motor::graphics::result::in_transit;
    }

#if 1
    return this_t::render( obj->borrow_render_object(), detail );
#else
    motor::graphics::result res = motor::graphics::result::failed;
    _pimpl->_msls.try_access( oid, [ & ]( d3d11_pimpl_data::msl_data_ref_t msl )
    { return this_t::render( obj->borrow_render_object(), detail ); } );
    return res;
#endif
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::render(
    motor::graphics::gen4::backend::render_detail_cref_t ) noexcept
{
    return motor::graphics::result::failed;
}

//************************************************************************************************************
void_t d3d11_backend::render_begin( void_t ) noexcept
{
    _pimpl->begin_frame();
}

//************************************************************************************************************
void_t d3d11_backend::render_end( void_t ) noexcept
{
    _pimpl->end_frame();
}

//************************************************************************************************************
void_t d3d11_backend::report_live_device_objects( D3D11_RLDO_FLAGS const flags ) noexcept
{
#ifdef D3D_DEBUG
    if( _context != nullptr ) _context->debug()->ReportLiveDeviceObjects( flags );
#endif
}

void_t d3d11_backend::commit_frame_snapshot( void_t ) noexcept {}