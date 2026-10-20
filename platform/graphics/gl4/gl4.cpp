// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "gl4.h"
#include "gl4_convert.h"
#include "../datas.hpp"

#include "gl4_support_thread.hpp"

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

#include <motor/ogl/gl/gl.h>
#include <motor/ogl/gl/convert.hpp>
#include <motor/ogl/gl/error.hpp>
#include <motor/memory/global.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/std/vector>
#include <motor/std/stack>
#include <motor/std/string_split.hpp>
#include <motor/std/hash_map>

#include "gl4_log.h"
#include "gl4_independent.h"
#include "gl4_pimpl_data.h"

#include <tuple>

using namespace motor::platform::gen4;
using namespace motor::ogl;

using namespace gl4_pimpl_data;

struct gl4_backend::pimpl
{
    motor_this_typedefs( pimpl );

    gl4_pimpl_data::feedbacks_t _feedbacks;
    gl4_pimpl_data::geo_datas_t _geometries;
    gl4_pimpl_data::shader_datas_t _shaders;
    gl4_pimpl_data::states_t _states;
    motor::stack< motor::graphics::render_state_sets_t, 10 > _state_stack;

    gl4_pimpl_data::render_datas_t _renders;
    gl4_pimpl_data::image_datas_t _images;
    gl4_pimpl_data::arrays_t _arrays;
    gl4_pimpl_data::framebuffer_datas_t _framebuffers;
    gl4_pimpl_data::msls_t _msls;

    GLsizei vp_width = 0;
    GLsizei vp_height = 0;

    // the current render state set
    motor::graphics::render_state_sets_t render_states;

    motor::platform::opengl::rendering_context_ptr_t _ctx;
    motor::platform::opengl::rendering_context_mtr_t _shd_ctx;

    size_t _tf_active_id = size_t( -1 );

    size_t _bid = size_t( -1 );

  private: // support thread

    using shared_data_t = motor::platform::gl4::support_thread< this_t >::shared_data_t;
    shared_data_t * _ctsd = nullptr;

    motor::platform::gl4::support_thread< this_t > _support_thread;

  public:

    //****************************************************************************************
    pimpl( size_t const bid, motor::platform::opengl::rendering_context_ptr_t ctx )
    {
        _ctx = ctx;
        _shd_ctx = _ctx->create_shared();
        _bid = bid;

        {
            motor::graphics::state_object_t obj( "gl4_default_states" );

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

            /*size_t const oid =*/this_t::construct_state( obj );
        }
        _ctsd = _support_thread.start_support_thread( this, _shd_ctx );
    }

    //****************************************************************************************
    pimpl( this_rref_t rhv ) noexcept
        : _shd_ctx( motor::move( rhv._shd_ctx ) ),
          _support_thread( std::move( rhv._support_thread ) )
    {
        _ctsd = motor::move( rhv._ctsd );
        _ctsd->owner = this;

        _shaders = std::move( rhv._shaders );
        _renders = std::move( rhv._renders );
        _geometries = std::move( rhv._geometries );
        _images = std::move( rhv._images );
        _framebuffers = std::move( rhv._framebuffers );
        _states = std::move( rhv._states );
        _arrays = std::move( rhv._arrays );
        _feedbacks = std::move( rhv._feedbacks );
        _msls = std::move( rhv._msls );

        _state_stack = std::move( rhv._state_stack );
        vp_width = rhv.vp_width;
        vp_height = rhv.vp_height;

        render_states = std::move( rhv.render_states );
        _ctx = motor::move( rhv._ctx );
        _tf_active_id = rhv._tf_active_id;
        _bid = rhv._bid;
    }

    //****************************************************************************************
    ~pimpl( void_t ) noexcept
    {
        _support_thread.stop_support_thread();
        motor::release( motor::move( _shd_ctx ) );

        this_t::release_and_clear_all_geometry_data( false );
        this_t::release_and_clear_all_shader_data( false );
        this_t::release_and_clear_all_render_data( false );
        this_t::release_and_clear_all_image_data( false );
        this_t::release_and_clear_all_framebuffer_data( false );
        this_t::release_and_clear_all_array_data( false );
        this_t::release_and_clear_all_feedback_data( false );
        this_t::release_and_clear_all_msl_data( false );
        this_t::release_and_clear_all_state_data( false );
    }

    // silent clear. No gl release will be done.
    // the context probably is already gone.
    void_t on_context_destruction( void_t ) noexcept
    {
        _support_thread.stop_support_thread();
        _ctsd = nullptr;

        this_t::release_and_clear_all_shader_data( false );
        this_t::release_and_clear_all_geometry_data( false );
        this_t::release_and_clear_all_render_data( false );
        this_t::release_and_clear_all_image_data( false );
        this_t::release_and_clear_all_framebuffer_data( false );
        this_t::release_and_clear_all_array_data( false );
        this_t::release_and_clear_all_feedback_data( false );
        this_t::release_and_clear_all_msl_data( false );
        this_t::release_and_clear_all_state_data( false );
    }

