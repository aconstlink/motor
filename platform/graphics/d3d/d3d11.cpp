

#include "d3d11.h"
#include "d3d11_convert.h"
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

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <directxcolors.h>
#include <cstdio>

#if _DEBUG
#define D3D_DEBUG
#endif

#define d3d11_backend_log( text ) "[D3D11] : " text
#define d3d11_log_warningv( text, ... ) motor::log::global_t::warning<2048>( "[d3d11] : " text, __VA_ARGS__ ) ;
#define d3d11_log_errorv( text, ... ) motor::log::global_t::error<2048>( "[d3d11] : " text, __VA_ARGS__ ) ;


using namespace motor::platform ;
using namespace motor::platform::gen4 ;

template< typename T >
class guard
{
    motor_this_typedefs( guard< T > ) ;

private:

    T * _ptr = nullptr ;

public:

    this_ref_t invalidate( void_t ) noexcept
    {
        if( _ptr != nullptr ) _ptr->Release() ;
        _ptr = nullptr ;
        return *this ;
    }

    guard( void_t ) noexcept {}
    guard( this_rref_t rhv ) noexcept {
        motor_move_member_ptr( _ptr, rhv ) ;
    }
    ~guard( void_t ) noexcept
    {
        this_t::invalidate() ;
    }

    operator T const * ( void_t ) const noexcept
    {
        return _ptr ;
    }

    operator T* ( void_t ) noexcept
    {
        return _ptr ;
    }

    operator T** ( void_t ) noexcept
    {
        return &_ptr ;
    }

    operator void** ( void_t ) noexcept
    {
        return (void**)&_ptr ;
    }

    T* operator -> ( void_t ) noexcept
    {
        return _ptr ;
    }

    T* move_out( void_t ) noexcept 
    {
        T* ret = _ptr ;
        _ptr = nullptr ;
        return ret ;
    }

    bool_t operator != ( T * other ) const noexcept 
    {
        return _ptr != other ;
    }

    bool_t operator == ( T * other ) const noexcept 
    {
        return _ptr == other ;
    }

    // just replaces the pointer
    this_ref_t operator = ( T * other ) noexcept
    {
        _ptr = other ;
        return *this ;
    }

    // invalidates this and takes other
    this_ref_t operator = ( this_rref_t other ) noexcept
    {
        this_t::invalidate() ;
        _ptr = other.move_out() ;
        return *this ;
    }
} ;

struct d3d11_backend::pimpl
{
    motor_this_typedefs( d3d11_backend::pimpl ) ;
    
    //*******************************************************************************************
    template< typename T >
    using datas = motor::platform::datas<T> ;

    //*******************************************************************************************
    struct so_data
    {
        motor_this_typedefs( so_data ) ;
        
        // this is for rendering the streamout buffer
        struct layout_element
        {
            motor::graphics::vertex_attribute va ;
            motor::graphics::type type ;
            motor::graphics::type_struct type_struct ;

            size_t sib( void_t ) const noexcept
            {
                return size_t( motor::graphics::size_of( type ) *
                    motor::graphics::size_of( type_struct ) ) ;
            }
        };
        using elements_t = motor::vector< layout_element > ;
        elements_t elements ;

        motor::graphics::primitive_type pt = motor::graphics::primitive_type::undefined ;

        // per vertex sib
        UINT stride = 0 ;

        static const size_t max_buffers = 4 ;
        size_t buffers_used = 0 ;

        typedef struct 
        {
            guard< ID3D11Buffer > buffers[max_buffers] ;
            guard< ID3D11ShaderResourceView > views[max_buffers] ;

            guard< ID3D11Query > query ;
        } buffer_t ;
        
        buffer_t _ping_pong[ 2 ] ;

        // read index of ping-pong
        size_t rd_idx = 0 ;

        size_t read_index( void_t ) const noexcept { return rd_idx ; }
        size_t write_index( void_t ) const noexcept { return (rd_idx+1) % 2 ; }
        void_t swap( void_t ) noexcept { rd_idx = ++rd_idx % 2 ;}

        buffer_t & read_buffer( void_t ) noexcept { return _ping_pong[ this_t::read_index() ] ; }
        buffer_t const & read_buffer( void_t ) const noexcept { return _ping_pong[ this_t::read_index() ] ; }

        buffer_t & write_buffer( void_t ) noexcept { return _ping_pong[ this_t::write_index() ] ; }
        buffer_t const & write_buffer( void_t ) const noexcept { return _ping_pong[ this_t::write_index() ] ; }

        so_data( void_t ) noexcept{}
        so_data( so_data const & ) = delete ;
        so_data( so_data && rhv ) noexcept
        {
            stride = rhv.stride ;
            pt = rhv.pt ;

            elements = std::move( rhv.elements ) ;
            buffers_used = rhv.buffers_used ;

            for( size_t b=0; b<2; ++b )
            {
                for( size_t i=0; i<buffers_used; ++i )
                {
                    _ping_pong[b].buffers[i] = std::move( rhv._ping_pong[b].buffers[i] ) ;
                    _ping_pong[b].query = std::move( rhv._ping_pong[b].query ) ;
                }
            }

            rd_idx = rhv.rd_idx ;
        }
        ~so_data( void_t ) noexcept
        {
            invalidate("") ;
        }

        DXGI_FORMAT get_format_from_element( motor::graphics::vertex_attribute const va ) const noexcept
        {
            return this_t::get_format_from_element( va, this->elements ) ;
        }

        static DXGI_FORMAT get_format_from_element( motor::graphics::vertex_attribute const va, 
            this_t::elements_t const & elems  ) noexcept
        {
            for ( auto const & e : elems )
            {
                if ( e.va == va )
                {
                    return motor::platform::d3d11::convert_type_to_vec_format( e.type, e.type_struct ) ;
                }
            }

            return DXGI_FORMAT_UNKNOWN ;
        }

        UINT get_sib( motor::graphics::vertex_attribute const va ) const noexcept
        {
            return this_t::get_sib( va, elements ) ;
        }

        static UINT get_sib( motor::graphics::vertex_attribute const va,
            this_t::elements_t const & elems ) noexcept
        {
            size_t ret = 0 ;
            for ( auto const & e : elems )
            {
                if ( e.va == va )
                {
                    ret = motor::graphics::size_of( e.type ) * motor::graphics::size_of( e.type_struct ) ;
                }
            }
            return UINT( ret ) ;
        }

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            elements.clear() ;

            for( size_t b=0; b<2; ++b )
            {
                for( size_t i=0; i<buffers_used; ++i )
                {
                    _ping_pong[b].buffers[i].invalidate() ;
                    _ping_pong[b].query.invalidate() ;
                }
            }
            buffers_used = 0 ;
            rd_idx = size_t( -1 ) ;
        }
    };
    motor_typedef( so_data ) ;

    
    using streamout_datas_t = datas< pimpl::so_data > ;
    streamout_datas_t _streamouts ;
    size_t _cur_streamout_active = size_t( -1 ) ;


    //*******************************************************************************************
    struct geo_data
    {
        void_ptr_t vb_mem = nullptr ;
        void_ptr_t ib_mem = nullptr ;

        size_t num_elements_vb = 0 ;
        size_t num_elements_ib = 0 ;

        size_t sib_vb = 0 ;
        size_t sib_ib = 0 ;

        guard< ID3D11Buffer > vb ;
        guard< ID3D11Buffer > ib ;

        // per vertex sib
        UINT stride = 0 ;

        motor::graphics::primitive_type pt = motor::graphics::primitive_type::undefined ;

        struct layout_element
        {
            motor::graphics::vertex_attribute va ;
            motor::graphics::type type ;
            motor::graphics::type_struct type_struct ;

            size_t sib( void_t ) const noexcept
            {
                return size_t( motor::graphics::size_of( type ) *
                    motor::graphics::size_of( type_struct ) ) ;
            }
        };
        using elements_t = motor::vector< layout_element > ;
        elements_t elements ;

        geo_data( void_t ) noexcept{}
        geo_data( geo_data const & ) = delete ;
        geo_data( geo_data && rhv ) noexcept
        {
            motor_move_member_ptr( vb_mem, rhv ) ;
            motor_move_member_ptr( ib_mem, rhv ) ;

            num_elements_vb = rhv.num_elements_vb ;
            num_elements_ib = rhv.num_elements_ib ;

            sib_vb = rhv.sib_vb ;
            sib_ib = rhv.sib_ib ;

            vb = std::move( rhv.vb ) ;
            ib = std::move( rhv.ib ) ;
            
            stride = rhv.stride ;

            pt = rhv.pt ;

            elements = std::move( rhv.elements ) ;
        }
        geo_data & operator = ( geo_data && rhv ) noexcept
        {
            motor_move_member_ptr( vb_mem, rhv ) ;
            motor_move_member_ptr( ib_mem, rhv ) ;

            num_elements_vb = rhv.num_elements_vb ;
            num_elements_ib = rhv.num_elements_ib ;

            sib_vb = rhv.sib_vb ;
            sib_ib = rhv.sib_ib ;

            vb = std::move( rhv.vb ) ;
            ib = std::move( rhv.ib ) ;
            
            stride = rhv.stride ;

            pt = rhv.pt ;

            elements = std::move( rhv.elements ) ;

            return *this ;
        }

        ~geo_data( void_t ) noexcept
        {
            invalidate("") ;
        }

        DXGI_FORMAT get_format_from_element( motor::graphics::vertex_attribute const va ) const noexcept
        {
            for( auto const & e : elements )
            {
                if( e.va == va )
                {
                    return motor::platform::d3d11::convert_type_to_vec_format( e.type, e.type_struct ) ;
                }
            }

            return DXGI_FORMAT_UNKNOWN ;
        }

        static DXGI_FORMAT get_format_from_element( motor::graphics::vertex_attribute const va, 
            geo_data::elements_t const & elems ) noexcept
        {
            for ( auto const & e : elems )
            {
                if ( e.va == va )
                {
                    return motor::platform::d3d11::convert_type_to_vec_format( e.type, e.type_struct ) ;
                }
            }

            return DXGI_FORMAT_UNKNOWN ;
        }

        UINT get_sib( motor::graphics::vertex_attribute const va ) const noexcept
        {
            size_t ret = 0 ;
            for( auto const& e : elements )
            {
                if( e.va == va )
                {
                    ret = motor::graphics::size_of( e.type ) * motor::graphics::size_of( e.type_struct ) ;
                }
            }
            return UINT( ret ) ;
        }

        static UINT get_sib( motor::graphics::vertex_attribute const va, geo_data::elements_t const & elems ) noexcept
        {
            size_t ret = 0 ;
            for ( auto const & e : elems )
            {
                if ( e.va == va )
                {
                    ret = motor::graphics::size_of( e.type ) * motor::graphics::size_of( e.type_struct ) ;
                }
            }
            return UINT( ret ) ;
        }

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            motor::memory::global_t::dealloc_raw( vb_mem ) ;
            motor::memory::global_t::dealloc_raw( ib_mem ) ;

            vb_mem = nullptr ;
            ib_mem = nullptr ;

            num_elements_vb = 0 ;
            num_elements_ib = 0 ;

            sib_vb = 0 ;
            sib_ib = 0 ;

            vb = guard< ID3D11Buffer >() ;
            ib = guard< ID3D11Buffer >() ;

            stride = 0 ;

            pt = motor::graphics::primitive_type::undefined ;

            elements.clear() ;
        }
    };
    motor_typedef( geo_data ) ;
    
    using geometry_datas_t = datas< geo_data > ;
    geometry_datas_t _geos ;

    //*******************************************************************************************
    struct shader_data
    {
        bool_t compiled = false ;

        // vs, gs, ps, else?
        guard< ID3D11VertexShader > vs ;
        guard< ID3D11GeometryShader > gs ;
        guard< ID3D11PixelShader > ps ;

        // required for later vertex layout creation
        guard< ID3DBlob > vs_blob ;

        struct vertex_input_binding
        {
            motor::graphics::vertex_attribute va ;
            motor::string_t name ;
        };
        motor_typedefs( motor::vector< vertex_input_binding >, vibs ) ;
        vibs_t vertex_inputs ;

        struct data_variable
        {
            motor::string_t name ;
            motor::graphics::type t ;
            motor::graphics::type_struct ts ;
            uint_t offset ;
        };
        motor_typedef( data_variable ) ;

        struct image_variable
        {
            // texture and its sampler must be on the same slot
            motor::string_t name ;
            UINT slot ;
        } ;
        motor_typedef( image_variable ) ;
        motor_typedefs( motor::vector< image_variable_t >, image_variables ) ;

        struct buffer_variable
        {
            motor::string_t name ;
            UINT slot ;
        } ;
        motor_typedef( buffer_variable ) ;
        motor_typedefs( motor::vector< buffer_variable_t >, buffer_variables ) ;

        // a constant buffer represent a way to send 
        // variable data to the shader. This struct
        // keeps track of a buffer in the shader and its
        // variables and represents the layout of the buffer.
        struct cbuffer
        {
            // the slot the cbuffer needs to be set to
            // when bound to a shader state
            // done by : register(bn) statement
            UINT slot ;

            motor::string_t name ;
            motor::vector< data_variable > datas ;

            size_t sib = 0 ;
        } ;
        motor_typedef( cbuffer ) ;
        motor_typedefs( motor::vector< cbuffer_t >, cbuffers ) ;
        
        cbuffers_t vs_cbuffers ;
        cbuffers_t gs_cbuffers ;
        cbuffers_t ps_cbuffers ;

        image_variables_t vs_textures ;
        image_variables_t gs_textures ;
        image_variables_t ps_textures ;

        buffer_variables_t vs_buffers ;
        buffer_variables_t gs_buffers ;
        buffer_variables_t ps_buffers ;

        shader_data( void_t ) noexcept {}
        shader_data( shader_data const & ) = delete ;
        shader_data( shader_data && rhv ) noexcept : compiled( rhv.compiled ),
            vs( std::move( rhv.vs ) ), gs( std::move( rhv.gs ) ), 
            ps( std::move( rhv.ps ) ), vs_blob( std::move( rhv.vs_blob ) ),
            vertex_inputs( std::move( rhv.vertex_inputs) ), 
            vs_cbuffers( std::move( rhv.vs_cbuffers ) ), 
            gs_cbuffers( std::move( rhv.gs_cbuffers ) ),
            ps_cbuffers( std::move( rhv.ps_cbuffers ) ),
            vs_textures( std::move( rhv.vs_textures ) ),
            gs_textures( std::move( rhv.gs_textures ) ),
            ps_textures( std::move( rhv.ps_textures ) ),
            vs_buffers( std::move( rhv.vs_buffers ) ),
            gs_buffers( std::move( rhv.gs_buffers ) ),
            ps_buffers( std::move( rhv.ps_buffers ) )

        {}

        shader_data & operator = ( shader_data && rhv ) noexcept
        {
            compiled = rhv.compiled ;

            vs = std::move( rhv.vs ) ;
            gs = std::move( rhv.gs ) ;
            ps = std::move( rhv.ps ) ;
            vs_blob = std::move( rhv.vs_blob ) ;
            
            vertex_inputs = std::move( rhv.vertex_inputs ) ;
            
            vs_cbuffers = std::move( rhv.vs_cbuffers ) ;
            gs_cbuffers = std::move( rhv.gs_cbuffers ) ;
            ps_cbuffers = std::move( rhv.ps_cbuffers ) ;

            vs_textures = std::move( rhv.vs_textures ) ;
            gs_textures = std::move( rhv.gs_textures ) ;
            ps_textures = std::move( rhv.ps_textures ) ;

            vs_buffers = std::move( rhv.vs_buffers ) ;
            gs_buffers = std::move( rhv.gs_buffers ) ;
            ps_buffers = std::move( rhv.ps_buffers ) ;

            return *this ;
        }

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            compiled = false ;

            vs = guard< ID3D11VertexShader >() ;
            gs = guard< ID3D11GeometryShader >() ;
            ps = guard< ID3D11PixelShader >() ;
            vs_blob = guard< ID3DBlob >() ;
            
            vertex_inputs.clear() ;
            vs_cbuffers.clear() ;
            gs_cbuffers.clear() ;
            ps_cbuffers.clear() ;
            vs_textures.clear() ;
            gs_textures.clear() ;
            ps_textures.clear() ;
            vs_buffers.clear() ;
            gs_buffers.clear() ;
            ps_buffers.clear() ;
        }
    } ;
    motor_typedef( shader_data ) ;
    using shader_datas_t = datas< shader_data > ;
    shader_datas_t _shaders ;

    //*******************************************************************************************
    struct image_data
    {
        guard< ID3D11ShaderResourceView > view ;
        guard< ID3D11Texture2D > texture ;
        guard< ID3D11SamplerState > sampler ;

        // 0.0 : do not flip uv.v
        // 1.0 : do flip uv.v
        // render targets can not be flipped. those
        // are rendered upside down, so in the shader
        // the user needs to access this variable for
        // flipping information.
        float_t requires_y_flip = 0.0f ;

        image_data( void_t ) noexcept{}
        image_data( image_data const & ) = delete ;
        image_data( image_data && rhv ) noexcept
        {
            view = std::move( rhv.view ) ;
            texture = std::move( rhv.texture ) ;
            sampler = std::move( rhv.sampler ) ;
            requires_y_flip = rhv.requires_y_flip ;
        }

        ~image_data( void_t ) noexcept
        {
            invalidate("") ;
        }

        image_data & operator = ( image_data && rhv ) noexcept
        {
            view = std::move( rhv.view ) ;
            texture = std::move( rhv.texture ) ;
            sampler = std::move( rhv.sampler ) ;
            requires_y_flip = rhv.requires_y_flip ;

            return *this ;
        }

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            view = guard< ID3D11ShaderResourceView >() ;
            texture = guard< ID3D11Texture2D >() ;
            sampler = guard< ID3D11SamplerState >() ;
            requires_y_flip = 0.0f ;
        }
    };
    motor_typedef( image_data ) ;
    //typedef motor::vector< this_t::image_data > image_datas_t ;
    using image_datas_t = datas< image_data_t > ;
    image_datas_t _images ;

    //*******************************************************************************************
    struct array_data
    {
        guard< ID3D11Buffer > buffer ;
        guard< ID3D11ShaderResourceView > view ;

        array_data( void_t ) noexcept{}
        array_data( array_data const & ) = delete ;
        array_data( array_data && rhv ) noexcept
        {
            buffer = std::move( rhv.buffer ) ;
            view = std::move( rhv.view ) ;
        }
        ~array_data( void_t ) noexcept
        {
            invalidate("") ;
        }

        array_data & operator = ( array_data && rhv ) noexcept
        {
            buffer = std::move( rhv.buffer ) ;
            view = std::move( rhv.view ) ;
            return *this ;
        }

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            buffer = guard< ID3D11Buffer >() ;
            view = guard< ID3D11ShaderResourceView >() ;
        }
    };
    motor_typedef( array_data ) ;
    using array_datas_t = datas< array_data > ;

    array_datas_t _arrays ;

    //*******************************************************************************************
    struct render_state_sets
    {
        motor::graphics::render_state_sets_t rss ;
        ID3D11DepthStencilState * depth_stencil_state = nullptr ;
        ID3D11RasterizerState * raster_state = nullptr ;
        ID3D11BlendState * blend_state = nullptr ;

        render_state_sets( void_t ) noexcept {}
        render_state_sets( render_state_sets const & rhv ) noexcept
        {
            rss = rhv.rss ;
            depth_stencil_state = rhv.depth_stencil_state ; 
            raster_state = rhv.raster_state ;
            blend_state = rhv.blend_state ;
        }
        render_state_sets( render_state_sets && rhv ) noexcept
        {
            rss = std::move( rhv.rss ) ;
            motor_move_member_ptr( depth_stencil_state, rhv ) ;
            motor_move_member_ptr( raster_state, rhv ) ;
            motor_move_member_ptr( blend_state, rhv ) ;
        }
        ~render_state_sets( void_t ) noexcept
        {
            // need to copy construct!
            //invalidate() ;
        }
        
        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            rss = motor::graphics::render_state_sets_t() ;
            depth_stencil_state = guard< ID3D11DepthStencilState >() ;
            raster_state = guard< ID3D11RasterizerState >() ;
            blend_state = guard< ID3D11BlendState >() ;
        }

        render_state_sets & operator = ( render_state_sets const & rhv ) noexcept
        {
            rss = rhv.rss ;
            depth_stencil_state = rhv.depth_stencil_state ; 
            raster_state = rhv.raster_state ;
            blend_state = rhv.blend_state ;
            return *this ;
        }
    };
    motor_typedef( render_state_sets ) ;

    //*******************************************************************************************
    struct state_data
    {
        motor::vector< motor::graphics::render_state_sets_t > states ;

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {}
    } ;
    motor_typedef( state_data ) ;

    //*******************************************************************************************
    struct render_data
    {
        motor::vector< size_t > geo_ids ;
        motor::vector< size_t > so_ids ; // feed from for geometry
        size_t shd_id = size_t( -1 ) ;

        // the layout requires information about the layout elements' type
        // in order to determine the proper d3d enumeration
        D3D11_INPUT_ELEMENT_DESC layout[ size_t( motor::graphics::vertex_attribute::num_attributes ) ] ;
        ID3D11InputLayout* vertex_layout = nullptr ;

        // streamout buffer input layout -> derived from soo vertex buffer
        D3D11_INPUT_ELEMENT_DESC layout_so[ size_t( motor::graphics::vertex_attribute::num_attributes ) ] ;
        ID3D11InputLayout* vertex_layout_so = nullptr ;

        ID3D11RasterizerState* raster_state = nullptr ;
        ID3D11BlendState* blend_state = nullptr ;

        // also keep this one for ref counting
        motor::vector< motor::graphics::variable_set_mtr_t > var_sets ;

        struct data_variable
        {
            motor::string_t name ;
            motor::graphics::ivariable_ptr_t ivar = nullptr ;
            motor::graphics::type t ;
            motor::graphics::type_struct ts ;
            
            size_t offset = 0 ;
            size_t sib = 0 ;

            size_t do_copy_funk( void_ptr_t dest )
            {
                if( ivar == 0 ) return 0 ;

                std::memcpy( dest, ivar->data_ptr(), sib ) ;
                return sib ;
            }

            size_t do_copy_funk_from_origin( void_ptr_t origin_no_shift )
            {
                if ( ivar == 0 ) return 0 ;

                std::memcpy( uint8_ptr_t(origin_no_shift)+offset, ivar->data_ptr(), sib ) ;
                return sib ;
            }
        } ;
        motor_typedef( data_variable ) ;
        motor_typedefs( motor::vector< data_variable_t >, data_variables ) ;

        struct image_variable
        {
            // into var_sets
            size_t var_set_idx ;

            // if initially assigned, this is computed
            // and can be used if the image changes during
            // the course of the application.
            size_t value_hash = 0 ;

            // texture and its sampler must be on the same slot
            motor::string_t name ;
            UINT slot ;

            // index into the images vector
            size_t id = size_t( -1 ) ;
        } ;
        motor_typedef( image_variable ) ;
        motor_typedefs( motor::vector< image_variable_t >, image_variables ) ;

        image_variables_t var_sets_imgs_vs ;
        image_variables_t var_sets_imgs_ps ;

        ///////////////////// BEGIN: Buffer variables <-> array data
        //

        struct buffer_variable
        {
            motor::string_t name ;
            UINT slot ;

            // index into the arrays vector
            size_t id = size_t( -1 ) ;
        } ;
        motor_typedef( buffer_variable ) ;
        motor_typedefs( motor::vector< buffer_variable_t >, buffer_variables ) ;

        typedef motor::vector< std::pair< motor::graphics::variable_set_mtr_t,
            buffer_variables_t > > varsets_to_buffers_t ;

        varsets_to_buffers_t var_sets_buffers_vs ;
        varsets_to_buffers_t var_sets_buffers_so_vs ;
        varsets_to_buffers_t var_sets_buffers_gs ;
        varsets_to_buffers_t var_sets_buffers_so_gs ;
        varsets_to_buffers_t var_sets_buffers_ps ;
        varsets_to_buffers_t var_sets_buffers_so_ps ;

        //
        ///////////////////// END: Buffer variables <-> array data
        
        
        ///////////////////// BEGIN: Cbuffer variables <-> normal data variables
        // 
        // represents the cbuffer data of a shader stage.
        // very linked shader in a render data object
        // requires its own constant buffer data.
        struct cbuffer
        {
            size_t var_set_idx ;

            UINT slot ;
            void_ptr_t mem = nullptr ;
            guard<ID3D11Buffer> ptr ;
            data_variables_t data_variables ;
        } ;
        motor_typedef( cbuffer ) ;
        motor_typedefs( motor::vector< cbuffer_t >, cbuffers ) ;

        cbuffers_t _cbuffers_vs ;
        cbuffers_t _cbuffers_gs ;
        cbuffers_t _cbuffers_ps ;

        //
        ///////////////////// END: Cbuffer variables <-> normal data variables

        render_data( void_t ) noexcept {}
        render_data( render_data const & ) = delete ;
        render_data( render_data && rhv ) noexcept
        {
            geo_ids = std::move( rhv.geo_ids ) ;
            so_ids = std::move( rhv.so_ids ) ;
            shd_id = rhv.shd_id ;
            rhv.shd_id = size_t( -1 ) ;

            motor_move_member_ptr( vertex_layout, rhv ) ;
            motor_move_member_ptr( vertex_layout_so, rhv ) ;
            motor_move_member_ptr( raster_state, rhv ) ;
            motor_move_member_ptr( blend_state, rhv ) ;

            var_sets = std::move( rhv.var_sets ) ;
            var_sets_imgs_vs = std::move( rhv.var_sets_imgs_vs ) ;
            var_sets_imgs_ps = std::move( rhv.var_sets_imgs_ps ) ;
            var_sets_buffers_vs = std::move( rhv.var_sets_buffers_vs ) ;
            var_sets_buffers_so_vs = std::move( rhv.var_sets_buffers_so_vs ) ;
            var_sets_buffers_gs = std::move( rhv.var_sets_buffers_gs ) ;
            var_sets_buffers_so_gs = std::move( rhv.var_sets_buffers_so_gs ) ;
            var_sets_buffers_ps = std::move( rhv.var_sets_buffers_ps ) ;
            var_sets_buffers_so_ps = std::move( rhv.var_sets_buffers_so_ps ) ;
            _cbuffers_vs = std::move( rhv._cbuffers_vs ) ;
            _cbuffers_gs = std::move( rhv._cbuffers_gs ) ;
            _cbuffers_ps = std::move( rhv._cbuffers_ps ) ;
        }

        ~render_data( void_t ) noexcept
        {
            invalidate("") ;
        }

        render_data & operator = ( render_data && rhv ) noexcept
        {
            geo_ids = std::move( rhv.geo_ids ) ;
            so_ids = std::move( rhv.so_ids ) ;
            shd_id = rhv.shd_id ;
            rhv.shd_id = size_t( -1 ) ;

            motor_move_member_ptr( vertex_layout, rhv ) ;
            motor_move_member_ptr( vertex_layout_so, rhv ) ;
            motor_move_member_ptr( raster_state, rhv ) ;
            motor_move_member_ptr( blend_state, rhv ) ;

            var_sets = std::move( rhv.var_sets ) ;
            var_sets_imgs_vs = std::move( rhv.var_sets_imgs_vs ) ;
            var_sets_imgs_ps = std::move( rhv.var_sets_imgs_ps ) ;
            var_sets_buffers_vs = std::move( rhv.var_sets_buffers_vs ) ;
            var_sets_buffers_so_vs = std::move( rhv.var_sets_buffers_so_vs ) ;
            var_sets_buffers_gs = std::move( rhv.var_sets_buffers_gs ) ;
            var_sets_buffers_so_gs = std::move( rhv.var_sets_buffers_so_gs ) ;
            var_sets_buffers_ps = std::move( rhv.var_sets_buffers_ps ) ;
            var_sets_buffers_so_ps = std::move( rhv.var_sets_buffers_so_ps ) ;
            _cbuffers_vs = std::move( rhv._cbuffers_vs ) ;
            _cbuffers_gs = std::move( rhv._cbuffers_gs ) ;
            _cbuffers_ps = std::move( rhv._cbuffers_ps ) ;

            return *this ;
        }

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            shd_id = size_t( -1 ) ;

            geo_ids.clear() ;
            so_ids.clear() ;

            std::memset( layout, 0, sizeof( D3D11_INPUT_ELEMENT_DESC ) * size_t( motor::graphics::vertex_attribute::num_attributes ) ) ;
            if( vertex_layout != nullptr )
            {
                vertex_layout->Release() ;
                vertex_layout = nullptr ;
            }

            std::memset( layout_so, 0, sizeof( D3D11_INPUT_ELEMENT_DESC ) * size_t( motor::graphics::vertex_attribute::num_attributes ) ) ;
            if( vertex_layout_so != nullptr )
            {
                vertex_layout_so->Release() ;
                vertex_layout_so = nullptr ;
            }

            if( raster_state != nullptr ) 
            {
                raster_state->Release() ;
                raster_state = nullptr ;
            }
            
            if( blend_state != nullptr )
            {
                blend_state->Release() ;
                blend_state = nullptr ;
            }

            var_sets_imgs_vs.clear() ;
            var_sets_imgs_ps.clear() ;
            var_sets_buffers_vs.clear() ;
            var_sets_buffers_so_vs.clear() ;
            var_sets_buffers_gs.clear() ;
            var_sets_buffers_so_gs.clear() ;
            var_sets_buffers_ps.clear() ;
            var_sets_buffers_so_ps.clear() ;

            {
                for( auto * mtr : var_sets ) motor::memory::release_ptr( mtr ) ;
                var_sets.clear() ;
            }

            {
                auto clear_funk = [&]( this_t::render_data::cbuffers_t & datum )
                {
                    for ( auto & d2 : datum )
                    {
                        motor::memory::global_t::dealloc_raw( d2.mem ) ;
                        d2.ptr = guard<ID3D11Buffer>() ; ;
                    }
                    datum.clear() ;
                } ;
                clear_funk( _cbuffers_vs ) ;
                clear_funk( _cbuffers_gs ) ;
                clear_funk( _cbuffers_ps ) ;
            }
        }

        size_t link_geometry( size_t const id ) noexcept
        {
            if( id == size_t( -1 ) ) return size_t( -1 ) ;

            for( size_t i=0; i<geo_ids.size(); ++i )
            {
                if( geo_ids[i] == id ) return i ;
            }

            for( size_t i=0; i<geo_ids.size(); ++i )
            {
                if( geo_ids[i] == size_t( -1 ) ) 
                {
                    geo_ids[i] = id ; return i ;
                }
            }
            geo_ids.emplace_back( id ) ;
            return geo_ids.size() - 1 ;
        }

        void_t unlink_geometry( size_t const id ) noexcept
        {
            if( id == size_t( -1 ) ) return  ;
            for( size_t i=0; i<geo_ids.size(); ++i )
            {
                if( geo_ids[i] == id ) 
                {
                    geo_ids[i] = size_t( -1 ) ;
                    return ;
                }
            }
        }

        size_t link_streamout( size_t const id ) noexcept
        {
            if( id == size_t( -1 ) ) return size_t( -1 ) ;

            for( size_t i=0; i<so_ids.size(); ++i )
            {
                if( so_ids[i] == id ) return i ;
            }

            for( size_t i=0; i<so_ids.size(); ++i )
            {
                if( so_ids[i] == size_t( -1 ) ) 
                {
                    so_ids[i] = id ; return i ;
                }
            }
            so_ids.emplace_back( id ) ;
            return so_ids.size() - 1 ;
        }

        void_t unlink_streamout( size_t const id ) noexcept
        {
            if( id == size_t( -1 ) ) return  ;
            for( size_t i=0; i<so_ids.size(); ++i )
            {
                if( so_ids[i] == id ) {
                    so_ids[i] = size_t( -1 ) ; return ;
                }
            }
        }
    };
    motor_typedef( render_data ) ;
    
    using render_datas_t = datas< render_data > ;
    render_datas_t _renders ;


