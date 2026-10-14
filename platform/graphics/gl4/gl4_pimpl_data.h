// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "gl4_log.h"


#include <motor/msl/symbol.hpp>
#include <motor/msl/database.hpp>
#include <motor/msl/parser.h>
#include <motor/msl/dependency_resolver.hpp>
#include <motor/msl/generators/generator.h>
#include <motor/msl/generators/glsl4_generator.h>

#include <motor/graphics/buffer/vertex_buffer.hpp>
#include <motor/graphics/buffer/index_buffer.hpp>
#include <motor/graphics/shader/msl_bridge.hpp>

#include <motor/profiling/probe_guard.hpp>
#include <motor/profiling/scoped_timer.hpp>

struct gl4_detail
{
    static motor::graphics::gen4::backend_type const bt = motor::graphics::gen4::backend_type::gl4;
    static motor::graphics::shader_api_type const sapi = motor::graphics::shader_api_type::glsl_4_0;
};

namespace pimpl_data
{
using namespace motor::core::types;
using namespace motor::ogl;

//********************************************************************************
// transform feedback buffers
struct feedback_data
{
    motor_this_typedefs( feedback_data );

    struct buffer
    {
        static constexpr size_t max_buffers = 4;

        size_t gids[ max_buffers ] = { size_t( -1 ), size_t( -1 ), size_t( -1 ), size_t( -1 ) };

        // buffer ids
        GLuint bids[ max_buffers ] = { GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ) };
        GLuint sibs[ max_buffers ] = { 0, 0, 0, 0 };

        // texture ids for TBOs
        GLuint tids[ max_buffers ] = { GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ) };

        // query object id
        GLuint qid = GLuint( -1 );

        // transform feedback object ids
        GLuint tfid = GLuint( -1 );
    };
    buffer _buffers[ 2 ];

    // if the tf is read and written at the
    // same time, this index is used for ping-ponging
    // the id array within a buffer.
    size_t _ridx = 0;

    // must be derived from the geometry object
    // that is rendered into the feedback buffer.
    // this could be used to overwrite whats set in the
    // geometry data of the transform feedback buffers.
    // GLenum pt = 0 ;

    // for tf reconfig - render data ids
    motor::vector< size_t > rd_ids;
    void_t remove_render_data_id( size_t const rid ) noexcept
    {
        if( rid == size_t( -1 ) ) return;
        for( auto & id : rd_ids )
            if( id == rid )
            {
                id = size_t( -1 );
                break;
            }
    }

    void_t add_render_data_id( size_t const rid ) noexcept
    {
        if( rid == size_t( -1 ) ) return;
        for( auto & id : rd_ids )
            if( id == size_t( -1 ) )
            {
                id = rid;
                return;
            }
        rd_ids.push_back( rid );
    }

    void_t swap_index( void_t ) noexcept
    {
        _ridx = ( _ridx + 1 ) & 1;
    }

    size_t write_index( void_t ) const noexcept
    {
        return ( _ridx + 1 ) & 1;
    }

    size_t read_index( void_t ) const noexcept
    {
        return _ridx;
    }

    buffer & read_buffer( void_t ) noexcept
    {
        return _buffers[ this_t::read_index() ];
    }
    buffer const & read_buffer( void_t ) const noexcept
    {
        return _buffers[ this_t::read_index() ];
    }
    buffer & write_buffer( void_t ) noexcept
    {
        return _buffers[ this_t::write_index() ];
    }
    buffer const & write_buffer( void_t ) const noexcept
    {
        return _buffers[ this_t::write_index() ];
    }

    void_t invalidate( std::string_view const & name ) noexcept
    {
        for( size_t rw = 0; rw < 2; ++rw )
        {
            auto & buffer = _buffers[ rw ];

            if( buffer.qid != GLuint( -1 ) )
            {
                glDeleteQueries( 1, &buffer.qid );
                gl4_log_error2( "glDeleteQueries", name.data() );
                buffer.qid = GLuint( -1 );
            }

            if( buffer.tfid != GLuint( -1 ) )
            {
                glDeleteTransformFeedbacks( 1, &buffer.tfid );
                gl4_log_error2( "glDeleteTransformFeedbacks", name.data() );
                buffer.tfid = GLuint( -1 );
            }

            // release textures
            if( buffer.tids[ 0 ] != GLuint( -1 ) )
            {
                glDeleteTextures( feedback_data::buffer::max_buffers, buffer.tids );
                gl4_log_error2( "glDeleteTextures", name.data() );
            }
        }
    }
};
motor_typedef( feedback_data );