    //****************************************************************************************
    bool_t construct_state( motor::graphics::state_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid );
        auto const res =
            _states.access( oid, obj.name(), [ & ]( gl4_pimpl_data::state_data_ref_t states )
        {
            states.states.resize( obj.size() );

            // @note
            // set all enables. Enable/Disable only possible during construction
            // values are assigned in the update function for the render states
            obj.for_each( [ & ]( size_t const i, motor::graphics::render_state_sets_cref_t rs )
            { states.states[ i ] = rs; } );

            return true;
        } );
        if( res ) obj.set_oid( _bid, oid );
        return res;
    }

    //******************************************************************************************************************************
    bool_t update_state( motor::graphics::state_object_ref_t obj ) noexcept
    {
        size_t const oid = obj.get_oid( _bid );
        assert( oid < _states.size() );
        if( !obj.check_and_reset_changed( _bid ) ) return false;

        auto const res = _states.access( oid, [ & ]( gl4_pimpl_data::state_data_ref_t s )
        {
            obj.for_each( [ & ]( size_t const i, motor::graphics::render_state_sets_cref_t rs )
            {
                if( i >= s.states.size() ) return;
                s.states[ i ] = rs;
            } );
        } );
        return res;
    }

    //****************************************************************************************
    void_t handle_render_state(
        motor::graphics::render_state_sets_cref_t new_states, bool_t const popped = false )
    {
        // depth test
        {
            if( new_states.depth_s.do_change )
            {
                if( new_states.depth_s.ss.do_activate )
                {
                    glEnable( GL_DEPTH_TEST );
                    gl4_log_error( "glEnable : GL_DEPTH_TEST" );

                    glDepthMask( new_states.depth_s.ss.do_depth_write ? GL_TRUE : GL_FALSE );
                    gl4_log_error( "glDepthMask" );

                    glDepthFunc(
                        motor::platform::gl3::convert( new_states.depth_s.ss.compare_funk ) );
                    gl4_log_error( "glDepthFunc" );

                    glDepthRange( 0.0f, 1.0f );
                    gl4_log_error( "glDepthRange" );
                }
                else
                {
                    glDisable( GL_DEPTH_TEST );
                    gl4_log_error( "glDisable" );

                    glDepthMask( GL_FALSE );
                    gl4_log_error( "glDepthMask" );
                }
            }
        }

        // blend mode
        {
            if( new_states.blend_s.do_change )
            {
                if( new_states.blend_s.ss.do_activate )
                {
                    glEnable( GL_BLEND );
                    motor::ogl::error::check_and_log( gl4_log( "glEnable" ) );

                    GLenum const glsrc =
                        motor::platform::gl3::convert( new_states.blend_s.ss.src_blend_factor );
                    GLenum const gldst =
                        motor::platform::gl3::convert( new_states.blend_s.ss.dst_blend_factor );

                    glBlendEquation( GL_FUNC_ADD );
                    motor::ogl::error::check_and_log( gl4_log( "glBlendEquation" ) );
                    glBlendFunc( glsrc, gldst );
                    motor::ogl::error::check_and_log( gl4_log( "glBlendFunc" ) );
                }
                else
                {
                    glDisable( GL_BLEND );
                    motor::ogl::error::check_and_log( gl4_log( "glDisable" ) );
                }
            }
        }

        // polygon mode
        {
            if( new_states.polygon_s.do_change )
            {
                if( new_states.polygon_s.ss.do_activate )
                {
                    glEnable( GL_CULL_FACE );
                    motor::ogl::error::check_and_log( gl4_log( "glEnable" ) );

                    glCullFace( motor::platform::gl3::convert( new_states.polygon_s.ss.cm ) );
                    motor::ogl::error::check_and_log( gl4_log( "glCullFace" ) );

                    glFrontFace( motor::platform::gl3::convert( new_states.polygon_s.ss.ff ) );
                    motor::ogl::error::check_and_log( gl4_log( "glFrontFace" ) );

                    glPolygonMode( GL_FRONT_AND_BACK,
                        motor::platform::gl3::convert( new_states.polygon_s.ss.fm ) );
                    motor::ogl::error::check_and_log( gl4_log( "glPolygonMode" ) );
                }
                else
                {
                    glDisable( GL_CULL_FACE );
                    motor::ogl::error::check_and_log( gl4_log( "glDisable" ) );
                }
            }
        }

        // scissor
        {
            if( new_states.scissor_s.do_change )
            {
                if( new_states.scissor_s.ss.do_activate )
                {
                    glEnable( GL_SCISSOR_TEST );
                    motor::ogl::error::check_and_log( gl4_log( "glEnable" ) );

                    glScissor( GLint( new_states.scissor_s.ss.rect.x() ),
                        GLint( new_states.scissor_s.ss.rect.y() ),
                        GLsizei( new_states.scissor_s.ss.rect.z() ),
                        GLsizei( new_states.scissor_s.ss.rect.w() ) );
                    motor::ogl::error::check_and_log( gl4_log( "glScissor" ) );
                }
                else
                {
                    glDisable( GL_SCISSOR_TEST );
                    motor::ogl::error::check_and_log( gl4_log( "glDisable" ) );
                }
            }
        }

        // always set the viewport if enabled
        {
            if( new_states.view_s.do_change )
            {
                if( new_states.view_s.ss.do_activate )
                {
                    auto const & vp = new_states.view_s.ss.vp;
                    glViewport(
                        GLint( vp.x() ), GLint( vp.y() ), GLsizei( vp.z() ), GLsizei( vp.w() ) );
                    motor::ogl::error::check_and_log( gl4_log( "glViewport" ) );
                }
            }
        }

        // do clear the frame
        if( !popped )
        {
            if( new_states.clear_s.do_change && new_states.clear_s.ss.do_activate )
            {
                bool_t const clear_color = new_states.clear_s.ss.do_color_clear;
                bool_t const clear_depth = new_states.clear_s.ss.do_depth_clear;

                motor::math::vec4f_t const color = new_states.clear_s.ss.clear_color;
                glClearColor( color.x(), color.y(), color.z(), color.w() );
                motor::ogl::error::check_and_log( gl4_log( "glClearColor" ) );

                GLbitfield const color_bit = clear_color ? GL_COLOR_BUFFER_BIT : 0;
                GLbitfield const depth_bit = clear_depth ? GL_DEPTH_BUFFER_BIT : 0;

                glClear( color_bit | depth_bit );
                motor::ogl::error::check_and_log( gl4_log( "glEnable" ) );
            }
        }
    }

    //****************************************************************************************
    // if oid == -1, the state is popped.
    void_t handle_render_state( size_t const oid, size_t const rs_id ) noexcept
    {
        // pop state
        if( oid == size_t( -1 ) )
        {
            if( _state_stack.size() == 1 )
            {
                motor::log::global::error< 1024 >( "no more render states to pop" );
                return;
            }
            auto const popped = _state_stack.pop();
            this_t::handle_render_state( popped - _state_stack.top(), true );
        }
        else
        {
            auto new_id = std::make_pair( oid, rs_id );

            _states.access( new_id.first, [ & ]( gl4_pimpl_data::state_data_ref_t s )
            { _state_stack.push( _state_stack.top() + s.states[ new_id.second ] ); } );
            this_t::handle_render_state( _state_stack.top(), false );
        }
    }

    //****************************************************************************************
    size_t construct_framebuffer( motor::graphics::framebuffer_object_ref_t obj ) noexcept
    {
        auto fbd = _framebuffers.access( obj.get_oid( _bid ), obj.name() );

        if( fbd->gl_id == GLuint( -1 ) )
        {
            glGenFramebuffers( 1, &fbd->gl_id );
            if( gl4_log_error3( "glGenFramebuffers", obj ) ) return false;
        }

        // bind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fbd->gl_id );
            if( gl4_log_error3( "glBindFramebuffer", obj ) ) return false;
        }

        size_t const nt = obj.get_num_color_targets();
        auto const ctt = obj.get_color_target();
        auto const dst = obj.get_depth_target();
        motor::math::vec2ui_t dims = obj.get_dims();

        // fix dims
        {
            dims.x( dims.x() + dims.x() % 2 );
            dims.y( dims.y() + dims.y() % 2 );
        }

        bool_t const requires_store = fbd->colors[ 0 ] == GLuint( -1 );

        {
            if( fbd->colors[ 0 ] == GLuint( -1 ) )
            {
                glGenTextures( GLsizei( 8 ), fbd->colors );
                if( gl4_log_error3( "glGenTextures : color buffers", obj ) ) return false;
            }

            if( fbd->depth == GLuint( -1 ) )
            {
                glGenTextures( GLsizei( 1 ), &fbd->depth );
                if( gl4_log_error3( "glGenTextures : depth buffer", obj ) ) return false;
            }
        }

        // construct color textures
        {
            for( size_t i = 0; i < nt; ++i )
            {
                GLuint const tid = fbd->colors[ i ];

                glBindTexture( GL_TEXTURE_2D, tid );
                if( gl4_log_error3( "glBindTexture : color buffer", obj ) ) continue;

                GLenum const target = GL_TEXTURE_2D;
                GLint const level = 0;
                GLsizei const width = dims.x();
                GLsizei const height = dims.y();
                GLenum const format = GL_RGBA;
                GLenum const type = motor::platform::gl3::to_pixel_type( ctt );
                GLint const border = 0;
                GLint const internal_format = motor::platform::gl3::to_gl_format( ctt );

// maybe required for memory allocation
// at the moment, render targets do not have system memory.
#if 0
                    size_t const sib = motor::platform::gl3::calc_sib( dims.x(), dims.y(), ctt ) ;
#endif
                void_cptr_t data = nullptr;

                glTexImage2D(
                    target, level, internal_format, width, height, border, format, type, data );
                gl4_log_error3( "glTexImage2D : color buffer", obj );
            }

            // attach
            for( size_t i = 0; i < nt; ++i )
            {
                GLuint const tid = fbd->colors[ i ];
                GLenum const att = GLenum( size_t( GL_COLOR_ATTACHMENT0 ) + i );
                glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 );
                gl4_log_error3( "glFramebufferTexture2D : color buffer", obj );
            }
        }

        // depth/stencil
        if( dst != motor::graphics::depth_stencil_target_type::unknown )
        {
            {
                GLuint const tid = fbd->depth;

                glBindTexture( GL_TEXTURE_2D, tid );
                if( !gl4_log_error3( "glBindTexture : depth buffer", obj ) )
                {
                    GLenum const target = GL_TEXTURE_2D;
                    GLint const level = 0;
                    GLsizei const width = dims.x();
                    GLsizei const height = dims.y();
                    GLenum const format = motor::platform::gl3::to_gl_format( dst );
                    GLenum const type = motor::platform::gl3::to_gl_type( dst );
                    GLint const border = 0;
                    GLint const internal_format = motor::platform::gl3::to_gl_format( dst );

// maybe required for memory allocation
// at the moment, render targets do not have system memory.
#if 0
                        size_t const sib = motor::platform::gl3::calc_sib( dims.x(), dims.y(), ctt ) ;
#endif
                    void_cptr_t data = nullptr;

                    glTexImage2D(
                        target, level, internal_format, width, height, border, format, type, data );
                    gl4_log_error3( "glTexImage2D : depth buffer", obj );
                }
            }

            // attach
            {
                GLuint const tid = fbd->depth;
                GLenum const att = motor::platform::gl3::to_gl_attachment( dst );
                glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 );
                gl4_log_error3( "glFramebufferTexture2D : depth buffer", obj );
            }
        }

        GLenum status = 0;
        // validate
        {
            status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
            gl4_log_error3( "glCheckFramebufferStatus", obj );

            if( motor::log::global_t::warning_if< 2048 >( status != GL_FRAMEBUFFER_COMPLETE,
                    "Incomplete framebuffer : [%s]", obj.name().c_str() ) )
                return false;
        }

        // unbind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            gl4_log_error3( "glBindFramebuffer", obj );
        }

        // remember data
        if( status == GL_FRAMEBUFFER_COMPLETE )
        {
            // color type maybe?
            fbd->nt = nt;
            fbd->dims = dims;
        }

        // store images
        if( requires_store )
        {
            for( size_t i = 0; i < nt; ++i )
            {
                size_t new_iid = size_t( -1 );
                motor::string_t name = fbd.name() + "." + motor::to_string( i );
                _images.access( new_iid, name, [ & ]( gl4_pimpl_data::image_data_ref_t id )
                {
                    id.tex_id = fbd->colors[ i ];
                    id.type = GL_TEXTURE_2D;
                    for( size_t j = 0; j < (size_t)motor::graphics::texture_wrap_mode::size; ++j )
                    {
                        id.wrap_types[ j ] = GL_CLAMP_TO_BORDER;
                    }
                    for( size_t j = 0; j < (size_t)motor::graphics::texture_filter_mode::size; ++j )
                    {
                        id.filter_types[ j ] = GL_LINEAR;
                    }
                    return true;
                } );
            }
        }

        // store depth/stencil
        if( requires_store )
        {
            size_t new_iid = size_t( -1 );
            motor::string_t name = fbd.name() + ".depth";
            _images.access( new_iid, name, [ & ]( gl4_pimpl_data::image_data_ref_t id )
            {
                id.tex_id = fbd->depth;
                id.type = GL_TEXTURE_2D;

                for( size_t j = 0; j < (size_t)motor::graphics::texture_wrap_mode::size; ++j )
                {
                    id.wrap_types[ j ] = GL_REPEAT;
                }

                for( size_t j = 0; j < (size_t)motor::graphics::texture_filter_mode::size; ++j )
                {
                    id.filter_types[ j ] = GL_NEAREST;
                }

                return true;
            } );
        }

        obj.set_oid( _bid, fbd.oid() );
        return true;
    }

    //****************************************************************************************
    bool_t release_framebuffer( size_t const oid ) noexcept
    {
        auto fbd = _framebuffers.access( oid );
        if( !fbd )
        {
            motor::log::global_t::error< 1024 >(
                "[gl4] : can not find framebuffer with oid %d", oid );
            return false;
        }

        return this_t::release_framebuffer( framebuffer_datas_t::checked_handle_t( fbd ) );
    }

    //****************************************************************************************
    bool_t release_framebuffer( framebuffer_datas_t::checked_handle_t fbd ) noexcept
    {
        // #1 clear the images buffers, i.e remove references
        _images.for_each( [ & ]( gl4_pimpl_data::image_data_ref_t id )
        {
            // try all colors
            for( size_t i = 0; i < 8; ++i )
            {
                if( fbd->colors[ i ] == GLuint( -1 ) ) continue;
                if( id.tex_id != fbd->colors[ i ] ) continue;

                // remove id before invalidate
                id.tex_id = GLuint( -1 );
                return;
            }

            if( fbd->depth != GLuint( -1 ) )
            {
                if( id.tex_id == fbd->depth )
                {
                    // remove id before invalidate
                    id.tex_id = GLuint( -1 );
                }
            }
        } );

        // #2 now we can invalidate
        return _framebuffers.invalidate( fbd.oid() );
    }

    //****************************************************************************************
    bool_t activate_framebuffer( size_t const oid ) noexcept
    {
        auto fbd = _framebuffers.access( oid );
        if( !fbd )
        {
            motor::log::global_t::error< 1024 >(
                "[gl4] : can not find framebuffer with oid %d", oid );
            return false;
        }

        return pimpl::activate_framebuffer( framebuffer_datas_t::checked_handle_t( fbd ) );
    }

    //****************************************************************************************
    bool_t activate_framebuffer( framebuffer_datas_t::checked_handle_t fbd ) noexcept
    {
        // bind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fbd->gl_id );
            if( gl4_log_error( "glBindFramebuffer" ) ) return false;
        }

        // setup color
        {
            GLenum attachments[ 15 ];
            size_t const num_color = fbd->nt;

            for( size_t i = 0; i < num_color; ++i )
            {
                attachments[ i ] = GLenum( size_t( GL_COLOR_ATTACHMENT0 ) + i );
            }

            glDrawBuffers( GLsizei( num_color ), attachments );
            if( gl4_log_error( "glDrawBuffers" ) ) return false;
        }
        return true;
    }

    //****************************************************************************************
    bool_t deactivate_framebuffer( void_t ) noexcept
    {
        // unbind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 );
            if( gl4_log_error( "glGenFramebuffers" ) ) return false;
        }

        return true;
    }

    //****************************************************************************************
    bool_t install_shader_data( motor::graphics::shader_object_ref_t obj,
        gl4_pimpl_data::shader_data_ref_t new_shader_data ) noexcept
    {

        auto shd = _shaders.access( obj.get_oid( _bid ), obj.name() );
        if( !shd ) return false;

        *shd = std::move( new_shader_data );

        obj.set_oid( _bid, shd.oid() );

        // if the shader is redone, all render objects using
        // the shader need to be updated
        {
            //_renders.for_each( [ &, shd ]( size_t const rd_id, gl4_pimpl_data::render_data_ref_t
            // rd )
            _renders.for_each( [ &, shdc = shader_datas_t::checked_handle_t( shd ) ](
                                   render_datas_t::checked_handle_t rd )
            {
                if( rd->shd_id != shd.oid() ) return;

                rd->var_sets_data.clear();
                rd->var_sets_texture.clear();
                rd->var_sets_array.clear();
                rd->var_sets_streamout.clear();

                auto sets = std::move( rd->var_sets );
                auto idxs = std::move( rd->varset_to_idx );

                // here i is the variable set index.
                // the entry is the index into the internal var_sets
                // vector.
                for( size_t i = 0; i < idxs.size(); ++i )
                {
                    if( idxs[ i ].idx == size_t( -1 ) ) continue;
                    auto & vs = sets[ idxs[ i ].idx ];
                    this_t::connect( rd, shdc, i, vs.hash, motor::move( vs.vs ) );
                }

                this_t::render_object_variable_memory( rd, shdc );

                for( size_t i = 0; i < rd->varset_to_idx.size(); ++i )
                {
                    this_t::update_variables( rd, shdc, i );
                }
            } );
        }

        return true;
    }

    // this function compiles the shader. It is not yet in the support thread.
    // @todo put in support thread
    bool_t construct_shader_data( motor::graphics::shader_object_ref_t obj ) noexcept
    {
        gl4_pimpl_data::shader_data_t new_shader_data;
        if( !gl4_independent::construct_shader_data( obj, new_shader_data ) )
        {
            new_shader_data.invalidate();
            return false;
        }

        return this_t::install_shader_data( obj, new_shader_data );
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_msl_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _msls.for_each( [ & ]( gl4_pimpl_data::msl_data_ref_t d ) {} );
        }

        _msls.invalidate_and_clear();
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_feedback_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _feedbacks.for_each( [ & ]( gl4_pimpl_data::feedback_data_ref_t d )
            {
                for( size_t rw = 0; rw < 2; ++rw )
                {
                    auto & buffer = d._buffers[ rw ];
                    buffer.qid = GLuint( -1 );
                    buffer.tfid = GLuint( -1 );
                    buffer.tids[ 0 ] = GLuint( -1 );
                }
            } );
        }

        _feedbacks.invalidate_and_clear();
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_array_data( bool_t const with_gl = false ) noexcept
    {
        // #1 first, reset the gl ids
        if( !with_gl )
        {
            _arrays.for_each( [ & ]( gl4_pimpl_data::array_data_ref_t d )
            {
                d.buf_id = GLuint( -1 );
                d.tex_id = GLuint( -1 );
            } );
        }

        // #2 now, the data can be invalidated
        _arrays.invalidate_and_clear();
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_framebuffer_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _framebuffers.for_each(
                [ & ]( gl4_pimpl_data::framebuffer_data_ref_t d ) { d.reset_gl_ids(); } );
        }

        _framebuffers.invalidate_and_clear();
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_image_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _images.for_each(
                [ & ]( gl4_pimpl_data::image_data_ref_t d ) { d.tex_id = GLuint( -1 ); } );
        }

        _images.invalidate_and_clear();
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_render_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _renders.for_each(
                [ & ]( gl4_pimpl_data::render_data_ref_t rd ) { rd.geo_to_vaos.clear(); } );
        }

        _renders.invalidate_and_clear();
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_shader_data( bool_t const with_gl = false ) noexcept
    {
        // if with_gl, just reset all gl ids.
        // => no gl calls will be made
        if( !with_gl )
        {
            _shaders.for_each( [ & ]( gl4_pimpl_data::shader_data_ref_t shd )
            {
                shd.pg_id = GLuint( -1 );
                shd.vs_id = GLuint( -1 );
                shd.gs_id = GLuint( -1 );
                shd.ps_id = GLuint( -1 );
            } );
        }

        _shaders.invalidate_and_clear();
    }

    //****************************************************************************************
    void_t release_and_clear_all_geometry_data( bool_t const with_gl = false ) noexcept
    {
        // if with_gl, just reset all gl ids.
        // => no gl calls will be made
        if( !with_gl )
        {
            _geometries.for_each( [ & ]( gl4_pimpl_data::geo_data_ref_t gd )
            {
                gd.ib_id = GLuint( -1 );
                gd.vb_id = GLuint( -1 );
            } );
        }

        _renders.for_each( [ & ]( gl4_pimpl_data::render_data_ref_t rd ) { rd.geo_ids.clear(); } );

        _geometries.invalidate_and_clear();
    }

    //****************************************************************************************
    void_t release_and_clear_all_state_data( bool_t const with_gl = false ) noexcept
    {
        _states.invalidate_and_clear();
    }

    //****************************************************************************************
    bool_t release_shader_data( size_t const oid ) noexcept
    {
        return _shaders.invalidate( oid );
    }

    //************************************************************************************************************
    void_t release_msl_data( size_t const oid ) noexcept
    {
        _msls.access( oid, [ & ]( gl4_pimpl_data::msl_data & data )
        {
            // we do not need the name, the id should suffice because
            // we have the id internally so it must be valid.
            this_t::release_render_data(
                render_datas_t::checked_handle_t( _renders.access( data.ro_id ) ) );
            this_t::release_shader_data( data.so.get_oid( _bid ) );

            data.ro_id = size_t( -1 );
        } );

        _msls.invalidate( oid );
    }

    //****************************************************************************************
    void_t detach_shaders( GLuint const program_id )
    {
        GLsizei count = 0;
        GLuint shaders_[ 10 ];

        glGetAttachedShaders( program_id, 10, &count, shaders_ );
        gl4_log_error( "glGetAttachedShaders" );

        for( GLsizei i = 0; i < count; ++i )
        {
            glDetachShader( program_id, shaders_[ i ] );
            gl4_log_error( "glDetachShader" );
        }
    }

    //****************************************************************************************
    void_t delete_all_variables( gl4_pimpl_data::shader_data & config )
    {
        config.vertex_inputs.clear();

        // for( auto & v : config.attributes )
        {
            // delete memory
        }
        config.attributes.clear();
        config.uniforms.clear();
    }

    //****************************************************************************************
    bool_t check_link_status( size_t const rid ) noexcept
    {
        if( auto rd = _renders.access( rid ); rd )
        {
            if( auto shd = _shaders.access( rd->shd_id ); shd )
            {
                return shd->is_linkage_ok;
            }
        }

        return false;
    }

    //****************************************************************************************
    bool_t construct_image_config( motor::graphics::image_object_ref_t config ) noexcept
    {
        auto imgd = _images.access( config.get_oid( _bid ), config.name() );

        // sampler
        if( imgd->tex_id == GLuint( -1 ) )
        {
            GLuint id = GLuint( -1 );
            glGenTextures( 1, &id );
            if( gl4_log_error( "glGenTextures" ) ) return false;

            imgd->tex_id = id;
        }

        imgd->type = motor::platform::gl3::convert( config.get_type() );
        for( size_t j = 0; j < (size_t)motor::graphics::texture_wrap_mode::size; ++j )
        {
            imgd->wrap_types[ j ] = motor::platform::gl3::convert(
                config.get_wrap( (motor::graphics::texture_wrap_mode)j ) );
        }

        for( size_t j = 0; j < (size_t)motor::graphics::texture_filter_mode::size; ++j )
        {
            imgd->filter_types[ j ] = motor::platform::gl3::convert(
                config.get_filter( (motor::graphics::texture_filter_mode)j ) );
        }

        config.set_oid( _bid, imgd.oid() );
        return true;
    }

    //****************************************************************************************
    bool_t release_image_data( size_t const oid ) noexcept
    {
        return _images.invalidate( oid );
    }

    //****************************************************************************************
    void_t send_configure_to_st( motor::graphics::msl_object_mtr_t obj ) noexcept
    {
        motor::graphics::object_t::data_manipulator( obj, _bid ).change_to_in_transit();
        _ctsd->add_forward_item( motor::platform::local_gl4::work_item::make_item(
            motor::platform::local_gl4::work_item::work_type::compile, motor::share( obj ) ) );
    }

    //****************************************************************************************
    bool_t construct_msl_data( gl4_independent::compilation_result && cr ) noexcept
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
            _msls.access( oid, obj.name(), [ & ]( gl4_pimpl_data::msl_data_ref_t msl )
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
                _shaders.access( so.get_oid( _bid ), [ & ]( gl4_pimpl_data::shader_data_ref_t shd )
                {
                    obj.for_each( [ & ]( motor::graphics::compilation_listener_mtr_t lst )
                    {
                        auto const s =
                            shd.is_linkage_ok
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

        if( success )
        {
            mani.reset_in_transit();
            mani.change_to_ready( motor::graphics::result::ok );
        }
        else
        {
            mani.reset_in_transit();
            mani.change_to_raw( motor::graphics::result::failed );

            cr.object->for_each( [ & ]( motor::graphics::compilation_listener_mtr_t lst )
            {
                lst->set( motor::graphics::compilation_listener::state::failed,
                    motor::graphics::shader_bindings() );
            } );
        }

        cr.object->set_oid( _bid, oid );
        motor::release( motor::move( cr.object ) );

        return true;
    }

    //****************************************************************************************
    bool_t construct_render_data( motor::graphics::render_object_ref_t obj ) noexcept
    {

        if( this_t::release_render_data( obj ) )
        {
            obj.set_oid( _bid, size_t( -1 ) );
        }

        if( !this_t::construct_render_data_ext( obj ) )
        {
            motor::log::global_t::error( "[gl4] : construct_render_data_ext" );
        }

        return true;
    }

    //****************************************************************************************
    bool_t release_render_data( motor::graphics::render_object_ref_t obj ) noexcept
    {
        auto rd = _renders.access( obj.get_oid( _bid ) );
        if( rd.oid() == size_t( -1 ) ) return true;
        return this_t::release_render_data( render_datas_t::checked_handle_t( rd ) );
    }

    //****************************************************************************************
    bool_t release_render_data( render_datas_t::checked_handle_t rd ) noexcept
    {
        // #1 break connections BEFORE invalidating the item
        {
            for( auto const & item : rd->geo_ids )
            {
                _geometries.access( item.id, [ & ]( gl4_pimpl_data::geo_data_ref_t d )
                { d.remove_render_data_id( rd.oid() ); } );
            }

            for( auto const id : rd->tf_ids )
            {
                _feedbacks.access( id, [ & ]( gl4_pimpl_data::feedback_data_ref_t d )
                { d.remove_render_data_id( rd.oid() ); } );
            }
        }

        _renders.invalidate( rd.oid() );
        return true;
    }

    //****************************************************************************************
    bool_t construct_render_data_ext( motor::graphics::render_object_ref_t rc ) noexcept
    {
        assert( rc.get_oid( _bid ) == size_t( -1 ) &&
                "at the moment, rc must be released before this function is used." );

        auto rd = _renders.access( rc.get_oid( _bid ), rc.name() );

        // handle geometry links
        {
            rd->geo_ids.reserve( rc.get_num_geometry() );
            for( size_t i = 0; i < rc.get_num_geometry(); ++i )
            {
                auto const gid = _geometries.find_by_name( rc.get_geometry_link( i ).name );
                if( gid == size_t( -1 ) )
                {
                    motor::log::global_t::warning< 1024 >(
                        "[gl4] : no geometry with name [%s] for render_data [%s]",
                        rc.get_geometry_link( i ).name.c_str(), rc.name().c_str() );
                    continue;
                }
                rd->geo_ids.emplace_back( gl4_pimpl_data::render_data::geo_id_data{
                    gid, rc.get_geometry_link( i ).hash } );
                _geometries.access( gid, [ & ]( gl4_pimpl_data::geo_data_ref_t d )
                { d.add_render_data_id( rd.oid() ); } );
            }
        }

        // handle stream out links
        {
            rd->tf_ids.reserve( rc.get_num_streamout() );

            // find stream out object
            for( size_t i = 0; i < rc.get_num_streamout(); ++i )
            {
                size_t const idx = _feedbacks.find_by_name( rc.get_streamout( i ) );
                if( idx == size_t( -1 ) )
                {
                    motor::log::global_t::warning< 2048 >(
                        "[gl4] : no streamout object with name [%s] for render_data [%s]",
                        rc.get_streamout( i ).c_str(), rc.name().c_str() );
                    return false;
                }
                rd->tf_ids.emplace_back( idx );
            }
        }

        // find shader
        {
            rd->shd_id = _shaders.find_by_name( rc.get_shader() );
            if( rd->shd_id == size_t( -1 ) )
            {
                motor::log::global::warning< 1024 >(
                    "[gl4] : no shader with name [%s] for render_data [%s]",
                    rc.get_shader().c_str(), rc.name().c_str() );
                return false;
            }
        }

        auto shd = shader_datas_t::checked_handle( shader_datas_t::handle_t( rd->shd_id, _shaders ) );

        {
            // remember that rd.var_sets hold the ref count reminder!
            // no need to release any managed pointer!
            auto vars = std::move( rd->var_sets );
            auto vsidx_to_idx = std::move( rd->varset_to_idx );

            rd->var_sets_data.clear();
            rd->var_sets_texture.clear();
            rd->var_sets_array.clear();
            rd->var_sets_streamout.clear();
            rd->var_sets.clear();

            rc.for_each(
                [ & ]( size_t const i, motor::graphics::render_object_t::variable_set_cref_t vs )
            {
                auto const res = this_t::connect(
                    render_datas_t::checked_handle_t( rd ), shd, i, vs.hash, motor::share( vs.vs ) );
                motor::log::global_t::warning( !res, gl4_log( "connect" ) );
            } );

            for( auto & v : vars ) motor::release( motor::move( v.vs ) );
        }

        this_t::render_object_variable_memory( render_datas_t::checked_handle_t( rd ), shd );

        rc.set_oid( _bid, rd.oid() );

        return true;
    }

    //****************************************************************************************
    bool_t update_geometry_link(
        motor::graphics::render_object_ref_t ro, size_t const geo_idx ) noexcept
    {
        auto rd = _renders.access( ro.get_oid( _bid ), ro.name() );

        // do full binding
        if( rd->geo_ids.size() <= geo_idx )
        {
            auto const gid = _geometries.find_by_name( ro.get_geometry_link( geo_idx ).name );
            if( gid == size_t( -1 ) )
            {
                motor::log::global_t::warning< 1024 >(
                    "[gl4:update_geometry_link] : no geometry with name [%s] for "
                    "render_data "
                    "[%s]",
                    ro.get_geometry_link( geo_idx ).name.c_str(), ro.name().c_str() );
                return false;
            }
            rd->geo_ids.emplace_back( gl4_pimpl_data::render_data::geo_id_data{
                gid, ro.get_geometry_link( geo_idx ).hash } );
            _geometries.access( gid,
                [ & ]( gl4_pimpl_data::geo_data_ref_t d ) { d.add_render_data_id( rd.oid() ); } );
        }
        else if( rd->geo_ids[ geo_idx ].hash != ro.get_geometry_link( geo_idx ).hash )
        {
            auto const hash = ro.get_geometry_link( geo_idx ).hash;
            if( ro.get_geometry_link( geo_idx ).ref_count == 0 )
            {
                rd->invalidate_geometry_id( geo_idx, hash );
            }
            else if( rd->geo_ids[ geo_idx ].id != size_t( -1 ) )
            {
                // config geometry entry is valid.
                // maybe the data changed lets check it
                {
                    auto const gid =
                        _geometries.find_by_name( ro.get_geometry_link( geo_idx ).name );
                    if( gid != rd->geo_ids[ geo_idx ].id )
                    {
                        // ok, geometry changed
                        rd->geo_ids[ geo_idx ].id = gid;

                        // at this point, the geometry needs to be rebound
                        // to the shader inputs.
                        // At the moment, this is done before rendering
                    }
                }

                rd->geo_ids[ geo_idx ].hash = hash;
            }
            else
            {
                auto const gid = _geometries.find_by_name( ro.get_geometry_link( geo_idx ).name );
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

    //****************************************************************************************
    bool_t bind_attributes( GLuint const vao, geo_datas_t::checked_handle_t gd,
        shader_datas_t::checked_handle_t sd ) noexcept
    {
        glBindVertexArray( vao );
        if( gl4_log_error( "glBindVertexArray" ) ) return false;
        return gl4_independent::bind_attributes( sd, gd );
    }

    //****************************************************************************************
    bool_t construct_geometry( geo_datas_t::handle_t geod, motor::graphics::vertex_buffer_in_t vb )
    {
        motor::log::global::status< 1024 >( "[gl4] : construct geometry %s", geod.name().c_str() );

        // vertex buffer
        if( geod->vb_id == GLuint( -1 ) )
        {
            GLuint id = GLuint( -1 );
            glGenBuffers( 1, &id );
            if( gl4_log_error( "glGenBuffers : vertex buffer" ) ) return false;

            geod->vb_id = id;
        }

        // index buffer
        if( geod->ib_id == GLuint( -1 ) )
        {
            GLuint id = GLuint( -1 );
            glGenBuffers( 1, &id );
            if( gl4_log_error( "glGenBuffers : index buffer" ) ) return false;

            geod->ib_id = id;
        }

        {
            geod->stride = GLuint( vb.get_layout_sib() );

            geod->elements.clear();
            vb.for_each_layout_element( [ & ]( motor::graphics::vertex_buffer_t::data_cref_t d )
            {
                gl4_pimpl_data::geo_data::layout_element le;
                le.va = d.va;
                le.type = d.type;
                le.type_struct = d.type_struct;
                geod->elements.push_back( le );
            } );
        }

        return true;
    }

    //****************************************************************************************
    std::pair< bool_t, geo_datas_t::handle_t > construct_geometry(
        motor::string_in_t name, motor::graphics::vertex_buffer_in_t vb ) noexcept
    {
        auto geod = _geometries.access( size_t( -1 ), name );
        auto const res = this_t::construct_geometry( geod, vb );
        return std::make_pair( res, std::move( geod ) );
    }

    //****************************************************************************************
    std::pair< bool_t, geo_datas_t::handle_t > construct_geometry(
        motor::graphics::geometry_object_ref_t obj ) noexcept
    {
        auto geod = _geometries.access( obj.get_oid( _bid ), obj.name() );
        auto const res = this_t::construct_geometry( geod, obj.vertex_buffer() );

        if( res ) obj.set_oid( _bid, geod.oid() );
        return std::make_pair( res, std::move( geod ) );
    }

    //****************************************************************************************
    bool_t release_geometry( size_t const oid ) noexcept
    {
        auto geod = _geometries.access( oid );
        if( !geod )
        {
            motor::log::global_t::error< 1024 >( "[gl4] : unknown geometry id %d", geod.oid() );
            return false;
        }

        return this_t::release_geometry( std::move( geod ) );
    }

    //****************************************************************************************
    bool_t release_geometry( geo_datas_t::handle_t && geod ) noexcept
    {
        // #1 have to deal with external references first
        for( auto const id : geod->rd_ids )
        {
            if( id == size_t( -1 ) ) continue;

            auto hnd = _renders.access( id );
            if( hnd ) hnd->remove_geometry_id( geod.oid() );
        }
        geod->rd_ids.clear();

        // #2 now we can invalidate the data object itself
        return _geometries.invalidate( geod.oid() );
    }

    //****************************************************************************************
    // @todo if the update is errornous somewhere in the middle, the object would be invalid.
    //      the data would need some data_valid flag.
    bool_t update( motor::graphics::geometry_object_mtr_t geoo, bool_t const is_config = false )
    {
        auto geod = _geometries.access( geoo->get_oid( _bid ) );
        if( !geod )
        {
            motor::log::global_t::error< 1024 >( "[gl4] : unknown geometry id %d", geod.oid() );
            return false;
        }

        {
            geod->num_elements_ib = geoo->index_buffer().get_num_elements();
            geod->num_elements_vb = geoo->vertex_buffer().get_num_elements();
            geod->ib_elem_sib = 0;
            geod->ib_type = GL_UNSIGNED_INT;
            geod->pt = motor::platform::gl3::convert( geoo->primitive_type() );
        }

        // bind vertex buffer
        {
            glBindBuffer( GL_ARRAY_BUFFER, geod->vb_id );
            if( gl4_log_error( "glBindBuffer : vertex buffer" ) ) return false;
        }

        // allocate buffer memory
        // what about mapped memory?
        {
            GLuint const sib = GLuint( geoo->vertex_buffer().get_sib() );
            if( is_config || sib > geod->sib_vb )
            {
                glBufferData( GL_ARRAY_BUFFER, sib, geoo->vertex_buffer().data(), GL_STATIC_DRAW );
                if( gl4_log_error( "glBufferData : vertex buffer" ) ) return false;
                geod->sib_vb = sib;
            }
            else
            {
                glBufferSubData( GL_ARRAY_BUFFER, 0, sib, geoo->vertex_buffer().data() );
                if( gl4_log_error( "glBufferSubData : vertex buffer" ) ) return false;
            }
        }

        // allocate buffer memory
        // what about mapped memory?
        if( geoo->index_buffer().get_num_elements() > 0 )
        {
            // bind index buffer
            {
                GLint const id_ = geoo->index_buffer().get_num_elements() == 0 ? 0 : geod->ib_id;
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ );
                if( gl4_log_error( "glBindBuffer : index buffer" ) ) return false;
            }

            geod->ib_elem_sib = geoo->index_buffer().get_element_sib();

            GLuint const sib = GLuint( geoo->index_buffer().get_sib() );
            if( is_config || sib > geod->sib_ib )
            {
                glBufferData(
                    GL_ELEMENT_ARRAY_BUFFER, sib, geoo->index_buffer().data(), GL_STATIC_DRAW );
                if( gl4_log_error( "glBufferData : index buffer" ) ) return false;
                geod->sib_ib = sib;
            }
            else
            {
                glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, sib, geoo->index_buffer().data() );
                if( gl4_log_error( "glBufferSubData : index buffer" ) ) return false;
            }
        }

        if( is_config )
        {
            // force rebind of vertex attributes. Layout may have changed.
            for( auto const & rid : geod->rd_ids )
            {
                auto rd = _renders.access( rid );
                if( !rd || rd->shd_id == size_t( -1 ) ) continue;

                // this may happen if the geometry is configured a second time
                // after the render object is configured but the render object
                // is not rendered yet.
                size_t i = size_t( -1 );
                while( ++i < rd->geo_to_vaos.size() && rd->geo_to_vaos[ i ].gid != geod.oid() );
                if( i != rd->geo_to_vaos.size() )
                {
                    glDeleteVertexArrays( 1, &( rd->geo_to_vaos[ i ].vao ) );
                    gl4_log_error( "glDeleteVertexArrays" );

                    rd->geo_to_vaos[ i ].gid = size_t( -1 );
                    rd->geo_to_vaos[ i ].vao = GLuint( -1 );
                }
            }
        }

        return true;
    }

    //****************************************************************************************
    // @param is_config used to determine recreating the texture on the gpu side.
    //          This param should be true if an image is configured or re-configured.
    bool_t update( motor::graphics::image_object_ref_t confin, bool_t const is_config = false )
    {

        auto img_hnd = _images.access( confin.get_oid( _bid ) );

        if( !img_hnd )
        {
            motor::log::global_t::error< 1024 >(
                "[gl4] : unknown image id %d", confin.get_oid( _bid ) );
            return false;
        }

        auto & config = *img_hnd;

        glBindTexture( config.type, config.tex_id );
        if( gl4_log_error( "glBindTexture" ) ) return false;

        size_t const sib = confin.image().sib();
        GLenum const target = config.type;
        GLint const level = 0;
        GLsizei const width = GLsizei( confin.image().get_dims().x() );
        GLsizei const height = GLsizei( confin.image().get_dims().y() );
        GLsizei const depth = GLsizei( confin.image().get_dims().z() );
        GLenum const format =
            motor::platform::gl3::convert_to_gl_pixel_format( confin.image().get_image_format() );
        GLenum const type = motor::platform::gl3::convert_to_gl_pixel_type(
            confin.image().get_image_element_type() );
        void_cptr_t data = confin.image().get_image_ptr();

        // determine how to unpack the data
        // ideally, should be divideable by 4
        {
            GLint unpack = 0;

            if( width % 4 == 0 )
                unpack = 4;
            else if( width % 3 == 0 )
                unpack = 3;
            else if( width % 3 == 0 )
                unpack = 2;
            else
                unpack = 1;

            glPixelStorei( GL_UNPACK_ALIGNMENT, unpack );
            gl4_log_error( "glPixelStorei" );
        }

        if( is_config || ( sib == 0 || config.sib < sib ) )
        {
            GLint const border = 0;
            GLint const internal_format = motor::platform::gl3::convert_to_gl_format(
                confin.image().get_image_format(), confin.image().get_image_element_type() );

            if( target == GL_TEXTURE_2D )
            {
                glTexImage2D(
                    target, level, internal_format, width, height, border, format, type, data );
                gl4_log_error( "glTexImage2D" );
            }
            else if( target == GL_TEXTURE_2D_ARRAY )
            {
                glTexImage3D( target, level, internal_format, width, height, depth, border, format,
                    type, data );
                gl4_log_error( "glTexImage3D" );
            }
        }
        else
        {
            GLint const xoffset = 0;
            GLint const yoffset = 0;
            GLint const zoffset = 0;

            if( target == GL_TEXTURE_2D )
            {
                glTexSubImage2D(
                    target, level, xoffset, yoffset, width, height, format, type, data );
                gl4_log_error( "glTexSubImage2D" );
            }
            else if( target == GL_TEXTURE_2D_ARRAY )
            {
                glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth,
                    format, type, data );
                gl4_log_error( "glTexSubImage3D" );
            }
        }

        config.sib = confin.image().sib();

        return true;
    }

    //****************************************************************************************
    bool_t construct_array_data( motor::graphics::array_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid );
        auto const res =
            _arrays.access( oid, obj.name(), [ & ]( gl4_pimpl_data::array_data_ref_t data )
        {
            // buffer
            if( data.buf_id == GLuint( -1 ) )
            {
                GLuint id = GLuint( -1 );
                glGenBuffers( 1, &id );
                gl4_log_error3( "glGenBuffers", obj );
                data.buf_id = id;
            }

            // texture
            if( data.tex_id == GLuint( -1 ) )
            {
                GLuint id = GLuint( -1 );
                glGenTextures( 1, &id );
                gl4_log_error3( "glGenTextures", obj );

                data.tex_id = id;
            }
            return true;
        } );
        if( res ) obj.set_oid( _bid, oid );
        return res;
    }

    //****************************************************************************************
    bool_t release_array_data( size_t const oid ) noexcept
    {
        return _arrays.invalidate( oid );
    }

    //****************************************************************************************
    bool_t update( motor::graphics::array_object_ref_t obj, bool_t const is_config = false )
    {
        auto const [ a, b ] = _arrays.access< bool_t >( obj.get_oid( _bid ),
            [ & ]( gl4_pimpl_data::array_data_ref_t data )
        {
            // do buffer
            {
                // bind buffer
                glBindBuffer( GL_TEXTURE_BUFFER, data.buf_id );
                if( gl4_log_error3( "glBindBuffer", obj ) ) return false;

                // transfer data
                GLuint const sib = GLuint( obj.data_buffer().get_sib() );
                if( is_config || sib > data.sib )
                {
                    glBufferData(
                        GL_TEXTURE_BUFFER, sib, obj.data_buffer().data(), GL_DYNAMIC_DRAW );

                    if( gl4_log_error3( "glBufferData", obj ) ) return false;

                    data.sib = sib;
                }
                else
                {
                    glBufferSubData( GL_TEXTURE_BUFFER, 0, sib, obj.data_buffer().data() );
                    if( gl4_log_error3( "glBufferSubData", obj ) ) return false;
                }

                // bind buffer
                {
                    glBindBuffer( GL_TEXTURE_BUFFER, 0 );
                    if( gl4_log_error3( "glBindBuffer", obj ) ) return false;
                }
            }

            // do texture
            // glTexBuffer is required to be called after driver memory is aquired.
            {
                glBindTexture( GL_TEXTURE_BUFFER, data.tex_id );
                if( gl4_log_error3( "glBindTexture", obj ) ) return false;

                auto const le = obj.data_buffer().get_layout_element( 0 );
                glTexBuffer( GL_TEXTURE_BUFFER,
                    motor::platform::gl3::convert_for_texture_buffer( le.type, le.type_struct ),
                    data.buf_id );
                if( gl4_log_error3( "glTexBuffer", obj ) ) return false;
            }
            return true;
        } );
        return a && b;
    }

    //****************************************************************************************
    bool_t construct_feedback( motor::graphics::streamout_object_ref_t obj ) noexcept
    {
        auto fbd = _feedbacks.access( obj.get_oid( _bid ), obj.name() );

        size_t const req_buffers =
            std::min( obj.num_buffers(), gl4_pimpl_data::feedback_data::buffer::max_buffers );

        // rw : read/write - ping pong buffers
        for( size_t rw = 0; rw < 2; ++rw )
        {
            auto & buffer = fbd->_buffers[ rw ];

            for( size_t i = 0; i < req_buffers; ++i )
            {
                if( buffer.bids[ i ] != GLuint( -1 ) ) continue;

                motor::string_t const is = motor::to_string( i );
                motor::string_t const bs = motor::to_string( rw );

                auto [ res, hnd ] = this_t::construct_geometry(
                    obj.name() + ".feedback." + is + "." + bs, obj.get_buffer( i ) );

                if( !res || !hnd )
                {
                    motor::log::global::status< 1024 >(
                        "[gl4] : unable to create geometry for transform feedback [%s]",
                        obj.name().c_str() );
                    continue;
                }

                buffer.gids[ i ] = hnd.oid();
                buffer.sibs[ i ] = 0;

                _geometries.access( hnd.oid(),
                    [ & ]( gl4_pimpl_data::geo_data_ref_t d ) { buffer.bids[ i ] = d.vb_id; } );
            }

            // we just always gen max buffers tex ids
            {
                glGenTextures( gl4_pimpl_data::feedback_data::buffer::max_buffers, buffer.tids );
                if( gl4_log_error3( "glGenTextures", obj ) ) return false;
            }

            for( size_t i = req_buffers; i < gl4_pimpl_data::feedback_data::buffer::max_buffers;
                 ++i )
            {
                if( buffer.gids[ i ] == size_t( -1 ) ) break;

                this_t::release_geometry( buffer.gids[ i ] );

                buffer.bids[ i ] = GLuint( -1 );
                buffer.gids[ i ] = size_t( -1 );
            }

            if( buffer.qid == GLuint( -1 ) )
            {
                glGenQueries( 1, &buffer.qid );
                if( gl4_log_error3( "glGenQueries", obj ) ) return false;
            }

            if( buffer.tfid == GLuint( -1 ) )
            {
                glGenTransformFeedbacks( 1, &buffer.tfid );
                if( gl4_log_error3( "glGenTransformFeedbacks", obj ) ) return false;
            }
        }

        obj.set_oid( _bid, fbd.oid() );
        return true;
    }

    //****************************************************************************************
    bool_t release_tf_data( size_t const oid ) noexcept
    {
        _feedbacks.access( oid, [ & ]( gl4_pimpl_data::feedback_data_ref_t d )
        {
            for( size_t rw = 0; rw < 2; ++rw )
            {
                auto & buffer = d._buffers[ rw ];

                for( GLuint i = 0; i < gl4_pimpl_data::feedback_data_t::buffer::max_buffers; ++i )
                {
                    if( buffer.gids[ i ] == size_t( -1 ) ) break;

                    this_t::release_geometry( buffer.gids[ i ] );

                    buffer.bids[ i ] = GLuint( -1 );
                    buffer.sibs[ i ] = 0;
                }
            }
        } );
        return _feedbacks.invalidate( oid );
    }

    //****************************************************************************************
    bool_t update( motor::graphics::streamout_object_ref_t obj, bool_t const is_config = false )
    {
        auto const [ a, b ] = _feedbacks.access< bool_t >( obj.get_oid( _bid ),
            [ & ]( gl4_pimpl_data::feedback_data_ref_t data )
        {
            for( size_t rw = 0; rw < 2; ++rw )
            {
                auto & buffer = data._buffers[ rw ];

                size_t i = 0;
                while( buffer.bids[ i ] != GLuint( -1 ) )
                {
                    // bind buffer
                    {
                        glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, buffer.bids[ i ] );
                        if( gl4_log_error3( "glBindBuffer", obj ) ) continue;
                    }

                    // allocate data
                    GLuint const sib = GLuint( obj.get_buffer( i ).get_layout_sib() * obj.size() );
                    if( is_config || sib > buffer.sibs[ i ] )
                    {
                        glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, sib, nullptr, GL_DYNAMIC_DRAW );
                        if( gl4_log_error3( "glBufferData", obj ) ) continue;
                        buffer.sibs[ i ] = sib;
                    }

                    // do texture
                    // glTexBuffer is required to be called after driver memory is aquired.
                    {
                        glBindTexture( GL_TEXTURE_BUFFER, buffer.tids[ i ] );
                        if( gl4_log_error3( "glBindTexture", obj ) ) continue;

                        auto const le = obj.get_buffer( i ).get_layout_element_zero();
                        glTexBuffer( GL_TEXTURE_BUFFER,
                            motor::platform::gl3::convert_for_texture_buffer(
                                le.type, le.type_struct ),
                            buffer.bids[ i ] );
                        if( gl4_log_error3( "glTexBuffer", obj ) ) continue;

                        glBindTexture( GL_TEXTURE_BUFFER, 0 );
                        if( gl4_log_error3( "glBindTexture", obj ) ) continue;
                    }

                    ++i;
                }
            }

            // unbind
            {
                glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, 0 );
                if( gl4_log_error3( "glBindBuffer", obj ) ) return false;
            }

            for( size_t rw = 0; rw < 2; ++rw )
            {
                auto & buffer = data._buffers[ rw ];

                {
                    glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, buffer.tfid );
                    if( gl4_log_error3( "glBindTransformFeedback", obj ) ) return false;
                }

                // bind buffers
                {
                    for( size_t i = 0; i < gl4_pimpl_data::feedback_data::buffer::max_buffers; ++i )
                    {
                        auto const bid = buffer.bids[ i ];
                        auto const sib = buffer.sibs[ i ];

                        if( bid == GLuint( -1 ) ) break;

                        glBindBufferRange( GL_TRANSFORM_FEEDBACK_BUFFER, GLuint( i ), bid, 0, sib );
                        if( gl4_log_error3( "glBindTransformFeedback", obj ) ) return false;
                    }
                }
            }
            return true;
        } );
        return a && b;
    }

    //****************************************************************************************
    bool_t use_transform_feedback( size_t const oid ) noexcept
    {
        if( oid == _tf_active_id ) return true;
        _tf_active_id = oid;

        return true;
    }

    //****************************************************************************************
    void_t unuse_transform_feedback( void_t ) noexcept
    {
        _tf_active_id = size_t( -1 );
    }

    //****************************************************************************************
    void_t activate_transform_feedback( GLenum const gpt ) noexcept
    {
        if( _tf_active_id == size_t( -1 ) ) return;
        if( auto fbd = feedbacks_t::handle_t( _tf_active_id, _feedbacks ); fbd )
        {
            auto const wrt_idx = fbd->write_index();
            auto & buffer = fbd->_buffers[ wrt_idx ];

            // EITHER just set prim type what was used for rendering.
            // OR use for overwriting the primitive type on render
            // data.pt = gdata.pt ;

            // bind transform feedback object and update primitive type for
            // rendering the transform feedback data.
            {
                for( size_t i = 0; i < gl4_pimpl_data::feedback_data::buffer::max_buffers; ++i )
                {
                    if( buffer.bids[ i ] == GLuint( -1 ) ) break;

                    _geometries.access( buffer.gids[ i ],
                        [ & ]( gl4_pimpl_data::geo_data_ref_t d ) { d.pt = gpt; } );
                }

                glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, buffer.tfid );
                gl4_log_error( "glBindTransformFeedback" );
            }

            // query written primitives
            {
                glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, buffer.qid );
                gl4_log_error( "glBeginQuery" );
            }

            // begin
            {
                // lets just use the primitive type of the used geometry
                GLenum const mode = gpt;
                glBeginTransformFeedback( mode );
                gl4_log_error( "glBeginTransformFeedback" );
            }
        }
    }

    //****************************************************************************************
    void_t deactivate_transform_feedback( void_t ) noexcept
    {
        if( _tf_active_id == size_t( -1 ) ) return;

        {
            glEndTransformFeedback();
            gl4_log_error( "glEndTransformFeedback" );
        }

        // query written primitives
        {
            glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN );
            gl4_log_error( "glEndQuery : GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN" );
        }

        // swap the indices of the ping-pong buffers
        if( auto fbd = feedbacks_t::handle_t( _tf_active_id, _feedbacks ); fbd )
        {
            fbd->swap_index();
        }

