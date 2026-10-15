
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "gl4.h"
#include "gl4_convert.h"
#include "gl4_pimpl_data.h"

#include <motor/ogl/gl/gl.h>
#include <motor/ogl/gl/convert.hpp>
#include <motor/ogl/gl/error.hpp>
#include <motor/memory/global.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/std/vector>
#include <motor/std/stack>
#include <motor/std/string_split.hpp>
#include <motor/std/hash_map>

#include <variant>

using namespace motor::platform::gen4;
using namespace motor::ogl;

struct gl4_independent
{
    struct compilation_result
    {
        bool_t success = false;

        // safe reference. hold ref count
        motor::graphics::msl_object_mtr_t object = nullptr;

        motor::graphics::shader_object_t shader;
        gl4_pimpl_data::shader_data_t data;
    };
    struct release_result
    {
    };
    using backward_message = std::variant< compilation_result, release_result >;

    motor_this_typedefs( gl4_independent );

    //****************************************************************************************
    static bool_t link( GLuint const program_id ) noexcept
    {
        glLinkProgram( program_id );
        if( motor::ogl::error::check_and_log( gl4_log( "glLinkProgram" ) ) ) return false;

        {
            GLint ret;
            glGetProgramiv( program_id, GL_LINK_STATUS, &ret );

            GLint length;
            glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &length );

            if( ret == GL_TRUE && length <= 1 ) return true;

            if( motor::log::global_t::error( length == 0, gl4_log( "unknown" ) ) ) return false;

            motor::memory::malloc_guard< char > info_log( length );

            glGetProgramInfoLog( program_id, length, 0, info_log );
            if( motor::ogl::error::check_and_log( gl4_log( "glGetProgramInfoLog" ) ) ) return false;

            std::string info_log_string = std::string( (const char *)info_log );

            {
                motor::vector< motor::string_t > tokens;
                motor::mstd::string_ops::split( motor::string_t( info_log ), '\n', tokens );

                for( auto token : tokens )
                {
                    motor::log::global_t::error( token );
                }
            }
        }
        return false;
    }

    //****************************************************************************************
    static void_t post_link_attributes( gl4_pimpl_data::shader_data & config ) noexcept
    {
        GLuint const program_id = config.pg_id;

        GLint num_active_attributes = 0;
        GLint name_length = 0;

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTES, &num_active_attributes );
        motor::ogl::error::check_and_log( gl4_log( "glGetProgramiv(GL_ACTIVE_ATTRIBUTES)" ) );

        if( num_active_attributes == 0 ) return;

        config.attributes.resize( num_active_attributes );

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_length );
        motor::ogl::error::check_and_log(
            gl4_log( "glGetProgramiv(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)" ) );

        GLint size;
        GLenum gl_attrib_type;

        motor::memory::malloc_guard< char > buffer( name_length );

        for( GLint i = 0; i < num_active_attributes; ++i )
        {
            glGetActiveAttrib( program_id, i, name_length, 0, &size, &gl_attrib_type, buffer );

            if( motor::ogl::error::check_and_log( "glGetActiveAttrib failed. continue loop." ) )
                continue;

            // build-ins are not needed in the shader variables container.
            if( name_length >= 3 && buffer.equals( 0, 'g' ) && buffer.equals( 1, 'l' ) &&
                buffer.equals( 2, '_' ) )
                continue;

            GLuint const location_id = glGetAttribLocation( program_id, buffer );
            if( motor::ogl::error::check_and_log( "glGetAttribLocation failed. continue loop." ) )
                continue;

            motor::string_t const variable_name = motor::string_t( (const char *)buffer );

            gl4_pimpl_data::shader_data::attribute_variable_t vd;
            vd.name = std::move( variable_name );
            vd.loc = location_id;
            vd.type = gl_attrib_type;

            {
                motor::graphics::vertex_attribute va = motor::graphics::vertex_attribute::undefined;
                auto const res = config.find_vertex_input_binding_by_name( vd.name, va );
                motor::log::global_t::error(
                    !res, gl4_log( "can not find vertex attribute - " + vd.name ) );
                vd.va = va;
            }
            config.attributes[ i ] = vd;
        }

        for( auto iter = config.attributes.begin(); iter != config.attributes.end(); )
        {
            if( iter->va == motor::graphics::vertex_attribute::undefined )
            {
                iter = config.attributes.erase( iter );
                continue;
            }
            ++iter;
        }
    }

    //****************************************************************************************
    static bool_t bind_attributes( motor::string_in_t shd_name,
        gl4_pimpl_data::shader_data & sconfig, gl4_pimpl_data::geo_data & gconfig ) noexcept
    {
        // bind vertex buffer
        {
            glBindBuffer( GL_ARRAY_BUFFER, gconfig.vb_id );
            if( motor::ogl::error::check_and_log( gl4_log( "glBindBuffer(GL_ARRAY_BUFFER)" ) ) )
                return false;
        }

        // bind index buffer
        {
            GLint const id_ = gconfig.ib_elem_sib == 0 ? 0 : gconfig.ib_id;
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ );
            if( motor::ogl::error::check_and_log(
                    gl4_log( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)" ) ) )
                return false;
        }

        // disable old locations
        for( size_t i = 0; i < sconfig.attributes.size(); ++i )
        {
            glDisableVertexAttribArray( sconfig.attributes[ i ].loc );
            motor::ogl::error::check_and_log( gl4_log( "glDisableVertexAttribArray" ) );
        }

        GLuint uiStride = gconfig.stride;
        GLuint uiBegin = 0;
        GLuint uiOffset = 0;

        // sometimes some shader input attributes might not be used but
        // are defined in the vertex layout. So a new location need to be
        // figured out so that those vertex buffer attributes could
        // be bound to the shader.
        GLuint max_loc = 0;
        if( sconfig.attributes.size() != gconfig.elements.size() )
        {
            for( auto const & a : sconfig.attributes )
            {
                max_loc = std::max( max_loc, a.loc );
            }
        }

        for( auto & e : gconfig.elements )
        {
            uiBegin += uiOffset;
            uiOffset = e.sib();

            auto iter = std::find_if( sconfig.attributes.begin(), sconfig.attributes.end(),
                [ & ]( gl4_pimpl_data::shader_data::attribute_variable_cref_t av )
            { return av.va == e.va; } );

            GLuint loc = 0;
            GLenum type = GL_NONE;

            if( iter == sconfig.attributes.end() )
            {
                loc = ++max_loc;
                type = motor::platform::gl3::convert( e.type );

                motor::log::global_t::warning(
                    gl4_log( "Vertex attribute (" + motor::graphics::to_string( e.va ) +
                             ") in shader (" + shd_name +
                             ") not used."
                             "Will bind geometry (" +
                             shd_name + ") layout attribute to custom location (" +
                             motor::to_string( uint_t( loc ) ) + ")." ) );
            }
            else
            {
                loc = iter->loc;
                type = iter->type;
            }

            glEnableVertexAttribArray( loc );
            motor::ogl::error::check_and_log( gl4_log( "glEnableVertexAttribArray" ) );

            glVertexAttribPointer( loc, GLint( motor::graphics::size_of( e.type_struct ) ),
                motor::ogl::complex_to_simple_type( type ), GL_FALSE, (GLsizei)uiStride,
                (const GLvoid *)(size_t)uiBegin );

            motor::ogl::error::check_and_log( gl4_log( "glVertexAttribPointer" ) );
        }

        {
            glBindVertexArray( 0 );
            if( motor::ogl::error::check_and_log( gl4_log( "glBindVertexArray" ) ) ) return false;
        }
        return true;
    }

    //****************************************************************************************
    static void_t post_link_uniforms( gl4_pimpl_data::shader_data & config ) noexcept
    {
        GLuint const program_id = config.pg_id;

        GLint num_active_uniforms = 0;
        GLint name_length = 0;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORMS, &num_active_uniforms );
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORMS" );

        if( num_active_uniforms == 0 ) return;

        config.uniforms.resize( num_active_uniforms );

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_length );
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_MAX_LENGTH" );

        GLint size;
        GLenum gl_uniform_type;

        motor::memory::malloc_guard< char > buffer( name_length );

        for( GLint i = 0; i < num_active_uniforms; ++i )
        {
            glGetActiveUniform( program_id, i, name_length, 0, &size, &gl_uniform_type, buffer );
            if( motor::ogl::error::check_and_log( "[gl_33_api] : glGetActiveUniform" ) ) continue;

            // build-ins are not needed in the shader variables container.
            if( name_length >= 3 && buffer.equals( 0, 'g' ) && buffer.equals( 1, 'l' ) &&
                buffer.equals( 2, '_' ) )
                continue;

            GLuint const location_id = glGetUniformLocation( program_id, buffer );
            if( motor::ogl::error::check_and_log( "[glGetUniformLocation]" ) ) continue;

            if( motor::log::global_t::error(
                    location_id == GLuint( -1 ), gl4_log( "invalid uniform location id." ) ) )
                continue;

            motor::string_t const variable_name = motor::string_t( char_cptr_t( buffer ) );

            gl4_pimpl_data::shader_data::uniform_variable_t vd;
            vd.name = std::move( variable_name );
            vd.loc = location_id;
            vd.type = gl_uniform_type;
            vd.uniform_funk = motor::ogl::uniform_funk( gl_uniform_type );

            config.uniforms[ i ] = vd;
        }
    }

    //****************************************************************************************
    static void_t post_link_uniform_blocks( gl4_pimpl_data::shader_data & config ) noexcept
    {
        GLuint const program_id = config.pg_id;

        GLint num_active_uniform_blocks = 0;
        GLint max_block_name_length = 0;
        GLint max_uniform_name_length = 0;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_BLOCKS, &num_active_uniform_blocks );
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_BLOCKS" );

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length );
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_MAX_LENGTH" );

        if( num_active_uniform_blocks == 0 ) return;

        glGetProgramiv(
            program_id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &max_block_name_length );
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_MAX_LENGTH" );

        // GLint size ;
        // GLenum gl_uniform_type ;

        motor::memory::malloc_guard< char > block_name( max_block_name_length );
        motor::memory::malloc_guard< char > uniform_name( max_uniform_name_length );

        for( GLuint i = 0; i < (GLuint)num_active_uniform_blocks; ++i )
        {
            GLsizei block_name_len = 0;

            glGetActiveUniformBlockName(
                program_id, i, (GLsizei)max_block_name_length, &block_name_len, block_name );
            motor::ogl::error::check_and_log( "[glGetActiveUniformBlockName]" );

            GLuint const block_index = glGetUniformBlockIndex( program_id, block_name );

            int bp = 0;
        }
    }

    //****************************************************************************************
    static bool_t construct_shader_data( motor::graphics::shader_object_ref_t obj,
        gl4_pimpl_data::shader_data_out_t constructed ) noexcept
    {
        //
        // SECTION: Pre-Compilation
        //

        motor::graphics::shader_set_t ss;
        {
            auto const res = obj.shader_set( gl4_detail::sapi, ss );
            if( !res )
            {
                motor::log::global::warning< 2048 >( gl4_log( "config [%s] has no shaders for %s" ),
                    obj.name().c_str(), motor::graphics::gen4::to_string( gl4_detail::bt ) );

                return false;
            }
        }

        struct shader_compilation
        {
            GLuint pg_id = GLuint( -1 );
            GLuint vs_id = GLuint( -1 );
            GLuint gs_id = GLuint( -1 );
            GLuint ps_id = GLuint( -1 );
        };

        shader_compilation scomp;

        // pre-compile all the shaders
        {
            motor::log::global::status< 1024 >(
                "[gl4] : start shader compilation %s", obj.name().c_str() );
            motor::profiling::scoped_timer time_this( "[gl4] : compilation done in" );

            // Compile Vertex Shader
            {
                GLuint const id = glCreateShader( GL_VERTEX_SHADER );
                gl4_log_error( "glCreateShader : vertex shader" );

                if( !gl4_independent::compile_shader( id, ss.vertex_shader().code() ) )
                {
                    glDeleteShader( id );
                    gl4_log_error( "glDeleteShader : vertex shader" );
                    return false;
                }

                scomp.vs_id = id;
            }

            // Compile Geometry Shader
            if( ss.has_geometry_shader() )
            {
                GLuint const id = glCreateShader( GL_GEOMETRY_SHADER );
                gl4_log_error( "glCreateShader : geometry shader" );

                if( !gl4_independent::compile_shader( id, ss.geometry_shader().code() ) )
                {
                    glDeleteShader( scomp.vs_id );
                    gl4_log_error( "glDeleteShader : vertex shader" );

                    glDeleteShader( id );
                    gl4_log_error( "glDeleteShader : geometry shader" );

                    return false;
                }

                scomp.gs_id = id;
            }

            // Compile Pixel Shader
            if( ss.has_pixel_shader() )
            {
                GLuint const id = glCreateShader( GL_FRAGMENT_SHADER );
                gl4_log_error( "glCreateShader : fragment shader" );

                if( !gl4_independent::compile_shader( id, ss.pixel_shader().code() ) )
                {
                    glDeleteShader( scomp.vs_id );
                    gl4_log_error( "glDeleteShader : vertex shader" );

                    if( ss.has_geometry_shader() )
                    {
                        glDeleteShader( scomp.gs_id );
                        gl4_log_error( "glDeleteShader : geometry shader" );
                    }
                    glDeleteShader( id );
                    gl4_log_error( "glDeleteShader : fragment shader" );

                    return false;
                }
                scomp.ps_id = id;
            }
        }

        //
        // SECTION : Shader/Program setup
        //

        {
            GLuint const id = glCreateProgram();
            gl4_log_error( "glCreateProgram : program" );

            scomp.pg_id = id;
        }

        // attach vertex
        {
            glAttachShader( scomp.pg_id, scomp.vs_id );
            gl4_log_error( "glAttachShader : vertex shader" );
        }

        // attach geometry shader
        if( scomp.gs_id != GLuint( -1 ) )
        {
            glAttachShader( scomp.pg_id, scomp.gs_id );
            gl4_log_error( "glAttachShader : geometry shader" );
        }

        // attach pixel shader
        if( scomp.ps_id != GLuint( -1 ) )
        {
            glAttachShader( scomp.pg_id, scomp.ps_id );
            gl4_log_error( "glAttachShader : pixel shader" );
        }

        gl4_pimpl_data::shader_data_t sd;

        {
            sd.pg_id = scomp.pg_id;
            sd.vs_id = scomp.vs_id;
            sd.gs_id = scomp.gs_id;
            sd.ps_id = scomp.ps_id;

            // check max output vertices
            if( sd.gs_id != GLuint( -1 ) )
            {
                GLint max_out = 0;
                glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_out );
                gl4_log_error( "glGetIntegerv : GL_MAX_GEOMETRY_OUTPUT_VERTICES" );
                ( void_t ) max_out;
            }

            {
                obj.shader_bindings().for_each_vertex_input_binding(
                    [ & ]( size_t const, motor::graphics::vertex_attribute const va,
                        motor::string_cref_t name )
                {
                    sd.vertex_inputs.emplace_back(
                        gl4_pimpl_data::shader_data::vertex_input_binding{ va, name } );
                } );
            }

            if( ( obj.shader_bindings().get_num_output_bindings() != 0 ) &&
                ( obj.get_streamout_mode() != motor::graphics::streamout_mode::unknown ) )
            {
                sd.output_names = (char const **)motor::memory::global_t::alloc_raw< char * >(
                    obj.shader_bindings().get_num_output_bindings() );

                obj.shader_bindings().for_each_vertex_output_binding(
                    [ & ]( size_t const i, motor::graphics::vertex_attribute const va,
                        motor::graphics::ctype const, motor::string_cref_t name )
                {
                    sd.vertex_outputs.emplace_back(
                        gl4_pimpl_data::shader_data::vertex_output_binding{ va, name } );
                    sd.output_names[ i ] = name.c_str();
                } );

                // the mode interleaved or separate depends on the number of
                // buffers in the streamout object. So if a streamout
                // object is used, the engine needs to relink this shader
                // based on the number of buffers attached to the streamout
                // object.
                GLenum const mode = motor::platform::gl3::convert( obj.get_streamout_mode() );
                glTransformFeedbackVaryings( sd.pg_id,
                    GLsizei( obj.shader_bindings().get_num_output_bindings() ), sd.output_names,
                    mode );

                gl4_log_error( "glTransformFeedbackVaryings" );
                motor::log::global_t::status( mode == GL_NONE,
                    "Did you miss to set the streamout mode in the shader object?" );

                motor::memory::global_t::dealloc( sd.output_names );
                sd.output_names = nullptr;
            }

            // link
            {
                auto const r4 = this_t::link( sd.pg_id );
                if( !r4 )
                {
                    sd.is_linkage_ok = false;
                    return false;
                }
                sd.is_linkage_ok = true;
                motor::log::global::status< 1024 >(
                    "[GL4] : Compilation Successful : [%s]", obj.name().c_str() );
            }

            {
                this_t::post_link_attributes( sd );
                this_t::post_link_uniforms( sd );
                this_t::post_link_uniform_blocks( sd );
            }
        }

        constructed = std::move( sd );

        return true;
    }

    //****************************************************************************************
    static bool_t compile_shader( GLuint const id, motor::string_cref_t code )
    {
        if( code.empty() ) return false;

        GLchar const * source_string = (GLchar const *)( code.c_str() );

        glShaderSource( id, 1, &source_string, 0 );
        if( gl4_log_error( "glShaderSource" ) ) return false;

        glCompileShader( id );
        if( gl4_log_error( "glCompileShader" ) ) return false;

        GLint ret;
        glGetShaderiv( id, GL_COMPILE_STATUS, &ret );

        GLint length;
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length );

        if( ret == GL_TRUE && length <= 1 ) return true;

        if( length == 0 )
        {
            gl4_log_error( "shader compilation failed, but info log length is 0." );
            return false;
        }

        // print first line for info
        // user can place the shader name or any info there.
        {
            size_t pos = code.find_first_of( '\n' );
            motor::log::global::error( gl4_log( "First Line: " + code.substr( 0, pos ) ) );
        }

        // get the error message it is and print it
        {
            motor::memory::malloc_guard< char > info_log( length );

            glGetShaderInfoLog( id, length, 0, info_log );
            gl4_log_error( "glGetShaderInfoLog" );

            motor::vector< motor::string_t > tokens;
            motor::mstd::string_ops::split( motor::string_t( info_log ), '\n', tokens );

            for( auto const & msg : tokens )
            {
                motor::log::global::error( msg );
            }
        }
        return false;
    }

    //****************************************************************************************
    // call from support thread
    static std::tuple< bool_t, motor::graphics::shader_object_t, gl4_pimpl_data::shader_data_t >
    construct_shader_from_msl_data_st(
        motor::graphics::msl_object_ref_t msl_obj_in, motor::msl::database_ref_t mdb ) noexcept
    {
        motor::vector< motor::msl::symbol_t > config_symbols;

        size_t parsing_milli = 0;

        {
            auto tp_begin = std::chrono::high_resolution_clock::now();
            msl_obj_in.for_each_msl( motor::graphics::msl_api_type::msl_4_0,
                [ & ]( motor::string_in_t code )
            {
                motor::msl::post_parse::document_t doc =
                    motor::msl::parser_t( "gl4" ).process( code );

                mdb.insert( std::move( doc ), config_symbols );
            } );

            {
                parsing_milli = std::chrono::duration_cast< std::chrono::milliseconds >(
                    std::chrono::high_resolution_clock::now() - tp_begin )
                                    .count();
            }
        }

        if( config_symbols.size() > 1 )
        {
            motor::log::global::warning< 1024 >( "[gl4::construct_msl_data] : "
                                                 "library shader recompilation disabled" );
            return std::make_tuple(
                false, motor::graphics::shader_object_t(), gl4_pimpl_data::shader_data_t() );
        }

        if( config_symbols.size() == 0 )
        {
            motor::log::global::warning< 1024 >(
                "[gl4::construct_msl_data] : no shader submitted for %s",
                msl_obj_in.name().c_str() );
            return std::make_tuple(
                false, motor::graphics::shader_object_t(), gl4_pimpl_data::shader_data_t() );
        }

        auto const & c = config_symbols[ 0 ];
        auto const c_exp = c.expand();

        motor::msl::generatable_t res = motor::msl::dependency_resolver_t().resolve( &mdb, c );
        if( res.missing.size() != 0 )
        {
            motor::log::global_t::warning( "[gl4] : We have missing symbols for " + c_exp + " :" );
            for( auto const & s : res.missing )
            {
                motor::log::global_t::status( s.expand() );
            }
            return std::make_tuple(
                false, motor::graphics::shader_object_t(), gl4_pimpl_data::shader_data_t() );
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
                if( dynamic_cast< motor::msl::generic_default_value< bool_t > * >( df ) != nullptr )
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
                             motor::msl::generic_default_value< motor::math::vec3f_t > * >( df ) !=
                         nullptr )
                {
                    using ptr_t = motor::msl::generic_default_value< motor::math::vec3f_t > *;
                    ptr_t gdv = static_cast< ptr_t >( df );
                    default_values.data_variable< motor::math::vec3f_t >( var_.name )
                        ->set( gdv->get() );
                }
                else if( dynamic_cast<
                             motor::msl::generic_default_value< motor::math::vec4f_t > * >( df ) !=
                         nullptr )
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

        motor::graphics::shader_object_t so( c_exp );

        // generate code
        {
            auto tp_begin = std::chrono::high_resolution_clock::now();

            motor::msl::generator_t gen( std::move( res ) );
            auto const code = gen.generate< motor::msl::glsl::glsl4_generator_t >();
            motor::graphics::msl_bridge::create_by_api_type(
                motor::graphics::shader_api_type::glsl_4_0, code, so );

            {
                size_t const milli = std::chrono::duration_cast< std::chrono::milliseconds >(
                    std::chrono::high_resolution_clock::now() - tp_begin )
                                         .count();

                motor::log::global_t::status< 2048 >( "[gl4] : msl %s shader took [%zu|%zu] ms",
                    c_exp.c_str(), parsing_milli, milli );
            }
        }

        gl4_pimpl_data::shader_data_t new_shader_data;
        if( !gl4_independent::construct_shader_data( so, new_shader_data ) )
        {
            new_shader_data.invalidate();
            return std::make_tuple(
                false, motor::graphics::shader_object_t(), gl4_pimpl_data::shader_data_t() );
        }

        new_shader_data.default_values = std::move( default_values );

        return std::make_tuple( true, std::move( so ), std::move( new_shader_data ) );
    }
};