public: // msl data

    //*******************************************************************************************
    struct msl_data
    {
        // purpose: keep track of the data within the msl object
        // if recompilation is triggered. On recompilation, there
        // might be no/lost data of the original msl object, so 
        // in that case, only the new shader code is passed and
        // the old data is used.
        motor::graphics::msl_object_t msl_obj ;

        motor::vector< motor::graphics::render_object_t > ros ; 
        motor::vector< motor::graphics::shader_object_t > sos ; 

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            msl_obj.~msl_object() ;
        }
    } ;
    motor_typedef( msl_data ) ;

    using msl_datas_t = datas< msl_data > ;
    msl_datas_t _msls ;

    // find render object by name
    static bool_t find_ro( msl_datas_t & items, motor::string_in_t name, 
        std::function< void_t ( size_t const, this_t::msl_data_ref_t ) > funk ) noexcept
    {
        auto const res = items.for_each_with_break( 
            [&]( size_t const j, this_t::msl_data_ref_t d )
        {
            auto i = size_t( -1 ) ;
            while ( ++i < d.ros.size() && d.ros[i].name() != name ) ;
            if( i == d.ros.size() ) return true ;

            funk( j, d ) ;
            return false ;
        } ) ;
        return res ;
    }

    // find a msl object by a render object name
    static std::pair< size_t, motor::graphics::msl_object_t > find_pair_by_ro_name( motor::string_in_t name, msl_datas_t & msls ) noexcept
    {
        auto ret = std::make_pair( size_t(-1), motor::graphics::msl_object_t() ) ;
        auto const res = this_t::find_ro( msls, name, [&]( size_t const id, pimpl::msl_data_ref_t d )
        {
            ret = std::make_pair( id, d.msl_obj ) ;
        } ) ;
        
        return ret ;
    }

    motor::msl::database_t _mdb ;

public: // framebuffer

    //*******************************************************************************************
    struct framebuffer_data
    {
        bool_t valid = false ;
        motor::string_t name ;
        size_t num_color = 0 ;

        FLOAT width = 0.0f ;
        FLOAT height = 0.0f ;

        // 8 color targets + 1 depth stencil
        size_t image_ids[ 9 ] = {
            size_t( -1 ), size_t( -1 ), size_t( -1 ), size_t( -1 ),
            size_t( -1 ), size_t( -1 ), size_t( -1 ), size_t( -1 ), 
            size_t( -1 )} ;

        ID3D11RenderTargetView * rt_view[ 8 ] = 
            { nullptr, nullptr, nullptr, nullptr, 
                nullptr, nullptr, nullptr, nullptr } ;

        ID3D11DepthStencilView * ds_view = nullptr ;

        void_t invalidate( motor::string_in_t ) noexcept
        {
            for( size_t i=0; i<9; ++i ) image_ids[i] = size_t( -1 ) ;
            for( size_t i=0; i<8; ++i ) if( rt_view[i] ) rt_view[i]->Release() ;
            if( ds_view ) ds_view->Release() ;
        }
    };
    motor_typedef( framebuffer_data ) ;
    //typedef motor::vector< this_t::framebuffer_data > framebuffer_datas_t ;
    using framebuffers_t = datas< this_t::framebuffer_data_t> ;
    framebuffers_t _framebuffers ;
    size_t _cur_fb_active = size_t( -1 ) ;

public: // other variables

    motor::graphics::gen4::backend_type const bt = motor::graphics::gen4::backend_type::d3d11 ;
    motor::graphics::shader_api_type const sapi = motor::graphics::shader_api_type::hlsl_5_0 ;
    motor::platform::d3d11::rendering_context_mtr_t _ctx ;

    //typedef motor::vector< this_t::state_data_t > states_t ;
    using states_t = datas< this_t::state_data_t > ;
    states_t _states ;

    motor::stack< this_t::render_state_sets, 10 > _state_stack ;

    FLOAT vp_width = FLOAT( 0 ) ;
    FLOAT vp_height = FLOAT( 0 ) ;

    // set by the backend using this pimpl
    size_t _bid = size_t(-1) ;

public: // compilation thread ;

    struct work_item
    {
        enum class work_type
        {
            configure,
            release
        };

        enum class obj_type
        {
            unknown,
            msl
        };

        work_type wt ;
        obj_type ot ;
        void_ptr_t ptr ;

        template< typename T >
        static obj_type deduce_type( T * ptr ) noexcept
        {
            return obj_type::unknown ;
        }

        template<>
        static obj_type deduce_type<motor::graphics::msl_object>( motor::graphics::msl_object * ptr ) noexcept
        {
            return obj_type::msl ;
        }

        template< typename T >
        static work_item make_item( work_type const t, motor::core::mtr_safe<T> obj ) noexcept
        {
            return work_item{ t, work_item::deduce_type( obj.mtr() ), 
                static_cast<void_ptr_t>( obj.mtr() ) } ;
        }
    };
    motor_typedef( work_item ) ;

    struct shared_data
    {
        bool_t running = false ;
        bool_t has_work = false ;
        this_ptr_t owner = nullptr ;

        std::mutex mtx ;
        std::condition_variable cv ;

        motor::vector< work_item > items ;

        void_t add_item( work_item_rref_t item ) noexcept
        {
            std::unique_lock< std::mutex > lk( mtx ) ;

            if ( items.size() == items.capacity() )
                items.reserve( items.size() + 50 ) ;

            items.emplace_back( std::move( item ) ) ;

            has_work = true ;
        }

        void_t notify_thread( void_t ) noexcept
        {
            if ( has_work ) cv.notify_one() ;
        }
    };
    motor_typedef( shared_data ) ;

    shared_data_ptr_t _ctsd = nullptr ;
    std::thread _compilation_thread ;

    void_t start_support_thread( void_t ) noexcept
    {
        assert( _ctsd == nullptr && "Just start/stop the thread." ) ;

        if ( _ctsd == nullptr )
        {
            _ctsd = motor::memory::global::alloc<shared_data>
                ( "support shared data" ) ;
            _ctsd->running = true ;
            _ctsd->owner = this ;
        }

        auto * ctsd = _ctsd ;

        _compilation_thread = std::thread( [ctsd] ( void_t )
        {
            motor::log::global_t::status( "[d3d11] : support thread started" ) ;

            motor::msl::database_t mdb ;

            motor::vector< this_t::work_item_t > items ;
            items.reserve( 50 ) ;

            while ( ctsd->running )
            {
                {
                    std::unique_lock< std::mutex > lk( ctsd->mtx ) ;
                    while ( !ctsd->has_work && ctsd->running ) ctsd->cv.wait( lk ) ;

                    auto tmp = std::move( items ) ;
                    items = std::move( ctsd->items ) ;
                    ctsd->items = std::move( tmp ) ;

                    ctsd->has_work = false ;
                }

                for( auto & item : items )
                {
                    switch( item.ot )
                    {
                    case work_item::obj_type::msl: 
                    {
                        auto * msl = static_cast<motor::graphics::msl_object_ptr_t>(item.ptr) ;
                        switch( item.wt )
                        {
                        case work_item::work_type::configure: 
                            ctsd->owner->construct_msl_data_st( msl ) ;
                            break ;
                        case work_item::work_type::release: break ;
                        default: break ;
                        }
                    } break ;
                    default: break ;
                    }

                    if( !ctsd->running ) break ;
                }

                for( auto & item : items )
                {
                    auto obj_ptr = static_cast<motor::graphics::object*>
                        ( item.ptr ) ;
                    motor::release( motor::move( obj_ptr ) ) ;
                }
                items.clear() ;
            }

            motor::log::global_t::status( "[d3d11] : compilation thread shut down" ) ;
        } ) ;
    }

    void_t stop_compilation_thread( void_t ) noexcept
    {
        if ( _ctsd == nullptr ) return ;

        if ( _ctsd->running )
        {
            {
                std::unique_lock<std::mutex> lk( _ctsd->mtx ) ;
                _ctsd->running = false ;
            }
            _ctsd->cv.notify_one() ;
            if ( _compilation_thread.joinable() )
                _compilation_thread.join() ;
        }

        motor::memory::global::dealloc<shared_data>( motor::move( _ctsd ) ) ;
    }