//********************************************************************************
struct geo_data
{
    GLuint vb_id = GLuint( -1 );
    GLuint ib_id = GLuint( -1 );

    size_t num_elements_vb = 0;
    size_t num_elements_ib = 0;

    size_t sib_vb = 0;
    size_t sib_ib = 0;

    struct layout_element
    {
        motor::graphics::vertex_attribute va;
        motor::graphics::type type;
        motor::graphics::type_struct type_struct;

        GLuint sib( void_t ) const noexcept
        {
            return GLuint(
                motor::graphics::size_of( type ) * motor::graphics::size_of( type_struct ) );
        }
    };
    motor::vector< layout_element > elements;

    // per vertex sib
    GLuint stride = 0;

    GLenum ib_type;
    size_t ib_elem_sib = 0;
    GLenum pt;

    // for geo reconfig - render data ids
    motor::vector< size_t > rd_ids;
    void_t remove_render_data_id( size_t const rid ) noexcept
    {
        if( rid == size_t( -1 ) ) return;
        for( auto & id : rd_ids )
            if( id == rid )
            {
                id = size_t( -1 );
                break;
            }
    }

    void_t add_render_data_id( size_t const rid ) noexcept
    {
        if( rid == size_t( -1 ) ) return;
        for( auto & id : rd_ids )
            if( id == size_t( -1 ) )
            {
                id = rid;
                return;
            }
        rd_ids.push_back( rid );
    }

    void_t invalidate( std::string_view const & /*name*/ ) noexcept
    {
        elements.clear();
        ib_elem_sib = 0;
        stride = 0;

        if( vb_id != GLuint( -1 ) )
        {
            glDeleteBuffers( 1, &vb_id );
            gl4_log_error( "glDeleteBuffers : vertex buffer" );
            vb_id = GLuint( -1 );
        }

        if( ib_id != GLuint( -1 ) )
        {
            glDeleteBuffers( 1, &ib_id );
            gl4_log_error( "glDeleteBuffers : index buffer" );
            ib_id = GLuint( -1 );
        }

        ib_elem_sib = 0;
        rd_ids.clear();
    }
};
motor_typedef( geo_data );

//********************************************************************************
struct shader_data
{
    motor_this_typedefs( shader_data );

    struct vertex_input_binding
    {
        motor::graphics::vertex_attribute va;
        motor::string_t name;
    };

    struct vertex_output_binding
    {
        motor::graphics::vertex_attribute va;
        motor::string_t name;
    };

    bool_t find_vertex_input_binding_by_name(
        motor::string_cref_t name_, motor::graphics::vertex_attribute & va ) const noexcept
    {
        auto iter = std::find_if( vertex_inputs.begin(), vertex_inputs.end(),
            [ & ]( vertex_input_binding const & b ) { return b.name == name_; } );
        if( iter == vertex_inputs.end() ) return false;

        va = iter->va;

        return true;
    }

    bool_t find_vertex_output_binding_by_name(
        motor::string_cref_t name_, motor::graphics::vertex_attribute & va ) const noexcept
    {
        size_t i = size_t(-1) ;
        while( ++i < vertex_outputs.size() && vertex_outputs[i].name != name_ ) ;

        if( i == vertex_outputs.size() ) return false ;

        va = vertex_outputs[i].va ;

        return true;
    }

    struct attribute_variable
    {
        motor::graphics::vertex_attribute va;
        motor::string_t name;
        GLuint loc;
        GLenum type;
    };
    motor_typedef( attribute_variable );

    struct uniform_variable
    {
        motor::string_t name;
        GLuint loc;
        GLenum type;

        // the GL uniform function
        motor::ogl::uniform_funk_t uniform_funk;

        // @return false if failed to set uniform
        bool_t do_uniform_funk( void_ptr_t mem_ ) noexcept
        {
            uniform_funk( loc, 1, mem_ );
            return !motor::ogl::error::check_and_log( gl4_log( "glUniform" ) );
        }

        void_t do_copy_funk( void_ptr_t mem_, motor::graphics::ivariable_ptr_t var ) noexcept
        {
            std::memcpy( mem_, var->data_ptr(), motor::ogl::uniform_size_of( type ) );
        }
    };
    motor_typedef( uniform_variable );

