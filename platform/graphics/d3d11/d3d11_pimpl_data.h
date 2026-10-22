// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "d3d11.h"
#include "d3d11_convert.h"
#include "../datas.hpp"

#include <d3d11.h>

#include <optional>

namespace d3d11_pimpl_data
{
using namespace motor::core::types;
using namespace motor::platform ;

template < typename T >
class guard
{
    motor_this_typedefs( guard< T > );

  private:

    T * _ptr = nullptr;

  public:

    this_ref_t invalidate( void_t ) noexcept
    {
        if( _ptr != nullptr ) _ptr->Release();
        _ptr = nullptr;
        return *this;
    }

    guard( void_t ) noexcept {}
    guard( this_rref_t rhv ) noexcept
    {
        motor_move_member_ptr( _ptr, rhv );
    }
    ~guard( void_t ) noexcept
    {
        this_t::invalidate();
    }

    operator T const *( void_t ) const noexcept
    {
        return _ptr;
    }

    operator T *( void_t ) noexcept
    {
        return _ptr;
    }

    operator T **( void_t ) noexcept
    {
        return &_ptr;
    }

    operator void **( void_t ) noexcept
    {
        return (void **)&_ptr;
    }

    T * operator->( void_t ) noexcept
    {
        return _ptr;
    }

    T * move_out( void_t ) noexcept
    {
        T * ret = _ptr;
        _ptr = nullptr;
        return ret;
    }

    bool_t operator!=( T * other ) const noexcept
    {
        return _ptr != other;
    }

    bool_t operator==( T * other ) const noexcept
    {
        return _ptr == other;
    }

    // just replaces the pointer
    this_ref_t operator=( T * other ) noexcept
    {
        _ptr = other;
        return *this;
    }

    // invalidates this and takes other
    this_ref_t operator=( this_rref_t other ) noexcept
    {
        this_t::invalidate();
        _ptr = other.move_out();
        return *this;
    }
};

//*******************************************************************************************
struct so_data
{
    motor_this_typedefs( so_data );

    // this is for rendering the streamout buffer
    struct layout_element
    {
        motor::graphics::vertex_attribute va;
        motor::graphics::type type;
        motor::graphics::type_struct type_struct;

        size_t sib( void_t ) const noexcept
        {
            return size_t(
                motor::graphics::size_of( type ) * motor::graphics::size_of( type_struct ) );
        }
    };
    using elements_t = motor::vector< layout_element >;
    elements_t elements;

    motor::graphics::primitive_type pt = motor::graphics::primitive_type::undefined;

    // per vertex sib
    UINT stride = 0;

    static const size_t max_buffers = 4;
    size_t buffers_used = 0;

    typedef struct
    {
        guard< ID3D11Buffer > buffers[ max_buffers ];
        guard< ID3D11ShaderResourceView > views[ max_buffers ];

        guard< ID3D11Query > query;
    } buffer_t;

    buffer_t _ping_pong[ 2 ];

    // read index of ping-pong
    size_t rd_idx = 0;

    size_t read_index( void_t ) const noexcept
    {
        return rd_idx;
    }
    size_t write_index( void_t ) const noexcept
    {
        return ( rd_idx + 1 ) % 2;
    }
    void_t swap( void_t ) noexcept
    {
        rd_idx = ++rd_idx % 2;
    }

    buffer_t & read_buffer( void_t ) noexcept
    {
        return _ping_pong[ this_t::read_index() ];
    }
    buffer_t const & read_buffer( void_t ) const noexcept
    {
        return _ping_pong[ this_t::read_index() ];
    }

    buffer_t & write_buffer( void_t ) noexcept
    {
        return _ping_pong[ this_t::write_index() ];
    }
    buffer_t const & write_buffer( void_t ) const noexcept
    {
        return _ping_pong[ this_t::write_index() ];
    }

    so_data( void_t ) noexcept {}
    so_data( so_data const & ) = delete;
    so_data( so_data && rhv ) noexcept
    {
        stride = rhv.stride;
        pt = rhv.pt;

        elements = std::move( rhv.elements );
        buffers_used = rhv.buffers_used;

        for( size_t b = 0; b < 2; ++b )
        {
            for( size_t i = 0; i < buffers_used; ++i )
            {
                _ping_pong[ b ].buffers[ i ] = std::move( rhv._ping_pong[ b ].buffers[ i ] );
                _ping_pong[ b ].query = std::move( rhv._ping_pong[ b ].query );
            }
        }

        rd_idx = rhv.rd_idx;
    }
    ~so_data( void_t ) noexcept
    {
        invalidate( std::nullopt );
    }