public: // functions

    //************************************************************************************************************
    pimpl( size_t const bid, motor::platform::d3d11::rendering_context_mtr_t ctx ) noexcept : _bid( bid )
    {
        _ctx = ctx ;

        {
            motor::graphics::state_object_t obj( "d3d11_default_states" ) ;

            auto new_states = motor::graphics::gen4::backend_t::default_render_states() ;

            new_states.view_s.do_change = true ;
            new_states.view_s.ss.do_activate = true ;
            new_states.scissor_s.do_change = true ;
            new_states.scissor_s.ss.do_activate = false ;
            new_states.blend_s.do_change = true ;
            new_states.blend_s.ss.do_activate = false ;
            new_states.polygon_s.do_change = true ;
            new_states.polygon_s.ss.do_activate = true ;
            new_states.depth_s.do_change = true ;

            obj.add_render_state_set( new_states ) ;

            bool_t const res = this->construct_state( obj ) ;
            assert( res ) ;
        }

        this_t::start_support_thread() ;
    }

    //************************************************************************************************************
    // @todo is the move required? pimpl is only used via
    // pointer movement.
    pimpl( pimpl && rhv ) noexcept
    {
        rhv.stop_compilation_thread() ;

        motor_move_member_ptr( _ctx, rhv ) ;
        _geos = std::move( rhv._geos ) ;
        _images = std::move( rhv._images ) ;
        _shaders = std::move( rhv._shaders ) ;
        _arrays = std::move( rhv._arrays ) ;
        _renders = std::move( rhv._renders ) ;

        _msls = std::move( rhv._msls ) ;
        _state_stack = std::move( rhv._state_stack ) ;
        _states = std::move( rhv._states ) ;
        _cur_fb_active = rhv._cur_fb_active ;

        _bid = rhv._bid ;

        this_t::start_support_thread() ;
    }
    
    //************************************************************************************************************
    ~pimpl( void_t ) 
    {
        this_t::stop_compilation_thread() ;

        
        _geos.invalidate_and_clear() ;
        _shaders.invalidate_and_clear() ;
        _renders.invalidate_and_clear() ;
        _images.invalidate_and_clear() ;
        _arrays.invalidate_and_clear() ;
        _framebuffers.invalidate_and_clear() ;
        _streamouts.invalidate_and_clear() ;
        _msls.invalidate_and_clear() ;

        _states.invalidate_and_clear() ;

        _cur_fb_active = size_t( -1 ) ;
    }

    //************************************************************************************************************
    bool_t construct_state( motor::graphics::state_object_ref_t obj ) noexcept
    {
        #if 0
        oid = determine_oid( obj.get_oid(_bid), obj.name(), _states ) ;

        auto& states = _states[ oid ] ;
        #endif

        size_t oid = obj.get_oid(_bid) ;
        auto const res = _states.access( oid, obj.name(), [&]( this_t::state_data_ref_t states )
        {
            states.states.resize( obj.size() ) ;

            // @note
            // set all enables. Enable/Disable only possible during construction
            // values are assigned in the update function for the render states
            obj.for_each( [&] ( size_t const i, motor::graphics::render_state_sets_cref_t rs )
            {
                states.states[ i ] = rs ;
            } ) ;

            return true ;
        } ) ;

        if( !res ) return false ;

        obj.set_oid( _bid, oid ) ;
        return true ;
    }

    //************************************************************************************************************
    void_t update_state( size_t const oid, motor::graphics::state_object_ref_t obj ) noexcept
    {
        assert( oid < _states.size() ) ;

        if( !obj.check_and_reset_changed( _bid ) ) return ;

        auto const res = _states.access( oid, [&]( this_t::state_data_ref_t sd )
        {
            obj.for_each( [&]( size_t const i, motor::graphics::render_state_sets_cref_t rs )
            {
                if( i >= sd.states.size() ) return ;
                sd.states[i] = rs ;
            } ) ;
            return true ;
        } ) ;
    }

    //************************************************************************************************************
    void_t release_state( size_t const oid ) noexcept 
    {
    }

    //************************************************************************************************************
    void_t handle_render_state( this_t::render_state_sets & incoming_states, bool_t const popped )
    {
        //  viewport
        if( incoming_states.rss.view_s.do_change )
        {
            if( incoming_states.rss.view_s.ss.do_activate )
            {
                auto const& vp_ = incoming_states.rss.view_s.ss.vp  ;

                // Setup the viewport
                D3D11_VIEWPORT vp ;
                vp.Width = FLOAT( vp_.z() ) ;
                vp.Height = FLOAT( vp_.w() ) ;
                vp.MinDepth = 0.0f ;
                vp.MaxDepth = 1.0f ;
                vp.TopLeftX = FLOAT( vp_.x() ) ;
                vp.TopLeftY = FLOAT( vp_.y() ) ;
                
                // this solution here only works because
                // - framebuffers do not stack. They must be used and unused.
                // - render states targeting a framebuffer must be activated 
                //   within the use/unuse functions by the user.
                if( _cur_fb_active == size_t(-1) || popped ) 
                    vp.TopLeftY = vp_height - vp.Height - vp.TopLeftY ;
                else
                {
                    _framebuffers.access(_cur_fb_active, [&]( this_t::framebuffer_data_ref_t fbd )
                    {
                        vp.TopLeftY = fbd.height - vp.Height - vp.TopLeftY ;
                    } ) ;
                }

                _ctx->ctx()->RSSetViewports( 1, &vp ) ;
            }
        }

        if( incoming_states.rss.clear_s.do_change && incoming_states.rss.clear_s.ss.do_activate && !popped )
        {
            if( _cur_fb_active == size_t(-1) )
            {
                bool_t const clear_color = incoming_states.rss.clear_s.ss.do_color_clear ;
                bool_t const clear_depth = incoming_states.rss.clear_s.ss.do_depth_clear ;

                if( clear_color )
                {
                    motor::math::vec4f_t const color = incoming_states.rss.clear_s.ss.clear_color ;
                    _ctx->clear_render_target_view( color ) ;
                }

                if( clear_depth )
                {
                    _ctx->clear_depth_stencil_view() ;
                }
            }
            else 
            {
                _framebuffers.access( _cur_fb_active, [&]( this_t::framebuffer_data_ref_t fb )
                {
                    bool_t const clear_color = incoming_states.rss.clear_s.ss.do_color_clear ;
                    bool_t const clear_depth = incoming_states.rss.clear_s.ss.do_depth_clear ;

                    if( clear_color )
                    {
                        motor::math::vec4f_t const color = incoming_states.rss.clear_s.ss.clear_color ;
                        FLOAT const dxcolor[ 4 ] = { color.x(), color.y(), color.z(), color.w() } ;
                        for( size_t i=0; i<fb.num_color; ++i )
                            _ctx->ctx()->ClearRenderTargetView( fb.rt_view[ i ], dxcolor ) ;
                    }

                    if( clear_depth && fb.ds_view != nullptr )
                    {
                        _ctx->ctx()->ClearDepthStencilView( fb.ds_view, D3D11_CLEAR_DEPTH, 1.0f, 0 ) ;
                    }
                } ) ;
            }
        }

        if( incoming_states.rss.depth_s.do_change )
        {
            if ( popped )
            {
                assert( incoming_states.depth_stencil_state != nullptr ) ;

                _ctx->ctx()->OMSetDepthStencilState( incoming_states.depth_stencil_state, 0 ) ;

                incoming_states.depth_stencil_state->Release() ;
                incoming_states.depth_stencil_state = nullptr ;
            }
            else
            {
                assert( incoming_states.depth_stencil_state == nullptr ) ;

                D3D11_DEPTH_STENCIL_DESC desc = {} ;
                desc.DepthEnable = incoming_states.rss.depth_s.ss.do_activate ? TRUE : FALSE ;
                desc.DepthFunc = D3D11_COMPARISON_LESS ;
                desc.DepthWriteMask = incoming_states.rss.depth_s.ss.do_depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO ;

                auto const res = _ctx->dev()->CreateDepthStencilState( &desc, &incoming_states.depth_stencil_state ) ;
                if ( FAILED( res ) )
                {
                    motor::log::global_t::error( "CreateDepthStencilState" ) ;
                }

                _ctx->ctx()->OMSetDepthStencilState( incoming_states.depth_stencil_state, 0 ) ;
            }
        }

        // blend
        {
            if ( popped )
            {
                assert( incoming_states.blend_state != nullptr ) ;

                _ctx->ctx()->OMSetBlendState( incoming_states.blend_state, 0, 0xffffffff );

                incoming_states.blend_state->Release() ;
                incoming_states.blend_state = nullptr ;
            }
            else
            {
                assert( incoming_states.blend_state == nullptr ) ;

                D3D11_BLEND_DESC desc = {} ;

                desc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
                desc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                desc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE;
                desc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
                desc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

                if ( incoming_states.rss.blend_s.do_change )
                {
                    desc.RenderTarget[ 0 ].BlendEnable = incoming_states.rss.blend_s.ss.do_activate ;

                    desc.RenderTarget[ 0 ].SrcBlend = motor::platform::d3d11::convert(
                        incoming_states.rss.blend_s.ss.src_blend_factor ) ;
                    desc.RenderTarget[ 0 ].DestBlend = motor::platform::d3d11::convert(
                        incoming_states.rss.blend_s.ss.dst_blend_factor ) ;
                    desc.RenderTarget[ 0 ].BlendOp = motor::platform::d3d11::convert(
                        incoming_states.rss.blend_s.ss.blend_func ) ;

                    desc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ONE ;
                    desc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO ;
                    desc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD ;

                    auto const res = _ctx->dev()->CreateBlendState( &desc, &incoming_states.blend_state ) ;
                    if ( SUCCEEDED( res ) )
                    {
                        _ctx->ctx()->OMSetBlendState( incoming_states.blend_state, 0, 0xffffffff );
                    }
                }
            }
            
        }
        // rasterizer state conglomerate
        {
            if ( popped )
            {
                assert( incoming_states.raster_state != nullptr ) ;

                _ctx->ctx()->RSSetState( incoming_states.raster_state ) ;

                incoming_states.raster_state->Release() ;
                incoming_states.raster_state = nullptr ;
            }
            else
            {
                assert( incoming_states.raster_state == nullptr ) ;

                D3D11_RASTERIZER_DESC raster_desc = {} ;

                if ( incoming_states.rss.polygon_s.do_change )
                {
                    if ( incoming_states.rss.polygon_s.ss.do_activate )
                        raster_desc.CullMode = motor::platform::d3d11::convert( incoming_states.rss.polygon_s.ss.cm ) ;
                    else
                        raster_desc.CullMode = D3D11_CULL_NONE ;

                    raster_desc.FillMode = motor::platform::d3d11::convert( incoming_states.rss.polygon_s.ss.fm ) ;

                    raster_desc.FrontCounterClockwise = incoming_states.rss.polygon_s.ss.ff == motor::graphics::front_face::counter_clock_wise ;
                }

                if ( incoming_states.rss.scissor_s.do_change )
                {
                    raster_desc.ScissorEnable = incoming_states.rss.scissor_s.ss.do_activate ;
                    if ( incoming_states.rss.scissor_s.ss.do_activate )
                    {
                        LONG const h = _state_stack.top().rss.view_s.ss.vp.w() ;

                        D3D11_RECT rect ;
                        rect.left = incoming_states.rss.scissor_s.ss.rect.x() ;
                        rect.right = incoming_states.rss.scissor_s.ss.rect.x() + incoming_states.rss.scissor_s.ss.rect.z() ;
                        rect.top = h - ( incoming_states.rss.scissor_s.ss.rect.y() + incoming_states.rss.scissor_s.ss.rect.w() ) ;
                        rect.bottom = rect.top + incoming_states.rss.scissor_s.ss.rect.w() ;

                        _ctx->ctx()->RSSetScissorRects( 1, &rect ) ;
                    }
                }
                if ( incoming_states.raster_state != nullptr )
                {
                    incoming_states.raster_state->Release() ;
                    incoming_states.raster_state = nullptr ;
                }

                auto const res = _ctx->dev()->CreateRasterizerState( &raster_desc, &incoming_states.raster_state ) ;
                if ( FAILED( res ) )
                {
                    motor::log::global_t::error( "CreateRasterizerState" ) ;
                }

                _ctx->ctx()->RSSetState( incoming_states.raster_state ) ;
            }
        }
    }

    //************************************************************************************************************
    // if oid == -1, the state is popped.
    void_t handle_render_state( size_t const oid, size_t const rs_id ) noexcept
    {
        auto new_id = std::make_pair( oid, rs_id ) ;

        // pop state
        if( oid == size_t( -1 ) )
        {
            if( _state_stack.size() == 1 )
            {
                motor::log::global_t::error( d3d11_backend_log( "no more render states to pop" ) ) ;
                return ;
            }
            auto old = _state_stack.pop() ;

            // undo render state effects 
            {
                old.rss = old.rss - _state_stack.top().rss ;
                this->handle_render_state( old, true ) ;
            }
        }
        else
        {
            this_t::render_state_sets rss ;
            _states.access( new_id.first, [&]( this_t::state_data_ref_t sd )
            {
                rss.rss = _state_stack.top().rss + sd.states[ new_id.second ] ;
            } ) ;
            this->handle_render_state( rss, false ) ;
            _state_stack.push( rss ) ;
        }
    }

    //************************************************************************************************************
    bool_t construct_framebuffer( motor::graphics::framebuffer_object_ref_t obj ) noexcept
    {
        //oid = determine_oid( obj.get_oid(_bid), obj.name(), _framebuffers ) ;
        //auto & fb = _framebuffers[ oid ] ;


        size_t oid = obj.get_oid( _bid ) ;
        auto const res = _framebuffers.access( oid, obj.name(), [&]( this_t::framebuffer_data_ref_t fb )
        {
            ID3D11Device * dev = _ctx->dev() ;

            // colors
            {
                for( size_t i=0; i<fb.num_color; ++i )
                {
                    auto*& view = fb.rt_view[ i ] ;
                    if( view != nullptr )
                    {
                        view->Release() ;
                        view = nullptr ;
                    }
                }

                // release images
                for( size_t i = 0; i < fb.num_color; ++i )
                {
                    if( fb.image_ids[ i ] != size_t( -1 ) )
                    {
                        _images.access( fb.image_ids[ i ], [&]( this_t::image_data_ref_t img )
                        {
                            //
                            // could just call img.invalidate() i guess
                            // 
                            img.view->Release() ;
                            img.texture->Release() ;
                            img.sampler->Release() ;

                            img.view = nullptr  ;
                            img.texture = nullptr ;
                            img.sampler = nullptr ;
                        } ) ;
                    }
                }

                size_t const nt = obj.get_num_color_targets() ;
                auto const ctt = obj.get_color_target() ;
                auto const dst = obj.get_depth_target();

                for( size_t i = 0; i < nt; ++i )
                {
                    // sampler
                    guard< ID3D11SamplerState > smp ;
                    {
                        D3D11_SAMPLER_DESC sampDesc = { } ;
                        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR ;
                        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP ;
                        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP ;
                        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP ;
                        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER ;
                        sampDesc.MinLOD = 0;
                        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

                        auto const hr = dev->CreateSamplerState( &sampDesc, smp ) ;
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateSamplerState" ) ) ;
                            continue ;
                        }
                    }

                    // texture
                    guard< ID3D11Texture2D > tex ;
                    {
                        auto const dims = obj.get_dims() ;
                        size_t const width = UINT( dims.x() ) ;
                        size_t const height = UINT( dims.y() ) ;
                    
                        D3D11_TEXTURE2D_DESC desc = { } ;
                        desc.Width = static_cast< UINT >( width ) ;
                        desc.Height = static_cast< UINT >( height ) ;
                        desc.MipLevels = static_cast< UINT >( 1 ) ;
                        desc.ArraySize = static_cast< UINT >( 1 ) ;
                        desc.Format = motor::platform::d3d11::convert( ctt ) ;
                        desc.SampleDesc.Count = 1 ;
                        desc.SampleDesc.Quality = 0 ;
                        desc.Usage = D3D11_USAGE_DEFAULT ;
                        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE ;
                        desc.CPUAccessFlags = 0 ;
                        desc.MiscFlags = 0 ;

                        // create the texture object
                        {
                            auto const hr = dev->CreateTexture2D( &desc, nullptr, tex ) ;
                            if( FAILED( hr ) )
                            {
                                motor::log::global_t::error( d3d11_backend_log( "CreateTexture2D" ) ) ;
                                continue ;
                            }
                        }
                    }

                    // shader resource view
                    guard< ID3D11ShaderResourceView > srv ;
                    {
                        D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = { } ;
                        res_desc.Format = motor::platform::d3d11::convert( ctt ) ;
                        res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D ;
                        res_desc.Texture2D.MostDetailedMip = 0 ;
                        res_desc.Texture2D.MipLevels = UINT( 1 ) ;

                        auto const hr = dev->CreateShaderResourceView( tex, &res_desc, srv ) ;
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateShaderResourceView for texture : [" + 
                                obj.name() + "]" ) ) ;
                            continue ;
                        }
                    }

                    // render target view
                    guard< ID3D11RenderTargetView > view ;
                    {
                        D3D11_RENDER_TARGET_VIEW_DESC desc = { } ;
                        desc.Format = motor::platform::d3d11::convert( ctt ) ;
                        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D ;
                        desc.Texture2D.MipSlice = 0 ;

                        auto const res = _ctx->dev()->CreateRenderTargetView( tex, &desc, view ) ;
                        if( FAILED( res ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateRenderTargetView" ) ) ;
                            continue ;
                        }
                    }

                    // store data
                    {
                        motor::string_t const name = obj.name() + "." + motor::to_string( i ) ;

                        _images.access( fb.image_ids[ i ], name, [&] ( this_t::image_data_ref_t img )
                        {
                            img.sampler = smp.move_out() ;
                            img.texture = tex.move_out() ;
                            img.view = srv.move_out() ;
                            img.requires_y_flip = 1.0f ;
                            return true ;
                        } ) ;
                        {
                            auto const dims = obj.get_dims() ;
                            fb.width = FLOAT( dims.x() ) ;
                            fb.height = FLOAT( dims.y() ) ;
                        }

                        fb.num_color = nt ;
                        fb.rt_view[ i ] = view.move_out() ;
                    }
                }
            }

            // depth stencil
            {
                if( fb.ds_view != nullptr )
                {
                    fb.ds_view->Release() ;
                    fb.ds_view = nullptr ;
                }

                // release images
                if( fb.image_ids[ fb.num_color ] != size_t( -1 ) )
                {
                    _images.access( fb.image_ids[ fb.num_color ], [&]( this_t::image_data_ref_t img )
                    {
                        //
                        // could just call img.invalidate() i guess
                        // 
                        img.view->Release() ;
                        img.texture->Release() ;
                        img.sampler->Release() ;

                        img.view = nullptr  ;
                        img.texture = nullptr ;
                        img.sampler = nullptr ;
                    } ) ;
                }
            
                auto const dst = obj.get_depth_target();
            
                if( dst != motor::graphics::depth_stencil_target_type::unknown )
                {
                    // sampler
                    guard< ID3D11SamplerState > smp ;
                    {
                        D3D11_SAMPLER_DESC sampDesc = { } ;
                        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR ;
                        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP ;
                        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP ;
                        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP ;
                        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER ;
                        sampDesc.MinLOD = 0 ;
                        sampDesc.MaxLOD = D3D11_FLOAT32_MAX ;

                        auto const hr = dev->CreateSamplerState( &sampDesc, smp ) ;
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateSamplerState" ) ) ;
                            return false ;
                        }
                    }

                    // texture
                    guard< ID3D11Texture2D > tex ;
                    {
                        auto const dims = obj.get_dims() ;
                        size_t const width = UINT( dims.x() ) ;
                        size_t const height = UINT( dims.y() ) ;

                        D3D11_TEXTURE2D_DESC desc = { } ;
                        desc.Width = static_cast< UINT >( width ) ;
                        desc.Height = static_cast< UINT >( height ) ;
                        desc.MipLevels = static_cast< UINT >( 1 ) ;
                        desc.ArraySize = static_cast< UINT >( 1 ) ;
                        desc.Format = motor::platform::d3d11::convert_texture( dst ) ;
                        desc.SampleDesc.Count = 1 ;
                        desc.SampleDesc.Quality = 0 ;
                        desc.Usage = D3D11_USAGE_DEFAULT ;
                        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE ;
                        desc.CPUAccessFlags = 0 ;
                        desc.MiscFlags = 0 ;

                        // create the texture object
                        {
                            auto const hr = dev->CreateTexture2D( &desc, nullptr, tex ) ;
                            if( FAILED( hr ) )
                            {
                                motor::log::global_t::error( d3d11_backend_log( "CreateTexture2D" ) ) ;
                                return false ;
                            }
                        }
                    }

                    // shader resource view
                    guard< ID3D11ShaderResourceView > srv ;
                    {
                        D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = { } ;
                        res_desc.Format = motor::platform::d3d11::convert_shader_resource( dst ) ;
                        res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D ;
                        res_desc.Texture2D.MostDetailedMip = 0 ;
                        res_desc.Texture2D.MipLevels = UINT( 1 ) ;

                        auto const hr = dev->CreateShaderResourceView( tex, &res_desc, srv ) ;
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateShaderResourceView for texture : [" +
                                obj.name() + "]" ) ) ;
                            return false ;
                        }
                    }

                    // render target view
                    guard< ID3D11DepthStencilView > view ;
                    {
                        D3D11_DEPTH_STENCIL_VIEW_DESC desc = { } ;
                        desc.Format = motor::platform::d3d11::convert_depth_stencil_view( dst ) ;
                        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D ;
                        desc.Texture2D.MipSlice = 0 ;

                        auto const res = _ctx->dev()->CreateDepthStencilView( tex, &desc, view ) ;
                        if( FAILED( res ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( "CreateDepthStencilView" ) ) ;
                            return false ;
                        }
                    }

                    // store data
                    {
                        size_t const i = obj.get_num_color_targets() ;
                        motor::string_t const name = obj.name() + ".depth" ;
                        _images.access( fb.image_ids[ i ], name, [&] ( this_t::image_data_ref_t img )
                        {
                            img.sampler = smp.move_out() ;
                            img.texture = tex.move_out() ;
                            img.view = srv.move_out() ;
                            img.requires_y_flip = 1.0f ;
                            return true ;
                        } ) ;
                        {
                            auto const dims = obj.get_dims() ;
                            fb.width = FLOAT( dims.x() ) ;
                            fb.height = FLOAT( dims.y() ) ;
                        }

                        fb.ds_view = view.move_out() ;
                    }
                }
            }

            return true ;
        } ) ;

        if( res ) obj.set_oid( _bid, oid ) ;

        return res ;
    }

    //************************************************************************************************************
    void_t release_framebuffer( size_t const oid ) noexcept 
    {
        //auto & fb = _framebuffers[ oid ] ;
        //fb.invalidate() ;

        _framebuffers.invalidate( oid ) ;
    }

    //************************************************************************************************************
    bool_t activate_framebuffer( size_t const oid ) noexcept
    {
        //framebuffer_data_ref_t fb = framebuffer_data_ref_t( _framebuffers[ oid ] ) ;

        _framebuffers.access( oid, [&]( this_t::framebuffer_data_ref_t fb )
        {
            _ctx->ctx()->OMSetRenderTargets( UINT( fb.num_color ), fb.rt_view, fb.ds_view ) ;
        } ) ;

        _cur_fb_active = oid ;

        return true ;
    }

    //************************************************************************************************************
    void_t deactivate_framebuffer( void_t )
    {
        _ctx->ctx()->OMSetRenderTargets( 0, nullptr, nullptr ) ;
        _ctx->activate_framebuffer() ;
        
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

        _cur_fb_active = size_t( -1 ) ;
    }

    //************************************************************************************************************
    size_t construct_streamout( size_t oid, motor::graphics::streamout_object_ref_t obj ) noexcept
    {
        auto const res = _streamouts.access( oid, obj.name(), [&] ( motor::string_in_t conf_name, this_t::so_data_ref_t config )
        {
            // capture the vertex layout for stream out
            if ( obj.num_buffers() > 0 )
            {
                obj.get_buffer( 0 ).for_each_layout_element(
                    [&] ( motor::graphics::vertex_buffer_t::data_cref_t d )
                {
                    this_t::so_data::layout_element le ;
                    le.va = d.va ;
                    le.type = d.type ;
                    le.type_struct = d.type_struct ;
                    config.elements.push_back( le ) ;
                } ) ;

                config.stride = UINT( obj.get_buffer( 0 ).get_layout_sib() ) ;
            }

            // create buffers
            {
                obj.for_each( [&] ( size_t const i, motor::graphics::vertex_buffer_cref_t vb )
                {
                    if ( i >= this_t::so_data::max_buffers ) return ;

                    size_t const byte_width = obj.size() * obj.get_buffer( i ).get_layout_sib() ;

                    // create buffer
                    {
                        D3D11_BUFFER_DESC bd = {} ;
                        bd.ByteWidth = UINT( byte_width ) ;
                        bd.Usage = D3D11_USAGE_DEFAULT ;
                        bd.BindFlags = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER | D3D10_BIND_SHADER_RESOURCE ;

                        for ( size_t j = 0; j < 2; ++j )
                        {
                            auto & b = config._ping_pong[ j ].buffers[ i ].invalidate() ;

                            HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, NULL, b ) ;
                            motor::log::global_t::error( FAILED( hr ), d3d11_backend_log( "CreateBuffer - D3D11_BIND_STREAM_OUTPUT" ) ) ;
                        }
                    }

                    // create the resource view
                    {
                        auto const le = obj.get_buffer( i ).get_layout_element_zero() ;
                        D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {} ;
                        res_desc.Format = motor::platform::d3d11::convert_type_to_vec_format( le.type, le.type_struct ) ;
                        res_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER ;

                        res_desc.Buffer.FirstElement = 0 ;

                        // somehow, this can not be 0
                        // otherwise, hr will be in valid.
                        res_desc.Buffer.NumElements = UINT( obj.size() * obj.get_buffer( i ).get_num_layout_elements() ) ;

                        for ( size_t j = 0; j < 2; ++j )
                        {
                            auto & b = config._ping_pong[ j ].buffers[ i ] ;
                            auto & v = config._ping_pong[ j ].views[ i ].invalidate() ;
                            auto const hr = _ctx->dev()->CreateShaderResourceView( b, &res_desc, v ) ;
                            if( FAILED( hr ) )
                            {
                                d3d11_log_errorv("CreateShaderResourceView for buffer : [%s]", conf_name.c_str() ) ;
                            }
                        }
                    }
                } ) ;

                config.buffers_used = std::min( obj.num_buffers(), this_t::so_data::max_buffers ) ;
            }

            // create query objects
            {
                D3D11_QUERY_DESC qd ;
                qd.Query = D3D11_QUERY_SO_STATISTICS ;
                qd.MiscFlags = 0 ;

                for ( size_t j = 0; j < 2; ++j )
                {
                    HRESULT const hr = _ctx->dev()->CreateQuery( &qd, config._ping_pong[ j ].query ) ;
                    motor::log::global_t::error( FAILED( hr ), d3d11_backend_log( "CreateQuery - D3D11_QUERY_SO_STATISTICS" ) ) ;
                }
            }

            return true ;
        } ) ;
        return oid ;
    }

    //************************************************************************************************************
    void_t release_streamout( size_t const oid ) noexcept
    {
        _renders.for_each( [&]( this_t::render_data_ref_t rd )
        {
            rd.unlink_streamout( oid ) ;
        } ) ;

        //auto & d = _streamouts[ oid ] ;
        _streamouts.invalidate( oid ) ;
        
    }

    //************************************************************************************************************
    bool_t update( size_t const oid, motor::graphics::streamout_object_ref_t obj, bool_t const is_config )
    {
        //auto & d = _streamouts[oid] ;
        return _streamouts.access( oid, [&]( this_t::so_data_ref_t d )
        {
            if( d.buffers_used != obj.num_buffers() )
            {
                this_t::construct_streamout( oid, obj ) ;
                return true ;
            }

            obj.for_each( [&]( size_t const i, motor::graphics::vertex_buffer_cref_t vb )
            {
                if( i >= this_t::so_data::max_buffers ) return ;

                size_t const byte_width = obj.size() * obj.get_buffer(i).get_layout_sib() ;

                for( size_t pp=0; pp<2; ++pp )
                {
                    D3D11_BUFFER_DESC bd ;
                    d._ping_pong[pp].buffers[i]->GetDesc( &bd ) ;

                    if( byte_width <= bd.ByteWidth ) continue ;
                    bd.ByteWidth = UINT( byte_width ) ;

                    auto & b = d._ping_pong[pp].buffers[i].invalidate() ;
                    HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, NULL, b ) ;
                    motor::log::global_t::error( FAILED( hr ), d3d11_backend_log( 
                        "CreateBuffer - D3D11_BIND_STREAM_OUTPUT" ) ) ;
                }
            } ) ;
            return true ;
        } ) ;
    }

    //************************************************************************************************************
    bool_t activate_streamout( size_t const oid ) noexcept
    {
        _cur_streamout_active = oid ;

        if( _cur_streamout_active == size_t(-1) ) return true ;

        //auto & d = _streamouts[ _cur_streamout_active] ;
        _streamouts.access( _cur_streamout_active,[&]( this_t::so_data_ref_t d )
        {
            auto & wrtb = d.write_buffer() ;
            _ctx->ctx()->Begin( wrtb.query ) ;
        
            {
                UINT offsets[ this_t::so_data::max_buffers] = { 0, 0, 0, 0 } ;
                ID3D11Buffer * buffers[ this_t::so_data::max_buffers ] ;
                for( size_t i=0; i<this_t::so_data::max_buffers; ++i )
                {
                    if( wrtb.buffers[i] == nullptr ) break ;
                    buffers[i] = wrtb.buffers[i] ;
                }
                _ctx->ctx()->SOSetTargets( UINT(d.buffers_used), buffers, offsets ) ;
            }
        } ) ;

        return true ;
    }

    //************************************************************************************************************
    void_t deactivate_streamout( void_t )
    {
        if( _cur_streamout_active == size_t(-1) ) return ;
        
        //auto & d = _streamouts[_cur_streamout_active] ;
        _streamouts.access( _cur_streamout_active, [&]( this_t::so_data_ref_t d )
        {
            d.swap() ;
            _ctx->ctx()->End( d.read_buffer().query ) ;
            _ctx->ctx()->SOSetTargets( 0, nullptr, nullptr ) ;
        } ) ;

        _cur_streamout_active = size_t( -1 ) ;
    }

    //****************************************************************************************
    void_t send_configure_to_st( motor::graphics::msl_object_mtr_t obj ) noexcept
    {
        obj->set_oid( _bid, size_t(-2) ) ;
        _ctsd->add_item( this_t::work_item::make_item( 
            this_t::work_item::work_type::configure, motor::share( obj ) ) ) ;
    }

    //****************************************************************************************
    // call from support thread
    bool_t construct_msl_data_st( motor::graphics::msl_object_ptr_t obj_in ) noexcept
    {
        auto obj = *obj_in ;

        motor::vector< motor::msl::symbol_t > config_symbols ;

        obj.for_each_msl( motor::graphics::msl_api_type::msl_4_0,
            [&] ( motor::string_in_t code )
        {
            motor::msl::post_parse::document_t doc =
                motor::msl::parser_t( "d3d11" ).process( code ) ;

            _mdb.insert( std::move( doc ), config_symbols ) ;
        } ) ;

        // cases:
        // obj.oid == -1 && obj.name.empty() : library/config shader
        // obj.oid == -1 && !obj.name.empty() : initial configuration

        // obj.oid != -1 && obj.name.empty() : invalid
        // obj.oid != -1 && !obj.name.empty() : was already configured

        // if the incoming msl shader is a library shader for example,
        // it does not need to have a associated background object
        size_t oid = obj_in->get_oid( _bid ) ;
        _msls.access( oid, obj.name(), [] ( this_t::msl_data_ref_t ) { return true ; } ) ;

        // if -1, it is probably a library shader or some tmp 
        // msl object. So do not return any valid is below.
        bool_t const is_valid_msl = oid != size_t( -1 ) ;

        for ( auto const & c : config_symbols )
        {
            auto const c_exp = c.expand() ;

            // this most likely came from a library dependency.
            // need to figure out the msl object associated
            // with this render configuration/object.
            // the msl object is required in order to reconstruct
            // the render_object and the shader_object.

            // 1. find the msl object associated to c
            // 2. use the found oid for further processing
            if ( oid == size_t( -1 ) )
            {
                auto [i, o] = this_t::find_pair_by_ro_name( c_exp, _msls ) ;
                oid = i ;
                obj = o ;
            }

            // msl database contains render configuration 
            // which has not been configured by the user...
            if ( oid == size_t( -1 ) )
            {
                motor_warning2( 1024,
                    "[d3d11::construct_msl_data] : "
                    "render configuration not found : %s", c_exp.c_str() ) ;
                continue ;
            }

            motor::msl::generatable_t res = motor::msl::dependency_resolver_t().resolve( &_mdb, c ) ;
            if ( res.missing.size() != 0 )
            {
                motor::log::global_t::warning( "[d3d11] : We have missing symbols for " + c_exp + " :" ) ;
                for ( auto const & s : res.missing )
                {
                    motor::log::global_t::status( s.expand() ) ;
                }
                continue ;
            }
                        
            // inject default variable values into the 
            // variable sets
            for ( auto & shd_ : res.config.shaders )
            {
                for ( auto & var_ : shd_.variables )
                {
                    if ( var_.def_val == size_t( -1 ) ) continue ;

                    auto * df = res.config.def_values[ var_.def_val ] ;
                    if ( dynamic_cast<motor::msl::generic_default_value< motor::math::vec3f_t >*> ( df ) != nullptr )
                    {
                        using ptr_t = motor::msl::generic_default_value< motor::math::vec3f_t > * ;
                        ptr_t gdv = static_cast<ptr_t>( df ) ;
                        for ( auto & vs : obj.borrow_varibale_sets() )
                        {
                            // for @overwrite specifier
                            //if( vs->has_data_variable( var_.name ) ) continue ;
                            vs->data_variable<motor::math::vec3f_t>( var_.name )->set( gdv->get() ) ;
                        }
                    }
                    else if ( dynamic_cast<motor::msl::texture_dv_ptr_t> ( df ) != nullptr )
                    {
                        using ptr_t = motor::msl::texture_dv_ptr_t ;
                        ptr_t gdv = static_cast<ptr_t>( df ) ;
                        for ( auto & vs : obj.borrow_varibale_sets() )
                        {
                            // for @overwrite specifier
                            // if variable is already in the variable set, do not overwrite it
                            //if( vs->has_texture_variable( var_.name ) ) continue ;

                            // have the type here
                            // gdv->get().t == motor::msl::texture_tag_dv::type::tex1d
                            vs->texture_variable( var_.name )->set( gdv->get().name ) ;
                        }
                    }
                }
            }
                        
            motor::graphics::render_object_t ro( c_exp ) ;
            motor::graphics::shader_object_t so( c_exp ) ;

            // generate code
            {
                auto tp_begin = std::chrono::high_resolution_clock::now() ;

                motor::msl::generator_t gen( std::move( res ) ) ;
                auto const code = gen.generate<motor::msl::hlsl::hlsl5_generator_t>() ;
                motor::graphics::msl_bridge::create_by_api_type( 
                    motor::graphics::shader_api_type::hlsl_5_0, code, so ) ;

                {
                    size_t const milli = std::chrono::duration_cast<std::chrono::milliseconds>
                        ( std::chrono::high_resolution_clock::now() - tp_begin ).count() ;

                    char buffer[ 2048 ] ;
                    std::snprintf( buffer, 2048, "[d3d11] : generating hlsl shader took %zu ms", milli ) ;
                    motor::log::global_t::status( buffer ) ;
                }
            }

            {
                if ( obj.get_streamout().size() != 0 && obj.get_geometry().size() != 0 )
                {
                    ro.link_geometry( obj.get_geometry()[ 0 ], obj.get_streamout()[ 0 ] ) ;
                }
                else
                {
                    ro.link_geometry( obj.get_geometry() ) ;
                }

                ro.link_shader( c_exp ) ;
                ro.add_variable_sets( obj.get_varibale_sets() ) ;
            }

            auto const access_res = _msls.access( oid, obj.name(), [&] ( this_t::msl_data_ref_t msl )
            {
                {
                    //so.set_oid( bid, this_t::construct_shader_config( so.get_oid( bid ), so ) ) ;
                    if ( !this_t::construct_shader_config( so ) )
                    {
                        // construction/compilation failed
                        // @todo return here.
                        return false ;
                    }
                    if( !this_t::construct_render_config( ro ) )
                    {
                        return false ;
                    }
                }

                // render object
                {
                    size_t i = size_t( -1 ) ;
                    while ( ++i < msl.ros.size() &&
                        std::strcmp( c_exp.c_str(), msl.ros[ i ].name().c_str() ) != 0 ) ;

                    if ( i == msl.ros.size() ) msl.ros.emplace_back( std::move( ro ) ) ;
                    else msl.ros[ i ] = std::move( ro ) ;
                }

                // shader object
                {
                    // reflect compilation result to the user
                    _shaders.access( so.get_oid( _bid ),[&]( pimpl::shader_data_ref_t shd )
                    {
                        obj.for_each( [&] ( motor::graphics::compilation_listener_mtr_t lst )
                        {
                            auto const s = shd.compiled ?
                                motor::graphics::compilation_listener::state::successful :
                                motor::graphics::compilation_listener::state::failed ;

                            lst->set( s, so.shader_bindings() ) ;
                        } ) ;
                    } ) ;

                    {
                        size_t i = size_t( -1 ) ;
                        while ( ++i < msl.sos.size() &&
                            std::strcmp( msl.sos[ i ].name().c_str(), c_exp.c_str() ) != 0 ) ;

                        if ( i == msl.sos.size() ) msl.sos.emplace_back( std::move( so ) ) ;
                        else msl.sos[ i ] = std::move( so ) ;
                    }
                }

                msl.msl_obj = std::move( obj )  ;

                return true ;
            } ) ;
        }
                    
        // true: was msl object. so the id
        // needs to go back to the caller
        if( is_valid_msl ) 
        {
            obj_in->set_oid( _bid, oid ) ;
        }
        else
        {
            obj_in->set_oid( _bid, size_t(-1) ) ;
        }

        return is_valid_msl ;
    }

    //************************************************************************************************************
    size_t construct_geo( size_t oid, motor::graphics::geometry_object_ref_t obj )
    {
        auto const res = _geos.access( oid, obj.name(), [&]( this_t::geo_data_ref_t config )
        {
            config.pt = obj.primitive_type() ;

            // vertex buffer object
            {
                obj.vertex_buffer().for_each_layout_element(
                    [&] ( motor::graphics::vertex_buffer_t::data_cref_t d )
                {
                    this_t::geo_data::layout_element le ;
                    le.va = d.va ;
                    le.type = d.type ;
                    le.type_struct = d.type_struct ;
                    config.elements.push_back( le ) ;
                } ) ;

                config.stride = UINT( obj.vertex_buffer().get_layout_sib() ) ;

                // = number of vertices * sizeof( vertex )
                // if there are vertices already setup, take the numbers
                size_t const byte_width = obj.vertex_buffer().get_sib() ;

                D3D11_BUFFER_DESC bd = {} ;
                bd.Usage = D3D11_USAGE_DYNAMIC ;
                // using max of 1 sib so empty vbs will create and will 
                // be resized in the update function. Important for now
                // is that the buffer will be NOT nullptr.
                bd.ByteWidth = UINT( std::max( byte_width, size_t( 1 ) ) ) ;
                bd.BindFlags = D3D11_BIND_VERTEX_BUFFER ;
                bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE ;

                motor::memory::global_t::dealloc_raw( config.vb_mem ) ;
                config.vb_mem = motor::memory::global_t::alloc_raw< uint8_t >(
                    byte_width, "[d3d11] : vertex buffer memory" ) ;

                if ( config.vb != nullptr )
                {
                    config.vb->Release() ;
                    config.vb = nullptr ;
                }

                if ( byte_width != 0 )
                {
                    D3D11_SUBRESOURCE_DATA init_data = {} ;
                    init_data.pSysMem = (void_cptr_t) config.vb_mem ;

                    std::memcpy( config.vb_mem, obj.vertex_buffer().data(), byte_width ) ;

                    HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, config.vb ) ;
                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_VERTEX_BUFFER" ) ) ;
                    }
                }
                else
                {
                    HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, nullptr, config.vb ) ;
                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_VERTEX_BUFFER" ) ) ;
                    }
                }
            }

            // index buffer object
            if ( obj.index_buffer().get_num_elements() != 0 )
            {
                // = number of vertices * sizeof( index_t )
                size_t const byte_width = obj.index_buffer().get_sib() ;

                D3D11_BUFFER_DESC bd = {} ;
                bd.Usage = D3D11_USAGE_DYNAMIC ;
                // using max of 1 sib so empty ibs will create and will 
                // be resized in the update function. Important for now
                // is that the buffer will be NOT nullptr.
                bd.ByteWidth = uint_t( std::max( byte_width, size_t( 1 ) ) ) ;
                bd.BindFlags = D3D11_BIND_INDEX_BUFFER ;
                bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE ;

                motor::memory::global_t::dealloc_raw( config.ib_mem ) ;
                config.ib_mem = motor::memory::global_t::alloc_raw< uint8_t >(
                    byte_width, "[d3d11] : index buffer memory" ) ;

                if ( config.ib != nullptr )
                {
                    config.ib->Release() ;
                    config.ib = nullptr ;
                }

                if ( byte_width != 0 )
                {
                    D3D11_SUBRESOURCE_DATA init_data = {} ;
                    init_data.pSysMem = (void_cptr_t) config.ib_mem ;

                    std::memcpy( config.ib_mem, obj.index_buffer().data(), byte_width ) ;

                    HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, config.ib ) ;
                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_INDEX_BUFFER" ) ) ;
                    }
                }
                else
                {
                    HRESULT const hr = _ctx->dev()->CreateBuffer( &bd, nullptr, config.ib ) ;
                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_INDEX_BUFFER" ) ) ;
                    }
                }
            }

            return true ;
        } ) ;

        return oid ;
    }

    //************************************************************************************************************
    void_t release_geometry( size_t const oid ) noexcept 
    {
        _renders.for_each( [&]( this_t::render_data_ref_t d )
        {
            d.unlink_geometry( oid ) ;
        } ) ;
        _geos.invalidate( oid ) ;
    }

    //************************************************************************************************************
    bool_t update( size_t const id, motor::graphics::geometry_object_mtr_t geo )
    {
        //auto& config = _geos[ id ] ;
        return _geos.access( id, [&]( this_t::geo_data_ref_t config )
        {
            if( geo->index_buffer().get_num_elements() != 0 && config.ib == nullptr )
            {
                this_t::construct_geo( id, *geo ) ;
            }

            // vb: check memory space
            {
                D3D11_BUFFER_DESC bd ;
                config.vb->GetDesc( &bd ) ;

                size_t const sib = geo->vertex_buffer().get_sib() ;
                if( sib > bd.ByteWidth )
                {
                    bd.ByteWidth = uint_t( sib ) ;

                    motor::memory::global_t::dealloc_raw( config.vb_mem ) ;
                    config.vb_mem = motor::memory::global_t::alloc_raw< uint8_t >( sib, 
                        "[d3d11] : vertex buffer memory" ) ;

                    D3D11_SUBRESOURCE_DATA init_data = { } ;
                    init_data.pSysMem = ( void_ptr_t ) config.vb_mem ;
                
                    config.vb->Release() ;
                    {
                        auto const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, config.vb ) ;
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( 
                                "CreateBuffer( vertex_buffer )" ) ) ;
                            return false ;
                        }
                    }
                }

                config.num_elements_vb = geo->vertex_buffer().get_num_elements() ;

                // copy data
                {
                    // exactly the size in bytes of the 1d buffer
                    LONG const lsib = LONG( geo->vertex_buffer().get_layout_sib() ) ;
                    LONG const ne = LONG( geo->vertex_buffer().get_num_elements() ) ;
                    void_cptr_t data = geo->vertex_buffer().data() ;
                    CD3D11_BOX const box( 0, 0, 0, ne, 1, 1 ) ;

                    //_ctx->ctx()->UpdateSubresource( config.vb, 0, nullptr /*&box*/, data, lsib, 0 ) ;
                    D3D11_MAPPED_SUBRESOURCE resource;
                    _ctx->ctx()->Map( config.vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ;
                    std::memcpy( resource.pData, data, lsib * ne ) ;
                    _ctx->ctx()->Unmap( config.vb, 0 ) ;
                }
            }

            // ib: check memory space
            if( config.ib != nullptr )
            {
                D3D11_BUFFER_DESC bd ;
                config.ib->GetDesc( &bd ) ;

                size_t const sib = geo->index_buffer().get_sib() ;
                if( sib > bd.ByteWidth )
                {
                    bd.ByteWidth = uint_t( sib ) ;

                    motor::memory::global_t::dealloc_raw( config.ib_mem ) ;
                    config.ib_mem = motor::memory::global_t::alloc_raw< uint8_t >(
                        sib, "[d3d11] : index buffer memory" ) ;

                    D3D11_SUBRESOURCE_DATA init_data = { } ;
                    init_data.pSysMem = ( void_ptr_t ) config.ib_mem ;

                    config.ib->Release() ;
                    {
                        auto const hr = _ctx->dev()->CreateBuffer( &bd, &init_data, config.ib ) ;
                        if( FAILED( hr ) )
                        {
                            motor::log::global_t::error( d3d11_backend_log( 
                                "CreateBuffer( index_buffer )" ) ) ;
                            return false ;
                        }
                    }
                }

                config.num_elements_ib = geo->index_buffer().get_num_elements() ;

                // copy data
                {
                    // exactly the size in bytes of the 1d buffer
                    LONG const lsib = LONG( sizeof( uint32_t ) ) ;
                    LONG const ne = LONG( geo->index_buffer().get_num_elements() ) ;
                    void_cptr_t data = geo->index_buffer().data() ;
                    CD3D11_BOX const box( 0, 0, 0, ne, 1, 1 ) ;

                    //_ctx->ctx()->UpdateSubresource( config.ib, 0, &box, data, lsib, ne ) ;
                    D3D11_MAPPED_SUBRESOURCE resource;
                    _ctx->ctx()->Map( config.ib, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ;
                    std::memcpy( resource.pData, data, lsib * ne ) ;
                    _ctx->ctx()->Unmap( config.ib, 0 ) ;
                }
            }

            return true ;
        } ) ;
    }

    //************************************************************************************************************
    bool_t construct_shader_data( motor::graphics::shader_object_ref_t obj, this_t::shader_data_inout_t shd ) const noexcept
    {
        auto tp_begin = std::chrono::high_resolution_clock::now() ;

        shd.compiled = false ;

        // shader code
        motor::graphics::shader_set_t ss ;
        {
            auto const res = obj.shader_set( this->sapi, ss ) ;
            if ( !res )
            {
                motor::log::global_t::warning( d3d11_backend_log(
                    "config [" + obj.name() + "] has no shaders for " +
                    motor::graphics::gen4::to_string( this->bt ) ) ) ;
                return false ;
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

                auto code = this->remove_unwanded_characters( ss.vertex_shader().code() ) ;
                void_cptr_t cptr = code.c_str() ;
                size_t const sib = ss.vertex_shader().code().size() ;

                // "vs_5_0" : Direct3D 11 and 11.1
                // "vs_4_1" : Direct3D 10.1
                // "vs_4_0" : Direct3D 10
                {
                    auto const hr = D3DCompile( cptr, sib, obj.name().c_str(), 
                        Shader_Macros, nullptr, "VS", "vs_5_0", 0, 0, &pVSBlob, &errblob );

                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::warning( d3d11_backend_log(
                            "vertex shader [" + obj.name() + "] failed " +
                            motor::graphics::gen4::to_string( this->bt ) ) ) ;

                        if ( errblob != nullptr )
                        {
                            char_cptr_t ptr = (char_cptr_t) errblob->GetBufferPointer() ;
                            size_t const ssib = (size_t) errblob->GetBufferSize() ;
                            motor::string_t s( ptr, ssib ) ;

                            auto const pos = s.find( obj.name() ) ;
                            s = s.substr( pos, s.size() - pos ) ;
                            motor::log::global_t::error( s ) ;
                            errblob->Release() ;
                        }

                        shd.invalidate( obj.name() ) ;

                        return false ;
                    }
                }

                // find shader variables in constant buffer
                {
                    shd.vs_cbuffers = this_t::determine_cbuffer( pVSBlob ) ;
                    this_t::determine_texture( pVSBlob, shd.vs_textures, shd.vs_buffers ) ;
                }
            }

            // geometry shader
            if ( ss.has_geometry_shader() )
            {
                D3D_SHADER_MACRO Shader_Macros[] = { NULL, NULL };

                auto code = this->remove_unwanded_characters( ss.geometry_shader().code() ) ;
                void_cptr_t cptr = code.c_str() ;
                size_t const sib = ss.geometry_shader().code().size() ;

                // "gs_5_0" : Direct3D 11 and 11.1
                // "gs_4_1" : Direct3D 10.1
                // "gs_4_0" : Direct3D 10
                auto const hr = D3DCompile( cptr, sib, obj.name().c_str(), 
                    Shader_Macros, nullptr, "GS", "gs_5_0", 0, 0, &pGSBlob, &errblob );

                if ( FAILED( hr ) )
                {
                    motor::log::global_t::warning( d3d11_backend_log(
                        "geometry shader [" + obj.name() + "] failed " +
                        motor::graphics::gen4::to_string( this->bt ) ) ) ;

                    if ( errblob != nullptr )
                    {
                        char_cptr_t ptr = (char_cptr_t) errblob->GetBufferPointer() ;
                        size_t const ssib = (size_t) errblob->GetBufferSize() ;
                        motor::string_t s( ptr, ssib ) ;

                        auto const pos = s.find( obj.name() ) ;
                        s = s.substr( pos, s.size() - pos ) ;
                        motor::log::global_t::error( s ) ;
                        errblob->Release() ;
                    }

                    shd.invalidate( obj.name() ) ;

                    return false ;
                }

                // find shader variables in constant buffer
                {
                    shd.gs_cbuffers = this_t::determine_cbuffer( pGSBlob ) ;
                    this_t::determine_texture( pGSBlob, shd.gs_textures, shd.gs_buffers ) ;
                }
            }

            // pixel shader
            if ( ss.has_pixel_shader() )
            {
                D3D_SHADER_MACRO Shader_Macros[] = { NULL, NULL };

                auto code = this->remove_unwanded_characters( ss.pixel_shader().code() ) ;
                void_cptr_t cptr = code.c_str() ;
                size_t const sib = ss.pixel_shader().code().size() ;

                // "ps_5_0" : Direct3D 11 and 11.1
                // "ps_4_1" : Direct3D 10.1
                // "ps_4_0" : Direct3D 10
                auto const hr = D3DCompile( cptr, sib, obj.name().c_str(), 
                    Shader_Macros, nullptr, "PS", "ps_5_0", 0, 0, &pPSBlob, &errblob );

                if ( FAILED( hr ) )
                {
                    motor::log::global_t::warning( d3d11_backend_log(
                        "pixel shader [" + obj.name() + "] failed " +
                        motor::graphics::gen4::to_string( this->bt ) ) ) ;

                    if ( errblob != nullptr )
                    {
                        char_cptr_t ptr = (char_cptr_t) errblob->GetBufferPointer() ;
                        size_t const ssib = (size_t) errblob->GetBufferSize() ;
                        motor::string_t s( ptr, ssib ) ;

                        auto const pos = s.find( obj.name() ) ;
                        s = s.substr( pos, s.size() - pos ) ;
                        motor::log::global_t::error( s ) ;
                        errblob->Release() ;
                    }

                    shd.invalidate( obj.name() ) ;

                    return false ;
                }

                // find shader variables in constant buffer
                {
                    shd.ps_cbuffers = this_t::determine_cbuffer( pPSBlob ) ;
                    this_t::determine_texture( pPSBlob, shd.ps_textures, shd.ps_buffers ) ;
                }
            }
        }

        // vertex shader
        if ( pVSBlob != nullptr )
        {
            auto const hr = _ctx->dev()->CreateVertexShader(
                pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, shd.vs ) ;

            motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreateVertexShader" ) ;

            // the vs blob is required for later layout creation
            shd.vs_blob = pVSBlob ;
        }

        // Create Streamout shader w or w/o geometry shader
        if ( obj.shader_bindings().get_num_output_bindings() > 0 &&
            obj.get_streamout_mode() != motor::graphics::streamout_mode::unknown )
        {
            static size_t const max_entries = 10 ;
            D3D11_SO_DECLARATION_ENTRY decl[ max_entries ] ;
            if ( obj.shader_bindings().get_num_output_bindings() > max_entries )
            {
                motor::log::global_t::warning( "[d3d11_backend] : you can not have more than " +
                    motor::to_string( max_entries ) + " vertex output bindings for stream out" ) ;
            }

            obj.shader_bindings().for_each_vertex_output_binding( [&] ( size_t const i, motor::graphics::vertex_attribute const va,
                motor::graphics::ctype const ct, motor::string_in_t )
            {
                D3D11_SO_DECLARATION_ENTRY e
                {
                    0,
                    motor::platform::d3d11::vertex_output_binding_to_semantic( va ),
                    motor::platform::d3d11::vertex_output_binding_to_semantic_index( va ),
                    0,
                    (BYTE) motor::graphics::num_components_of( ct.ts ),
                    0
                } ;
                decl[ std::min( max_entries, i ) ] = e ;
            } ) ;

            // @todo which stream should go to the rasterizer?
            UINT const rasterized_stream = pPSBlob == nullptr ? D3D11_SO_NO_RASTERIZED_STREAM : 0 ;

            // if no geometry shader, use vertex shader
            if ( pGSBlob == nullptr )
            {
                auto const hr = _ctx->dev()->CreateGeometryShaderWithStreamOutput( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), decl,
                    std::min( UINT( max_entries ), UINT( obj.shader_bindings().get_num_output_bindings() ) ), NULL, 0, rasterized_stream, NULL, shd.gs ) ;
                motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreateGeometryShaderWithStreamOutput" ) ;
            }
            else
            {
                auto const hr = _ctx->dev()->CreateGeometryShaderWithStreamOutput( pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), decl,
                    std::min( UINT( max_entries ), UINT( obj.shader_bindings().get_num_output_bindings() ) ), NULL, 0, rasterized_stream, NULL, shd.gs ) ;
                motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreateGeometryShaderWithStreamOutput" ) ;
            }
        }

        // geometry shader w/o streamout
        else if ( pGSBlob != nullptr )
        {
            auto const hr = _ctx->dev()->CreateGeometryShader(
                pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, shd.gs ) ;

            motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreateGeometryShader" ) ;
            pGSBlob->Release();
            pGSBlob = nullptr ;
        }

        // pixel shader
        if ( pPSBlob != nullptr )
        {
            auto const hr = _ctx->dev()->CreatePixelShader(
                pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, shd.ps ) ;

            motor::log::global_t::error( FAILED( hr ), "[d3d11] : CreatePixelShader" ) ;
            pPSBlob->Release();
            pPSBlob = nullptr ;
        }

        // shader input variables
        // the final layout elements for the d3d input layout
        // is done if the render configuration is known.
        {
            obj.shader_bindings().for_each_vertex_input_binding( [&] ( size_t const,
                motor::graphics::vertex_attribute const va, motor::string_cref_t name )
            {
                shd.vertex_inputs.emplace_back( this_t::shader_data::vertex_input_binding
                    { va, name } ) ;
            } ) ;
        }

        motor_status2( 1024, "[D3D11] : Compilation Successful : [%s]", obj.name().c_str() ) ;

        shd.compiled = true ;

        {
            size_t const milli = std::chrono::duration_cast<std::chrono::milliseconds>
                ( std::chrono::high_resolution_clock::now() - tp_begin ).count() ;

            motor_status2( 2048, "[d3d11] : shader compilation %zu ms [%s]", 
                milli, obj.name().c_str() ) ;
        }

        return true ;
    }

    //************************************************************************************************************
    bool_t construct_shader_config( motor::graphics::shader_object_ref_t obj )
    {
        size_t oid = obj.get_oid( _bid ) ;

        this_t::shader_data_t new_shader ;
        if( this_t::construct_shader_data( obj, new_shader ) ) 
        {
            auto const res = _shaders.access( oid, obj.name(), [&]( pimpl::shader_data_ref_t sd )
            {
                sd = std::move( new_shader ) ;
                return true ;
            } ) ;
            obj.set_oid( _bid, oid ) ;
            return res ;
        }
        
        // compilation failed.
        return false ;
    }

    //************************************************************************************************************
    void_t release_shader_data( size_t const oid ) noexcept 
    {
        _renders.for_each( [&]( this_t::render_data & rd )
        {
            if( rd.shd_id == oid ) rd.shd_id = size_t( -1 ) ;
        } ) ;

        _shaders.invalidate( oid ) ;
    }

    //************************************************************************************************************
    // @param sid shader oid if known
    bool_t construct_render_config( motor::graphics::render_object_ref_t obj, size_t const sid = size_t(-1) )
    {
        size_t oid = obj.get_oid( _bid ) ;
        auto const res = _renders.access( oid, obj.name(), [&]( this_t::render_data_ref_t rd )
        {
            if ( rd.vertex_layout != nullptr )
            {
                rd.vertex_layout->Release() ;
                rd.vertex_layout = nullptr ;
            }

            if ( rd.vertex_layout_so != nullptr )
            {
                rd.vertex_layout_so->Release() ;
                rd.vertex_layout_so = nullptr ;
            }

            std::memset( rd.layout, 0, ARRAYSIZE( rd.layout ) ) ;

            {
                auto const res = this_t::update( rd, obj ) ;
                motor::log::global_t::error( !res, "[d3d11] : update from construct_render_config" ) ;
            }

            return true ;
        } ) ;
        obj.set_oid( _bid, oid ) ;
        return res ;
    }

    //************************************************************************************************************
    void_t release_render_data( size_t const oid ) noexcept 
    {
        _renders.invalidate( oid ) ;
    }

    //************************************************************************************************************
    // must be called from within a safe area(i.e. through an access into _renders)
    bool_t update( this_t::render_data_ref_t rd, motor::graphics::render_object_ref_t rc )
    {     
        rd.shd_id = size_t( -1 ) ;

        // find geometry
        {
            rd.geo_ids.clear() ;
            for ( size_t i = 0; i < rc.get_num_geometry(); ++i )
            {
                auto const id_ = _geos.find_by_name( rc.get_geometry( i ) ) ;

                if ( id_ == size_t( -1 ) )
                {
                    motor::log::global_t::warning( d3d11_backend_log(
                        "no geometry with name [" + rc.get_geometry( i ) + "] for render_config [" + rc.name() + "]" ) ) ;
                    continue ;
                }

                rd.geo_ids.emplace_back( id_ ) ;
            }
        }

        // handle stream out links
        {
            rd.so_ids.clear() ;
            for ( size_t i = 0; i < rc.get_num_streamout(); ++i )
            {
                auto const id_ = _streamouts.find_by_name( rc.get_streamout( i ) ) ;

                if ( id_ == size_t( -1 ) )
                {
                    motor::log::global_t::warning( d3d11_backend_log(
                        "no streamout with name [" + rc.get_streamout( i ) + "] for render_config [" + rc.name() + "]" ) ) ;
                    continue ;
                }

                rd.so_ids.emplace_back( id_ ) ;
            }
        }

        if ( rd.geo_ids.size() == 0 && rd.so_ids.size() == 0 )
        {
            motor::log::global_t::warning( d3d11_backend_log(
                "no geometry nor streamout linked to render_object with name [" + rc.name() + "]" ) ) ;
            return false ;
        }

        // find shader
        if ( rd.shd_id == size_t( -1 ) )
        {
            auto const id_ = _shaders.find_by_name( rc.get_shader() ) ;
            if ( id_ == size_t( -1 ) )
            {
                motor::log::global_t::warning( d3d11_backend_log(
                    "no streamout with name [" + rc.get_shader() + "] for render_config [" + rc.name() + "]" ) ) ;
                return false ;
            }
            rd.shd_id = id_ ;
        }

        // may happen if shaders did not compile properly the first time.
        if ( rd.shd_id == size_t( -1 ) /*|| _shaders[ rd.shd_id ].vs_blob == nullptr*/ )
        {
            motor::log::global_t::warning( d3d11_backend_log(
                "something strange happened to render_config [" + rc.name() + "]" ) ) ;
            return false ;
        }

        if ( rd.geo_ids.size() != 0 )
        {
            this_t::geo_data_t::elements_t elems ;
            {
                bool_t const valid_obj = _geos.access( rd.geo_ids[ 0 ], [&] ( this_t::geo_data_ref_t geo )
                {
                    elems = geo.elements ;
                } )  ;
                assert( valid_obj ) ;
            }

            ID3DBlob * blob_ptr = nullptr ;
            this_t::shader_data_t::vibs_t vibs ;
            {
                bool_t const valid_obj = _shaders.access( rd.shd_id, [&] ( this_t::shader_data_ref_t shd )
                {
                    vibs = shd.vertex_inputs ;
                    blob_ptr = shd.vs_blob ;
                } ) ;
                assert( valid_obj ) ;
            }

            // sort shader input attributes based on geometry layout.
            // in hlsl the order seems to be required to match for the
            // used geometry layout (next section) and the shader input
            // attribute order.
            {
                for ( size_t i = 0; i < vibs.size(); ++i )
                {
                    size_t j = i ;

                    while ( ( j < vibs.size() ) && ( elems[ i ].va != vibs[ j ].va ) ) ++j ;

                    if ( j == i ) continue ;
                    // input variable declared in shader not found in geometry, remove ...
                    else if ( j == vibs.size() )
                    {
                        vibs.erase( vibs.begin() + i ) ;
                        motor::log::global_t::warning( d3d11_backend_log(
                            "removed shader input attribute in [" + rc.name() + "]. "
                            "Attribute not found in geometry layout." ) ) ;
                    }
                    // .. or entries are not at the same spot. exchange.
                    else if ( j != i )
                    {
                        auto const tmp = vibs[ i ] ;
                        vibs[ i ] = vibs[ j ] ;
                        vibs[ j ] = tmp ;
                    }
                }
            }

            // for binding attributes, the shader and the geometry is required.
            {
                size_t i = 0 ;
                UINT offset = 0 ;
                for ( auto const & b : vibs )
                {
                    char_cptr_t name = motor::platform::d3d11::vertex_binding_to_semantic( b.va ) ;
                    UINT const semantic_index = motor::platform::d3d11::vertex_output_binding_to_semantic_index( b.va ) ;
                    DXGI_FORMAT const fmt = this_t::geo_data_t::get_format_from_element( b.va, elems ) ;
                    UINT input_slot = 0 ;
                    UINT aligned_byte_offset = offset ;
                    D3D11_INPUT_CLASSIFICATION const iclass = D3D11_INPUT_PER_VERTEX_DATA ;
                    UINT instance_data_step_rate = 0 ;

                    rd.layout[ i++ ] = { name, semantic_index, fmt, input_slot,
                        aligned_byte_offset, iclass, instance_data_step_rate } ;
                    
                    offset += this_t::geo_data_t::get_sib( b.va, elems ) ;
                }

                UINT const num_elements = UINT( i ) ;

                if ( rd.vertex_layout != nullptr )
                {
                    rd.vertex_layout->Release() ;
                    rd.vertex_layout = nullptr ;
                }

                bool_t failed = false ;
                _shaders.access( rd.shd_id, [&] ( motor::string_in_t name, this_t::shader_data_ref_t shd )
                {
                    auto const hr = _ctx->dev()->CreateInputLayout( rd.layout, num_elements,
                        shd.vs_blob->GetBufferPointer(),
                        shd.vs_blob->GetBufferSize(), &rd.vertex_layout ) ;

                    if ( FAILED( hr ) )
                    {
                        d3d11_log_warningv(
                            "CreateInputLayout for shader [%s] and render object[%s]",
                            name.c_str(), rc.name().c_str() ) ;

                        failed = true ;
                    }
                } ) ;

                if ( failed ) return false ;
            }
        }

        if ( rd.so_ids.size() != 0 )
        {
            this_t::shader_data_t::vibs_t vibs ;
            this_t::so_data_t::elements_t elems ;

            _shaders.access( rd.shd_id, [&] ( this_t::shader_data_ref_t shd )
            {
                vibs = shd.vertex_inputs ;
            } ) ;

            // sort shader input attributes based on geometry layout.
            // in hlsl the order seems to be required to match for the
            // used geometry layout (next section) and the shader input
            // attribute order.
            {
                size_t n = 0 ;
                _streamouts.access( rd.so_ids[ 0 ], [&] ( this_t::so_data_ref_t sod )
                {
                    elems = sod.elements ;
                    n = std::min( sod.elements.size(), vibs.size() ) ;
                } ) ;

                //this_t::so_data_ref_t sod = _streamouts[ rd.so_ids[0] ] ;
                //size_t const n = std::min( sod.elements.size(), vibs.size() ) ;

                if ( vibs.size() != elems.size() )
                {
                    motor::log::global_t::error(
                        "[d3d11] : number of shader input attributes(" + motor::to_string( vibs.size() ) + ") do not match the number of streamout "
                        "vertex attributes(" + motor::to_string( elems.size() ) + ") for render object[" + rc.name() + "] " ) ;
                }

                for ( size_t i = 0; i < n; ++i )
                {
                    size_t j = i ;

                    while ( j < vibs.size() && elems[ i ].va != vibs[ j ].va ) ++j ;

                    if ( j == i ) continue ;

                    // input variable declared in shader not found in geometry, remove ...
                    else if ( j == vibs.size() )
                    {
                        vibs.erase( vibs.begin() + i ) ;
                        _shaders.access( rd.shd_id, [&] ( motor::string_in_t shd_name, this_t::shader_data_ref_t shd )
                        {
                            motor::log::global_t::warning( d3d11_backend_log(
                                "removed shader input attribute in [" + shd_name + "]. "
                                "Attribute not found in geometry layout." ) ) ;
                        } ) ;
                    }
                    // .. or entries are not at the same spot. exchange.
                    else if ( j != i )
                    {
                        auto const tmp = vibs[ i ] ;
                        vibs[ i ] = vibs[ j ] ;
                        vibs[ j ] = tmp ;
                    }
                }
            }

            // for binding attributes, the shader and the geometry is required.
            // in this case it is the streamout buffer.
            {
                size_t i = 0 ;
                UINT offset = 0 ;
                for ( auto const & b : vibs )
                {
                    char_cptr_t name = motor::platform::d3d11::vertex_binding_to_semantic( b.va ) ;
                    UINT const semantic_index = motor::platform::d3d11::vertex_output_binding_to_semantic_index( b.va ) ;
                    DXGI_FORMAT const fmt = this_t::so_data_t::get_format_from_element( b.va, elems ) ;
                    UINT input_slot = 0 ;
                    UINT aligned_byte_offset = offset ;
                    D3D11_INPUT_CLASSIFICATION const iclass = D3D11_INPUT_PER_VERTEX_DATA ;
                    UINT instance_data_step_rate = 0 ;

                    rd.layout_so[ i++ ] = { name, semantic_index, fmt, input_slot,
                        aligned_byte_offset, iclass, instance_data_step_rate } ;

                    offset += this_t::so_data_t::get_sib( b.va, elems ) ;
                }

                UINT const num_elements = UINT( i ) ;

                if ( rd.vertex_layout_so != nullptr )
                {
                    rd.vertex_layout_so->Release() ;
                    rd.vertex_layout_so = nullptr ;
                }

                auto const res = _shaders.access( rd.shd_id, [&] ( motor::string_in_t shd_name, this_t::shader_data_ref_t shd )
                {
                    HRESULT const hr = _ctx->dev()->CreateInputLayout( rd.layout_so, num_elements,
                        shd.vs_blob->GetBufferPointer(),
                        shd.vs_blob->GetBufferSize(), &rd.vertex_layout_so ) ;

                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::warning( d3d11_backend_log(
                            "CreateInputLayout for shader [" + shd_name + "] and "
                            "render object[" + rc.name() + "]" ) ) ;
                        return false ;
                    }
                    return true ;
                } ) ;

                if( !res ) return false ;
            }
        }

        // release placeholder/ref count manager variable sets
        {
            for ( auto * vs : rd.var_sets ) motor::memory::release_ptr( vs ) ;
            rd.var_sets.clear() ;
        }

        {
            auto release_funk = [&] ( this_t::render_data::cbuffers_ref_t datum )
            {
                for ( auto & b : datum )
                {
                    if ( b.mem != nullptr )
                    {
                        motor::memory::global_t::dealloc_raw( b.mem ) ;
                        b.mem = nullptr ;
                    }
                    b.ptr = guard<ID3D11Buffer>() ;
                }
                datum.clear() ;
            } ;
            release_funk( rd._cbuffers_vs ) ;
            release_funk( rd._cbuffers_gs ) ;
            release_funk( rd._cbuffers_ps ) ;
        }

        {
            rd.var_sets_imgs_vs.clear() ;
            rd.var_sets_imgs_ps.clear() ;

            rd._cbuffers_vs.clear() ;
            rd._cbuffers_gs.clear() ;
            rd._cbuffers_ps.clear() ;

            rd.var_sets_buffers_vs.clear() ;
            rd.var_sets_buffers_gs.clear() ;
            rd.var_sets_buffers_ps.clear() ;

            rd.var_sets_buffers_so_vs.clear() ;
            rd.var_sets_buffers_so_gs.clear() ;
            rd.var_sets_buffers_so_ps.clear() ;
        }

        // track ref count for variable set
        {
            rc.for_each( [&] ( size_t const /*i*/, motor::graphics::variable_set_mtr_t vs )
            {
                rd.var_sets.emplace_back( motor::memory::copy_ptr( vs ) ) ;
            } ) ;
        }

        // constant buffer mapping
        {
            auto var_funk = [] ( ID3D11Device * dev, size_t const var_set_idx, motor::graphics::variable_set_mtr_t vs,
                this_t::shader_data_t::cbuffers_ref_t cbuffers, this_t::render_data::cbuffers_inout_t cbs )
            {
                for ( auto & c : cbuffers )
                {
                    this_t::render_data_t::cbuffer_t cb ;
                    cb.mem = motor::memory::global_t::alloc_raw< uint8_t >( c.sib, "[d3d11] : vertex shader cbuffer variable" ) ;
                    cb.slot = c.slot ;
                    cb.var_set_idx = var_set_idx ;

                    D3D11_BUFFER_DESC bd = {} ;
                    bd.Usage = D3D11_USAGE_DEFAULT ;
                    bd.ByteWidth = UINT( c.sib ) ;
                    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER ;
                    bd.CPUAccessFlags = 0 ;

                    D3D11_SUBRESOURCE_DATA InitData = {} ;
                    InitData.pSysMem = cb.mem ;
                    auto const hr = dev->CreateBuffer( &bd, &InitData, cb.ptr ) ;
                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_CONSTANT_BUFFER" ) ) ;
                    }

                    for ( auto & var : c.datas )
                    {
                        render_data_t::data_variable_t dv ;
                        dv.ivar = vs->data_variable( var.name, var.t, var.ts ) ;
                        dv.sib = motor::graphics::size_of( var.t ) * motor::graphics::size_of( var.ts ) ;
                        dv.name = var.name ;
                        dv.t = var.t ;
                        dv.ts = var.ts ;
                        dv.offset = var.offset ;
                        cb.data_variables.emplace_back( dv ) ;
                    }
                    cbs.emplace_back( std::move( cb ) ) ;
                }
            } ;

            //this_t::shader_data_ref_t shd = _shaders[ rd.shd_id ] ;
            _shaders.access( rd.shd_id, [&]( this_t::shader_data_ref_t shd )
            {
                rc.for_each( [&] ( size_t const i, motor::graphics::variable_set_mtr_t vs )
                {
                    var_funk( _ctx->dev(), i, vs, shd.vs_cbuffers, rd._cbuffers_vs ) ;
                    var_funk( _ctx->dev(), i, vs, shd.gs_cbuffers, rd._cbuffers_gs ) ;
                    var_funk( _ctx->dev(), i, vs, shd.ps_cbuffers, rd._cbuffers_ps ) ;
                } ) ;
            } ) ;
        }

        // texture variable mapping
        {
            //this_t::shader_data_ref_t shd = _shaders[ rd.shd_id ] ;
            _shaders.access( rd.shd_id, [&]( this_t::shader_data_ref_t shd )
            {
                rc.for_each( [&] ( size_t const vs_id, motor::graphics::variable_set_mtr_t vs )
                {
                    for ( auto & t : shd.ps_textures )
                    {
                        auto * dv = vs->texture_variable( t.name ) ;
                        motor::string_t const img_name = dv->get().name() ;
                        
                        size_t const i = _images.find_by_name( img_name ) ;
                        if( i == size_t(-1) ) continue ;

                        this_t::render_data_t::image_variable_t iv ;
                        iv.var_set_idx = vs_id ;
                        iv.value_hash = dv->get().hash() ;
                        iv.id = i ;
                        iv.name = t.name ;
                        iv.slot = t.slot ;
                        rd.var_sets_imgs_ps.emplace_back( std::move( iv ) ) ;

                        // set y flip for the current texture in the current variable set
                        _images.access( i, [&]( this_t::image_data_ref_t d )
                        {
                            auto * var = vs->data_variable< float_t >( "sys_flipv_" + t.name ) ;
                            var->set( d.requires_y_flip ) ;
                        } ) ;
                    }
                } ) ;
            } ) ;

            
        }

        auto array_variable_mapping = [&]( motor::graphics::render_object_ref_t rc_,
            pimpl::streamout_datas_t & streamouts,
            this_t::render_data_t::varsets_to_buffers_t & var_sets_buffers,
            this_t::render_data_t::varsets_to_buffers_t & var_sets_buffers_so,
            pimpl::shader_data_t::buffer_variables_ref_t the_buffer )
        {
            rc_.for_each( [&] ( size_t const /*i*/, motor::graphics::variable_set_mtr_t vs )
            {
                this_t::render_data_t::buffer_variables_t bvs ;
                this_t::render_data_t::buffer_variables_t bvs_so ;
                for ( auto & t : the_buffer )
                {
                    // first try data_buffers...
                    motor::string_t const name = vs->array_variable( t.name )->get().name() ;
                    size_t const i = _arrays.find_by_name( name ) ;

                    // ... if the stored variable name is found in the data_buffers array, it is used ...
                    if ( i < _arrays.size() )
                    {
                        this_t::render_data_t::buffer_variable_t bv ;
                        bv.id = i ;
                        bv.name = t.name ;
                        bv.slot = t.slot ;
                        bvs.emplace_back( std::move( bv ) ) ;
                    }
                    // ... otherwise we default to the streamout objects
                    else
                    {
                        motor::string_t const name2 = vs->array_variable_streamout( t.name )->get().name() ;
                        size_t const i2 = streamouts.find_by_name( name2 ) ;

                        if ( i2 < streamouts.size() )
                        {
                            this_t::render_data_t::buffer_variable_t bv ;
                            bv.id = i2 ;
                            bv.name = t.name ;
                            bv.slot = t.slot ;
                            bvs_so.emplace_back( std::move( bv ) ) ;
                        }
                    }
                }
                var_sets_buffers.emplace_back( std::make_pair( vs, std::move( bvs ) ) ) ;
                var_sets_buffers_so.emplace_back( std::make_pair( vs, std::move( bvs_so ) ) ) ;
            } ) ;
        } ;

        {
            //this_t::shader_data_ref_t shd = _shaders[ rd.shd_id ] ;
            _shaders.access( rd.shd_id, [&]( this_t::shader_data_ref_t shd )
            {
                array_variable_mapping( rc, _streamouts, rd.var_sets_buffers_vs, rd.var_sets_buffers_so_vs, shd.vs_buffers ) ;
                array_variable_mapping( rc, _streamouts, rd.var_sets_buffers_gs, rd.var_sets_buffers_so_gs, shd.gs_buffers ) ;
                array_variable_mapping( rc, _streamouts, rd.var_sets_buffers_ps, rd.var_sets_buffers_so_ps, shd.ps_buffers ) ;
            } ) ;
        }

        return true ;
    }

    //************************************************************************************************************
    bool_t update( size_t const oid, motor::graphics::msl_object_ref_t obj, size_t const vs_id )
    {
        _msls.access( oid, [&]( this_t::msl_data_ref_t msl )
        {
            size_t const num_ros = msl.ros.size() ;
            for ( size_t i = 0; i < num_ros; ++i )
            {
                this_t::update( msl.ros[ i ].get_oid( _bid ), msl.ros[ i ], vs_id ) ;
            }
        } ) ;
        
        return true ;
    }

    //************************************************************************************************************
    bool_t construct_image_config( motor::graphics::image_object_ref_t obj )
    {
        #if 0
        oid = this_t::determine_oid( oid, obj.name(), _images ) ;
        this_t::image_data_ref_t img = _images[ oid ] ;
        #endif

        size_t oid = obj.get_oid( _bid ) ;
        auto const res = _images.access( oid, obj.name(), [&]( motor::string_in_t img_name, this_t::image_data_ref_t img )
        {
            if ( img.texture != nullptr )
            {
                img.texture->Release() ;
                img.sampler->Release() ;
                img.view->Release() ;

                img.texture = nullptr ;
                img.sampler = nullptr ;
                img.view = nullptr ;
            }

            auto * dev = _ctx->dev() ;

            // sampler
            {
                D3D11_SAMPLER_DESC sampDesc = {} ;
                sampDesc.Filter = motor::platform::d3d11::convert(
                    obj.get_filter( motor::graphics::texture_filter_mode::min_filter ),
                    obj.get_filter( motor::graphics::texture_filter_mode::mag_filter ) ) ;

                sampDesc.AddressU = motor::platform::d3d11::convert(
                    obj.get_wrap( motor::graphics::texture_wrap_mode::wrap_s ) ) ;

                sampDesc.AddressV = motor::platform::d3d11::convert(
                    obj.get_wrap( motor::graphics::texture_wrap_mode::wrap_t ) ) ;

                sampDesc.AddressW = motor::platform::d3d11::convert(
                    obj.get_wrap( motor::graphics::texture_wrap_mode::wrap_r ) ) ;

                sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER ;
                sampDesc.MinLOD = 0;
                sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

                auto const hr = dev->CreateSamplerState( &sampDesc, img.sampler );
                if ( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "CreateSamplerState" ) ) ;
                    return false ;
                }
            }

            // texture
            {
                auto & iref = obj.image() ;

                size_t const width = iref.get_dims().x() ;
                size_t const height = iref.get_dims().y() ;
                size_t const depth = iref.get_dims().z() ;

                D3D11_TEXTURE2D_DESC desc = {} ;
                desc.Width = static_cast<UINT>( width ) ;
                desc.Height = static_cast<UINT>( height ) ;
                desc.ArraySize = static_cast<UINT>( depth ) ;
                desc.MipLevels = static_cast<UINT>( 1 ) ;
                desc.Format = motor::platform::d3d11::convert( iref.get_image_format(), iref.get_image_element_type() ) ;
                desc.SampleDesc.Count = 1;
                desc.SampleDesc.Quality = 0;
                desc.Usage = D3D11_USAGE_DEFAULT ;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE ;
                desc.CPUAccessFlags = 0 ;
                desc.MiscFlags = 0 ;

                size_t const bpp = motor::graphics::size_of( iref.get_image_format() ) *
                    motor::graphics::size_of( iref.get_image_element_type() ) ;

                //uint8_ptr_t mem = motor::memory::global_t::alloc_raw<uint8_t>( width * height * 4 ) ;
                //for( size_t p = 0; p < width * height * 4; ++p ) mem[ p ] = uint8_t( 255 ) ;

                // create the texture object
                {
                    motor::memory::malloc_guard< D3D11_SUBRESOURCE_DATA > init_datas( depth ) ;

                    for ( size_t i = 0; i < depth; ++i )
                    {
                        init_datas[ i ].pSysMem = iref.get_image_ptr( i ) ;
                        init_datas[ i ].SysMemPitch = UINT( width * bpp ) ;
                        init_datas[ i ].SysMemSlicePitch = UINT( width * height * bpp ) ;
                    }

                    auto const hr = dev->CreateTexture2D( &desc, init_datas.get(), img.texture ) ;
                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "CreateTexture2D" ) ) ;
                        return false ;
                    }
                }

                // create the resource view for the texture in order to be
                // shader variable bindable.
                if ( img.texture != nullptr )
                {
                    D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {} ;
                    res_desc.Format = desc.Format ;
                    if ( obj.get_type() == motor::graphics::texture_type::texture_2d )
                    {
                        res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D ;
                        res_desc.Texture2D.MostDetailedMip = 0 ;
                        res_desc.Texture2D.MipLevels = UINT( 1 ) ;
                    }
                    else if ( obj.get_type() == motor::graphics::texture_type::texture_2d_array )
                    {
                        res_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY ;
                        res_desc.Texture2DArray.ArraySize = UINT( depth ) ;
                        res_desc.Texture2DArray.MostDetailedMip = 0 ;
                        res_desc.Texture2DArray.MipLevels = UINT( 1 ) ;
                        res_desc.Texture2DArray.FirstArraySlice = UINT( 0 ) ;
                    }

                    auto const hr = dev->CreateShaderResourceView( img.texture, &res_desc, img.view ) ;
                    if ( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log( 
                            "CreateShaderResourceView for texture : [" + img_name + "]" ) ) ;
                        return false ;
                    }
                }
            }

            return true ;
        } ) ;

        if( res ) obj.set_oid( _bid, oid ) ;
        return res ;
    }

    //************************************************************************************************************
    void_t release_image_data( size_t const oid ) noexcept 
    {
        _images.invalidate( oid ) ;
    }

    //************************************************************************************************************
    size_t construct_array_data( size_t oid, motor::graphics::array_object_ref_t obj ) noexcept
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : construct array object" ) ;

        // only vec4 float allowed
        {
            bool_t valid = true ;
            obj.data_buffer().for_each_layout_element( [&] ( motor::graphics::data_buffer::layout_element_cref_t le )
            {
                if ( le.type != motor::graphics::type::tfloat &&
                    le.type_struct != motor::graphics::type_struct::vec4 ) valid = false ;
            } ) ;

            if ( !valid )
            {
                motor::log::global_t::error( "[d3d11] : data buffer must only consist of vec4f layout elements." ) ;
                return oid ;
            }
        }

        //oid = this_t::determine_oid( oid, obj.name(), _arrays ) ;
        //this_t::array_data_ref_t data = _arrays[ oid ] ;
        auto const res = _arrays.access( oid, obj.name(), [&]( this_t::array_data_ref_t data )
        {
            ID3D11Device * dev = _ctx->dev() ;

            // = number of vertices * sizeof( vertex )
            // if there are vertices already setup, take the numbers
            size_t const byte_width = obj.data_buffer().get_sib() ;

            if ( data.buffer != nullptr )
            {
                data.buffer->Release() ;
                data.buffer = nullptr ;
            }

            // this release is important for releasing the
            // buffer reference too.
            if ( data.view != nullptr )
            {
                data.view->Release() ;
                data.view = nullptr ;
            }

            // must be 16 in order to have access to all data.
            // this is independent from the number of layout elements.
            // layout elements may only have vec4f inserted!
            size_t const elem_sib = 16 ; // sizeof( vec4f ) ;

            // create the buffer
            {
                D3D11_BUFFER_DESC bd = {} ;
                bd.Usage = D3D11_USAGE_DYNAMIC ;
                // using max of 1 sib so empty vbs will create and will 
                // be resized in the update function. Important for now
                // is that the buffer will be NOT nullptr.
                bd.ByteWidth = UINT( std::max( byte_width, elem_sib ) ) ;
                bd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D10_BIND_SHADER_RESOURCE ;
                bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE ;

                HRESULT const hr = dev->CreateBuffer( &bd, nullptr, data.buffer ) ;
                if ( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_BIND_DATA_BUFFER" ) ) ;
                }
            }

            // create the resource view
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC res_desc = {} ;
                res_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT ;
                res_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER ;

                res_desc.Buffer.FirstElement = 0 ;

                // somehow, this can not be 0
                // otherwise, hr will be in valid.
                res_desc.Buffer.NumElements = std::max( UINT( byte_width ) / UINT( elem_sib ), 1u ) ;

                auto const hr = dev->CreateShaderResourceView( data.buffer, &res_desc, data.view ) ;
                if ( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( 
                        "CreateShaderResourceView for buffer : [" + obj.name() + "]" ) ) ;
                }
            }

            return true ;
        } ) ;

        return oid ;
    }

    //************************************************************************************************************
    void_t release_array_data( size_t const oid ) noexcept 
    {
        _arrays.invalidate( oid ) ;
    }

    //************************************************************************************************************
    bool_t update( size_t const id, motor::graphics::array_object_ref_t obj, bool_t const is_config )
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : update array object" ) ;

        //auto & data = _arrays[ id ] ;

        _arrays.access( id, [&]( this_t::array_data_ref_t data )
        {
            D3D11_BUFFER_DESC bd ;
            data.buffer->GetDesc( &bd ) ;

            size_t const sib = obj.data_buffer().get_sib() ;
            if( sib > bd.ByteWidth )
            {
                this->construct_array_data( id, obj ) ;
            }

            // copy data
            {
                // exactly the size in bytes of the 1d buffer
                LONG const lsib = LONG( obj.data_buffer().get_layout_sib() ) ;
                LONG const ne = LONG( obj.data_buffer().get_num_elements() ) ;
                void_cptr_t data_ptr = obj.data_buffer().data() ;

                D3D11_MAPPED_SUBRESOURCE resource;
                _ctx->ctx()->Map( data.buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ;
                std::memcpy( resource.pData, data_ptr, lsib * ne ) ;
                _ctx->ctx()->Unmap( data.buffer, 0 ) ;
            }
        } ) ;

        return true ;
    }

    //************************************************************************************************************
    bool_t update( size_t const id, motor::graphics::image_object_ref_t obj, bool_t const is_config ) noexcept
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : update image object" ) ;
        
        auto & iref = obj.image() ;

        return _images.access( id, [&]( this_t::image_data_ref_t data )
        {
            // check if image has changed
            {
                D3D11_TEXTURE2D_DESC bd ;
                data.texture->GetDesc( &bd ) ;

                size_t const width = iref.get_dims().x() ;
                size_t const height = iref.get_dims().y() ;
                size_t const depth = iref.get_dims().z() ;

                bool_t const b1 = bd.Width != width ;
                bool_t const b2 = bd.Height != height ;
                bool_t const b3 = bd.ArraySize != depth ;
                bool_t const b4 = bd.Format != motor::platform::d3d11::convert( iref.get_image_format(), iref.get_image_element_type() ) ;

                if ( motor::math::vec4b_t( b1, b2, b3, b4 ).any() )
                {
                    return this->construct_image_config( obj ) ;
                }
            }

            // copy data if only the content has changed.
            {
                size_t const width = iref.get_dims().x();
                size_t const height = iref.get_dims().y();
                size_t const depth = iref.get_dims().z();
                
                D3D11_BOX region ;
                region.left = 0u ;
                region.right = (UINT) width ;
                region.top = 0u ;
                region.bottom = (UINT) height ;
                region.front = 0u ;
                region.back = (UINT) depth ;

                UINT const row_pitch = (UINT) iref.row_sib() ;
                UINT const depth_pitch = (UINT) iref.layer_sib() ;

                _ctx->ctx()->UpdateSubresource( data.texture, 0, &region, iref.get_image_ptr(), row_pitch, depth_pitch ) ;
            }

            return true ;
        } ) ;
    }

    //************************************************************************************************************
    // update the variable data in the api, i.e. it copies the data from engine to api memory
    bool_t update( size_t const id, motor::graphics::render_object_ref_t obj, size_t const varset_id )
    {
        MOTOR_PROBE( "Graphics", "[d3d11] : update render object" ) ;

        #if 0
        this_t::render_data_ref_t rnd = _renders[ id ] ;
        if( !rnd.valid ) return false ;
        #endif

        auto const res = _renders.access( id, [&]( this_t::render_data_ref_t rnd )
        {
            // data variables
            {
                auto update_funk = [&] ( ID3D11DeviceContext * ctx_, size_t const vsid, this_t::render_data::cbuffers_t & cbuffers )
                {
                    size_t idx = size_t( -1 ) ;
                    while ( ++idx < cbuffers.size()
                        && cbuffers[ idx ].var_set_idx < vsid ) ;

                    if ( idx == cbuffers.size() ) return ;
                    if ( cbuffers[ idx ].var_set_idx != vsid ) return ;

                    auto * vs = rnd.var_sets[ vsid ] ;

                    auto & cb = cbuffers[ idx ] ;
                    for ( size_t i = 0; i < cb.data_variables.size(); ++i )
                    {
                        auto & dv = cb.data_variables[ i ] ;

                        if ( dv.ivar == nullptr ) continue ;
                        dv.do_copy_funk_from_origin( cb.mem ) ;
                    }
                    ctx_->UpdateSubresource( cb.ptr, 0, nullptr, cb.mem, 0, 0 ) ;
                } ;

                update_funk( _ctx->ctx(), varset_id, rnd._cbuffers_vs ) ;
                update_funk( _ctx->ctx(), varset_id, rnd._cbuffers_gs ) ;
                update_funk( _ctx->ctx(), varset_id, rnd._cbuffers_ps ) ;
            }

            // textures
            // all we need to do is to figure out if a texture variable has
            // changed its name. If so, assign correct image id.
            {
                auto update_funk = [&] ( size_t const vsid, this_t::render_data::image_variables_t & image_variables )
                {
                    auto * vs = rnd.var_sets[ vsid ] ;

                    for ( size_t i = 0; i < image_variables.size(); ++i )
                    {
                        auto & iv = image_variables[ i ] ;
                        if ( iv.var_set_idx != vsid ) continue ;

                        auto * tx_var = vs->find_texture_variable( iv.name.c_str() ) ;

                        // if nullptr, variable does not exist anymore
                        // what to do then?
                        if ( tx_var == nullptr ) continue ;

                        if ( tx_var->get().hash() == iv.value_hash ) continue ;
                        iv.value_hash = tx_var->get().hash() ;
                        
                        size_t const idx = _images.find_by_name( tx_var->get().name() ) ;
                        iv.id = idx ;
                    }
                } ;

                update_funk( varset_id, rnd.var_sets_imgs_vs ) ;
                update_funk( varset_id, rnd.var_sets_imgs_ps ) ;

                #if 0
                size_t idx = size_t( -1 ) ;
                while ( ++idx < cbuffers.size()
                    && cbuffers[ idx ].var_set_idx < vsid ) ;

                rnd.var_sets_imgs_ps
                    #endif
            }

            return true ;
        } ) ;

        return res ;
    }

    //************************************************************************************************************
    bool_t render( size_t const id, size_t const geo_idx = 0, bool_t feed_from_so = false, bool_t use_streamout_count = false, size_t const varset_id = size_t( 0 ), UINT const start_element = UINT( 0 ),
        UINT const num_elements = UINT( -1 ) )
    {        
        MOTOR_PROBE( "Graphics", "[d3d11] : render" ) ;

        

        //this_t::render_data_ref_t rnd = _renders[ id ] ;

        return _renders.access( id, [&]( motor::string_in_t rnd_name, this_t::render_data_ref_t rnd )
        {
            if ( rnd.shd_id == size_t( -1 ) )
            {
                motor::log::global_t::error( d3d11_backend_log( "shader invalid. First shader "
                    "compilation failed probably." ) ) ;
                return false ;
            }

            if ( geo_idx >= rnd.geo_ids.size() && !feed_from_so )
            {
                motor::log::global_t::error( "[d3d11::render] : used geometry idx invalid "
                    "because exceeds array size for render object : " + rnd_name ) ;
                return false ;
            }

            ID3D11DeviceContext * ctx = _ctx->ctx() ;

            //this_t::shader_data_ref_t shd = _shaders[ rnd.shd_id ] ;

            {
                auto const res = _shaders.access( rnd.shd_id, [&] ( motor::string_in_t shd_name, this_t::shader_data_ref_t shd )
                {
                    if ( shd.vs == nullptr )
                    {
                        motor::log::global_t::error( d3d11_backend_log( "shader missing for " + shd_name ) ) ;
                        return false ;
                    }

                    // SECTION: Set all shaders
                    {
                        ctx->VSSetShader( shd.vs, nullptr, 0 ) ;
                        ctx->GSSetShader( shd.gs, nullptr, 0 ) ;
                        ctx->PSSetShader( shd.ps, nullptr, 0 ) ;
                    }
                    return true ;
                } ) ;

                if ( !res ) return false ;
            }


            // SECTION: vertex shader variables
            {
                for ( auto & cb : rnd._cbuffers_vs )
                {
                    if ( cb.var_set_idx > varset_id ) break  ;
                    if ( cb.var_set_idx < varset_id ) continue ;
                    ctx->VSSetConstantBuffers( cb.slot, 1, cb.ptr ) ;
                }

                for ( auto & buf : rnd.var_sets_buffers_vs[ varset_id ].second )
                {
                    _arrays.access( buf.id, [&]( this_t::array_data_ref_t ad )
                    {
                        ctx->VSSetShaderResources( buf.slot, 1, ad.view ) ;
                    } ) ;
                }

                for ( auto & buf : rnd.var_sets_buffers_so_vs[ varset_id ].second )
                {
                    _streamouts.access( buf.id, [&]( this_t::so_data_ref_t sod )
                    {
                        auto & ppb = sod.read_buffer() ;
                        ctx->VSSetShaderResources( buf.slot, 1, ppb.views[ 0 ] ) ;
                    } ) ;
                }
            }

            // SECTION: geometry shader variables
            {
                for ( auto & cb : rnd._cbuffers_gs )
                {
                    if ( cb.var_set_idx > varset_id ) break  ;
                    if ( cb.var_set_idx < varset_id ) continue ;

                    ctx->GSSetConstantBuffers( cb.slot, 1, cb.ptr ) ;
                }

                for ( auto & buf : rnd.var_sets_buffers_gs[ varset_id ].second )
                {
                    _arrays.access( buf.id, [&]( this_t::array_data_ref_t ad )
                    {
                        ctx->GSSetShaderResources( buf.slot, 1, ad.view ) ;
                    } ) ;
                }

                for ( auto & buf : rnd.var_sets_buffers_so_gs[ varset_id ].second )
                {
                    _streamouts.access( buf.id, [&]( this_t::so_data_ref_t sod )
                    {
                        auto & ppb = sod.read_buffer() ;
                        ctx->GSSetShaderResources( buf.slot, 1, ppb.views[ 0 ] ) ;
                    } ) ;
                }
            }

            // SECTION: tesselation shader variables
            {
            }

            // SECTION: pixel shader variables
            {
                for ( auto & cb : rnd._cbuffers_ps )
                {
                    if ( cb.var_set_idx > varset_id ) break  ;
                    if ( cb.var_set_idx < varset_id ) continue ;

                    ctx->PSSetConstantBuffers( cb.slot, 1, cb.ptr ) ;
                }

                for ( auto & img : rnd.var_sets_imgs_ps )
                {
                    if ( img.var_set_idx > varset_id ) break  ;
                    if ( img.var_set_idx < varset_id ) continue ;

                    _images.access( img.id, [&]( this_t::image_data_ref_t imgd )
                    {
                        ctx->PSSetShaderResources( img.slot, 1, imgd.view ) ;
                        ctx->PSSetSamplers( img.slot, 1, imgd.sampler ) ;
                    } ) ;
                }

                for ( auto & buf : rnd.var_sets_buffers_ps[ varset_id ].second )
                {
                    _arrays.access( buf.id, [&]( this_t::array_data_ref_t ad )
                    {
                        ctx->PSSetShaderResources( buf.slot, 1, ad.view ) ;
                    } ) ;
                }
            }

            // feed from streamout path
            if ( ( feed_from_so || use_streamout_count ) && rnd.so_ids.size() != 0 )
            {
                D3D11_QUERY_DATA_SO_STATISTICS so_stats = { 0 };

                if ( !feed_from_so )
                {
                    //this_t::so_data_ref_t so = _streamouts[ rnd.so_ids[ 0 ] ] ;
                    _streamouts.access( rnd.so_ids[ 0 ], [&] ( this_t::so_data_ref_t so )
                    {
                        size_t const ridx = so.read_index() ;
                        auto & rbuffer = so.read_buffer() ;

                        HRESULT hr = S_FALSE ;
                        while ( hr != S_OK )
                        {
                            hr = _ctx->ctx()->GetData( so.read_buffer().query, &so_stats, 
                                sizeof( D3D11_QUERY_DATA_SO_STATISTICS ), 0 ) ;
                        }
                        return true ;
                    } ) ;
                }

                // this path uses the streamout primitives as input data and the number of captured primitives using DrawAuto
                if ( feed_from_so )
                {
                    //this_t::so_data_ref_t so = _streamouts[ rnd.so_ids[ 0 ] ] ;
                    _streamouts.access( rnd.so_ids[ 0 ], [&] ( this_t::so_data_ref_t so )
                    {
                        auto & rbuffer = so.read_buffer() ;

                        UINT const stride = so.stride ;
                        UINT const offset = 0 ;
                        ctx->IASetVertexBuffers( 0, 1, rbuffer.buffers[ 0 ], &stride, &offset );
                        ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( so.pt ) ) ;
                        ctx->IASetInputLayout( rnd.vertex_layout_so ) ;

                        ctx->DrawAuto() ;
                        return true ;
                    } ) ;
                }

                // this path uses the geometry but should render just the number of captured primitives using DrawAuto
                else
                {
                    //this_t::geo_data_ref_t geo = _geos[ rnd.geo_ids[ geo_idx ] ] ;

                    _geos.access( rnd.geo_ids[geo_idx], [&]( this_t::geo_data_ref_t geo )
                    {
                        UINT const stride = geo.stride ;
                        UINT const offset = 0 ;
                        ctx->IASetVertexBuffers( 0, 1, geo.vb, &stride, &offset );
                        ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( geo.pt ) ) ;
                        ctx->IASetInputLayout( rnd.vertex_layout ) ;

                        // AutoDraw does not seem to work when binding a different vertex buffer
                        // then the used streamout buffer. If the other geometry uses index or instanced
                        // data, AutoDraw doesn't work anyways according to doc.
                        // @todo no index geo implemented, see remarks in geo path below.
                        ctx->Draw( UINT( so_stats.NumPrimitivesWritten ), UINT( start_element ) ) ;

                        return true ;
                    } ) ;
                }
            }

            // @todo
            // if use_streamout_count && !feed_from_so 
            // this path must be used with the queried number of prims written in so_stats
            else // feed from geometry path
            {
                //this_t::geo_data_ref_t geo = _geos[ rnd.geo_ids[ geo_idx ] ] ;
                _geos.access( rnd.geo_ids[ geo_idx ], [&]( this_t::geo_data_ref_t geo )
                {
                    if ( _cur_streamout_active != size_t( -1 ) )
                    {
                        _streamouts.access( _cur_streamout_active, [&] ( this_t::so_data_ref_t so )
                        {
                            so.pt = geo.pt ;
                        } ) ;
                    }

                    {
                        UINT const stride = geo.stride ;
                        UINT const offset = 0 ;
                        ctx->IASetVertexBuffers( 0, 1, geo.vb, &stride, &offset );
                        ctx->IASetPrimitiveTopology( motor::platform::d3d11::convert( geo.pt ) ) ;
                    }

                    ctx->IASetInputLayout( rnd.vertex_layout ) ;

                    if ( geo.num_elements_ib != 0 )
                    {
                        ctx->IASetIndexBuffer( geo.ib, DXGI_FORMAT_R32_UINT, 0 );

                        UINT const max_elems = num_elements == UINT( -1 ) ? UINT( geo.num_elements_ib ) : num_elements ;
                        ctx->DrawIndexed( max_elems, start_element, 0 ) ;
                    }
                    else
                    {
                        UINT const max_elems = num_elements == UINT( -1 ) ? UINT( geo.num_elements_vb ) : num_elements ;
                        ctx->Draw( max_elems, UINT( start_element ) ) ;
                    }
                } ) ;
            }

            // SECTION: UNBIND pixel shader variables
            {
                for ( auto & cb : rnd._cbuffers_ps )
                {
                    if ( cb.var_set_idx > varset_id ) break  ;
                    if ( cb.var_set_idx < varset_id ) continue ;

                    ID3D11Buffer * const null_buffer[ 1 ] = { nullptr };
                    ctx->PSSetConstantBuffers( cb.slot, 1, null_buffer ) ;
                }

                for ( auto & img : rnd.var_sets_imgs_ps )
                {
                    if ( img.var_set_idx > varset_id ) break  ;
                    if ( img.var_set_idx < varset_id ) continue ;

                    ID3D11ShaderResourceView * const null_view[ 1 ] = { nullptr };
                    ctx->PSSetShaderResources( img.slot, 1, null_view ) ;
                    //ctx->PSSetSamplers( img.slot, 1, images[ img.id ].sampler ) ;
                }

                for ( auto & buf : rnd.var_sets_buffers_ps[ varset_id ].second )
                {
                    ID3D11ShaderResourceView * const null_view[ 1 ] = { nullptr };
                    ctx->PSSetShaderResources( buf.slot, 1, null_view ) ;
                }
            }

            return true ;
        } ) ;
    }

    //************************************************************************************************************
    void_t begin_frame( void_t )
    {
        // set the viewport to the default new state, 
        // so the correct viewport is set automatically.
        {
            auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) ) ;

            //_states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp
            _states.access( ids_new.first, [&]( this_t::state_data_ref_t sd )
            {
                motor::math::vec4ui_t vp = sd.states[ ids_new.second ].view_s.ss.vp ;
                vp.z( uint_t( vp_width ) ) ;
                vp.w( uint_t( vp_height ) ) ;
                sd.states[ ids_new.second ].view_s.ss.vp = vp ;

                {
                    this_t::render_state_sets rss ;
                    rss.rss = sd.states[ ids_new.second ] ;
                    _state_stack.push( rss ) ;
                    this->handle_render_state( rss, false ) ;

                    // rss is filled in handle_render_state, so repush that one
                    _state_stack.pop() ;
                    _state_stack.push( rss  ) ;
                }
            } ) ;
            //_states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp = vp ;
            
            #if 0
            {
                this_t::render_state_sets rss ;
                rss.rss = _states[ ids_new.first ].states[ ids_new.second ] ;
                _state_stack.push( rss ) ;
                this->handle_render_state( rss, false ) ;

                // rss is filled in handle_render_state, so repush that one
                _state_stack.pop() ;
                _state_stack.push( rss  ) ;
            }
            #endif
        }

        {
            motor::math::vec4f_t const color = _state_stack.top().rss.clear_s.ss.clear_color ;

            _ctx->clear_render_target_view( color ) ;
            _ctx->clear_depth_stencil_view() ;
        }
    }

    //************************************************************************************************************
    void_t end_frame( void_t )
    {
        auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) ) ;

        assert( _state_stack.size() == 1 ) ;

        // simply pop the begin frame render states
        // and also release all state objects.
        auto rss = _state_stack.pop() ;
        this->handle_render_state( rss, true ) ;

        _ctsd->notify_thread() ;
    }

    //************************************************************************************************************
    motor::string_t remove_unwanded_characters( motor::string_cref_t code_in ) const noexcept
    {
        auto code = code_in ;

        // remove \r
        {
            size_t pos = 0 ;
            while( pos != code.size() )
            {
                if( code[ pos ] == '\r' ) code[ pos ] = ' ' ; ++pos ;
            }
        }

        // remove preceding, trailing ' '
        {
            size_t a = code.find_first_not_of( ' ' ) ;
            size_t b = code.find_last_not_of( ' ' ) ;
            a = a == std::string::npos ? 0 : a ;
            b = b == std::string::npos ? code.size() - a : b - a ;
            code = code.substr( a, b + 1 ) ;
        }

        return std::move( code ) ;
    }

    //************************************************************************************************************
    static bool_t determine_texture( ID3DBlob * blob,
        shader_data_t::image_variables_out_t img_vars, 
        shader_data_t::buffer_variables_out_t buf_vars ) noexcept
    {
        guard< ID3D11ShaderReflection >  reflector ;
        D3DReflect( blob->GetBufferPointer(), blob->GetBufferSize(),
            IID_ID3D11ShaderReflection, reflector ) ;

        D3D11_SHADER_DESC shd_desc ;
        {
            auto const hr = reflector->GetDesc( &shd_desc ) ;
            if( FAILED( hr ) )
            {
                motor::log::global_t::error( d3d11_backend_log( "Can not get shader reflection desc" ) ) ;
                return false ;
            }
        }

        UINT const num_irb = shd_desc.BoundResources ;
        for( UINT i=0; i<num_irb; ++i )
        {
            D3D11_SHADER_INPUT_BIND_DESC ibd ;
            {
                auto const hr = reflector->GetResourceBindingDesc( i, &ibd ) ;
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "GetResourceBindingDesc" ) ) ;
                    return false ;
                }
            }

            if( (ibd.Type != D3D_SIT_TEXTURE) && (ibd.Type != D3D_SIT_SAMPLER) ) continue ;

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
                shader_data_t::image_variable_t ivar ;
                ivar.name = ibd.Name ;
                ivar.slot = ibd.BindPoint ;
                img_vars.emplace_back( ivar ) ;
            }
            else if( ibd.Type == D3D_SIT_TEXTURE && ibd.Dimension == D3D_SRV_DIMENSION_TEXTURE2DARRAY )
            {
                shader_data_t::image_variable_t ivar ;
                ivar.name = ibd.Name ;
                ivar.slot = ibd.BindPoint ;
                img_vars.emplace_back( ivar ) ;
            }
            else if( ibd.Type == D3D_SIT_TEXTURE && ibd.Dimension == D3D_SRV_DIMENSION_BUFFER )
            {
                shader_data_t::buffer_variable_t ivar ;
                ivar.name = ibd.Name ;
                ivar.slot = ibd.BindPoint ;
                buf_vars.emplace_back( ivar ) ;
            }
            else
            {
                motor::log::global_t::warning( d3d11_backend_log("Detected texture but type is not supported.") ) ;
            }
        }

        return true ;
    }

    //************************************************************************************************************
    // performs reflection on the constant buffers present in
    // a shader for user variable binding
    static shader_data_t::cbuffers_t determine_cbuffer( ID3DBlob * blob ) noexcept
    {
        shader_data_t::cbuffers_t ret ;

        ID3D11ShaderReflection * reflector = nullptr ;
        D3DReflect( blob->GetBufferPointer(), blob->GetBufferSize(),
            IID_ID3D11ShaderReflection, ( void** ) &reflector ) ;

        D3D11_SHADER_DESC shd_desc ;
        {
            auto const hr = reflector->GetDesc( &shd_desc ) ;
            if( FAILED( hr ) )
            {
                motor::log::global_t::error( d3d11_backend_log( "Can not get shader reflection desc" ) ) ;
                return shader_data_t::cbuffers_t() ;
            }
        }
        
        UINT const num_cb = shd_desc.ConstantBuffers ;
        for( UINT i = 0; i < num_cb; ++i )
        {
            shader_data_t::cbuffer cbuffer ;

            ID3D11ShaderReflectionConstantBuffer* cbr = reflector->GetConstantBufferByIndex( i ) ;

            D3D11_SHADER_BUFFER_DESC buf_desc ;
            {
                auto hr = cbr->GetDesc( &buf_desc ) ;
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_SHADER_BUFFER_DESC" ) ) ;
                    continue ;
                }
            }
            cbuffer.name = motor::string_t( buf_desc.Name ) ;
            cbuffer.sib = size_t( buf_desc.Size ) ;

            // figure out the register the resource is bound to
            // this is done through cbuffer xyz : register(bn)
            // where n needs to be determined
            {
                D3D11_SHADER_INPUT_BIND_DESC res_desc ;
                auto const hr = reflector->GetResourceBindingDescByName( buf_desc.Name, &res_desc ) ;
                if( FAILED( hr ) )
                {
                    motor::log::global_t::error( d3d11_backend_log( "D3D11_SHADER_INPUT_BIND_DESC" ) ) ;
                    continue ;
                }
                cbuffer.slot = res_desc.BindPoint ;
            }

            for( UINT j = 0; j < buf_desc.Variables; ++j )
            {
                ID3D11ShaderReflectionVariable* var = cbr->GetVariableByIndex( j ) ;

                D3D11_SHADER_VARIABLE_DESC var_desc ;
                {
                    auto const hr = var->GetDesc( &var_desc ) ;
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log(
                            "D3D11_SHADER_VARIABLE_DESC" ) ) ;
                        continue ;
                    }
                }

                ID3D11ShaderReflectionType* type = var->GetType() ;
                D3D11_SHADER_TYPE_DESC type_desc ;
                {
                    auto const hr = type->GetDesc( &type_desc ) ;
                    if( FAILED( hr ) )
                    {
                        motor::log::global_t::error( d3d11_backend_log(
                            "D3D11_SHADER_TYPE_DESC" ) ) ;
                        continue ;
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
                    UINT const elems = std::max( type_desc.Rows, type_desc.Columns ) ;
                    auto const ts = motor::platform::d3d11::to_type_struct( type_desc.Class, elems ) ;
                    auto const t = motor::platform::d3d11::to_type( type_desc.Type ) ;
                    if( ts == motor::graphics::type_struct::undefined ||
                        t == motor::graphics::type::undefined ) continue ;

                    shader_data_t::data_variable_t dv ;
                    dv.name = motor::string_t( var_desc.Name ) ;
                    dv.offset = var_desc.StartOffset ;
                    dv.t = t ;
                    dv.ts = ts ;

                    cbuffer.datas.emplace_back( dv ) ;
                }
            }

            ret.emplace_back( std::move( cbuffer ) ) ;
        }
        return std::move( ret ) ;
    }
} ;