  public: // variables    

    GLuint vs_id = GLuint( -1 );
    GLuint gs_id = GLuint( -1 );
    GLuint ps_id = GLuint( -1 );
    GLuint pg_id = GLuint( -1 );

    bool_t is_linkage_ok = false;

    char const ** output_names = nullptr;

    motor::vector< vertex_input_binding > vertex_inputs;
    motor::vector< vertex_output_binding > vertex_outputs;
    motor::vector< attribute_variable_t > attributes;
    motor::vector< uniform_variable > uniforms;

    // the default shader variable values.
    motor::graphics::variable_set_t default_values;

  public:

    shader_data( void_t ) noexcept = default;
    shader_data( this_cref_t ) noexcept = delete;
    shader_data( this_rref_t rhv ) noexcept
        : vs_id( rhv.vs_id ), gs_id( rhv.gs_id ), ps_id( rhv.ps_id ), pg_id( rhv.pg_id ),
          is_linkage_ok( rhv.is_linkage_ok ), default_values( std::move( rhv.default_values ) ),
          vertex_inputs( std::move( rhv.vertex_inputs ) ), vertex_outputs( rhv.vertex_outputs ),
          output_names( motor::move( rhv.output_names ) ),
          attributes( std::move( rhv.attributes ) ), uniforms( std::move( rhv.uniforms ) )
    {
        rhv.vs_id = GLuint( -1 );
        rhv.gs_id = GLuint( -1 );
        rhv.ps_id = GLuint( -1 );
        rhv.pg_id = GLuint( -1 );
    }

    ~shader_data( void_t ) noexcept
    {
        this_t::invalidate();
    }

    //*******************************************************
    // detach shaders from program
    void_t detach_shaders( void_t ) noexcept
    {
        if( pg_id == GLuint( -1 ) ) return;

        GLsizei count = 0;
        GLuint shaders_[ 10 ];

        glGetAttachedShaders( pg_id, 10, &count, shaders_ );
        gl4_log_error( "glGetAttachedShaders" );

        for( GLsizei i = 0; i < count; ++i )
        {
            glDetachShader( pg_id, shaders_[ i ] );
            gl4_log_error( "glDetachShader" );
        }
    }

    //*******************************************************
    void_t delete_shaders( void_t ) noexcept
    {
        if( vs_id != GLuint( -1 ) )
        {
            glDeleteShader( vs_id );
            gl4_log_error( "glDeleteShader : vertex shader" );
            vs_id = GLuint( -1 );
        }

        if( gs_id != GLuint( -1 ) )
        {
            glDeleteShader( gs_id );
            gl4_log_error( "glDeleteShader : geometry shader" );
            gs_id = GLuint( -1 );
        }

        if( ps_id != GLuint( -1 ) )
        {
            glDeleteShader( ps_id );
            gl4_log_error( "glDeleteShader : pixel shader" );
            ps_id = GLuint( -1 );
        }
    }

    void_t delete_program( void_t ) noexcept
    {
        if( pg_id != GLuint( -1 ) )
        {
            glDeleteProgram( pg_id );
            gl4_log_error( "glDeleteProgram" );
            pg_id = GLuint( -1 );
        }
    }

  public:

    //*******************************************************
    void_t invalidate( std::string_view const & = "" ) noexcept
    {
        is_linkage_ok = false;

        detach_shaders();
        delete_shaders();
        delete_program();

        if( output_names != nullptr )
        {
            motor::memory::global_t::dealloc_raw( output_names );
        }

        uniforms.clear();
        attributes.clear();

        default_values.clear();

        output_names = nullptr ;
    }

    this_ref_t operator=( this_cref_t rhv ) = delete;
    this_ref_t operator=( this_rref_t rhv ) noexcept
    {
        this_t::invalidate();

        // the default shader variable values.
        default_values = std::move( rhv.default_values );

        vs_id = rhv.vs_id;
        rhv.vs_id = GLuint( -1 );

        gs_id = rhv.gs_id;
        rhv.gs_id = GLuint( -1 );

        ps_id = rhv.ps_id;
        rhv.ps_id = GLuint( -1 );

        pg_id = rhv.pg_id;
        rhv.pg_id = GLuint( -1 );

        is_linkage_ok = rhv.is_linkage_ok;
        rhv.is_linkage_ok = false;

        vertex_inputs = std::move( rhv.vertex_inputs );
        vertex_outputs = std::move( rhv.vertex_outputs );
        output_names = motor::move( rhv.output_names );
        attributes = std::move( rhv.attributes );
        uniforms = std::move( rhv.uniforms );

        return *this;
    }
};
motor_typedef( shader_data );