    DXGI_FORMAT get_format_from_element( motor::graphics::vertex_attribute const va ) const noexcept
    {
        return this_t::get_format_from_element( va, this->elements );
    }

    static DXGI_FORMAT get_format_from_element(
        motor::graphics::vertex_attribute const va, this_t::elements_t const & elems ) noexcept
    {
        for( auto const & e : elems )
        {
            if( e.va == va )
            {
                return motor::platform::d3d11::convert_type_to_vec_format( e.type, e.type_struct );
            }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    UINT get_sib( motor::graphics::vertex_attribute const va ) const noexcept
    {
        return this_t::get_sib( va, elements );
    }

    static UINT get_sib(
        motor::graphics::vertex_attribute const va, this_t::elements_t const & elems ) noexcept
    {
        size_t ret = 0;
        for( auto const & e : elems )
        {
            if( e.va == va )
            {
                ret =
                    motor::graphics::size_of( e.type ) * motor::graphics::size_of( e.type_struct );
            }
        }
        return UINT( ret );
    }

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        elements.clear();

        for( size_t b = 0; b < 2; ++b )
        {
            for( size_t i = 0; i < buffers_used; ++i )
            {
                _ping_pong[ b ].buffers[ i ].invalidate();
                _ping_pong[ b ].query.invalidate();
            }
        }
        buffers_used = 0;
        rd_idx = size_t( -1 );
    }
};
motor_typedef( so_data );
using streamout_datas_t = datas< d3d11_pimpl_data::so_data_t >;

//*******************************************************************************************
struct geo_data
{
    void_ptr_t vb_mem = nullptr;
    void_ptr_t ib_mem = nullptr;

    size_t num_elements_vb = 0;
    size_t num_elements_ib = 0;

    size_t sib_vb = 0;
    size_t sib_ib = 0;

    guard< ID3D11Buffer > vb;
    guard< ID3D11Buffer > ib;

    // per vertex sib
    UINT stride = 0;

    motor::graphics::primitive_type pt = motor::graphics::primitive_type::undefined;

    struct layout_element
    {
        motor::graphics::vertex_attribute va;
        motor::graphics::type type;
        motor::graphics::type_struct type_struct;

        size_t sib( void_t ) const noexcept
        {
            return size_t(
                motor::graphics::size_of( type ) * motor::graphics::size_of( type_struct ) );
        }
    };
    using elements_t = motor::vector< layout_element >;
    elements_t elements;

    geo_data( void_t ) noexcept {}
    geo_data( geo_data const & ) = delete;
    geo_data( geo_data && rhv ) noexcept
    {
        motor_move_member_ptr( vb_mem, rhv );
        motor_move_member_ptr( ib_mem, rhv );

        num_elements_vb = rhv.num_elements_vb;
        num_elements_ib = rhv.num_elements_ib;

        sib_vb = rhv.sib_vb;
        sib_ib = rhv.sib_ib;

        vb = std::move( rhv.vb );
        ib = std::move( rhv.ib );

        stride = rhv.stride;

        pt = rhv.pt;

        elements = std::move( rhv.elements );
    }
    geo_data & operator=( geo_data && rhv ) noexcept
    {
        motor_move_member_ptr( vb_mem, rhv );
        motor_move_member_ptr( ib_mem, rhv );

        num_elements_vb = rhv.num_elements_vb;
        num_elements_ib = rhv.num_elements_ib;

        sib_vb = rhv.sib_vb;
        sib_ib = rhv.sib_ib;

        vb = std::move( rhv.vb );
        ib = std::move( rhv.ib );

        stride = rhv.stride;

        pt = rhv.pt;

        elements = std::move( rhv.elements );

        return *this;
    }

    ~geo_data( void_t ) noexcept
    {
        invalidate( std::nullopt );
    }