//************************************************************************************************
//
//
//************************************************************************************************

//************************************************************************************************************
d3d11_backend::d3d11_backend( motor::platform::d3d11::rendering_context_ptr_t ctx ) noexcept 
{
    _pimpl = motor::memory::global_t::alloc( this_t::pimpl( this_t::get_bid(), ctx ), "d3d11_backend::pimpl" ) ;
    _context = ctx ;
}

//************************************************************************************************************
d3d11_backend::d3d11_backend( this_rref_t rhv ) noexcept : backend( std::move( rhv ) )
{
    _pimpl = motor::move( rhv._pimpl ) ;
    _context = motor::move( rhv._context ) ;
}

//************************************************************************************************************
d3d11_backend::~d3d11_backend( void_t ) 
{
    this_t::report_live_device_objects( D3D11_RLDO_SUMMARY ) ;
    motor::memory::global_t::dealloc( _pimpl ) ;
}

//************************************************************************************************************
void_t d3d11_backend::set_window_info( window_info_cref_t wi ) noexcept 
{
    {
        //bool_t change = false ;
        if( wi.width != 0 )
        {
            _pimpl->vp_width = FLOAT( wi.width ) ;
            
        }
        if( wi.height != 0 )
        {
            _pimpl->vp_height = FLOAT( wi.height ) ;
            
        }
    }
}