//********************************************************************************
struct state_data
{
    motor::vector< motor::graphics::render_state_sets_t > states;

    void_t invalidate( std::string_view const & ) {}
};
motor_typedef( state_data );

//********************************************************************************
struct render_data
{
    struct geo_id_data
    {
        size_t id;
        size_t hash;
    };
    motor::vector< geo_id_data > geo_ids;

    motor::vector< size_t > tf_ids; // feed from for geometry
    size_t shd_id = size_t( -1 );

    struct geo_to_vao
    {
        size_t gid;
        GLuint vao;
    };
    motor::vector< geo_to_vao > geo_to_vaos;

    struct variable_set
    {
        size_t hash;

        // the variable set index into the render objects
        // variable set list.
        // size_t vs_idx ;

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

    // index into var_sets. This is 1:1 to the render_objects'
    // variable set list.
    // an entry of value size_t(-1)
    struct varset_to_idx_data
    {
        size_t idx = size_t( -1 );

        // data uniform variables into
        // var_sets_data
        size_t data_idx_begin = size_t( -1 );
        // points one after the last entry.
        size_t data_idx_end = size_t( -1 );

        // texture uniform variables into
        // var_sets_texture
        size_t texture_idx_begin = size_t( -1 );
        // points one after the last entry.
        size_t texture_idx_end = size_t( -1 );

        // array uniform variables into
        // var_sets_array
        size_t array_idx_begin = size_t( -1 );
        // points one after the last entry.
        size_t array_idx_end = size_t( -1 );

        // streamout uniform variables into
        // var_sets_streamout
        size_t streamout_idx_begin = size_t( -1 );
        // points one after the last entry.
        size_t streamout_idx_end = size_t( -1 );
    };
    motor::vector< varset_to_idx_data > varset_to_idx;

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

    struct uniform_variable_link
    {
        // the user variable holding the data.
        motor::graphics::ivariable_ptr_t var;

        // the index into the shader_config::uniforms array
        size_t uniform_id;

        // pointing into the mem_block
        void_ptr_t mem = nullptr;
    };
    motor::vector< uniform_variable_link > var_sets_data;

    struct uniform_texture_link
    {
        // the index into the shader_config::uniforms array
        size_t uniform_id;
        GLint tex_id;
        size_t img_id;
        // the hash value of the images' name
        // so we can change the image linked
        // at run time.
        size_t img_hash;

        // the user variable holding the data.
        motor::graphics::texture_variable_t * var;

        // pointing into the mem_block
        void_ptr_t mem = nullptr;
    };
    motor::vector< uniform_texture_link > var_sets_texture;

    struct uniform_array_data_link
    {
        // the index into the shader_config::uniforms array
        size_t uniform_id;
        GLint tex_id;
        size_t buf_id;

        // pointing into the mem_block
        void_ptr_t mem = nullptr;
    };
    motor::vector< uniform_array_data_link > var_sets_array;

    struct uniform_streamout_link
    {
        // the index into the shader_config::uniforms array
        size_t uniform_id;
        GLint tex_id[ 2 ]; // streamout object do double buffer
        size_t so_id;      // link into _feedbacks

        // pointing into the mem_block
        void_ptr_t mem = nullptr;
    };
    motor::vector< uniform_streamout_link > var_sets_streamout;

    // memory block for all variables in all variable sets.
    void_ptr_t mem_block = nullptr;

    motor::vector< motor::graphics::render_state_sets > rss;

    void_t invalidate_geometry_id( size_t const idx, size_t const hash ) noexcept
    {
        if( idx >= geo_ids.size() ) return;
        geo_ids[ idx ].id = size_t( -1 );
        geo_ids[ idx ].hash = hash;
    }

    void_t remove_geometry_id( size_t const id ) noexcept
    {
        size_t i = size_t( -1 );
        while( ++i < geo_ids.size() && geo_ids[ i ].id != id );
        if( i != size_t( -1 ) )
        {
            geo_ids.erase( geo_ids.begin() + i );
        }
    }