// I believe this is for testing purposes.
#if 0
        GLuint num_prims = 0 ;
        {
            auto & tfd = _feedbacks[ _tf_active_id] ;
            auto const ridx = tfd.read_index();
            glGetQueryObjectuiv( tfd.read_buffer().qid, GL_QUERY_RESULT, &num_prims ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGetQueryObjectuiv" ) ) ;
        }
#endif

        _tf_active_id = size_t( -1 );
    }


    //
    // Update Variables
    // 

    //****************************************************************************************
    bool_t check_variable_connectivity(
        motor::graphics::render_object_ref_t ro, size_t const varset_id ) noexcept
    {
        if( auto rd = _renders.access( ro.get_oid( this_t::_bid ) ); rd )
        {
            auto [ hash, vs ] = ro.borrow_variable_set( varset_id );

            if( auto shd = shader_datas_t::handle_t( rd->shd_id, _shaders ); shd )
            {
                auto rdc = render_datas_t::checked_handle_t( rd );
                auto shdc = shader_datas_t::checked_handle_t( shd );

                if( rd->has_variable_set( varset_id ) &&
                    rd->has_variable_set_changed( varset_id, hash ) )
                {
                    this_t::reconnect( rdc, shdc, varset_id, hash, motor::share( vs ) );
                }
                else if( rd->has_not_variable_set( varset_id ) )
                {
                    this_t::connect( rdc, shdc, varset_id, hash, motor::share( vs ) );

                    this_t::render_object_variable_memory( rdc, shdc );
                }
            }
        }
        return true;
    }

    //****************************************************************************************
    bool_t update_variables( size_t const oid, size_t const varset_id ) noexcept
    {
        if( auto rd = render_datas_t::handle_t( oid, _renders ); rd )
        {
            if( auto shd = shader_datas_t::handle_t( rd->shd_id, _shaders ); shd )
            {
                return this_t::update_variables( render_datas_t::checked_handle_t( rd ),
                    shader_datas_t::checked_handle_t( shd ), varset_id );
            }
        }
        return false;
    }

    //****************************************************************************************
    bool_t update_variables( render_datas_t::checked_handle_t rd /*config*/,
        shader_datas_t::checked_handle_t shd, size_t const varset_id ) noexcept
    {
        gl4_pimpl_data::render_data_t::varset_to_idx_data entry;
        if( !rd->get_vs_to_idx_entry( varset_id, entry ) ) return false;

        {
            glUseProgram( shd->pg_id );
            if( gl4_log_error( "glUseProgram" ) ) return false;
        }

        // data vars
        {
            for( size_t i = entry.data_idx_begin; i < entry.data_idx_end; ++i )
            {
                auto const & link = rd->var_sets_data[ i ];

                auto & uv = shd->uniforms[ link.uniform_id ];
                uv.do_copy_funk( link.mem, link.var );
            }
        }

        // tex vars
        // this section must match with the section in the render function
        // because of the used texture unit
        {
            int_t tex_unit = 0;
            // textures
            {
                for( size_t i = entry.texture_idx_begin; i < entry.texture_idx_end; ++i )
                {
                    auto & link = rd->var_sets_texture[ i ];

                    auto var = motor::graphics::data_variable< int_t >( tex_unit );
                    auto & uv = shd->uniforms[ link.uniform_id ];

                    // automatic image update
                    if( link.img_hash != link.var->get().hash() )
                    {
                        size_t const img_idx = _images.find_by_name( link.var->get().name() );
                        if( img_idx == size_t( -1 ) )
                        {
                            motor::log::global::warning( "[gl4] : image variable value not found : "
                                                         "%s : Resetting to old name.",
                                link.var->get().name().c_str() );

                            _images.access( link.img_id, [ & ]( motor::string_in_t id_name,
                                                             gl4_pimpl_data::image_data_ref_t )
                            { link.var->set( id_name ); } );
                        }
                        else
                        {
                            _images.access( img_idx, [ & ]( gl4_pimpl_data::image_data_ref_t id )
                            {
                                link.img_hash = link.var->get().hash();
                                link.img_id = img_idx;
                                link.tex_id = id.tex_id;
                            } );
                        }
                    }

                    uv.do_copy_funk( link.mem, &var );

                    ++tex_unit;
                }
            }

            // array data vars
            {
                for( size_t i = entry.array_idx_begin; i < entry.array_idx_end; ++i )
                {
                    auto & link = rd->var_sets_array[ i ];

                    auto var = motor::graphics::data_variable< int_t >( tex_unit );
                    auto & uv = shd->uniforms[ link.uniform_id ];

                    uv.do_copy_funk( link.mem, &var );

                    ++tex_unit;
                }
            }

            // transform feedback bound vars
            {
                for( size_t i = entry.streamout_idx_begin; i < entry.streamout_idx_end; ++i )
                {
                    auto & link = rd->var_sets_streamout[ i ];

                    auto var = motor::graphics::data_variable< int_t >( tex_unit );
                    auto & uv = shd->uniforms[ link.uniform_id ];

                    uv.do_copy_funk( link.mem, &var );

                    ++tex_unit;
                }
            }
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////

    //****************************************************************************************
    // helper function requires context:
    // you need to know if a variable set is reconnecting.
    // at the moment, just really call if you know if a reconnect is happening.
    // a reconnection could happen, if a variable set is still there, but on the user side
    // the variable set has been replaced so the hash values differ.
    bool_t reconnect( render_datas_t::checked_handle_t rd,
        shader_datas_t::checked_handle_t shd, size_t const var_set_idx, size_t const hash,
        motor::graphics::variable_set_mtr_safe_t vs )
    {
        // enforce description - see function desc
        assert( rd->varset_to_idx.size() > var_set_idx );
        assert( rd->varset_to_idx[ var_set_idx ].idx != size_t( -1 ) );
        assert( rd->var_sets[ rd->varset_to_idx[ var_set_idx ].idx ].hash != hash );
        assert( rd->var_sets[ rd->varset_to_idx[ var_set_idx ].idx ].vs != nullptr );

        auto * old_vs = rd->via_indirection( var_set_idx ).vs;

        vs->clone_from_if_not_exist( &shd->default_values );

        // exchange data variables
        // the uniform link will be given the new variables from the new
        // variable set, which is basically a "reconect"
        {
            auto const s = rd->varset_to_idx[ var_set_idx ].data_idx_begin;
            auto const e = rd->varset_to_idx[ var_set_idx ].data_idx_end;

            motor::graphics::variable_set_t::data_variable_info_t dvi;
            for( size_t i = s; i < e; ++i )
            {
                auto * ptr = rd->var_sets_data[ i ].var;

                if( !old_vs->data_variable_name( ptr, dvi ) )
                {
                    motor::log::global::error< 1024 >(
                        "[reconnect] : count not find variable %s", dvi.name.data() );
                    rd->var_sets_data[ i ].var = nullptr;
                    continue;
                }
                rd->var_sets_data[ i ].var =
                    vs->data_variable( motor::string_t( dvi.name ), dvi.type, dvi.type_struct );
            }
        }

        // exchange texture variables
        {
            auto const s = rd->varset_to_idx[ var_set_idx ].texture_idx_begin;
            auto const e = rd->varset_to_idx[ var_set_idx ].texture_idx_end;

            std::string_view name;

            for( size_t i = s; i < e; ++i )
            {
                auto * ptr = rd->var_sets_texture[ i ].var;

                if( !old_vs->texture_variable_name( ptr, name ) )
                {
                    motor::log::global::error< 1024 >(
                        "[reconnect] : count not find variable %s", name.data() );
                    rd->var_sets_texture[ i ].var = nullptr;
                    continue;
                }
                rd->var_sets_texture[ i ].var = vs->texture_variable( motor::string_t( name ) );
            }
        }

        // exchange array variables
        {
            auto const s = rd->varset_to_idx[ var_set_idx ].array_idx_begin;
            auto const e = rd->varset_to_idx[ var_set_idx ].array_idx_end;

            for( size_t i = s; i < e; ++i )
            {
                auto & entry = rd->var_sets_array[ i ];
                auto const & uniform = shd->uniforms[ entry.uniform_id ];

                auto * var = vs->find_array_variable( uniform.name.c_str() );
                if( var == nullptr )
                {
                    motor::log::global::error( "[reconnect] : array variable missing" );
                    continue;
                }

                bool const found = _arrays.access_by_name( var->get().name(),
                    [ & ]( size_t const id, gl4_pimpl_data::array_data_ref_t data )
                {
                    entry.buf_id = id;
                    entry.tex_id = data.tex_id;
                } );

                if( !found )
                {
                    motor::log::global::error( "[reconnect] : array resource missing" );
                }
            }
        }

        // exchange streamout variables
        {
            auto const s = rd->varset_to_idx[ var_set_idx ].streamout_idx_begin;
            auto const e = rd->varset_to_idx[ var_set_idx ].streamout_idx_end;

            for( size_t i = s; i < e; ++i )
            {
                auto & entry = rd->var_sets_streamout[ i ];
                auto const & uniform = shd->uniforms[ entry.uniform_id ];

                auto * var = vs->find_streamout_variable( uniform.name.c_str() );
                if( var == nullptr )
                {
                    motor::log::global::error( "[reconnect] : streamout variable missing" );
                    continue;
                }

                bool const found = _feedbacks.access_by_name( var->get().name(),
                    [ & ]( size_t const id, gl4_pimpl_data::feedback_data_ref_t data )
                {
                    entry.so_id = id;
                    entry.tex_id[ 0 ] = data._buffers[ 0 ].tids[ 0 ];
                    entry.tex_id[ 1 ] = data._buffers[ 1 ].tids[ 0 ];
                } );

                if( !found )
                {
                    motor::log::global::error( "[reconnect] : streamout resource missing" );
                }
            }
        }

        {
            motor::release( motor::move( old_vs ) );

            auto & entry = rd->via_indirection( var_set_idx );
            entry.vs = motor::move( vs );
            entry.hash = hash;
        }

        return true;
    }

    //****************************************************************************************
    bool_t connect( render_datas_t::checked_handle_t rd,
        shader_datas_t::checked_handle_t shd, size_t const var_set_idx, size_t const hash,
        motor::graphics::variable_set_mtr_safe_t vs )
    {
        vs->clone_from_if_not_exist( &shd->default_values );

        gl4_pimpl_data::render_data_t::varset_to_idx_data vtidx_data;
        vtidx_data.data_idx_begin = rd->var_sets_data.size();
        vtidx_data.data_idx_end = rd->var_sets_data.size();

        vtidx_data.texture_idx_begin = rd->var_sets_texture.size();
        vtidx_data.texture_idx_end = rd->var_sets_texture.size();

        vtidx_data.array_idx_begin = rd->var_sets_array.size();
        vtidx_data.array_idx_end = rd->var_sets_array.size();

        vtidx_data.streamout_idx_begin = rd->var_sets_streamout.size();
        vtidx_data.streamout_idx_end = rd->var_sets_streamout.size();

        size_t id = 0;
        for( auto & uv : shd->uniforms )
        {
            // is it a data uniform variable?
            if( motor::ogl::uniform_is_data( uv.type ) )
            {
                auto const types = motor::platform::gl3::to_type_type_struct( uv.type );
                auto * var = vs->data_variable( uv.name, types.first, types.second );
                if( var == nullptr )
                {
                    motor::log::global_t::error( gl4_log( "can not claim variable " + uv.name ) );
                    continue;
                }

                gl4_pimpl_data::render_data::uniform_variable_link link;
                link.uniform_id = id++;
                link.var = var;

                rd->var_sets_data.emplace_back( link );

                ++vtidx_data.data_idx_end;
            }
            else if( motor::ogl::uniform_is_texture( uv.type ) )
            {
                // auto const types = motor::platform::gl3::to_type_type_struct( uv.type ) ;
                auto * var = vs->texture_variable( uv.name );

                if( var == nullptr )
                {
                    motor::log::global_t::error( gl4_log( "can not claim variable " + uv.name ) );
                    continue;
                }

                // looking for image
                {
                    auto const & tx_name = var->get();

                    gl4_pimpl_data::render_data::uniform_texture_link link;
                    link.uniform_id = id++;
                    link.img_hash = var->get().hash();
                    link.var = var;

                    auto const res = _images.access_by_name( tx_name.name(),
                        [ & ]( size_t const img_id, gl4_pimpl_data::image_data_ref_t d )
                    {
                        link.img_id = img_id;
                        link.tex_id = d.tex_id;
                    } );

                    if( !res )
                    {
                        motor::log::global_t::error< 1024 >(
                            "[gl4] : Could not find image [%s]", tx_name.name().c_str() );
                        continue;
                    }
                    rd->var_sets_texture.emplace_back( link );
                    ++vtidx_data.texture_idx_end;
                }
            }
            else if( motor::ogl::uniform_is_buffer( uv.type ) )
            {
                motor::string_t tx_name;

                {
                    tx_name = vs->array_variable( uv.name )->get().name();

                    if( tx_name.empty() )
                        tx_name = vs->array_variable_streamout( uv.name )->get().name();
                }

                // looking for data buffer
                auto handle_buffer_link = [ & ]( void_t )
                {
                    gl4_pimpl_data::render_data::uniform_array_data_link link;

                    auto const res = _arrays.access_by_name( tx_name,
                        [ & ]( size_t const id_, gl4_pimpl_data::array_data_ref_t d )
                    {
                        link.buf_id = id_;
                        link.tex_id = d.tex_id;
                    } );
                    if( !res ) return false;

                    link.uniform_id = id++;

                    rd->var_sets_array.emplace_back( link );
                    ++vtidx_data.array_idx_end;
                    return true;
                };

                // looking for streamout/transform feedback
                auto handle_feedback_link = [ & ]( void_t )
                {
                    gl4_pimpl_data::render_data::uniform_streamout_link link;

                    auto const res = _feedbacks.access_by_name( tx_name,
                        [ & ]( size_t const id_, gl4_pimpl_data::feedback_data_ref_t d )
                    {
                        link.tex_id[ 0 ] = d._buffers[ 0 ].tids[ 0 ];
                        link.tex_id[ 1 ] = d._buffers[ 1 ].tids[ 0 ];
                        link.so_id = id_;
                    } );
                    if( !res ) return false;

                    link.uniform_id = id++;
                    rd->var_sets_streamout.emplace_back( link );
                    ++vtidx_data.streamout_idx_end;
                    return true;
                };

                if( !handle_buffer_link() )
                {
                    if( !handle_feedback_link() )
                    {
                        motor::log::global_t::error< 2048 >(
                            "[gl4] : Could not find array nor streamout object [%s]",
                            tx_name.c_str() );
                        continue;
                    }
                }
            }
        }

        // ref count one copy here for all stored items
        {
            if( rd->varset_to_idx.size() <= var_set_idx )
                rd->varset_to_idx.resize( var_set_idx + 1 );

            size_t idx = size_t( -1 );
            while( ++idx < rd->var_sets.size() && rd->var_sets[ idx ].is_valid() );

            if( idx == rd->var_sets.size() ) rd->var_sets.resize( idx + 1 );

            vtidx_data.idx = idx;

            rd->varset_to_idx[ var_set_idx ] = std::move( vtidx_data );
            rd->var_sets[ idx ] = gl4_pimpl_data::render_data::variable_set{
                hash, /*var_set_idx,*/ motor::move( vs ) };
        }

        return true;
    }

    //****************************************************************************************
    void_t render_object_variable_memory(
        render_datas_t::checked_handle_t rd, shader_datas_t::checked_handle_t shd ) noexcept
    {
        // construct memory block
        {
            size_t sib = 0;
            for( auto & link : rd->var_sets_data )
            {
                auto & uv = shd->uniforms[ link.uniform_id ];
                sib += motor::ogl::uniform_size_of( uv.type );
            }

            for( auto & link : rd->var_sets_texture )
            {
                auto & uv = shd->uniforms[ link.uniform_id ];
                sib += motor::ogl::uniform_size_of( uv.type );
            }

            for( auto & link : rd->var_sets_array )
            {
                auto & uv = shd->uniforms[ link.uniform_id ];
                sib += motor::ogl::uniform_size_of( uv.type );
            }

            for( auto & link : rd->var_sets_streamout )
            {
                auto & uv = shd->uniforms[ link.uniform_id ];
                sib += motor::ogl::uniform_size_of( uv.type );
            }

            motor::memory::global_t::dealloc( rd->mem_block );
            rd->mem_block =
                motor::memory::global_t::alloc( sib, "[gl3] : render config memory block" );
        }

        // assign memory locations
        {
            void_ptr_t mem = rd->mem_block;
            for( auto & link : rd->var_sets_data )
            {
                link.mem = mem;

                auto & uv = shd->uniforms[ link.uniform_id ];
                mem = reinterpret_cast< void_ptr_t >(
                    size_t( mem ) + size_t( motor::ogl::uniform_size_of( uv.type ) ) );
            }

            for( auto & link : rd->var_sets_texture )
            {
                link.mem = mem;

                auto & uv = shd->uniforms[ link.uniform_id ];
                mem = reinterpret_cast< void_ptr_t >(
                    size_t( mem ) + size_t( motor::ogl::uniform_size_of( uv.type ) ) );
            }

            for( auto & link : rd->var_sets_array )
            {
                link.mem = mem;

                auto & uv = shd->uniforms[ link.uniform_id ];
                mem = reinterpret_cast< void_ptr_t >(
                    size_t( mem ) + size_t( motor::ogl::uniform_size_of( uv.type ) ) );
            }

            for( auto & link : rd->var_sets_streamout )
            {
                link.mem = mem;

                auto & uv = shd->uniforms[ link.uniform_id ];
                mem = reinterpret_cast< void_ptr_t >(
                    size_t( mem ) + size_t( motor::ogl::uniform_size_of( uv.type ) ) );
            }
        }
    }

    //****************************************************************************************
    bool_t render( size_t const id, size_t const geo_idx = 0, bool_t feed_from_tf = false,
        bool_t const use_streamout_count = false, size_t const varset_id = size_t( 0 ),
        GLsizei const start_element = GLsizei( 0 ), GLsizei const num_elements = GLsizei( -1 ) )
    {
        MOTOR_PROBE( "Graphics", "[gl4] : render" );

        auto rd = _renders.access( id );
        if( !rd )
        {
            motor::log::global::warning< 1024 >(
                "[gl4] : do not know render data with oid(%d)", id );
            return false;
        }

        auto shd = _shaders.access( rd->shd_id );
        if( !shd )
        {
            motor::log::global::warning< 1024 >(
                "[gl4] : do not know shader data with oid(%d)", rd->shd_id );
            return false;
        }

        if( !shd->is_linkage_ok ) return false;

        size_t gid = size_t( -1 );
        size_t tfid = size_t( -1 );

        // #1 : check feedback geometry
        if( ( feed_from_tf || use_streamout_count ) && rd->tf_ids.size() != 0 )
        {
            tfid = rd->tf_ids[ 0 ];
            if( feed_from_tf )
            {
                _feedbacks.access( tfid, [ & ]( gl4_pimpl_data::feedback_data_ref_t d )
                { gid = d.read_buffer().gids[ 0 ]; } );
            }
            else
                gid = rd->geo_ids[ geo_idx ].id;
        }
        // #2 : check geometry
        else if( rd->geo_ids.size() > geo_idx )
        {
            gid = rd->geo_ids[ geo_idx ].id;
        }
        // #3 : otherwise we can not render
        else
        {
            motor::log::global::error< 1024 >( "[gl4] : used geometry idx invalid because"
                                               "exceeds array size for render object : %s",
                rd.name().c_str() );

            return false;
        }

        // we know that gid is cached, so just construct a handle here.
        auto gd = geo_datas_t::handle_t( gid, _geometries );
        if( !gd )
        {
            motor::log::global::error< 1024 >(
                "[gl4] : can not find geometry id %d for %s", gid, rd.name().c_str() );
            return false;
        }

        // find vao
        {
            GLuint vao_id = GLuint( 0 );

            {
                size_t vao_free = size_t( -1 );
                size_t vao_idx = size_t( -1 );
                while(
                    ++vao_idx != rd->geo_to_vaos.size() && rd->geo_to_vaos[ vao_idx ].gid != gid )
                {
                    if( vao_free != size_t( -1 ) ) continue;
                    if( rd->geo_to_vaos[ vao_idx ].gid == size_t( -1 ) ) vao_free = vao_idx;
                }

                if( vao_idx == rd->geo_to_vaos.size() )
                {
                    {
                        glGenVertexArrays( 1, &vao_id );
                        if( motor::ogl::error::check_and_log( gl4_log( "Vertex Array creation" ) ) )
                            return false;
                    }

                    // bind vertex attributes to shader locations for vao
                    if( this_t::bind_attributes( vao_id, geo_datas_t::checked_handle_t( gd ),
                            shader_datas_t::checked_handle_t( shd ) ) )
                    {
                        if( vao_free == size_t( -1 ) )
                        {
                            rd->geo_to_vaos.emplace_back(
                                gl4_pimpl_data::render_data::geo_to_vao{ gid, vao_id } );
                        }
                        else
                        {
                            rd->geo_to_vaos[ vao_free ] =
                                gl4_pimpl_data::render_data::geo_to_vao{ gid, vao_id };
                        }
                    }
                }
                else
                    vao_id = rd->geo_to_vaos[ vao_idx ].vao;
            }

            {
                glBindVertexArray( vao_id );
                if( motor::ogl::error::check_and_log( gl4_log( "glBindVertexArray" ) ) )
                    return false;
            }
        }

        {
            glUseProgram( shd->pg_id );
            if( motor::ogl::error::check_and_log( gl4_log( "glUseProgram" ) ) )
            {
                glBindVertexArray( 0 );
                return false;
            }
        }

        // if( config.var_sets.size() > varset_id )
        if( rd->has_variable_set( varset_id ) )
        {
            auto const & vtidx = rd->varset_to_idx[ varset_id ];

            // data vars
            {
                for( size_t i = vtidx.data_idx_begin; i < vtidx.data_idx_end; ++i )
                {
                    auto const & link = rd->var_sets_data[ i ];

                    auto & uv = shd->uniforms[ link.uniform_id ];
                    if( !uv.do_uniform_funk( link.mem ) )
                    {
                        motor::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." );
                    }
                }
            }

            // tex vars
            // this section must match with the section in the update(render_object)
            // function
            {
                int_t tex_unit = 0;
                // textures
                {
                    for( size_t i = vtidx.texture_idx_begin; i < vtidx.texture_idx_end; ++i )
                    {
                        auto const & link = rd->var_sets_texture[ i ];

                        glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) );
                        gl4_log_error( "glActiveTexture" );

                        if( auto imgd = _images.access( link.img_id ); imgd )
                        {
                            glBindTexture( imgd->type, link.tex_id );
                            gl4_log_error( "glBindTexture" );

                            glTexParameteri( imgd->type, GL_TEXTURE_WRAP_S, imgd->wrap_types[ 0 ] );
                            glTexParameteri( imgd->type, GL_TEXTURE_WRAP_T, imgd->wrap_types[ 1 ] );
                            glTexParameteri( imgd->type, GL_TEXTURE_WRAP_R, imgd->wrap_types[ 2 ] );
                            glTexParameteri(
                                imgd->type, GL_TEXTURE_MIN_FILTER, imgd->filter_types[ 0 ] );
                            glTexParameteri(
                                imgd->type, GL_TEXTURE_MAG_FILTER, imgd->filter_types[ 1 ] );
                            gl4_log_error( "glTexParameteri" );
                        }

                        {
                            auto & uv = shd->uniforms[ link.uniform_id ];
                            if( !uv.do_uniform_funk( link.mem ) )
                            {
                                motor::log::global_t::error(
                                    "[gl4] : uniform " + uv.name + " failed." );
                            }
                        }

                        ++tex_unit;
                    }
                }

                // array data vars
                {
                    for( size_t i = vtidx.array_idx_begin; i < vtidx.array_idx_end; ++i )
                    {
                        auto const & link = rd->var_sets_array[ i ];

                        glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) );
                        motor::ogl::error::check_and_log( gl4_log( "glActiveTexture" ) );
                        glBindTexture( GL_TEXTURE_BUFFER, link.tex_id );
                        motor::ogl::error::check_and_log( gl4_log( "glBindTexture" ) );

                        {
                            auto & uv = shd->uniforms[ link.uniform_id ];
                            if( !uv.do_uniform_funk( link.mem ) )
                            {
                                motor::log::global_t::error(
                                    "[gl4] : uniform " + uv.name + " failed." );
                            }
                        }

                        ++tex_unit;
                    }
                }

                // transform feedback as TBO
                {
                    for( size_t i = vtidx.streamout_idx_begin; i < vtidx.streamout_idx_end; ++i )
                    {
                        auto & link = rd->var_sets_streamout[ i ];

                        // auto const & tfd = _feedbacks[ link.so_id ] ;
                        auto const [ a, b ] = _feedbacks.access< bool_t >( link.so_id,
                            [ & ](
                                motor::string_in_t name, gl4_pimpl_data::feedback_data_ref_t tfd )
                        {
                            GLuint const tid = link.tex_id[ tfd.read_index() ];

                            glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) );
                            if( gl4_log_error2( "glActiveTexture", name.c_str() ) ) return false;
                            glBindTexture( GL_TEXTURE_BUFFER, tid );
                            if( gl4_log_error2( "glBindTexture", name.c_str() ) ) return false;

                            return true;
                        } );

                        if( !( a && b ) ) return false;
                        {
                            auto & uv = shd->uniforms[ link.uniform_id ];
                            if( motor::log::global_t::error_if< 2048 >(
                                    !uv.do_uniform_funk( link.mem ), "[gl4] : uniform %s failed.",
                                    uv.name.c_str() ) )
                            {
                                return false;
                            }
                        }
                        ++tex_unit;
                    }
                }
            }
        }

        // disable raster if no pixel shader
        if( shd->ps_id == GLuint( -1 ) )
        {
            glEnable( GL_RASTERIZER_DISCARD );
            gl4_log_error( "glEnable : GL_RASTERIZER_DISCARD" );
        }

        auto const g_pt = gd->pt;
        {
            this_t::activate_transform_feedback( g_pt );
        }

        // render section
        {
            GLenum const pt = g_pt;
            // GLuint const ib = gconfig.ib_id ;
            // GLuint const vb = config.geo->vb_id ;

            if( tfid != size_t( -1 ) )
            {
                if( auto fbd = _feedbacks.access( tfid ); fbd )
                {
#if 0 // this is gl 3.x
                    GLuint num_prims = 0 ;
                    {
                        glGetQueryObjectuiv( fbd->read_buffer().qid, GL_QUERY_RESULT, &num_prims ) ;
                        motor::ogl::error::check_and_log( gl4_backend_log( "glGetQueryObjectuiv" ) ) ;
                    }
                    glDrawArrays( pt, start_element, num_prims * motor::platform::gl3::primitive_type_to_num_vertices( pt ) ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDrawArrays" ) ) ;
#else
                    glDrawTransformFeedback( pt, fbd->read_buffer().tfid );
                    gl4_log_error( "glDrawTransformFeedback" );
#endif
                }
            }
            else
            {
                if( gd->num_elements_ib > 0 )
                {
                    GLsizei const max_elems = GLsizei( gd->num_elements_ib );
                    GLsizei const ne =
                        std::min( num_elements >= 0 ? num_elements : max_elems, max_elems );

                    GLenum const glt = gd->ib_type;

                    void_cptr_t offset = void_cptr_t(
                        byte_cptr_t( nullptr ) + start_element * GLsizei( gd->ib_elem_sib ) );

                    glDrawElements( pt, ne, glt, offset );
                    gl4_log_error( "glDrawElements" );
                }
                else
                {
                    GLsizei const max_elems = GLsizei( gd->num_elements_vb );
                    GLsizei const ne =
                        std::min( num_elements >= 0 ? num_elements : max_elems, max_elems );

                    glDrawArrays( pt, start_element, ne );
                    gl4_log_error( "glDrawArrays" );
                }
            }
        }

        {
            this_t::deactivate_transform_feedback();
        }

        // enable raster back again.
        if( shd->ps_id == GLuint( -1 ) )
        {
            glDisable( GL_RASTERIZER_DISCARD );
            gl4_log_error( "glDisable : GL_RASTERIZER_DISCARD" );
        }

        {
            glBindVertexArray( 0 );
            gl4_log_error( "glBindVertexArray" );
        }

        return true;
    }

    //****************************************************************************************
    void_t begin_frame( void_t )
    {
        // set default render states
        {
            auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) );

            _states.access( ids_new.first, [ & ]( gl4_pimpl_data::state_data_ref_t s )
            {
                // set the viewport to the default new state,
                // so the correct viewport is set automatically.
                {
                    motor::math::vec4ui_t vp = s.states[ ids_new.second ].view_s.ss.vp;
                    vp.z( uint_t( vp_width ) );
                    vp.w( uint_t( vp_height ) );
                    s.states[ ids_new.second ].view_s.ss.vp = vp;
                }

                _state_stack.push( s.states[ ids_new.second ] );
            } );
        }

        // do clear the frame
        {
            motor::math::vec4f_t const clear_color = _state_stack.top().clear_s.ss.clear_color;
            glClearColor( clear_color.x(), clear_color.y(), clear_color.z(), clear_color.w() );
            motor::ogl::error::check_and_log( gl4_log( "glClearColor" ) );

            GLbitfield const color = GL_COLOR_BUFFER_BIT;
            GLbitfield const depth = GL_DEPTH_BUFFER_BIT;

            glClear( color | depth );
            motor::ogl::error::check_and_log( gl4_log( "glEnable" ) );
        }
    }

    //****************************************************************************************
    void_t end_frame( void_t )
    {
        glFlush();

        _ctsd->for_each( [ & ]( gl4_independent::backward_message & item )
        {
            std::visit( [ this ]( auto && arg )
            {
                using T = std::decay_t< decltype( arg ) >;
                if constexpr( std::is_same_v< T, gl4_independent::compilation_result > )
                {
                    this_t::construct_msl_data( std::move( arg ) );
                }
                else if constexpr( std::is_same_v< T, gl4_independent::release_result > )
                {
                }
                else
                    static_assert( false, "non-exhaustive visitor!" );
            }, item );
        } );

        glFlush();

        _state_stack.pop();

        _ctsd->notify_thread();
    }
};