    DXGI_FORMAT get_format_from_element( motor::graphics::vertex_attribute const va ) const noexcept
    {
        for( auto const & e : elements )
        {
            if( e.va == va )
            {
                return motor::platform::d3d11::convert_type_to_vec_format( e.type, e.type_struct );
            }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    static DXGI_FORMAT get_format_from_element(
        motor::graphics::vertex_attribute const va, geo_data::elements_t const & elems ) noexcept
    {
        for( auto const & e : elems )
        {
            if( e.va == va )
            {
                return motor::platform::d3d11::convert_type_to_vec_format( e.type, e.type_struct );
            }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    UINT get_sib( motor::graphics::vertex_attribute const va ) const noexcept
    {
        size_t ret = 0;
        for( auto const & e : elements )
        {
            if( e.va == va )
            {
                ret =
                    motor::graphics::size_of( e.type ) * motor::graphics::size_of( e.type_struct );
            }
        }
        return UINT( ret );
    }

    static UINT get_sib(
        motor::graphics::vertex_attribute const va, geo_data::elements_t const & elems ) noexcept
    {
        size_t ret = 0;
        for( auto const & e : elems )
        {
            if( e.va == va )
            {
                ret =
                    motor::graphics::size_of( e.type ) * motor::graphics::size_of( e.type_struct );
            }
        }
        return UINT( ret );
    }

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        motor::memory::global_t::dealloc_raw( vb_mem );
        motor::memory::global_t::dealloc_raw( ib_mem );

        vb_mem = nullptr;
        ib_mem = nullptr;

        num_elements_vb = 0;
        num_elements_ib = 0;

        sib_vb = 0;
        sib_ib = 0;

        vb = guard< ID3D11Buffer >();
        ib = guard< ID3D11Buffer >();

        stride = 0;

        pt = motor::graphics::primitive_type::undefined;

        elements.clear();
    }
};
motor_typedef( geo_data );
using geometry_datas_t = datas< d3d11_pimpl_data::geo_data_t >;

//*******************************************************************************************
struct shader_data
{
    bool_t compiled = false;

    // the default shader variable values.
    motor::graphics::variable_set_t default_values;

    // vs, gs, ps, else?
    guard< ID3D11VertexShader > vs;
    guard< ID3D11GeometryShader > gs;
    guard< ID3D11PixelShader > ps;

    // required for later vertex layout creation
    guard< ID3DBlob > vs_blob;

    struct vertex_input_binding
    {
        motor::graphics::vertex_attribute va;
        motor::string_t name;
    };
    motor_typedefs( motor::vector< vertex_input_binding >, vibs );
    vibs_t vertex_inputs;

    struct data_variable
    {
        motor::string_t name;
        motor::graphics::type t;
        motor::graphics::type_struct ts;
        uint_t offset;
    };
    motor_typedef( data_variable );

    struct image_variable
    {
        // texture and its sampler must be on the same slot
        motor::string_t name;
        UINT slot;
    };
    motor_typedef( image_variable );
    motor_typedefs( motor::vector< image_variable_t >, image_variables );

    struct buffer_variable
    {
        motor::string_t name;
        UINT slot;
    };
    motor_typedef( buffer_variable );
    motor_typedefs( motor::vector< buffer_variable_t >, buffer_variables );

    // a constant buffer represent a way to send
    // variable data to the shader. This struct
    // keeps track of a buffer in the shader and its
    // variables and represents the layout of the buffer.
    struct cbuffer
    {
        // the slot the cbuffer needs to be set to
        // when bound to a shader state
        // done by : register(bn) statement
        UINT slot;

        motor::string_t name;
        motor::vector< data_variable > datas;

        size_t sib = 0;
    };
    motor_typedef( cbuffer );
    motor_typedefs( motor::vector< cbuffer_t >, cbuffers );

    cbuffers_t vs_cbuffers;
    cbuffers_t gs_cbuffers;
    cbuffers_t ps_cbuffers;

    image_variables_t vs_textures;
    image_variables_t gs_textures;
    image_variables_t ps_textures;

    buffer_variables_t vs_buffers;
    buffer_variables_t gs_buffers;
    buffer_variables_t ps_buffers;

    shader_data( void_t ) noexcept {}
    shader_data( shader_data const & ) = delete;
    shader_data( shader_data && rhv ) noexcept
        : compiled( rhv.compiled ), vs( std::move( rhv.vs ) ), gs( std::move( rhv.gs ) ),
          ps( std::move( rhv.ps ) ), vs_blob( std::move( rhv.vs_blob ) ),
          vertex_inputs( std::move( rhv.vertex_inputs ) ),
          vs_cbuffers( std::move( rhv.vs_cbuffers ) ), gs_cbuffers( std::move( rhv.gs_cbuffers ) ),
          ps_cbuffers( std::move( rhv.ps_cbuffers ) ), vs_textures( std::move( rhv.vs_textures ) ),
          gs_textures( std::move( rhv.gs_textures ) ), ps_textures( std::move( rhv.ps_textures ) ),
          vs_buffers( std::move( rhv.vs_buffers ) ), gs_buffers( std::move( rhv.gs_buffers ) ),
          ps_buffers( std::move( rhv.ps_buffers ) ),
          default_values( std::move( rhv.default_values ) )

    {
    }

    shader_data & operator=( shader_data && rhv ) noexcept
    {
        compiled = rhv.compiled;

        vs = std::move( rhv.vs );
        gs = std::move( rhv.gs );
        ps = std::move( rhv.ps );
        vs_blob = std::move( rhv.vs_blob );

        vertex_inputs = std::move( rhv.vertex_inputs );

        vs_cbuffers = std::move( rhv.vs_cbuffers );
        gs_cbuffers = std::move( rhv.gs_cbuffers );
        ps_cbuffers = std::move( rhv.ps_cbuffers );

        vs_textures = std::move( rhv.vs_textures );
        gs_textures = std::move( rhv.gs_textures );
        ps_textures = std::move( rhv.ps_textures );

        vs_buffers = std::move( rhv.vs_buffers );
        gs_buffers = std::move( rhv.gs_buffers );
        ps_buffers = std::move( rhv.ps_buffers );

        default_values = std::move( rhv.default_values );

        return *this;
    }

    void_t invalidate( std::optional< std::string_view > const & /*name*/ ) noexcept
    {
        compiled = false;

        vs = guard< ID3D11VertexShader >();
        gs = guard< ID3D11GeometryShader >();
        ps = guard< ID3D11PixelShader >();
        vs_blob = guard< ID3DBlob >();

        vertex_inputs.clear();
        vs_cbuffers.clear();
        gs_cbuffers.clear();
        ps_cbuffers.clear();
        vs_textures.clear();
        gs_textures.clear();
        ps_textures.clear();
        vs_buffers.clear();
        gs_buffers.clear();
        ps_buffers.clear();

        default_values.clear();
    }
};
motor_typedef( shader_data );
using shader_datas_t = datas< d3d11_pimpl_data::shader_data_t >;

//*******************************************************************************************
struct image_data
{
    guard< ID3D11ShaderResourceView > view;
    guard< ID3D11Texture2D > texture;
    guard< ID3D11SamplerState > sampler;

    // 0.0 : do not flip uv.v
    // 1.0 : do flip uv.v
    // render targets can not be flipped. those
    // are rendered upside down, so in the shader
    // the user needs to access this variable for
    // flipping information.
    float_t requires_y_flip = 0.0f;

    image_data( void_t ) noexcept {}
    image_data( image_data const & ) = delete;
    image_data( image_data && rhv ) noexcept
    {
        view = std::move( rhv.view );
        texture = std::move( rhv.texture );
        sampler = std::move( rhv.sampler );
        requires_y_flip = rhv.requires_y_flip;
    }

    ~image_data( void_t ) noexcept
    {
        invalidate( "" );
    }

    image_data & operator=( image_data && rhv ) noexcept
    {
        view = std::move( rhv.view );
        texture = std::move( rhv.texture );
        sampler = std::move( rhv.sampler );
        requires_y_flip = rhv.requires_y_flip;

        return *this;
    }

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        view = guard< ID3D11ShaderResourceView >();
        texture = guard< ID3D11Texture2D >();
        sampler = guard< ID3D11SamplerState >();
        requires_y_flip = 0.0f;
    }
};
motor_typedef( image_data );
using image_datas_t = datas< d3d11_pimpl_data::image_data_t >;

//*******************************************************************************************
struct array_data
{
    guard< ID3D11Buffer > buffer;
    guard< ID3D11ShaderResourceView > view;

    array_data( void_t ) noexcept {}
    array_data( array_data const & ) = delete;
    array_data( array_data && rhv ) noexcept
    {
        buffer = std::move( rhv.buffer );
        view = std::move( rhv.view );
    }
    ~array_data( void_t ) noexcept
    {
        invalidate( "" );
    }

    array_data & operator=( array_data && rhv ) noexcept
    {
        buffer = std::move( rhv.buffer );
        view = std::move( rhv.view );
        return *this;
    }

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        buffer = guard< ID3D11Buffer >();
        view = guard< ID3D11ShaderResourceView >();
    }
};
motor_typedef( array_data );
using array_datas_t = datas< d3d11_pimpl_data::array_data_t >;

//*******************************************************************************************
struct render_state_sets
{
    motor::graphics::render_state_sets_t rss;
    ID3D11DepthStencilState * depth_stencil_state = nullptr;
    ID3D11RasterizerState * raster_state = nullptr;
    ID3D11BlendState * blend_state = nullptr;

