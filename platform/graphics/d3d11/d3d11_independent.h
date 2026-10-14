// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "d3d11_log.h"
#include "d3d11_pimpl_data.h"

#include <variant>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <directxcolors.h>

namespace d3d11_detail
{

using namespace motor::core::types;

struct d3d11_independent
{
    motor_this_typedefs( d3d11_independent );

    static motor::graphics::gen4::backend_type const bt =
        motor::graphics::gen4::backend_type::d3d11;
    static motor::graphics::shader_api_type const sapi = motor::graphics::shader_api_type::hlsl_5_0;

    struct compilation_result
    {
        bool_t success = false;

        // safe reference. hold ref count
        motor::graphics::msl_object_mtr_t object = nullptr;

        motor::graphics::shader_object_t shader;
        d3d11_pimpl_data::shader_data_t data;
    };
    struct release_result
    {
    };
    using backward_message = std::variant< compilation_result, release_result >;

    //************************************************************************************************************
    static motor::string_t remove_unwanded_characters( motor::string_cref_t code_in ) noexcept
    {
        auto code = code_in;

        // remove \r
        {
            size_t pos = 0;
            while( pos != code.size() )
            {
                if( code[ pos ] == '\r' ) code[ pos ] = ' ';
                ++pos;
            }
        }

        // remove preceding, trailing ' '
        {
            size_t a = code.find_first_not_of( ' ' );
            size_t b = code.find_last_not_of( ' ' );
            a = a == std::string::npos ? 0 : a;
            b = b == std::string::npos ? code.size() - a : b - a;
            code = code.substr( a, b + 1 );
        }

        return std::move( code );
    }

    //************************************************************************************************************
    // performs reflection on the constant buffers present in
    // a shader for user variable binding
    static d3d11_pimpl_data::shader_data_t::cbuffers_t determine_cbuffer( ID3DBlob * blob ) noexcept
    {
        d3d11_pimpl_data::shader_data_t::cbuffers_t ret;

        ID3D11ShaderReflection * reflector = nullptr;
        D3DReflect( blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection,
            (void **)&reflector );

        D3D11_SHADER_DESC shd_desc;
        {
            auto const hr = reflector->GetDesc( &shd_desc );
            if( FAILED( hr ) )
            {
                motor::log::global_t::error(
                    d3d11_backend_log( "Can not get shader reflection desc" ) );
                return d3d11_pimpl_data::shader_data_t::cbuffers_t();
            }
        }

        UINT const num_cb = shd_desc.ConstantBuffers;
        for( UINT i = 0; i < num_cb; ++i )
        {
            d3d11_pimpl_data::shader_data_t::cbuffer cbuffer;

            ID3D11ShaderReflectionConstantBuffer * cbr = reflector->GetConstantBufferByIndex( i );

            D3D11_SHADER_BUFFER_DESC buf_desc;
            {
                auto hr = cbr->GetDesc( &buf_desc );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_SHADER_BUFFER_DESC" ) );
                    continue;
                }
            }
            cbuffer.name = motor::string_t( buf_desc.Name );
            cbuffer.sib = size_t( buf_desc.Size );

            // figure out the register the resource is bound to
            // this is done through cbuffer xyz : register(bn)
            // where n needs to be determined
            {
                D3D11_SHADER_INPUT_BIND_DESC res_desc;
                auto const hr = reflector->GetResourceBindingDescByName( buf_desc.Name, &res_desc );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error(
                        d3d11_backend_log( "D3D11_SHADER_INPUT_BIND_DESC" ) );
                    continue;
                }
                cbuffer.slot = res_desc.BindPoint;
            }

            for( UINT j = 0; j < buf_desc.Variables; ++j )
            {
                ID3D11ShaderReflectionVariable * var = cbr->GetVariableByIndex( j );

                D3D11_SHADER_VARIABLE_DESC var_desc;
                {
                    auto const hr = var->GetDesc( &var_desc );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error(
                            d3d11_backend_log( "D3D11_SHADER_VARIABLE_DESC" ) );
                        continue;
                    }
                }

                ID3D11ShaderReflectionType * type = var->GetType();
                D3D11_SHADER_TYPE_DESC type_desc;
                {
                    auto const hr = type->GetDesc( &type_desc );
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error(
                            d3d11_backend_log( "D3D11_SHADER_TYPE_DESC" ) );
                        continue;
                    }
                }