//************************************************************************************************
//
//
//************************************************************************************************

//******************************************************************************************************
gl4_backend::gl4_backend( motor::platform::opengl::rendering_context_ptr_t ctx ) noexcept
{
    _pimpl =
        motor::memory::global_t::alloc( pimpl( this_t::get_bid(), ctx ), "gl4_backend::pimpl" );
    _context = ctx;
}

//******************************************************************************************************
gl4_backend::gl4_backend( this_rref_t rhv ) noexcept : backend( std::move( rhv ) )
{
    _pimpl = motor::move( rhv._pimpl );
    _context = motor::move( rhv._context );
}

//******************************************************************************************************
gl4_backend::~gl4_backend( void_t )
{
    motor::memory::global_t::dealloc( _pimpl );
}

//******************************************************************************************************
void_t gl4_backend::set_window_info( window_info_cref_t wi ) noexcept
{
    {
        // bool_t change = false ;
        if( wi.width != 0 )
        {
            _pimpl->vp_width = GLsizei( wi.width );
            // change = true ;
        }
        if( wi.height != 0 )
        {
            _pimpl->vp_height = GLsizei( wi.height );
            // change = true ;
        }
    }
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::msl_object_mtr_t obj ) noexcept
{
    if( obj == nullptr )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->send_configure_to_st( obj );
    return motor::graphics::result::in_transit;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure(
    motor::graphics::geometry_object_mtr_t obj ) noexcept
{
    auto [ res, hnd ] = _pimpl->construct_geometry( *obj );

    if( !res || !hnd ) return motor::graphics::result::failed;

    if( !_pimpl->update( obj, true ) )
    {
        return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::render_object_mtr_t obj ) noexcept
{
#if 0
    size_t const oid = obj->set_oid( this_t::get_bid(), _pimpl->construct_render_data( 
        obj->get_oid( this_t::get_bid() ), *obj ) ) ;
#endif

    auto const res = _pimpl->construct_render_data( *obj );
    return res ? motor::graphics::result::ok : motor::graphics::result::failed;
}

//******************************************************************************************************
// compiles directly. Do not use at run-time. Application initialization is ok though
// @todo put shader compilation in support thread.
motor::graphics::result gl4_backend::configure( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    auto const res = _pimpl->construct_shader_data( *obj );
    return res ? motor::graphics::result::ok : motor::graphics::result::failed;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::image_object_mtr_t obj ) noexcept
{
    auto const res = _pimpl->construct_image_config( *obj );
    if( !res )
    {
        return motor::graphics::result::failed;
    }

    if( !_pimpl->update( *obj, true ) )
    {
        return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure(
    motor::graphics::framebuffer_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        gl4_log_error( "[configure] : Object must be valid and requires a name" );
        return motor::graphics::result::invalid_argument;
    }

    auto const res = _pimpl->construct_framebuffer( *obj );

    return res ? motor::graphics::result::ok : motor::graphics::result::failed;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    auto const res = _pimpl->construct_state( *obj );
    return res ? motor::graphics::result::ok : motor::graphics::result::failed;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::array_object_mtr_t obj ) noexcept
{
    auto const res = _pimpl->construct_array_data( *obj );
    if( !res ) return motor::graphics::result::failed;

    if( !_pimpl->update( *obj, true ) ) return motor::graphics::result::failed;

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure(
    motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    auto const res = _pimpl->construct_feedback( *obj );
    if( !res ) return motor::graphics::result::failed;

    if( !_pimpl->update( *obj, true ) ) return motor::graphics::result::failed;

    return motor::graphics::result::ok;
}

//************************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::msl_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "msl object is nullptr or has no name" ) );
        return motor::graphics::result::invalid_argument;
    }

    _pimpl->release_msl_data( obj->get_oid( this_t::get_bid() ) );
    obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    obj->borrow_render_object()->set_oid( this_t::get_bid(), size_t( -1 ) );

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::geometry_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->release_geometry( obj->get_oid( this_t::get_bid() ) );
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::render_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->release_render_data( *obj );
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->release_shader_data( obj->get_oid( this_t::get_bid() ) );
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::image_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->release_image_data( obj->get_oid( this_t::get_bid() ) );
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release(
    motor::graphics::framebuffer_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->release_framebuffer( obj->get_oid( this_t::get_bid() ) );
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        //_pimpl->relese( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::array_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->release_array_data( obj->get_oid( this_t::get_bid() ) );
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) );
        return motor::graphics::result::invalid_argument;
    }

    {
        _pimpl->release_tf_data( obj->get_oid( this_t::get_bid() ) );
        obj->set_oid( this_t::get_bid(), size_t( -1 ) );
    }

    return motor::graphics::result::ok;
}