    void_t invalidate( std::string_view const & name ) noexcept
    {
        // need to clear geometry connections before!
        // I am trying to remove those connections.
        geo_ids.clear();

        // need to clear transform fb connections before!
        // I am trying to remove those connections.
        tf_ids.clear();

        shd_id = GLuint( -1 );
        rss.clear();

        invalidate_variables();

        for( auto & d : geo_to_vaos )
        {
            glDeleteVertexArrays( 1, &d.vao );
            gl4_log_error2( "glDeleteVertexArrays", name.data() );
        }

        geo_to_vaos.clear();
    }

    void_t invalidate_variables( void_t ) noexcept
    {
        for( auto & v : var_sets ) motor::release( motor::move( v.vs ) );
        varset_to_idx.clear();

        // remember that rd.var_sets hold the ref count reminder!
        var_sets.clear();
        var_sets_array.clear();
        var_sets_streamout.clear();
        var_sets_data.clear();
        var_sets_texture.clear();

        motor::memory::global_t::dealloc( mem_block );
        mem_block = nullptr;
    }
};
motor_typedef( render_data );

//********************************************************************************
struct image_data
{
    GLenum type = GL_NONE;

    GLuint tex_id = GLuint( -1 );
    size_t sib = 0;

    GLenum wrap_types[ 3 ];
    GLenum filter_types[ 2 ];

    // sampler ids for gl>=3.3

    void_t invalidate( std::string_view const & name ) noexcept
    {
        type = GL_NONE;

        if( tex_id != GLuint( -1 ) )
        {
            glDeleteTextures( 1, &tex_id );
            gl4_log_error2( "glDeleteTextures", name.data() );
            tex_id = GLuint( -1 );
        }

        sib = 0;
    }
};
motor_typedef( image_data );

//********************************************************************************
struct array_data
{
    GLuint tex_id = GLuint( -1 );
    GLuint buf_id = GLuint( -1 );

    GLuint sib = 0;

    void_t invalidate( std::string_view const & name ) noexcept
    {
        if( buf_id != GLuint( -1 ) )
        {
            glDeleteBuffers( 1, &buf_id );
            gl4_log_error2( "glDeleteBuffers", name.data() );
            buf_id = GLuint( -1 );
        }

        if( tex_id != GLuint( -1 ) )
        {
            glDeleteTextures( 1, &tex_id );
            gl4_log_error2( "glDeleteTextures", name.data() );
            tex_id = GLuint( -1 );
        }

        sib = 0;
    }
};
motor_typedef( array_data );

//********************************************************************************
struct framebuffer_data
{
    GLuint gl_id = GLuint( -1 );

    size_t nt = 0;
    GLuint colors[ 8 ] = { GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ),
        GLuint( -1 ), GLuint( -1 ), GLuint( -1 ) };

    GLuint depth = GLuint( -1 );

    motor::math::vec2ui_t dims;

    // reset all gl ids. This is useful
    // if the context is already gone.
    void_t reset_gl_ids( void_t )
    {
        gl_id = GLuint( -1 );
        std::memset( colors, GLuint( -1 ), sizeof( GLuint ) * 8 );
        depth = GLuint( -1 );
    }

    void_t invalidate( std::string_view const & /*name*/ ) noexcept
    {
        if( gl_id != GLuint( -1 ) )
        {
            glDeleteFramebuffers( 1, &gl_id );
            gl4_log_error( "glDeleteFramebuffers" );

            {
                glDeleteTextures( 8, colors );
                gl4_log_error( "glDeleteTextures : colors" );
            }
        }

        if( depth != GLuint( -1 ) )
        {
            glDeleteTextures( 1, &depth );
            gl4_log_error( "glDeleteTextures : depth" );
        }

        gl_id = GLuint( -1 );
        depth = GLuint( -1 );
        for( size_t i = 0; i < 8; ++i ) colors[ i ] = GLuint( -1 );

        dims = motor::math::vec2ui_t();
    }
};
motor_typedef( framebuffer_data );

//********************************************************************************
struct msl_data
{
    size_t ro_id;
    motor::graphics::shader_object_t so;

    void_t invalidate( std::string_view const & /*name*/ ) noexcept
    {
        ro_id = size_t( -1 );
    }
};
motor_typedef( msl_data );
}; // namespace pimpl_data