                if( motor::platform::d3d11::is_texture_type( type_desc.Type ) )
                {
                    // this is in another function - determine_texture
                }
                else if( motor::platform::d3d11::is_buffer_type( type_desc.Type ) )
                {
                    // this is in another function - determin_texture
                }
                else
                {
                    UINT const elems = std::max( type_desc.Rows, type_desc.Columns );
                    auto const ts =
                        motor::platform::d3d11::to_type_struct( type_desc.Class, elems );
                    auto const t = motor::platform::d3d11::to_type( type_desc.Type );
                    if( ts == motor::graphics::type_struct::undefined ||
                        t == motor::graphics::type::undefined )
                        continue;

                    d3d11_pimpl_data::shader_data_t::data_variable_t dv;
                    dv.name = motor::string_t( var_desc.Name );
                    dv.offset = var_desc.StartOffset;
                    dv.t = t;
                    dv.ts = ts;

                    cbuffer.datas.emplace_back( dv );
                }
            }

            ret.emplace_back( std::move( cbuffer ) );
        }
        return std::move( ret );
    }

    //************************************************************************************************************
    static bool_t determine_texture( ID3DBlob * blob,
        d3d11_pimpl_data::shader_data_t::image_variables_out_t img_vars,
        d3d11_pimpl_data::shader_data_t::buffer_variables_out_t buf_vars ) noexcept
    {
        d3d11_pimpl_data::guard< ID3D11ShaderReflection > reflector;
        D3DReflect( blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection,
            reflector );

        D3D11_SHADER_DESC shd_desc;
        {
            auto const hr = reflector->GetDesc( &shd_desc );
            if( FAILED( hr ) )
            {
                motor::log::global_t::error(
                    d3d11_backend_log( "Can not get shader reflection desc" ) );
                return false;
            }
        }

        UINT const num_irb = shd_desc.BoundResources;
        for( UINT i = 0; i < num_irb; ++i )
        {
            D3D11_SHADER_INPUT_BIND_DESC ibd;
            {
                auto const hr = reflector->GetResourceBindingDesc( i, &ibd );
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "GetResourceBindingDesc" ) );
                    return false;
                }
            }

            if( ( ibd.Type != D3D_SIT_TEXTURE ) && ( ibd.Type != D3D_SIT_SAMPLER ) ) continue;

            // somehow, we can not do this:
            // ID3D11ShaderReflectionVariable * var = reflector->GetVariableByName( ibd.Name ) ;
            // getting the type from the variable will not work. So we are going through
            // yet another D3D11 types enum.
            if( ibd.Type == D3D_SIT_SAMPLER )
            {
                // ?
            }
            else if( ibd.Type == D3D_SIT_TEXTURE && ibd.Dimension == D3D_SRV_DIMENSION_TEXTURE2D )
            {
                d3d11_pimpl_data::shader_data_t::image_variable_t ivar;
                ivar.name = ibd.Name;
                ivar.slot = ibd.BindPoint;
                img_vars.emplace_back( ivar );
            }
            else if( ibd.Type == D3D_SIT_TEXTURE &&
                     ibd.Dimension == D3D_SRV_DIMENSION_TEXTURE2DARRAY )
            {
                d3d11_pimpl_data::shader_data_t::image_variable_t ivar;
                ivar.name = ibd.Name;
                ivar.slot = ibd.BindPoint;
                img_vars.emplace_back( ivar );
            }
            else if( ibd.Type == D3D_SIT_TEXTURE && ibd.Dimension == D3D_SRV_DIMENSION_BUFFER )
            {
                d3d11_pimpl_data::shader_data_t::buffer_variable_t ivar;
                ivar.name = ibd.Name;
                ivar.slot = ibd.BindPoint;
                buf_vars.emplace_back( ivar );
            }
            else
            {
                motor::log::global_t::warning(
                    d3d11_backend_log( "Detected texture but type is not supported." ) );
            }
        }

        return true;
    }

    //************************************************************************************************************
    static bool_t construct_shader_data( motor::graphics::shader_object_ref_t obj,
        d3d11_pimpl_data::shader_data_inout_t shd, motor::platform::d3d11::rendering_context_ptr_t ctx ) noexcept
    {
        auto tp_begin = std::chrono::high_resolution_clock::now();

        shd.compiled = false;

        // shader code
        motor::graphics::shader_set_t ss;
        {
            auto const res = obj.shader_set( this_t::sapi, ss );
            if( !res )
            {
                motor::log::global_t::warning(
                    d3d11_backend_log( "config [" + obj.name() + "] has no shaders for " +
                                       motor::graphics::gen4::to_string( this_t::bt ) ) );
                return false;
            }
        }

        ID3DBlob * pVSBlob = nullptr;
        ID3DBlob * pGSBlob = nullptr;
        ID3DBlob * pPSBlob = nullptr;

        // compile
        {
            ID3DBlob * errblob = nullptr;

            // vertex shader
            {
                D3D_SHADER_MACRO Shader_Macros[] = { NULL, NULL };

                auto code = this_t::remove_unwanded_characters( ss.vertex_shader().code() );
                void_cptr_t cptr = code.c_str();
                size_t const sib = ss.vertex_shader().code().size();

                // "vs_5_0" : Direct3D 11 and 11.1
                // "vs_4_1" : Direct3D 10.1
                // "vs_4_0" : Direct3D 10
                {
                    auto const hr = D3DCompile( cptr, sib, obj.name().c_str(), Shader_Macros,
                        nullptr, "VS", "vs_5_0", 0, 0, &pVSBlob, &errblob );

                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::warning(
                            d3d11_backend_log( "vertex shader [" + obj.name() + "] failed " +
                                               motor::graphics::gen4::to_string( this_t::bt ) ) );

                        if( errblob != nullptr )
                        {
                            char_cptr_t ptr = (char_cptr_t)errblob->GetBufferPointer();
                            size_t const ssib = (size_t)errblob->GetBufferSize();
                            motor::string_t s( ptr, ssib );

                            auto const pos = s.find( obj.name() );
                            s = s.substr( pos, s.size() - pos );
                            motor::log::global_t::error( s );
                            errblob->Release();
                        }

                        shd.invalidate( obj.name() );

                        return false;
                    }
                }

                // find shader variables in constant buffer
                {
                    shd.vs_cbuffers = this_t::determine_cbuffer( pVSBlob );
                    this_t::determine_texture( pVSBlob, shd.vs_textures, shd.vs_buffers );
                }
            }

            // geometry shader
            if( ss.has_geometry_shader() )
            {
                D3D_SHADER_MACRO Shader_Macros[] = { NULL, NULL };

                auto code = this_t::remove_unwanded_characters( ss.geometry_shader().code() );
                void_cptr_t cptr = code.c_str();
                size_t const sib = ss.geometry_shader().code().size();

                // "gs_5_0" : Direct3D 11 and 11.1
                // "gs_4_1" : Direct3D 10.1
                // "gs_4_0" : Direct3D 10
                auto const hr = D3DCompile( cptr, sib, obj.name().c_str(), Shader_Macros, nullptr,
                    "GS", "gs_5_0", 0, 0, &pGSBlob, &errblob );

                if( FAILED( hr ) )
                {
                    motor::log::global_t::warning(
                        d3d11_backend_log( "geometry shader [" + obj.name() + "] failed " +
                                           motor::graphics::gen4::to_string( this_t::bt ) ) );

                    if( errblob != nullptr )
                    {
                        char_cptr_t ptr = (char_cptr_t)errblob->GetBufferPointer();
                        size_t const ssib = (size_t)errblob->GetBufferSize();
                        motor::string_t s( ptr, ssib );

                        auto const pos = s.find( obj.name() );
                        s = s.substr( pos, s.size() - pos );
                        motor::log::global_t::error( s );
                        errblob->Release();
                    }

                    shd.invalidate( obj.name() );

                    return false;
                }

                // find shader variables in constant buffer
                {
                    shd.gs_cbuffers = this_t::determine_cbuffer( pGSBlob );
                    this_t::determine_texture( pGSBlob, shd.gs_textures, shd.gs_buffers );
                }
            }

            // pixel shader
            if( ss.has_pixel_shader() )
            {
                D3D_SHADER_MACRO Shader_Macros[] = { NULL, NULL };

                auto code = this_t::remove_unwanded_characters( ss.pixel_shader().code() );
                void_cptr_t cptr = code.c_str();
                size_t const sib = ss.pixel_shader().code().size();

                // "ps_5_0" : Direct3D 11 and 11.1
                // "ps_4_1" : Direct3D 10.1
                // "ps_4_0" : Direct3D 10
                auto const hr = D3DCompile( cptr, sib, obj.name().c_str(), Shader_Macros, nullptr,
                    "PS", "ps_5_0", 0, 0, &pPSBlob, &errblob );

                if( FAILED( hr ) )
                {
                    motor::log::global_t::warning(
                        d3d11_backend_log( "pixel shader [" + obj.name() + "] failed " +
                                           motor::graphics::gen4::to_string( this_t::bt ) ) );

                    if( errblob != nullptr )
                    {
                        char_cptr_t ptr = (char_cptr_t)errblob->GetBufferPointer();
                        size_t const ssib = (size_t)errblob->GetBufferSize();
                        motor::string_t s( ptr, ssib );

                        auto const pos = s.find( obj.name() );
                        s = s.substr( pos, s.size() - pos );
                        motor::log::global_t::error( s );
                        errblob->Release();
                    }

                    shd.invalidate( obj.name() );

                    return false;
                }

                // find shader variables in constant buffer
                {
                    shd.ps_cbuffers = this_t::determine_cbuffer( pPSBlob );
                    this_t::determine_texture( pPSBlob, shd.ps_textures, shd.ps_buffers );
                }
            }
        }

        // vertex shader
        if( pVSBlob != nullptr )
        {
            auto const hr = ctx->dev()->CreateVertexShader(
                pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, shd.vs );

            motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreateVertexShader" );

            // the vs blob is required for later layout creation
            shd.vs_blob = pVSBlob;
        }

        // Create Streamout shader w or w/o geometry shader
        if( obj.shader_bindings().get_num_output_bindings() > 0 &&
            obj.get_streamout_mode() != motor::graphics::streamout_mode::unknown )
        {
            static size_t const max_entries = 10;
            D3D11_SO_DECLARATION_ENTRY decl[ max_entries ];
            if( obj.shader_bindings().get_num_output_bindings() > max_entries )
            {
                motor::log::global_t::warning( "[d3d11_backend] : you can not have more than " +
                                               motor::to_string( max_entries ) +
                                               " vertex output bindings for stream out" );
            }

            obj.shader_bindings().for_each_vertex_output_binding(
                [ & ]( size_t const i, motor::graphics::vertex_attribute const va,
                    motor::graphics::ctype const ct, motor::string_in_t )
            {
                D3D11_SO_DECLARATION_ENTRY e{ 0,
                    motor::platform::d3d11::vertex_output_binding_to_semantic( va ),
                    motor::platform::d3d11::vertex_output_binding_to_semantic_index( va ), 0,
                    (BYTE)motor::graphics::num_components_of( ct.ts ), 0 };
                decl[ std::min( max_entries, i ) ] = e;
            } );

            // @todo which stream should go to the rasterizer?
            UINT const rasterized_stream = pPSBlob == nullptr ? D3D11_SO_NO_RASTERIZED_STREAM : 0;

            // if no geometry shader, use vertex shader
            if( pGSBlob == nullptr )
            {
                auto const hr = ctx->dev()->CreateGeometryShaderWithStreamOutput(
                    pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), decl,
                    std::min( UINT( max_entries ),
                        UINT( obj.shader_bindings().get_num_output_bindings() ) ),
                    NULL, 0, rasterized_stream, NULL, shd.gs );
                motor::log::global_t::error(
                    FAILED( hr ), "[d3d11] : CreateGeometryShaderWithStreamOutput" );
            }
            else
            {
                auto const hr = ctx->dev()->CreateGeometryShaderWithStreamOutput(
                    pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), decl,
                    std::min( UINT( max_entries ),
                        UINT( obj.shader_bindings().get_num_output_bindings() ) ),
                    NULL, 0, rasterized_stream, NULL, shd.gs );
                motor::log::global_t::error(
                    FAILED( hr ), "[d3d11] : CreateGeometryShaderWithStreamOutput" );
            }
        }

        // geometry shader w/o streamout
        else if( pGSBlob != nullptr )
        {
            auto const hr = ctx->dev()->CreateGeometryShader(
                pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, shd.gs );

            motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreateGeometryShader" );
            pGSBlob->Release();
            pGSBlob = nullptr;
        }

        // pixel shader
        if( pPSBlob != nullptr )
        {
            auto const hr = ctx->dev()->CreatePixelShader(
                pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, shd.ps );

            motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreatePixelShader" );
            pPSBlob->Release();
            pPSBlob = nullptr;
        }

        // shader input variables
        // the final layout elements for the d3d input layout
        // is done if the render configuration is known.
        {
            obj.shader_bindings().for_each_vertex_input_binding(
                [ & ]( size_t const, motor::graphics::vertex_attribute const va,
                    motor::string_cref_t name )
            {
                shd.vertex_inputs.emplace_back(
                    d3d11_pimpl_data::shader_data_t::vertex_input_binding{ va, name } );
            } );
        }

        motor_status2( 1024, "[D3D11] : Compilation Successful : [%s]", obj.name().c_str() );

        shd.compiled = true;

        {
            size_t const milli = std::chrono::duration_cast< std::chrono::milliseconds >(
                std::chrono::high_resolution_clock::now() - tp_begin )
                                     .count();

            motor_status2(
                2048, "[d3d11] : shader compilation %zu ms [%s]", milli, obj.name().c_str() );
        }

        return true;
    }
};

} // namespace d3d11_detail