    render_state_sets( void_t ) noexcept {}
    render_state_sets( render_state_sets const & rhv ) noexcept
    {
        rss = rhv.rss;
        depth_stencil_state = rhv.depth_stencil_state;
        raster_state = rhv.raster_state;
        blend_state = rhv.blend_state;
    }
    render_state_sets( render_state_sets && rhv ) noexcept
    {
        rss = std::move( rhv.rss );
        motor_move_member_ptr( depth_stencil_state, rhv );
        motor_move_member_ptr( raster_state, rhv );
        motor_move_member_ptr( blend_state, rhv );
    }
    ~render_state_sets( void_t ) noexcept
    {
        // need to copy construct!
        // invalidate() ;
    }

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        rss = motor::graphics::render_state_sets_t();
        depth_stencil_state = guard< ID3D11DepthStencilState >();
        raster_state = guard< ID3D11RasterizerState >();
        blend_state = guard< ID3D11BlendState >();
    }

    render_state_sets & operator=( render_state_sets const & rhv ) noexcept
    {
        rss = rhv.rss;
        depth_stencil_state = rhv.depth_stencil_state;
        raster_state = rhv.raster_state;
        blend_state = rhv.blend_state;
        return *this;
    }
};
motor_typedef( render_state_sets );

//*******************************************************************************************
struct state_data
{
    motor::vector< motor::graphics::render_state_sets_t > states;