#if 0
//******************************************************************************************************
motor::graphics::result gl4_backend::connect( motor::graphics::render_object_mtr_t obj, 
    motor::graphics::variable_set_mtr_t vs ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        motor::log::global_t::error( gl4_backend_log( "invalid render configuration id" ) ) ;
        return motor::graphics::result::failed ;
    }

    auto const res = _pimpl->connect( oid, vs ) ;
    motor::log::global_t::error( !res, 
        gl4_backend_log( "connect variable set" ) ) ;
   
    return motor::graphics::result::ok ;
}
#endif

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::geometry_object_mtr_t obj ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        motor::log::global_t::error( gl4_log( "invalid geometry configuration id" ) );
        return motor::graphics::result::failed;
    }

    auto const res = _pimpl->update( obj );
    motor::log::global_t::error( !res, gl4_log( "update geometry" ) );

    return motor::graphics::result::ok;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    {
        auto const res = _pimpl->update( *obj, false );
        if( !res ) return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::array_object_mtr_t obj ) noexcept
{
    {
        auto const res = _pimpl->update( *obj, false );
        if( !res ) return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::image_object_mtr_t obj ) noexcept
{
    if( !_pimpl->update( *obj, false ) )
    {
        return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update(
    motor::graphics::render_object_mtr_t obj, size_t const varset ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() );

    {
        auto const res = _pimpl->update_variables( oid, varset );
        if( !res ) return motor::graphics::result::failed;
    }

    return motor::graphics::result::ok;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::use( motor::graphics::framebuffer_object_mtr_t obj ) noexcept
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

//*******************************************************************************************
motor::graphics::result gl4_backend::use( motor::graphics::streamout_object_mtr_t obj ) noexcept
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

    auto const res = _pimpl->use_transform_feedback( oid );
    if( !res ) return motor::graphics::result::failed;

    return motor::graphics::result::ok;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::unuse(
    motor::graphics::gen4::backend::unuse_type const t ) noexcept
{
    switch( t )
    {
    case motor::graphics::gen4::backend::unuse_type::framebuffer:
        _pimpl->deactivate_framebuffer();
        break;
    case motor::graphics::gen4::backend::unuse_type::streamout:
        _pimpl->unuse_transform_feedback();
        break;
    }

    return motor::graphics::result::ok;
}

//*************************************************************************************
motor::graphics::result gl4_backend::push(
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

    _pimpl->update_state( *obj );
    _pimpl->handle_render_state( oid, sid );

    return motor::graphics::result::ok;
}

//*****************************************************************************************
motor::graphics::result gl4_backend::pop( motor::graphics::gen4::backend::pop_type const ) noexcept
{
    _pimpl->handle_render_state( size_t( -1 ), size_t( -1 ) );
    return motor::graphics::result::ok;
}

//***************************************************************************************
motor::graphics::result gl4_backend::render( motor::graphics::render_object_mtr_t obj,
    motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept
{
    // motor::log::global_t::status( gl4_backend_log("render") ) ;

    size_t const oid = obj->get_oid( this_t::get_bid() );
    if( oid == size_t( -1 ) )
    {
        motor::log::global_t::error( gl4_log( "invalid id" ) );
        return motor::graphics::result::failed;
    }

    if( !_pimpl->check_link_status( oid ) )
    {
        motor::log::global_t::error( gl4_log( "shader did not compile. Abort render." ) );
        return motor::graphics::result::failed;
    }

    // handle dynamic variable sets
    {
        auto const res = _pimpl->check_variable_connectivity( *obj, detail.varset );
        if( !res ) return motor::graphics::result::failed;
    }

    // update variables before render
    {
        auto const res = _pimpl->update_variables( oid, detail.varset );
        if( !res ) return motor::graphics::result::failed;
    }

    {
        auto const res = _pimpl->update_geometry_link( *obj, detail.geo );
        if( !res ) return motor::graphics::result::failed;
    }

    // @todo
    // change per object render states here.
    _pimpl->render( oid, detail.geo, detail.feed_from_streamout, detail.use_streamout_count,
        detail.varset, (GLsizei)detail.start, (GLsizei)detail.num_elems );

    return motor::graphics::result::ok;
}

//*************************************************************************************
motor::graphics::result gl4_backend::render( motor::graphics::msl_object_mtr_t obj,
    motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept
{
    if( obj == nullptr ) return motor::graphics::result::invalid_argument;

    size_t const oid = obj->get_oid( this_t::get_bid() );

    if( oid == size_t( -1 ) )
    {
        motor::log::global_t::error( gl4_log( "invalid id" ) );
        return motor::graphics::result::failed;
    }

    auto const [ a, b ] = _pimpl->_msls.try_access< motor::graphics::result >( oid,
        [ & ]( gl4_pimpl_data::msl_data_ref_t msl )
    { return this_t::render( obj->borrow_render_object(), detail ); } );

    return ( a && b == motor::graphics::result::ok ) ? b : motor::graphics::result::failed;
}

//*************************************************************************************
void_t gl4_backend::render_begin( void_t ) noexcept
{
    _pimpl->begin_frame();
}

//*************************************************************************************
void_t gl4_backend::render_end( void_t ) noexcept
{
    _pimpl->end_frame();
}

//*************************************************************************************
void_t gl4_backend::on_context_destruction( void_t ) noexcept
{
    _pimpl->on_context_destruction();
}

//*************************************************************************************
void_t gl4_backend::commit_frame_snapshot( void_t ) noexcept {}