//******************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::msl_object_mtr_t obj ) noexcept
{
    if( obj == nullptr )
    {
        d3d11_log_errorv( "msl object is nullptr" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    auto const oid = obj->get_oid( this_t::get_bid() ) ;

    // is it in transit
    if( oid == size_t(-2) )
    {
        return motor::graphics::result::in_transit ; 
    }
    
    _pimpl->send_configure_to_st( obj ) ;
    return motor::graphics::result::in_transit ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::geometry_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "geometry object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    
    size_t const oid = obj->set_oid( this_t::get_bid(), 
        _pimpl->construct_geo( obj->get_oid( this_t::get_bid() ), *obj ) ) ;
    
    
    {
        auto const res = _pimpl->update( oid, obj ) ;
        if( !res )
        {
            return motor::graphics::result::failed ;
        }
    }

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::render_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "render object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
    
    if( _pimpl->construct_render_config( *obj ) )
    {
    }

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "shader object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
        
    {
        _pimpl->construct_shader_config( *obj ) ;
    }

    #if 0
    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *config ) ;
        if( !res )
        {
            return motor::graphics::result::failed ;
        }
    }
    #endif
    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "msl object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
    


    auto const res = _pimpl->construct_image_config( *obj ) ;
    return res ? motor::graphics::result::ok : motor::graphics::result::failed ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::framebuffer_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "framebuffer object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
    
    auto const res = _pimpl->construct_framebuffer( *obj ) ;
    
    return res ? motor::graphics::result::ok : motor::graphics::result::failed ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "state object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    auto const res = _pimpl->construct_state( *obj ) ;

    return res ? motor::graphics::result::ok : motor::graphics::result::failed ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::array_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "array object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
    
    size_t const oid = obj->set_oid( this_t::get_bid(), 
        _pimpl->construct_array_data( obj->get_oid( this_t::get_bid() ), *obj ) ) ;

    {
        auto const res = _pimpl->update( oid, *obj, true ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::configure( motor::graphics::streamout_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "streamout object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
    
    size_t const oid = obj->set_oid( this_t::get_bid(), 
        _pimpl->construct_streamout( obj->get_oid( this_t::get_bid() ), *obj ) ) ;

    {
        auto const res = _pimpl->update( oid, *obj, true ) ;
        if( !res ) return motor::graphics::result::failed ;
    }
    

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::msl_object_mtr_t ) noexcept 
{
    return motor::graphics::result::failed ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::geometry_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "geometry object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    _pimpl->release_geometry( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::render_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "render object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
    
    _pimpl->release_render_data( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "shader object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    _pimpl->release_shader_data( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "image object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    _pimpl->release_image_data( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::framebuffer_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "framebuffer object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    _pimpl->release_framebuffer( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "state object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    _pimpl->release_state( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::array_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "array object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    _pimpl->release_array_data( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::release( motor::graphics::streamout_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        d3d11_log_errorv( "streamout object is nullptr or has no name" ) ;
        return motor::graphics::result::invalid_argument ;
    }

    _pimpl->release_streamout( obj->get_oid( this_t::get_bid() ) ) ;
    obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;

    return motor::graphics::result::ok ;
}

#if 0
//************************************************************************************************************
motor::graphics::result d3d11_backend::connect( motor::graphics::render_object_mtr_t config, motor::graphics::variable_set_mtr_t vs ) noexcept
{
    #if 0
    if( id->is_not_valid( this_t::get_bid() ) )
    {
        motor::log::global_t::error( d3d11_backend_log( "invalid render configuration id" ) ) ;
        return motor::graphics::result::failed ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    auto const res = _pimpl->connect( oid, vs ) ;
    motor::log::global_t::error( motor::graphics::is_not( res ), 
        d3d11_backend_log( "connect variable set" ) ) ;
   
    #endif

    return motor::graphics::result::ok ;
}
#endif  
//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::geometry_object_mtr_t obj ) noexcept 
{    
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        motor::log::global_t::error( d3d11_backend_log( "invalid geometry object id" ) ) ;
        return motor::graphics::result::failed ;
    }

    auto const res = _pimpl->update( oid, obj ) ;
    motor::log::global_t::error( !res, d3d11_backend_log( "update geometry" ) ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::array_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, false ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::streamout_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, false ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( !_pimpl->update( oid, *obj, false ) )
    {
        return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::update( motor::graphics::render_object_mtr_t obj, size_t const varset ) noexcept 
{
    if( obj == nullptr ) return motor::graphics::result::invalid_argument ;

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        d3d11_log_errorv( "[update|render_object] : invalid id for %s", obj->name().c_str() ) ;
        return motor::graphics::result::failed ;
    }

    _pimpl->update( oid, *obj, varset ) ;
    
    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::use( motor::graphics::framebuffer_object_mtr_t obj ) noexcept
{
    if( obj == nullptr )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::framebuffer ) ;
    }

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::framebuffer ) ;
    }

    auto const res = _pimpl->activate_framebuffer( oid ) ;
    if( !res ) return motor::graphics::result::failed ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::use( motor::graphics::streamout_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::streamout ) ;
    }

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        return this_t::unuse( motor::graphics::gen4::backend::unuse_type::streamout ) ;
    }

    auto const res = _pimpl->activate_streamout( oid ) ;
    if( !res ) return motor::graphics::result::failed ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::unuse( motor::graphics::gen4::backend::unuse_type const t ) noexcept 
{
    switch( t )
    {
    case motor::graphics::gen4::backend::unuse_type::framebuffer: 
        _pimpl->deactivate_framebuffer() ; break ;
    case motor::graphics::gen4::backend::unuse_type::streamout: 
        _pimpl->deactivate_streamout() ;break ;
    }
    
    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::push( motor::graphics::state_object_mtr_t obj, size_t const sid, bool_t const ) noexcept 
{
    if( obj == nullptr )
    {
        return this_t::pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
    }

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        return this_t::pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
    }

    _pimpl->update_state( oid, *obj ) ;
    _pimpl->handle_render_state( oid, sid ) ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::pop( motor::graphics::gen4::backend::pop_type const ) noexcept 
{
    _pimpl->handle_render_state( size_t( -1 ), size_t( -1 ) ) ;
    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::render( motor::graphics::render_object_mtr_t obj, motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept 
{     
    if( obj == nullptr ) return motor::graphics::result::invalid_argument ;

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        d3d11_log_errorv( "[render|render_object] : invalid id for %s", obj->name().c_str() ) ;
        return motor::graphics::result::failed ;
    }

    // update variables
    {
        _pimpl->update( oid, *obj, detail.varset ) ;
    }

    // render
    {
        _pimpl->render( oid, detail.geo, detail.feed_from_streamout, detail.use_streamout_count,
        detail.varset, (UINT)detail.start, (UINT)detail.num_elems ) ;
    }
    
    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result d3d11_backend::render( motor::graphics::msl_object_mtr_t obj, motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept 
{
    if( obj == nullptr ) return motor::graphics::result::invalid_argument ;

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        d3d11_log_errorv( "[render|render_object] : invalid id for %s", obj->name().c_str() ) ;
        return motor::graphics::result::failed ;
    }

    if ( oid == size_t( -2  ) )
    {
        return motor::graphics::result::in_transit ;
    }

    motor::graphics::result res = motor::graphics::result::failed ;

    _pimpl->_msls.try_access( oid, [&]( pimpl::msl_data_ref_t msl )
    {
        // @note if ros == 0, the shader probably did not compile.
        // check the console! This happens only for the initial compilation
        // need fix.
        motor::graphics::render_object_mtr_t ro = &msl.ros[detail.ro_idx] ;
        res = this_t::render( ro, detail ) ;
    } ) ;
    return res ;

}

//************************************************************************************************************
void_t d3d11_backend::render_begin( void_t ) noexcept 
{
    _pimpl->begin_frame() ;
}

//************************************************************************************************************
void_t d3d11_backend::render_end( void_t ) noexcept 
{
    _pimpl->end_frame() ;
}

//************************************************************************************************************
void_t d3d11_backend::report_live_device_objects( D3D11_RLDO_FLAGS const flags ) noexcept 
{
    #ifdef D3D_DEBUG
    if( _context != nullptr ) _context->debug()->ReportLiveDeviceObjects( flags ) ;
    #endif
}