    void_t invalidate( std::optional< std::string_view > const & ) noexcept {}
};
motor_typedef( state_data );

//*******************************************************************************************
struct render_data
{
    struct geo_id_data
    {
        size_t id;
        size_t hash;
    };
    motor::vector< geo_id_data > geo_ids;

    motor::vector< size_t > so_ids; // feed from for geometry
    size_t shd_id = size_t( -1 );

    // the layout requires information about the layout elements' type
    // in order to determine the proper d3d enumeration
    D3D11_INPUT_ELEMENT_DESC layout[ size_t( motor::graphics::vertex_attribute::num_attributes ) ];
    ID3D11InputLayout * vertex_layout = nullptr;

    // streamout buffer input layout -> derived from soo vertex buffer
    D3D11_INPUT_ELEMENT_DESC
        layout_so[ size_t( motor::graphics::vertex_attribute::num_attributes ) ];
    ID3D11InputLayout * vertex_layout_so = nullptr;

    ID3D11RasterizerState * raster_state = nullptr;
    ID3D11BlendState * blend_state = nullptr;

    struct data_variable
    {
        motor::string_t name;
        motor::graphics::ivariable_ptr_t ivar = nullptr;
        motor::graphics::type t;
        motor::graphics::type_struct ts;

        size_t offset = 0;
        size_t sib = 0;

        size_t do_copy_funk( void_ptr_t dest )
        {
            if( ivar == 0 ) return 0;

            if( t == motor::graphics::type::tbool )
            {
                // HLSL constant buffers store each bool component in 32 bits.
                auto const src = static_cast< uint8_t const * >( ivar->data_ptr() );
                auto const dst = static_cast< uint8_ptr_t >( dest );
                size_t const count = motor::graphics::size_of( ts );
                for( size_t i = 0; i < count; ++i )
                {
                    bool_t value;
                    std::memcpy( &value, src + i * sizeof( bool_t ), sizeof( bool_t ) );
                    uint32_t const packed = value ? 1u : 0u;
                    std::memcpy( dst + i * sizeof( packed ), &packed, sizeof( packed ) );
                }
                return sib;
            }

            std::memcpy( dest, ivar->data_ptr(), sib );
            return sib;
        }

        size_t do_copy_funk_from_origin( void_ptr_t origin_no_shift )
        {
            if( ivar == 0 ) return 0;

            return this->do_copy_funk( uint8_ptr_t( origin_no_shift ) + offset );
        }
    };
    motor_typedef( data_variable );
    motor_typedefs( motor::vector< data_variable_t >, data_variables );

    struct image_variable
    {
        // if initially assigned, this is computed
        // and can be used if the image changes during
        // the course of the application.
        size_t value_hash = 0;

        // texture and its sampler must be on the same slot
        motor::string_t name;
        UINT slot;

        // index into the images vector
        size_t id = size_t( -1 );
    };
    motor_typedef( image_variable );
    motor_typedefs( motor::vector< image_variable_t >, image_variables );

    image_variables_t var_sets_imgs_vs;
    image_variables_t var_sets_imgs_ps;

    ///////////////////// BEGIN: Buffer variables <-> array data
    //

    struct buffer_variable
    {
        motor::string_t name;
        UINT slot;

        // index into the arrays vector
        size_t id = size_t( -1 );
    };
    motor_typedef( buffer_variable );
    motor_typedefs( motor::vector< buffer_variable_t >, buffer_variables );

    buffer_variables_t var_sets_buffers_vs;
    buffer_variables_t var_sets_buffers_so_vs;
    buffer_variables_t var_sets_buffers_gs;
    buffer_variables_t var_sets_buffers_so_gs;
    buffer_variables_t var_sets_buffers_ps;
    buffer_variables_t var_sets_buffers_so_ps;

    //
    ///////////////////// END: Buffer variables <-> array data

    ///////////////////// BEGIN: Cbuffer variables <-> normal data variables
    //
    // represents the cbuffer data of a shader stage.
    // very linked shader in a render data object
    // requires its own constant buffer data.
    struct cbuffer
    {
        UINT slot;
        void_ptr_t mem = nullptr;
        guard< ID3D11Buffer > ptr;
        data_variables_t data_variables;
    };
    motor_typedef( cbuffer );
    motor_typedefs( motor::vector< cbuffer_t >, cbuffers );

    cbuffers_t _cbuffers_vs;
    cbuffers_t _cbuffers_gs;
    cbuffers_t _cbuffers_ps;

    //
    ///////////////////// END: Cbuffer variables <-> normal data variables

    // index into var_sets. This is 1:1 to the render_objects'
    // variable set list.
    // an entry of value size_t(-1) means it is invalid
    struct varset_to_idx_data
    {
        enum indices
        {
            vertex_shader = 0,
            geometry_shader = 1,
            pixel_shader = 2,
            num_indices
        };
        // into var_sets
        size_t idx = size_t( -1 );

        // data variables into
        // cbuffers
        // indices:
        // 0: vertex shader cbuffers
        // 1: geometry shader cbuffers
        // 2: pixel shader cbuffers
        std::array< size_t, indices::num_indices > cbuffer_begin = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };
        std::array< size_t, indices::num_indices > cbuffer_end = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };

        // texture uniform variables into
        // var_sets_imgs_*
        std::array< size_t, indices::num_indices > imgs_begin = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };
        std::array< size_t, indices::num_indices > imgs_end = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };

        // buffer uniform variables into
        // var_sets_buffers_*
        std::array< size_t, indices::num_indices > buffers_begin = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };
        std::array< size_t, indices::num_indices > buffers_end = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };

        std::array< size_t, indices::num_indices > buffers_so_begin = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };
        std::array< size_t, indices::num_indices > buffers_so_end = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ) };
    };

    // index: variable set id
    // storage : index into var_sets
    motor::vector< varset_to_idx_data > varset_to_idx;

    struct variable_set
    {
        size_t hash;

        // if nullptr, this entry is invalid.
        // also keep this one for ref counting, so we can
        // safely use the internal variables.
        motor::graphics::variable_set_mtr_t vs = nullptr;

        bool_t is_valid( void_t ) const noexcept
        {
            return vs != nullptr;
        }

        bool_t is_invalid( void_t ) const noexcept
        {
            return vs == nullptr;
        }
    };
    motor_typedef( variable_set );
    motor::vector< variable_set_t > var_sets;

    // returns true if the entry exists and is valid, otherwise false.
    bool_t get_vs_to_idx_entry( size_t const varset_idx, varset_to_idx_data & data_out ) noexcept
    {
        if( has_not_variable_set( varset_idx ) ) return false;
        data_out = varset_to_idx[ varset_idx ];
        return true;
    }

    bool_t has_variable_set( size_t const vs_idx ) const noexcept
    {
        return varset_to_idx.size() > vs_idx && varset_to_idx[ vs_idx ].idx != size_t( -1 );
    }

    bool_t has_not_variable_set( size_t const vs_idx ) const noexcept
    {
        return !has_variable_set( vs_idx );
    }

    bool_t has_variable_set_changed( size_t const vs_idx, size_t const hash ) const noexcept
    {
        if( has_not_variable_set( vs_idx ) ) return false;
        return var_sets[ varset_to_idx[ vs_idx ].idx ].hash != hash;
    }

    // no bound checking
    variable_set_ref_t via_indirection( size_t const vs_idx ) noexcept
    {
        return var_sets[ varset_to_idx[ vs_idx ].idx ];
    }

    render_data( void_t ) noexcept {}
    render_data( render_data const & ) = delete;
    render_data( render_data && rhv ) noexcept
    {
        geo_ids = std::move( rhv.geo_ids );
        so_ids = std::move( rhv.so_ids );
        shd_id = rhv.shd_id;
        rhv.shd_id = size_t( -1 );

        motor_move_member_ptr( vertex_layout, rhv );
        motor_move_member_ptr( vertex_layout_so, rhv );
        motor_move_member_ptr( raster_state, rhv );
        motor_move_member_ptr( blend_state, rhv );

        // var_set_to_idx_map = std::move( rhv.var_set_to_idx_map ) ;

        varset_to_idx = std::move( rhv.varset_to_idx );
        var_sets = std::move( rhv.var_sets );
        var_sets_imgs_vs = std::move( rhv.var_sets_imgs_vs );
        var_sets_imgs_ps = std::move( rhv.var_sets_imgs_ps );
        var_sets_buffers_vs = std::move( rhv.var_sets_buffers_vs );
        var_sets_buffers_so_vs = std::move( rhv.var_sets_buffers_so_vs );
        var_sets_buffers_gs = std::move( rhv.var_sets_buffers_gs );
        var_sets_buffers_so_gs = std::move( rhv.var_sets_buffers_so_gs );
        var_sets_buffers_ps = std::move( rhv.var_sets_buffers_ps );
        var_sets_buffers_so_ps = std::move( rhv.var_sets_buffers_so_ps );
        _cbuffers_vs = std::move( rhv._cbuffers_vs );
        _cbuffers_gs = std::move( rhv._cbuffers_gs );
        _cbuffers_ps = std::move( rhv._cbuffers_ps );
    }

    ~render_data( void_t ) noexcept
    {
        invalidate( std::nullopt );
    }

    render_data & operator=( render_data && rhv ) noexcept
    {
        geo_ids = std::move( rhv.geo_ids );
        so_ids = std::move( rhv.so_ids );
        shd_id = rhv.shd_id;
        rhv.shd_id = size_t( -1 );

        motor_move_member_ptr( vertex_layout, rhv );
        motor_move_member_ptr( vertex_layout_so, rhv );
        motor_move_member_ptr( raster_state, rhv );
        motor_move_member_ptr( blend_state, rhv );

        // var_set_to_idx_map = std::move( rhv.var_set_to_idx_map ) ;
        varset_to_idx = std::move( rhv.varset_to_idx );
        var_sets = std::move( rhv.var_sets );
        var_sets_imgs_vs = std::move( rhv.var_sets_imgs_vs );
        var_sets_imgs_ps = std::move( rhv.var_sets_imgs_ps );
        var_sets_buffers_vs = std::move( rhv.var_sets_buffers_vs );
        var_sets_buffers_so_vs = std::move( rhv.var_sets_buffers_so_vs );
        var_sets_buffers_gs = std::move( rhv.var_sets_buffers_gs );
        var_sets_buffers_so_gs = std::move( rhv.var_sets_buffers_so_gs );
        var_sets_buffers_ps = std::move( rhv.var_sets_buffers_ps );
        var_sets_buffers_so_ps = std::move( rhv.var_sets_buffers_so_ps );
        _cbuffers_vs = std::move( rhv._cbuffers_vs );
        _cbuffers_gs = std::move( rhv._cbuffers_gs );
        _cbuffers_ps = std::move( rhv._cbuffers_ps );

        return *this;
    }

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        shd_id = size_t( -1 );

        geo_ids.clear();
        so_ids.clear();

        std::memset( layout, 0,
            sizeof( D3D11_INPUT_ELEMENT_DESC ) *
                size_t( motor::graphics::vertex_attribute::num_attributes ) );
        if( vertex_layout != nullptr )
        {
            vertex_layout->Release();
            vertex_layout = nullptr;
        }

        std::memset( layout_so, 0,
            sizeof( D3D11_INPUT_ELEMENT_DESC ) *
                size_t( motor::graphics::vertex_attribute::num_attributes ) );
        if( vertex_layout_so != nullptr )
        {
            vertex_layout_so->Release();
            vertex_layout_so = nullptr;
        }

        if( raster_state != nullptr )
        {
            raster_state->Release();
            raster_state = nullptr;
        }

        if( blend_state != nullptr )
        {
            blend_state->Release();
            blend_state = nullptr;
        }

        // var_set_to_idx_map.clear() ;
        varset_to_idx.clear();
        var_sets_imgs_vs.clear();
        var_sets_imgs_ps.clear();
        var_sets_buffers_vs.clear();
        var_sets_buffers_so_vs.clear();
        var_sets_buffers_gs.clear();
        var_sets_buffers_so_gs.clear();
        var_sets_buffers_ps.clear();
        var_sets_buffers_so_ps.clear();

        {
            for( auto & data : var_sets ) motor::release( motor::move( data.vs ) );
            var_sets.clear();
        }

        {
            auto clear_funk = [ & ]( render_data::cbuffers_t & datum )
            {
                for( auto & d2 : datum )
                {
                    motor::memory::global_t::dealloc_raw( d2.mem );
                    d2.ptr = guard< ID3D11Buffer >();
                    ;
                }
                datum.clear();
            };
            clear_funk( _cbuffers_vs );
            clear_funk( _cbuffers_gs );
            clear_funk( _cbuffers_ps );
        }
    }

    size_t link_geometry( size_t const id, size_t const hash ) noexcept
    {
        if( id == size_t( -1 ) ) return size_t( -1 );

        for( size_t i = 0; i < geo_ids.size(); ++i )
        {
            if( geo_ids[ i ].id == id ) return i;
        }

        for( size_t i = 0; i < geo_ids.size(); ++i )
        {
            if( geo_ids[ i ].id == size_t( -1 ) )
            {
                geo_ids[ i ].id = id;
                return i;
            }
        }
        geo_ids.emplace_back( render_data::geo_id_data{ id, hash } );
        return geo_ids.size() - 1;
    }

    void_t unlink_geometry( size_t const id ) noexcept
    {
        if( id == size_t( -1 ) ) return;
        for( size_t i = 0; i < geo_ids.size(); ++i )
        {
            if( geo_ids[ i ].id == id )
            {
                geo_ids[ i ].id = size_t( -1 );
                return;
            }
        }
    }

    void_t invalidate_geometry_index( size_t const idx, size_t const hash ) noexcept
    {
        if( idx >= geo_ids.size() ) return;
        geo_ids[ idx ].id = size_t( -1 );
        geo_ids[ idx ].hash = hash;
    }

    size_t link_streamout( size_t const id ) noexcept
    {
        if( id == size_t( -1 ) ) return size_t( -1 );

        for( size_t i = 0; i < so_ids.size(); ++i )
        {
            if( so_ids[ i ] == id ) return i;
        }

        for( size_t i = 0; i < so_ids.size(); ++i )
        {
            if( so_ids[ i ] == size_t( -1 ) )
            {
                so_ids[ i ] = id;
                return i;
            }
        }
        so_ids.emplace_back( id );
        return so_ids.size() - 1;
    }

    void_t unlink_streamout( size_t const id ) noexcept
    {
        if( id == size_t( -1 ) ) return;
        for( size_t i = 0; i < so_ids.size(); ++i )
        {
            if( so_ids[ i ] == id )
            {
                so_ids[ i ] = size_t( -1 );
                return;
            }
        }
    }
};
motor_typedef( render_data );
using render_datas_t = datas< d3d11_pimpl_data::render_data_t >;

//*******************************************************************************************
struct msl_data
{
    size_t ro_id;
    motor::graphics::shader_object_t so;

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        ro_id = size_t( -1 );
    }
};
motor_typedef( msl_data );
using msl_datas_t = datas< d3d11_pimpl_data::msl_data_t >;

//*******************************************************************************************
struct framebuffer_data
{
    bool_t valid = false;
    motor::string_t name;
    size_t num_color = 0;

    FLOAT width = 0.0f;
    FLOAT height = 0.0f;

    static size_t const max_images = 9;

    // 8 color targets + 1 depth stencil
    size_t image_ids[ max_images ] = { size_t( -1 ), size_t( -1 ), size_t( -1 ), size_t( -1 ),
        size_t( -1 ), size_t( -1 ), size_t( -1 ), size_t( -1 ), size_t( -1 ) };

    static size_t const max_rt_views = 8;

    ID3D11RenderTargetView * rt_view[ max_rt_views ] = {
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    ID3D11DepthStencilView * ds_view = nullptr;

    void_t invalidate( std::optional< std::string_view > const & ) noexcept
    {
        for( size_t i = 0; i < max_images; ++i ) image_ids[ i ] = size_t( -1 );
        for( size_t i = 0; i < max_rt_views; ++i )
            if( rt_view[ i ] )
            {
                rt_view[ i ]->Release();
                rt_view[ i ] = nullptr;
            }

        if( ds_view )
        {
            ds_view->Release();
            ds_view = nullptr;
        }
    }
};
motor_typedef( framebuffer_data );
using framebuffers_t = datas< d3d11_pimpl_data::framebuffer_data_t >;

} // namespace d3d11_pimpl_data