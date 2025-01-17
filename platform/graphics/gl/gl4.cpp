

#include "gl4.h"
#include "gl4_convert.h"

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

#include <motor/ogl/gl/gl.h>
#include <motor/ogl/gl/convert.hpp>
#include <motor/ogl/gl/error.hpp>
#include <motor/memory/global.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/std/vector>
#include <motor/std/stack>
#include <motor/std/string_split.hpp>


#define gl4_backend_log( text ) "[GL4] : " text

using namespace motor::platform::gen4 ;
using namespace motor::ogl ;

struct gl4_backend::pimpl
{
    motor_this_typedefs( pimpl ) ;

    //********************************************************************************
    // transform feedback buffers
    struct tf_data
    {
        motor_this_typedefs( tf_data ) ;

        bool_t valid = false ;
        motor::string_t name ;

        struct buffer
        {
            static constexpr size_t max_buffers = 4 ;

            size_t gids[max_buffers] = { size_t(-1), size_t(-1), size_t(-1), size_t(-1) }  ;

            // buffer ids
            GLuint bids[max_buffers] = { GLuint(-1), GLuint(-1), GLuint(-1), GLuint(-1) }  ;
            GLuint sibs[max_buffers] = { 0, 0, 0, 0 } ;

            // texture ids for TBOs
            GLuint tids[max_buffers] = { GLuint(-1), GLuint(-1), GLuint(-1), GLuint(-1) }  ;

            // query object id
            GLuint qid = GLuint(-1)  ;

            // transform feedback object ids
            GLuint tfid = GLuint(-1) ;
        };
        buffer _buffers[2] ;

        // if the tf is read and written at the 
        // same time, this index is used for ping-ponging
        // the id array within a buffer.
        size_t _ridx = 0 ;

        // must be derived from the geometry object
        // that is rendered into the feedback buffer.
        // this could be used to overwrite whats set in the 
        // geometry data of the transform feedback buffers.
        //GLenum pt = 0 ;

        // for tf reconfig - render data ids
        motor::vector< size_t > rd_ids ;
        void_t remove_render_data_id( size_t const rid ) noexcept
        {
            if( rid == size_t( -1 ) ) return ;
            for( auto & id : rd_ids ) if( id == rid ) { id = size_t( -1 ) ; break ; }
        }

        void_t add_render_data_id( size_t const rid ) noexcept
        {
            if( rid == size_t( -1 ) ) return ;
            for( auto& id : rd_ids ) if( id == size_t( -1 ) ) { id = rid ; return ; }
            rd_ids.push_back( rid ) ;
        }

        void_t swap_index( void_t ) noexcept
        {
            _ridx = (_ridx+1) & 1 ;
        }

        size_t write_index( void_t ) const noexcept
        {
            return (_ridx + 1) & 1 ;
        }

        size_t read_index( void_t ) const noexcept
        {
            return _ridx ;
        }

        buffer & read_buffer( void_t ) noexcept { return _buffers[ this_t::read_index() ] ; }
        buffer const & read_buffer( void_t ) const noexcept { return _buffers[ this_t::read_index() ] ; }
        buffer & write_buffer( void_t ) noexcept { return _buffers[ this_t::write_index() ] ; }
        buffer const & write_buffer( void_t ) const noexcept { return _buffers[ this_t::write_index() ] ; }
    };
    motor_typedef( tf_data ) ;

    //********************************************************************************
    struct geo_data
    {
        bool_t valid = false ;
        motor::string_t name ;

        GLuint vb_id = GLuint( -1 ) ;
        GLuint ib_id = GLuint( -1 ) ;

        size_t num_elements_vb = 0 ;
        size_t num_elements_ib = 0 ;

        size_t sib_vb = 0 ;
        size_t sib_ib = 0 ;

        struct layout_element
        {
            motor::graphics::vertex_attribute va ;
            motor::graphics::type type ;
            motor::graphics::type_struct type_struct ;

            GLuint sib( void_t ) const noexcept
            {
                return GLuint( motor::graphics::size_of( type ) *
                    motor::graphics::size_of( type_struct ) ) ;
            }
        };
        motor::vector< layout_element > elements ;

        // per vertex sib
        GLuint stride = 0 ;

        GLenum ib_type ;
        size_t ib_elem_sib = 0  ;
        GLenum pt ;

        // for geo reconfig - render data ids
        motor::vector< size_t > rd_ids ;
        void_t remove_render_data_id( size_t const rid ) noexcept
        {
            if( rid == size_t( -1 ) ) return ;
            for( auto & id : rd_ids ) if( id == rid ) { id = size_t( -1 ) ; break ; }
        }

        void_t add_render_data_id( size_t const rid ) noexcept
        {
            if( rid == size_t( -1 ) ) return ;
            for( auto& id : rd_ids ) if( id == size_t( -1 ) ) { id = rid ; return ; }
            rd_ids.push_back( rid ) ;
        }
    };
    motor_typedef( geo_data ) ;

    //********************************************************************************
    struct shader_data
    {
        bool_t valid = false ;
        motor::string_t name ;

        GLuint vs_id = GLuint( -1 ) ;
        GLuint gs_id = GLuint( -1 ) ;
        GLuint ps_id = GLuint( -1 ) ;
        GLuint pg_id = GLuint( -1 ) ;

        bool_t is_linkage_ok = false ;

        struct vertex_input_binding
        {
            motor::graphics::vertex_attribute va ;
            motor::string_t name ;
        };
        motor::vector< vertex_input_binding > vertex_inputs ;

        struct vertex_output_binding
        {
            motor::graphics::vertex_attribute va ;
            motor::string_t name ;
        };
        motor::vector< vertex_output_binding > vertex_outputs ;
        char const ** output_names = nullptr ;

        bool_t find_vertex_input_binding_by_name( motor::string_cref_t name_,
            motor::graphics::vertex_attribute& va ) const noexcept
        {
            auto iter = std::find_if( vertex_inputs.begin(), vertex_inputs.end(),
                [&] ( vertex_input_binding const& b )
            {
                return b.name == name_ ;
            } ) ;
            if( iter == vertex_inputs.end() ) return false ;

            va = iter->va ;

            return true ;
        }

        bool_t find_vertex_output_binding_by_name( motor::string_cref_t name_,
            motor::graphics::vertex_attribute& va ) const noexcept
        {
            auto iter = std::find_if( vertex_outputs.begin(), vertex_outputs.end(),
                [&] ( vertex_output_binding const& b )
            {
                return b.name == name_ ;
            } ) ;
            if( iter == vertex_outputs.end() ) return false ;

            va = iter->va ;

            return true ;
        }

        struct attribute_variable
        {
            motor::graphics::vertex_attribute va ;
            motor::string_t name ;
            GLuint loc ;
            GLenum type ;
        };
        motor_typedef( attribute_variable ) ;

        motor::vector< attribute_variable_t > attributes ;

        struct uniform_variable
        {
            motor::string_t name ;
            GLuint loc ;
            GLenum type ;

            // the GL uniform function
            motor::ogl::uniform_funk_t uniform_funk ;

            // @return false if failed to set uniform
            bool_t do_uniform_funk( void_ptr_t mem_ )
            {
                uniform_funk( loc, 1, mem_ ) ;
                return !motor::ogl::error::check_and_log( gl4_backend_log( "glUniform" ) ) ;
            }

            void_t do_copy_funk( void_ptr_t mem_, motor::graphics::ivariable_ptr_t var )
            {
                std::memcpy( mem_, var->data_ptr(), motor::ogl::uniform_size_of( type ) ) ;
            }
        };
        motor_typedef( uniform_variable ) ;

        motor::vector< uniform_variable > uniforms ;
    } ;
    motor_typedef( shader_data ) ;

    //********************************************************************************
    struct state_data
    {
        bool_t valid = false ;
        motor::string_t name ;
        motor::vector< motor::graphics::render_state_sets_t > states ;
    } ;
    motor_typedef( state_data ) ;

    //********************************************************************************
    struct render_data
    {
        bool_t valid = false ;
        motor::string_t name ;

        motor::vector< size_t > geo_ids ;
        motor::vector< size_t > tf_ids ; // feed from for geometry
        size_t shd_id = size_t( -1 ) ;

        struct geo_to_vao
        {
            size_t gid ;
            GLuint vao ;
        } ;
        motor::vector< geo_to_vao > geo_to_vaos ;

        // also keep this one for ref counting
        motor::vector< motor::graphics::variable_set_mtr_t > var_sets ;

        struct uniform_variable_link
        {
            // the user variable holding the data.
            motor::graphics::ivariable_ptr_t var ;

            // index into var_sets
            size_t var_set_idx ;

            // the index into the shader_config::uniforms array
            size_t uniform_id ;

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };
        motor::vector< uniform_variable_link > var_sets_data ;

        struct uniform_texture_link
        {
            // index into var_sets
            size_t var_set_idx ;

            // the index into the shader_config::uniforms array
            size_t uniform_id ;
            GLint tex_id ;
            size_t img_id ;

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };
        motor::vector< uniform_texture_link > var_sets_texture ;

        struct uniform_array_data_link
        {
            // index into var_sets
            size_t var_set_idx ;

            // the index into the shader_config::uniforms array
            size_t uniform_id ;
            GLint tex_id ;
            size_t buf_id ;

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };
        motor::vector< uniform_array_data_link > var_sets_array ;

        struct uniform_streamout_link
        {
            // index into var_sets
            size_t var_set_idx ;

            // the index into the shader_config::uniforms array
            size_t uniform_id ;
            GLint tex_id[2] ; // streamout object do double buffer
            size_t so_id ; // link into _feedbacks

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };
        motor::vector< uniform_streamout_link > var_sets_streamout ;

        // memory block for all variables in all variable sets.
        void_ptr_t mem_block = nullptr ;

        motor::vector< motor::graphics::render_state_sets > rss ;

        void_t remove_geometry_id( size_t const id ) noexcept
        {
            auto iter = std::find_if( geo_ids.begin(), geo_ids.end(), [&]( size_t const d )
            {
                return d == id ;
            } ) ;
            if( iter == geo_ids.end() ) return ;
            geo_ids.erase( iter ) ;
        }
    };
    motor_typedef( render_data ) ;

    //********************************************************************************
    struct image_data
    {
        motor::string_t name ;

        GLenum type = GL_NONE ;

        GLuint tex_id = GLuint( -1 ) ;
        size_t sib = 0 ;

        GLenum wrap_types[ 3 ] ;
        GLenum filter_types[ 2 ] ;

        // sampler ids for gl>=3.3
    };
    motor_typedef( image_data ) ;

    //********************************************************************************
    struct array_data
    {
        bool_t valid = false ;
        motor::string_t name ;

        GLuint tex_id = GLuint( -1 ) ;
        GLuint buf_id = GLuint( -1 ) ;

        GLuint sib = 0 ;
    } ;
    motor_typedef( array_data ) ;

    //********************************************************************************
    struct framebuffer_data
    {
        bool_t valid = false ;

        motor::string_t name ;

        GLuint gl_id = GLuint( -1 ) ;

        size_t nt = 0 ;
        GLuint colors[ 8 ] = {
            GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ),
            GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ) } ;

        GLuint depth = GLuint( -1 ) ;

        motor::math::vec2ui_t dims ;
    };
    motor_typedef( framebuffer_data ) ;

    //********************************************************************************
    struct msl_data
    {
        bool_t valid = false ;
        // empty names indicate free configs
        motor::string_t name ;

        // purpose: keep track of the data within the msl object
        // if recompilation is triggered.
        motor::graphics::msl_object_t msl_obj ;

        motor::vector< motor::graphics::render_object_t > ros ; 
        motor::vector< motor::graphics::shader_object_t > sos ; 
    };
    motor_typedef( msl_data ) ;

    typedef motor::vector< this_t::shader_data > shader_datas_t ;
    shader_datas_t _shaders ;

    typedef motor::vector< this_t::render_data > render_datas_t ;
    render_datas_t _renders ;

    typedef motor::vector< this_t::geo_data > geo_datas_t ;
    geo_datas_t _geometries ;

    typedef motor::vector< this_t::image_data_t > image_datas_t ;
    image_datas_t _images ;

    typedef motor::vector< this_t::framebuffer_data_t > framebuffer_datas_t ;
    framebuffer_datas_t _framebuffers ;

    typedef motor::vector< this_t::state_data_t > state_datas_t ;
    state_datas_t _states ;
    motor::stack< motor::graphics::render_state_sets_t, 10 > _state_stack ;

    typedef motor::vector< this_t::array_data_t > array_datas_t ;
    array_datas_t _arrays ;

    typedef motor::vector< this_t::tf_data_t > tf_datas_t ;
    tf_datas_t _feedbacks ;

    typedef motor::vector< this_t::msl_data_t > msl_datas_t ;
    msl_datas_t _msls ;
    motor::msl::database_t _mdb ;

    GLsizei vp_width = 0 ;
    GLsizei vp_height = 0 ;

    motor::graphics::gen4::backend_type const bt = motor::graphics::gen4::backend_type::gl4 ;
    motor::graphics::shader_api_type const sapi = motor::graphics::shader_api_type::glsl_4_0 ;

    // the current render state set
    motor::graphics::render_state_sets_t render_states ;

    motor::platform::opengl::rendering_context_ptr_t _ctx ;

    size_t _tf_active_id = size_t( -1 ) ;

    size_t _bid = size_t(-1) ;

    //****************************************************************************************
    pimpl( size_t const bid, motor::platform::opengl::rendering_context_ptr_t ctx ) 
    {
        _ctx = ctx ;

        {
            motor::graphics::state_object_t obj( "gl4_default_states" ) ;

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

            /*size_t const oid =*/ this_t::construct_state( size_t( -1 ), obj ) ;
        }
        _bid = bid ;
    }

    //****************************************************************************************
    pimpl( this_rref_t rhv ) noexcept
    {
        _shaders = std::move( rhv._shaders ) ;
        _renders = std::move( rhv._renders ) ;
        _geometries = std::move( rhv._geometries ) ;
        _images = std::move( rhv._images ) ;
        _framebuffers = std::move( rhv._framebuffers ) ;
        _states = std::move( rhv._states ) ;
        _arrays = std::move( rhv._arrays ) ;
        _feedbacks = std::move( rhv._feedbacks ) ;
        _msls = std::move( rhv._msls ) ;

        _state_stack = std::move( rhv._state_stack ) ;
        vp_width = rhv.vp_width ;
        vp_height = rhv.vp_height ;

        render_states = std::move( rhv.render_states ) ;
        _ctx = motor::move( rhv._ctx ) ;
        _tf_active_id = rhv._tf_active_id ;
        _bid = rhv._bid ;

        _mdb = std::move( rhv._mdb ) ;
    }

    //****************************************************************************************
    ~pimpl( void_t ) noexcept
    {
        for( size_t i = 0; i<_framebuffers.size(); ++i )
        {
            this_t::release_framebuffer( i ) ;
        }

        for( size_t i = 0; i<_renders.size(); ++i )
        {
            this_t::release_render_data( i, false ) ;
        }

        for( size_t i = 0; i<_shaders.size(); ++i )
        {
            this_t::release_shader_data( i ) ;
        }

        for( size_t i = 0; i<_geometries.size(); ++i )
        {
            this_t::release_geometry( i ) ;
        }

        for( size_t i = 0; i<_images.size(); ++i )
        {
            this_t::release_image_data( i ) ;
        }

        for( size_t i = 0; i<_arrays.size(); ++i )
        {
            this_t::release_array_data( i ) ;
        }

        for( size_t i = 0; i<_feedbacks.size(); ++i )
        {
            this_t::release_tf_data( i ) ;
        }

        // the particular objects will be released.
        // the msl object does not hold any gl object.
        for( auto & msl : _msls ) 
        {
        }

        _geometries.clear() ;
        _shaders.clear() ;
        _renders.clear() ;
        _framebuffers.clear() ;
        _arrays.clear() ;
        _images.clear() ;
        _feedbacks.clear() ;
        _msls.clear() ;
    }

    // silent clear. No gl release will be done.
    void_t clear_all_objects( void_t ) noexcept
    {
        for( size_t i = 0; i<_renders.size(); ++i )
        {
            this_t::release_render_data( i, false ) ;
        }

        for( size_t i = 0; i<_shaders.size(); ++i )
        {
            this_t::release_shader_data( i, true ) ;
        }

        _geometries.clear() ;
        _shaders.clear() ;
        _renders.clear() ;
        _framebuffers.clear() ;
        _arrays.clear() ;
        _images.clear() ;
        _feedbacks.clear() ;
        _msls.clear() ;
    }

    //****************************************************************************************
    template< typename T >
    static size_t determine_oid( size_t oid, motor::string_cref_t name, motor::vector< T >& v ) noexcept
    {
        // just reuse the same object
        if( v.size() > oid && v[oid].name == name )
        {
            v[ oid ].valid = true ;
            return oid ;
        }

        oid = size_t(-1) ;
        
        // this code is required if we only know the name
        // but do not have any oid.
        {
            auto iter = std::find_if( v.begin(), v.end(), [&] ( T const& c )
            {
                return c.name == name ;
            } ) ;

            if( iter != v.end() )
            {
                oid = std::distance( v.begin(), iter ) ;
            }
        }

        if( oid == size_t( -1 ) )
        {
            for( size_t i = 0 ; i < v.size(); ++i )
            {
                if( !v[ i ].valid )
                {
                    oid = i ;
                    break ;
                }
            }
        }

        if( oid >= v.size() )
        {
            oid = v.size() ;
            v.resize( oid + 1 ) ;
        }

        v[ oid ].valid = true ;
        v[ oid ].name = name ;

        return oid ;
    }

    //****************************************************************************************
    template< typename T >
    static size_t find_index_by_resource_name( motor::string_in_t name, 
                   motor::vector< T > const & resources ) noexcept
    {
        size_t i = 0 ; 
        for( auto const & r : resources ) 
        {
            if( r.name == name ) return i ;
            ++i ;
        }
        return size_t( -1 ) ;
    }

    //****************************************************************************************
    size_t construct_state( size_t oid, motor::graphics::state_object_ref_t obj ) noexcept
    {
        oid = determine_oid( obj.get_oid(_bid), obj.name(), _states ) ;

        auto& states = _states[ oid ] ;

        states.states.resize( obj.size() ) ;

        // @note
        // set all enables. Enable/Disable only possible during construction
        // values are assigned in the update function for the render states
        obj.for_each( [&] ( size_t const i, motor::graphics::render_state_sets_cref_t rs )
        {
            states.states[ i ] = rs ;
        } ) ;

        return oid ;
    }

    //******************************************************************************************************************************
    void_t update_state( size_t const oid, motor::graphics::state_object_ref_t obj ) noexcept
    {
        assert( oid < _states.size() ) ;

        if( !obj.check_and_reset_changed( _bid ) ) return ;

        
        obj.for_each( [&]( size_t const i, motor::graphics::render_state_sets_cref_t rs )
        {
            if( i >= _states[oid].states.size() ) return ;
            _states[oid].states[i] = rs ;
        } ) ;

    }

    //****************************************************************************************
    void_t handle_render_state( motor::graphics::render_state_sets_cref_t new_states, bool_t const popped = false )
    {
        // depth test
        {
            if( new_states.depth_s.do_change )
            {
                if( new_states.depth_s.ss.do_activate )
                {
                    glEnable( GL_DEPTH_TEST );
                    motor::ogl::error::check_and_log( gl4_backend_log( "glEnable" ) ) ;

                    glDepthMask( new_states.depth_s.ss.do_depth_write ? GL_TRUE : GL_FALSE ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDepthMask" ) ) ;

                    glDepthFunc( GL_LESS ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDepthFunc" ) ) ;
                }
                else
                {
                    glDisable( GL_DEPTH_TEST ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDisable( GL_DEPTH_TEST )" ) ) ;

                    glDepthMask( GL_FALSE ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDepthMask" ) ) ;
                }
            }
        }

        // blend mode
        {
            if( new_states.blend_s.do_change )
            {
                if( new_states.blend_s.ss.do_activate )
                {
                    glEnable( GL_BLEND ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glEnable" ) ) ;

                    GLenum const glsrc = motor::platform::gl3::convert( new_states.blend_s.ss.src_blend_factor ) ;
                    GLenum const gldst = motor::platform::gl3::convert( new_states.blend_s.ss.dst_blend_factor  );

                    glBlendFunc( glsrc, gldst ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glBlendFunc" ) ) ;
                }
                else
                {
                    glDisable( GL_BLEND ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDisable" ) ) ;
                }
            }
        }

        // polygon mode
        {
            if( new_states.polygon_s.do_change )
            {
                if( new_states.polygon_s.ss.do_activate )
                {
                    glEnable( GL_CULL_FACE ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glEnable" ) ) ;

                    glCullFace( motor::platform::gl3::convert( new_states.polygon_s.ss.cm ) ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glCullFace" ) ) ;

                    glFrontFace( motor::platform::gl3::convert(new_states.polygon_s.ss.ff ) ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glFrontFace" ) ) ;

                    glPolygonMode( GL_FRONT_AND_BACK, motor::platform::gl3::convert( new_states.polygon_s.ss.fm ) ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glPolygonMode" ) ) ;
                }
                else
                {
                    glDisable( GL_CULL_FACE ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDisable" ) ) ;
                }
            }
        }

        // scissor
        {
            if( new_states.scissor_s.do_change )
            {
                if( new_states.scissor_s.ss.do_activate )
                {
                    glEnable( GL_SCISSOR_TEST ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glEnable" ) ) ;

                    glScissor(
                        GLint( new_states.scissor_s.ss.rect.x() ), GLint( new_states.scissor_s.ss.rect.y() ),
                        GLsizei( new_states.scissor_s.ss.rect.z() ), GLsizei( new_states.scissor_s.ss.rect.w() ) ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glScissor" ) ) ;
                }
                else
                {
                    glDisable( GL_SCISSOR_TEST ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDisable" ) ) ;
                }
            }
        }

        // always set the viewport if enabled
        {
            if( new_states.view_s.do_change )
            {
                if( new_states.view_s.ss.do_activate )
                {
                    auto const& vp = new_states.view_s.ss.vp  ;
                    glViewport( GLint( vp.x() ), GLint( vp.y() ), GLsizei( vp.z() ), GLsizei( vp.w() ) ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glViewport" ) ) ;
                }
            }
        }

        // do clear the frame
        if( !popped )
        {
            if( new_states.clear_s.do_change && new_states.clear_s.ss.do_activate )
            {
                bool_t const clear_color = new_states.clear_s.ss.do_color_clear ;
                bool_t const clear_depth = new_states.clear_s.ss.do_depth_clear ;

                motor::math::vec4f_t const color = new_states.clear_s.ss.clear_color ;
                glClearColor( color.x(), color.y(), color.z(), color.w() ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glClearColor" ) ) ;

                GLbitfield const color_bit = clear_color ? GL_COLOR_BUFFER_BIT : 0 ;
                GLbitfield const depth_bit = clear_depth ? GL_DEPTH_BUFFER_BIT : 0 ;

                glClear( color_bit | depth_bit ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glEnable" ) ) ;
            }
        }
    }

    //****************************************************************************************
    // if oid == -1, the state is popped.
    void_t handle_render_state( size_t const oid, size_t const rs_id ) noexcept
    {
        // pop state
        if( oid == size_t(-1) )
        {
            if( _state_stack.size() == 1 )
            {
                motor::log::global_t::error( gl4_backend_log( "no more render states to pop" ) ) ;
                return ;
            }
            auto const popped = _state_stack.pop() ;
            this_t::handle_render_state( popped - _state_stack.top(), true ) ;
        }
        else
        {
            auto new_id = std::make_pair( oid, rs_id ) ;
            _state_stack.push( _state_stack.top() + _states[ new_id.first ].states[ new_id.second ] ) ;
            this_t::handle_render_state( _state_stack.top(), false ) ;
        }
    }

    //****************************************************************************************
    size_t construct_framebuffer( size_t oid, motor::graphics::framebuffer_object_ref_t obj ) noexcept
    {
        oid = determine_oid( obj.get_oid(_bid), obj.name(), _framebuffers ) ;

        framebuffer_data_ref_t fb = _framebuffers[ oid ] ;

        if( fb.gl_id == GLuint( -1 ) )
        {
            glGenFramebuffers( 1, &fb.gl_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGenFramebuffers" ) ) ;
        }

        if( fb.gl_id == GLuint( -1 ) )return oid ;
        
        // bind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fb.gl_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGenFramebuffers" ) ) ;
        }

        size_t const nt = obj.get_num_color_targets() ;
        auto const ctt = obj.get_color_target() ;
        auto const dst = obj.get_depth_target();
        motor::math::vec2ui_t dims = obj.get_dims() ;
        
        // fix dims
        {
            dims.x( dims.x() + dims.x() % 2 ) ;
            dims.y( dims.y() + dims.y() % 2 ) ;
        }

        bool_t const requires_store = fb.colors[0] == GLuint( -1 ) ;

        {
            if( fb.colors[0] == GLuint( -1 ) )
            {
                glGenTextures( GLsizei( 8 ), fb.colors ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glGenTextures" ) ) ;
            }

            if( fb.depth == GLuint(-1) )
            {
                glGenTextures( GLsizei( 1 ), &fb.depth ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glGenTextures" ) ) ;
            }
        }

        // construct color textures
        {
            for( size_t i=0; i<nt; ++i )
            {
                GLuint const tid = fb.colors[i] ;

                glBindTexture( GL_TEXTURE_2D, tid ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) )
                    continue ;

                GLenum const target = GL_TEXTURE_2D ;
                GLint const level = 0 ;
                GLsizei const width = dims.x() ;
                GLsizei const height = dims.y() ;
                GLenum const format = GL_RGBA ;
                GLenum const type = motor::platform::gl3::to_pixel_type( ctt ) ;
                GLint const border = 0 ;
                GLint const internal_format = motor::platform::gl3::to_gl_format( ctt ) ;

                // maybe required for memory allocation
                // at the moment, render targets do not have system memory.
                #if 0
                size_t const sib = motor::platform::gl3::calc_sib( dims.x(), dims.y(), ctt ) ;
                #endif
                void_cptr_t data = nullptr ;

                glTexImage2D( target, level, internal_format, width, height, border, format, type, data ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glTexImage2D" ) ) ;
            }

            // attach
            for( size_t i = 0; i < nt; ++i )
            {
                GLuint const tid = fb.colors[ i ] ;
                GLenum const att = GLenum( size_t( GL_COLOR_ATTACHMENT0 ) + i ) ;
                glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glFramebufferTexture2D" ) ) ;
            }
        }

        // depth/stencil
        if( dst != motor::graphics::depth_stencil_target_type::unknown )
        {
            {
                GLuint const tid = fb.depth ;

                glBindTexture( GL_TEXTURE_2D, tid ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) ;

                GLenum const target = GL_TEXTURE_2D ;
                GLint const level = 0 ;
                GLsizei const width = dims.x() ;
                GLsizei const height = dims.y() ;
                GLenum const format = motor::platform::gl3::to_gl_format( dst ) ;
                GLenum const type = motor::platform::gl3::to_gl_type( dst ) ;
                GLint const border = 0 ;
                GLint const internal_format = motor::platform::gl3::to_gl_format( dst ) ;

                // maybe required for memory allocation
                // at the moment, render targets do not have system memory.
                #if 0
                size_t const sib = motor::platform::gl3::calc_sib( dims.x(), dims.y(), ctt ) ;
                #endif
                void_cptr_t data = nullptr ;

                glTexImage2D( target, level, internal_format, width, height, border, format, type, data ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glTexImage2D" ) ) ;
            }

            // attach
            {
                GLuint const tid = fb.depth ;
                GLenum const att = motor::platform::gl3::to_gl_attachment(dst) ;
                glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glFramebufferTexture2D" ) ) ;
            }
        }


        GLenum status = 0 ;
        // validate
        {
            status = glCheckFramebufferStatus( GL_FRAMEBUFFER ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glCheckFramebufferStatus" ) ) ;

            motor::log::global_t::warning( status != GL_FRAMEBUFFER_COMPLETE, 
                "Incomplete framebuffer : [" + obj.name() + "]" ) ;
        }

        // unbind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGenFramebuffers" ) ) ;
        }

        // remember data
        if( status == GL_FRAMEBUFFER_COMPLETE )
        {
            // color type maybe?
            fb.nt = nt ;
            fb.dims = dims ;
        }

        // store images
        if( requires_store )
        {
            size_t const id = _images.size() ;
            _images.resize( _images.size() + nt ) ;

            for( size_t i = 0; i < nt; ++i )
            {
                size_t const idx = id + i ;
                _images[ idx ].name = fb.name + "." + motor::to_string( i ) ;
                _images[ idx ].tex_id = fb.colors[ i ] ;
                _images[ idx ].type = GL_TEXTURE_2D ; 

                for( size_t j = 0; j < ( size_t ) motor::graphics::texture_wrap_mode::size; ++j )
                {
                    _images[ idx ].wrap_types[ j ] = GL_CLAMP_TO_BORDER ;
                }

                for( size_t j = 0; j < ( size_t ) motor::graphics::texture_filter_mode::size; ++j )
                {
                    _images[ idx ].filter_types[ j ] = GL_LINEAR ;
                }
            }
        }

        // store depth/stencil
        if( requires_store )
        {
            size_t const id = _images.size() ;
            _images.resize( _images.size() + 1 ) ;

            {
                size_t const idx = id + 0 ;
                _images[ idx ].name = fb.name + ".depth" ;
                _images[ idx ].tex_id = fb.depth ;
                _images[ idx ].type = GL_TEXTURE_2D ; 

                for( size_t j = 0; j < ( size_t ) motor::graphics::texture_wrap_mode::size; ++j )
                {
                    _images[ idx ].wrap_types[ j ] = GL_REPEAT ;
                }

                for( size_t j = 0; j < ( size_t ) motor::graphics::texture_filter_mode::size; ++j )
                {
                    _images[ idx ].filter_types[ j ] = GL_NEAREST ;
                }
            }
        }

        return oid ;
    }

    //****************************************************************************************
    bool_t release_framebuffer( size_t const oid ) noexcept
    {
        auto & fbd = _framebuffers[ oid ] ;

        if( !fbd.valid ) return false ;

        // find image ids in images and remove them
        {
            for( size_t i=0; i<8; ++i )
            {
                if( fbd.colors[i] == GLuint(-1) ) continue ;

                for( auto & img : _images )
                {
                    if( img.tex_id == fbd.colors[i] )
                    {                        
                        img.tex_id = GLuint(-1) ;
                        img.name = "released framebuffer color buffer" ;
                        img.sib = 0 ;
                        break ;
                    }
                }
            }
        }
        

        {
            glDeleteFramebuffers( 1, &fbd.gl_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteFramebuffers" ) ) ;
        }

        {
            glDeleteTextures( 8, fbd.colors ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteTextures" ) ) ;
        }

        if( fbd.depth != GLuint(-1) )
        {
            // find image id in images and remove them
            {
                for( auto & img : _images )
                {
                    if( img.tex_id == fbd.depth )
                    {                        
                        img.tex_id = GLuint(-1) ;
                        img.name = "released framebuffer color buffer" ;
                        img.sib = 0 ;
                        break ;
                    }
                }
            }

            {
                glDeleteTextures( 1, &fbd.depth ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteTextures" ) ) ;
            }
        }

        fbd.name = "" ;
        fbd.valid = false ;
        fbd.gl_id = GLuint( -1 ) ;
        fbd.depth = GLuint( -1 ) ;
        for( size_t i=0; i<8; ++i ) fbd.colors[i] = GLuint( -1 ) ;

        fbd.dims = motor::math::vec2ui_t() ;

        return true ;
    }

    //****************************************************************************************
    bool_t activate_framebuffer( size_t const oid ) noexcept
    {
        framebuffer_data_ref_t fb = _framebuffers[ oid ] ;

        // bind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fb.gl_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGenFramebuffers" ) ) ;
        }

        // setup color
        {
            GLenum attachments[ 15 ] ;
            size_t const num_color = fb.nt ;
            
            for( size_t i = 0; i < num_color; ++i )
            {
                attachments[ i ] = GLenum( size_t( GL_COLOR_ATTACHMENT0 ) + i ) ;
            }

            glDrawBuffers( GLsizei( num_color ), attachments ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGenFramebuffers" ) ) ;
        }

        return true ;
    }

    //****************************************************************************************
    bool_t deactivate_framebuffer( void_t ) noexcept
    {
        // unbind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 ) ;
            if( !motor::ogl::error::check_and_log( gl4_backend_log( "glGenFramebuffers" ) ) ) return false ;
        }

        return true ;
    }

    //****************************************************************************************
    size_t construct_shader_data( size_t oid, motor::graphics::shader_object_ref_t obj ) noexcept
    {
        oid = determine_oid( obj.get_oid(_bid), obj.name(), _shaders ) ;

        //
        // SECTION: Pre-Compilation
        //

        motor::graphics::shader_set_t ss ;
        {
            auto const res = obj.shader_set( this_t::sapi, ss ) ;
            if( !res )
            {
                motor::log::global_t::warning( gl4_backend_log(
                    "config [" + obj.name() + "] has no shaders for " + 
                    motor::graphics::gen4::to_string( this_t::bt ) ) ) ;
                return oid ;
            }
        }

        struct shader_compilation
        {
            GLuint vs_id = GLuint(-1) ;
            GLuint gs_id = GLuint(-1) ;
            GLuint ps_id = GLuint(-1) ;
        } ;

        shader_compilation scomp ;

        // pre-compile all the shaders
        {
            // Compile Vertex Shader
            {
                GLuint const id = glCreateShader( GL_VERTEX_SHADER ) ;
                motor::ogl::error::check_and_log( 
                    gl4_backend_log( "Vertex Shader creation" ) ) ;

                if( !this_t::compile_shader( id, ss.vertex_shader().code() ) )
                {
                    glDeleteShader( id ) ;
                    motor::ogl::error::check_and_log( 
                        gl4_backend_log( "glDeleteShader : Vertex Shader" ) ) ;
                    return size_t(-1) ;
                }

                scomp.vs_id = id ;
            }

            // Compile Geometry Shader
            if( ss.has_geometry_shader() )
            {
                GLuint const id = glCreateShader( GL_GEOMETRY_SHADER ) ;
                motor::ogl::error::check_and_log(
                    gl4_backend_log( "Geometry Shader creation" ) ) ;

                if( !this_t::compile_shader( id, ss.geometry_shader().code() ) )
                {
                    glDeleteShader( scomp.vs_id ) ;
                    motor::ogl::error::check_and_log( 
                        gl4_backend_log( "glDeleteShader Vertex Shader" ) ) ;
                    glDeleteShader( id ) ;
                    motor::ogl::error::check_and_log( 
                        gl4_backend_log( "glDeleteShader Geometry Shader" ) ) ;
                    return size_t(-1) ;
                }

                scomp.gs_id = id ;
            }
            
            // Compile Pixel Shader
            if( ss.has_pixel_shader() )
            {
                GLuint const id = glCreateShader( GL_FRAGMENT_SHADER ) ;
                motor::ogl::error::check_and_log(
                    gl4_backend_log( "Fragment Shader creation" ) ) ;

                if( !this_t::compile_shader( id, ss.pixel_shader().code() ) )
                {
                    glDeleteShader( scomp.vs_id ) ;
                    motor::ogl::error::check_and_log( 
                        gl4_backend_log( "glDeleteShader Vertex Shader" ) ) ;
                    if( ss.has_geometry_shader() )
                    {
                        glDeleteShader( scomp.gs_id ) ;
                        motor::ogl::error::check_and_log( 
                            gl4_backend_log( "glDeleteShader Geometry Shader" ) ) ;
                    }
                    glDeleteShader( id ) ;
                    motor::ogl::error::check_and_log( 
                        gl4_backend_log( "glDeleteShader Pixel Shader" ) ) ;
                    return size_t(-1) ;
                }

                scomp.ps_id = id ;
            }
        }

        //
        // SECTION : Shader/Program setup
        //

        auto & sd = _shaders[ oid ] ;

        // program
        if( sd.pg_id == GLuint( -1 ) )
        {
            GLuint const id = glCreateProgram() ;
            motor::ogl::error::check_and_log(
                gl4_backend_log( "Shader Program creation" ) ) ;

            sd.pg_id = id ;
        }
        {
            this_t::detach_shaders( sd.pg_id ) ;
            this_t::delete_all_variables( sd ) ;
        }

        GLuint const pid = sd.pg_id ;

        // vertex shader
        {
            if( _shaders[oid].vs_id != GLuint(-1) )
            {
                glDeleteShader( _shaders[oid].vs_id ) ;
                motor::ogl::error::check_and_log( 
                    gl4_backend_log( "glDeleteShader Vertex Shader" ) ) ;
            }

            sd.vs_id = scomp.vs_id ;

            {
                glAttachShader( pid, _shaders[oid].vs_id ) ;
                motor::ogl::error::check_and_log(
                    gl4_backend_log( "Attaching vertex shader" ) ) ;
            }
        }

        // geometry shader
        {
            GLuint const id = sd.gs_id ;

            if( id != GLuint(-1) )
            {
                glDeleteShader( id ) ;
                motor::ogl::error::check_and_log( 
                    gl4_backend_log( "glDeleteShader Geometry Shader" ) ) ;
            }

            sd.gs_id = scomp.gs_id ;
        }

        if( ss.has_geometry_shader() )
        {
            glAttachShader( pid, sd.gs_id ) ;
            motor::ogl::error::check_and_log(
                gl4_backend_log( "Attaching geometry shader" ) ) ;

            // check max output vertices
            {
                GLint max_out = 0 ;
                glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_out ) ;
                motor::ogl::error::check_and_log(
                    gl4_backend_log( "Geometry Shader Max Output Vertices" ) ) ;
                ( void_t ) max_out ;
            }

            // @todo geometry shader program parameters
            {
                /*
                glProgramParameteri( pid, GL_GEOMETRY_INPUT_TYPE, 
                    ( GLint ) so_gl::convert( shd_ptr->get_input_type() ) ) ;
                error = motor::ogl::error::check_and_log(
                    gl4_backend_log( "Geometry Shader Input Type" ) ) ;

                glProgramParameteri( pid, GL_GEOMETRY_OUTPUT_TYPE, 
                    ( GLint ) so_gl::convert( shd_ptr->get_output_type() ) ) ;
                error = motor::ogl::error::check_and_log(
                    gl4_backend_log( "Geometry Shader Output Type" ) ) ;

                glProgramParameteri( pid, GL_GEOMETRY_VERTICES_OUT, 
                    ( GLint ) shd_ptr->get_num_output_vertices() ) ;
                error = motor::ogl::error::check_and_log(
                    gl4_backend_log( "Geometry Shader Vertices Out" ) ) ;
                    */
            }
        }

        // pixel shader
        {
            GLuint const id = sd.ps_id ;

            if( id != GLuint(-1) )
            {
                glDeleteShader( id ) ;
                motor::ogl::error::check_and_log( 
                    gl4_backend_log( "glDeleteShader Pixel Shader" ) ) ;
            }

            sd.ps_id = scomp.ps_id ;
        }

        if( ss.has_pixel_shader() )
        {
            glAttachShader( pid, scomp.ps_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "Attaching pixel shader" ) ) ;
        }

        //
        // SECTION : Further construction
        //

        {
            obj.shader_bindings().for_each_vertex_input_binding( [&]( size_t const,
                motor::graphics::vertex_attribute const va, motor::string_cref_t name )
            {
                sd.vertex_inputs.emplace_back( 
                    this_t::shader_data::vertex_input_binding { va, name } ) ;
            } ) ;
        }

        // !!! must be done pre-link !!!
        // set transform feedback varyings
        if( (obj.shader_bindings().get_num_output_bindings() != 0) && 
            (obj.get_streamout_mode() != motor::graphics::streamout_mode::unknown) )
        {
            sd.output_names = (char const **)motor::memory::global_t::
                alloc_raw<char *>( obj.shader_bindings().get_num_output_bindings() ) ;

            obj.shader_bindings().for_each_vertex_output_binding( [&]( size_t const i,
                motor::graphics::vertex_attribute const va, 
                motor::graphics::ctype const, motor::string_cref_t name )
            {
                sd.vertex_outputs.emplace_back( 
                    this_t::shader_data::vertex_output_binding { va, name } ) ;
                sd.output_names[i] = name.c_str() ;
            } ) ;

            // the mode interleaved or separate depends on the number of 
            // buffers in the streamout object. So if a streamout
            // object is used, the engine needs to relink this shader 
            // based on the number of buffers attached to the streamout
            // object.
            GLenum const mode = motor::platform::gl3::convert( obj.get_streamout_mode() ) ;
            glTransformFeedbackVaryings( sd.pg_id, GLsizei( obj.shader_bindings().get_num_output_bindings() ), 
                                         sd.output_names, mode ) ;

            motor::ogl::error::check_and_log( gl4_backend_log( "glTransformFeedbackVaryings" ) ) ;
            motor::log::global_t::status( mode == GL_NONE, 
                            "Did you miss to set the streamout mode in the shader object?" ) ;

            motor::memory::global_t::dealloc( sd.output_names ) ;
            sd.output_names = nullptr ;
        }

        // link
        {
            auto const r4 = this_t::link( sd.pg_id ) ;
            if( !r4 )
            {
                sd.is_linkage_ok = false ;
                return false ;
            }
            sd.is_linkage_ok = true ;
            motor::log::global_t::status( "[GL4] : Compilation Successful : [" + sd.name + "]" ) ;
        }

        {
            this_t::post_link_attributes( sd ) ;
            this_t::post_link_uniforms( sd ) ;
            this_t::post_link_uniform_blocks( sd ) ;
        }

        // if the shader is redone, all render objects using
        // the shader need to be updated
        {
            for( size_t i=0; i<_renders.size(); ++i )
            {
                auto & rd = _renders[i] ;

                if( rd.shd_id != oid ) continue ;
                
                rd.var_sets_data.clear() ;
                rd.var_sets_texture.clear() ;
                rd.var_sets_array.clear() ;
                rd.var_sets_streamout.clear() ;

                auto sets = std::move( rd.var_sets ) ;
                for ( size_t s = 0; s < sets.size(); ++s )
                {
                    auto & vs = sets[ s ] ;
                    this_t::connect( rd, s, vs ) ;
                }

                this_t::render_object_variable_memory( rd, _shaders[ rd.shd_id ] ) ;
                for( size_t vs_id=0; vs_id<rd.var_sets.size(); ++vs_id )
                {
                    this_t::update_variables( i, vs_id ) ;
                }
            }
        }

        return oid ;
    }

    //****************************************************************************************
    bool_t release_shader_data( size_t const oid, bool_t const silent = false ) noexcept
    {
        auto & shd = _shaders[ oid ] ;

        if( !shd.valid ) return true ;

        // delete shaders
        if( !silent )
        {
            glDetachShader( shd.pg_id, shd.vs_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDetachShader" ) ) ;

            glDeleteShader( shd.vs_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteShader" ) ) ;
        }

        if( shd.gs_id != GLuint(-1) && !silent )
        {
            glDetachShader( shd.pg_id, shd.gs_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDetachShader" ) ) ;

            glDeleteShader( shd.gs_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteShader" ) ) ;
        }

        if( shd.ps_id != GLuint(-1) && !silent )
        {
            glDetachShader( shd.pg_id, shd.ps_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDetachShader" ) ) ;

            glDeleteShader( shd.ps_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteShader" ) ) ;
        }

        // delete program
        if( !silent )
        {
            glDeleteProgram( shd.pg_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteProgram" ) ) ;
        }

        if( shd.output_names != nullptr )
        {
            motor::memory::global_t::dealloc_raw( shd.output_names ) ;
        }

        shd.valid = false ;
        shd.pg_id = GLuint( -1 ) ;
        shd.gs_id = GLuint( -1 ) ;
        shd.vs_id = GLuint( -1 ) ;
        shd.ps_id = GLuint( -1 ) ;
        shd.name = "released shader" ;
        
        shd.is_linkage_ok = false ;
        shd.uniforms.clear() ;
        shd.attributes.clear() ;

        return true ;
    }

    //****************************************************************************************
    void_t detach_shaders( GLuint const program_id )
    {
        GLsizei count = 0 ;
        GLuint shaders_[ 10 ] ;

        glGetAttachedShaders( program_id, 10, &count, shaders_ ) ;
        motor::ogl::error::check_and_log( gl4_backend_log( "glGetAttachedShaders" ) ) ;

        for( GLsizei i = 0; i < count; ++i )
        {
            glDetachShader( program_id, shaders_[ i ] ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDetachShader" ) ) ;
        }
    }

    //****************************************************************************************
    void_t delete_all_variables( this_t::shader_data & config )
    {
        config.vertex_inputs.clear() ;

        //for( auto & v : config.attributes )
        {
            // delete memory
        }
        config.attributes.clear() ;
        config.uniforms.clear() ;
    }

    //****************************************************************************************
    bool_t compile_shader( GLuint const id, motor::string_cref_t code )
    {
        if( code.empty() ) return false ;

        GLchar const* source_string = ( GLchar const* ) ( code.c_str() ) ;

        glShaderSource( id, 1, &source_string, 0 ) ;
        if( motor::ogl::error::check_and_log( gl4_backend_log( "glShaderSource" ) ) )
            return false ;

        glCompileShader( id ) ;
        if( motor::ogl::error::check_and_log( gl4_backend_log( "glCompileShader" ) ) )
            return false ;

        GLint ret ;
        glGetShaderiv( id, GL_COMPILE_STATUS, &ret ) ;

        GLint length ;
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length ) ;

        if( ret == GL_TRUE && length <= 1 ) return true ;

        if( motor::log::global::error( length == 0, 
            gl4_backend_log( "shader compilation failed, but info log length is 0." ) ) )
            return false ;

        // print first line for info
        // user can place the shader name or any info there.
        {
            size_t pos = code.find_first_of( '\n' ) ;
            motor::log::global::error( gl4_backend_log( "First Line: " + code.substr( 0, pos ) ) ) ;
        }

        // get the error message it is and print it
        {
            motor::memory::malloc_guard<char> info_log( length ) ;

            glGetShaderInfoLog( id, length, 0, info_log ) ;

            motor::vector< motor::string_t > tokens ;
            motor::mstd::string_ops::split( motor::string_t( info_log ), '\n', tokens ) ;

            for( auto const & msg : tokens )
            {
                motor::log::global::error( msg ) ;
            }
        }
        return false ;
    }

    //****************************************************************************************
    bool_t check_link_status( size_t const rid ) noexcept
    {
        auto & rd = _renders[ rid ] ;
        return _shaders[ rd.shd_id ].is_linkage_ok ;
    }

    //****************************************************************************************
    bool_t link( GLuint const program_id )
    {
        glLinkProgram( program_id ) ;
        if( motor::ogl::error::check_and_log( gl4_backend_log( "glLinkProgram" ) ) )
            return false ;

        {
            GLint ret ;
            glGetProgramiv( program_id, GL_LINK_STATUS, &ret ) ;

            GLint length ;
            glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &length ) ;

            if( ret == GL_TRUE && length <= 1 ) 
                return true ;

            if( motor::log::global_t::error( length == 0, gl4_backend_log("unknown") ) )
                return false ;

            motor::memory::malloc_guard<char> info_log( length ) ;

            glGetProgramInfoLog( program_id, length, 0, info_log ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log( "glGetProgramInfoLog" ) ) )
                return false ;

            std::string info_log_string = std::string( ( const char* ) info_log ) ;

            {
                motor::vector< motor::string_t > tokens ;
                motor::mstd::string_ops::split( motor::string_t( info_log ), '\n', tokens ) ;

                for( auto token : tokens )
                {
                    motor::log::global_t::error( token ) ;
                }
            }
        }
        return false ;
    }

    //****************************************************************************************
    void_t post_link_attributes( this_t::shader_data & config )
    {
        GLuint const program_id = config.pg_id ;

        GLint num_active_attributes = 0 ;
        GLint name_length = 0 ;

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTES, &num_active_attributes ) ;
        motor::ogl::error::check_and_log( gl4_backend_log( "glGetProgramiv(GL_ACTIVE_ATTRIBUTES)" ) ) ;

        if( num_active_attributes == 0 ) return ;

        config.attributes.resize( num_active_attributes ) ;

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_length ) ;
        motor::ogl::error::check_and_log( gl4_backend_log( 
            "glGetProgramiv(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)" ) ) ;

        GLint size ;
        GLenum gl_attrib_type ;

        motor::memory::malloc_guard<char> buffer( name_length ) ;

        for( GLint i = 0; i < num_active_attributes; ++i )
        {
            glGetActiveAttrib( program_id, i, name_length, 0, 
                &size, &gl_attrib_type, buffer ) ;

            if( motor::ogl::error::check_and_log( "glGetActiveAttrib failed. continue loop." ) ) continue ;

            // build-ins are not needed in the shader variables container.
            if( name_length >= 3 && buffer.equals( 0, 'g' ) && buffer.equals( 1, 'l' ) && buffer.equals( 2, '_' ) ) continue ;

            GLuint const location_id = glGetAttribLocation( program_id, buffer ) ;
            if( motor::ogl::error::check_and_log( "glGetAttribLocation failed. continue loop." ) ) continue ;

            motor::string_t const variable_name = motor::string_t( ( const char* ) buffer ) ;

            this_t::shader_data::attribute_variable_t vd ;
            vd.name = std::move( variable_name ) ;
            vd.loc = location_id ;
            vd.type = gl_attrib_type ;

            {
                motor::graphics::vertex_attribute va = motor::graphics::vertex_attribute::undefined ;
                auto const res = config.find_vertex_input_binding_by_name( vd.name, va ) ;
                motor::log::global_t::error( !res, 
                    gl4_backend_log("can not find vertex attribute - " + vd.name ) ) ;
                vd.va = va ;
            }
            config.attributes[i] = vd ;
        }

        for( auto iter = config.attributes.begin(); iter != config.attributes.end(); )
        {
            if( iter->va == motor::graphics::vertex_attribute::undefined )
            {
                iter = config.attributes.erase( iter ) ;
                continue ;
            }
            ++iter ;
        }
    }

    //****************************************************************************************
    bool_t bind_attributes( this_t::shader_data & sconfig, this_t::geo_data & gconfig ) noexcept
    {
        // bind vertex buffer
        {
            glBindBuffer( GL_ARRAY_BUFFER, gconfig.vb_id ) ;
            if( motor::ogl::error::check_and_log( 
                gl4_backend_log("glBindBuffer(GL_ARRAY_BUFFER)") ) ) 
                return false ;
        }

        // bind index buffer
        {
            GLint const id_ = gconfig.ib_elem_sib == 0 ? 0 : gconfig.ib_id ;
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ ) ;
            if( motor::ogl::error::check_and_log( 
                gl4_backend_log( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)" ) ) )
                return false ;
        }

        // disable old locations
        for( size_t i = 0; i < sconfig.attributes.size(); ++i )
        {
            glDisableVertexAttribArray( sconfig.attributes[ i ].loc ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDisableVertexAttribArray" ) ) ;
        }

        GLuint uiStride = gconfig.stride ;
        GLuint uiBegin = 0 ;
        GLuint uiOffset = 0 ;

        // sometimes some shader input attributes might not be used but 
        // are defined in the vertex layout. So a new location need to be
        // figured out so that those vertex buffer attributes could
        // be bound to the shader.
        GLuint max_loc = 0 ;
        if( sconfig.attributes.size() != gconfig.elements.size() )
        {
            for( auto const & a : sconfig.attributes )
            {
                max_loc = std::max( max_loc, a.loc ) ;
            }
        }

        for( auto & e : gconfig.elements )
        {
            uiBegin += uiOffset ;
            uiOffset = e.sib() ;

            auto iter = std::find_if( sconfig.attributes.begin(), sconfig.attributes.end(), 
                [&]( this_t::shader_data::attribute_variable_cref_t av )
            {
                return av.va == e.va ;
            } ) ;

            GLuint loc = 0 ;
            GLenum type = GL_NONE ;

            if( iter == sconfig.attributes.end() ) 
            {
                loc = ++max_loc ;
                type = motor::platform::gl3::convert( e.type ) ;

                motor::log::global_t::warning( gl4_backend_log( "Vertex attribute (" +
                    motor::graphics::to_string(e.va) + ") in shader (" + sconfig.name + ") not used."
                    "Will bind geometry (" +sconfig.name+ ") layout attribute to custom location (" 
                    + motor::to_string( uint_t(loc) ) + ").") ) ;
            }
            else
            {
                loc = iter->loc ;
                type = iter->type ;
            }

            glEnableVertexAttribArray( loc ) ;
            motor::ogl::error::check_and_log(
                gl4_backend_log( "glEnableVertexAttribArray" ) ) ;

            glVertexAttribPointer(
                loc,
                GLint( motor::graphics::size_of(e.type_struct) ),
                motor::ogl::complex_to_simple_type( type ),
                GL_FALSE,
                ( GLsizei ) uiStride,
                (const GLvoid*)(size_t)uiBegin 
                ) ;

            motor::ogl::error::check_and_log( gl4_backend_log( "glVertexAttribPointer" ) ) ;
        }
      
        {
            glBindVertexArray( 0 ) ;
            if( motor::ogl::error::check_and_log(
                gl4_backend_log( "glBindVertexArray" ) ) )
                return false ;
        }
        return true ;
    }

    //****************************************************************************************
    void_t post_link_uniforms( this_t::shader_data & config )
    {
        GLuint const program_id = config.pg_id ;

        GLint num_active_uniforms = 0 ;
        GLint name_length = 0 ;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORMS, &num_active_uniforms ) ;
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORMS" ) ;

        if( num_active_uniforms == 0 ) return ;

        config.uniforms.resize( num_active_uniforms ) ;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_length ) ;
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_MAX_LENGTH" ) ;

        GLint size ;
        GLenum gl_uniform_type ;

        motor::memory::malloc_guard<char> buffer( name_length ) ;

        for( GLint i = 0; i < num_active_uniforms; ++i )
        {
            glGetActiveUniform( program_id, i, name_length, 0, 
                &size, &gl_uniform_type, buffer ) ;
            if( motor::ogl::error::check_and_log( "[gl_33_api] : glGetActiveUniform" ) ) continue ;

            // build-ins are not needed in the shader variables container.
            if( name_length >= 3 && buffer.equals( 0, 'g' ) && buffer.equals( 1, 'l' ) && buffer.equals( 2, '_' ) ) continue ;

            GLuint const location_id = glGetUniformLocation( program_id, buffer ) ;
            if( motor::ogl::error::check_and_log( "[glGetUniformLocation]" ) ) continue ;

            if( motor::log::global_t::error( location_id == GLuint( -1 ), 
                gl4_backend_log( "invalid uniform location id." ) ) ) continue ;

            motor::string_t const variable_name = motor::string_t( char_cptr_t( buffer ) ) ;

            this_t::shader_data::uniform_variable_t vd ;
            vd.name = std::move( variable_name ) ;
            vd.loc = location_id ;
            vd.type = gl_uniform_type ;
            vd.uniform_funk = motor::ogl::uniform_funk( gl_uniform_type ) ;

            config.uniforms[ i ] = vd ;
        }
    }

    //****************************************************************************************
    void_t post_link_uniform_blocks( this_t::shader_data & config )
    {
        GLuint const program_id = config.pg_id ;

        GLint num_active_uniform_blocks = 0 ;
        GLint max_block_name_length = 0 ;
        GLint max_uniform_name_length = 0 ;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_BLOCKS, &num_active_uniform_blocks ) ;
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_BLOCKS" ) ;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length ) ;
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_MAX_LENGTH" ) ;

        if ( num_active_uniform_blocks == 0 ) return ;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &max_block_name_length ) ;
        motor::ogl::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_MAX_LENGTH" ) ;

        //GLint size ;
        //GLenum gl_uniform_type ;

        motor::memory::malloc_guard<char> block_name( max_block_name_length ) ;
        motor::memory::malloc_guard<char> uniform_name( max_uniform_name_length ) ;

        for ( GLuint i = 0; i < (GLuint)num_active_uniform_blocks; ++i )
        {
            GLsizei block_name_len = 0 ;

            glGetActiveUniformBlockName( program_id, i, (GLsizei) max_block_name_length, &block_name_len, block_name ) ;
            motor::ogl::error::check_and_log( "[glGetActiveUniformBlockName]" ) ;

            GLuint const block_index = glGetUniformBlockIndex( program_id, block_name ) ;

            int bp = 0;
        }
    }

    //****************************************************************************************
    size_t construct_image_config( size_t /*oid*/, motor::string_cref_t name, 
        motor::graphics::image_object_ref_t config )
    {
        // the name is unique
        {
            auto iter = std::find_if( _images.begin(), _images.end(),
                [&] ( this_t::image_data_cref_t config )
            {
                return config.name == name ;
            } ) ;

            if( iter != _images.end() )
                return iter - _images.begin() ;
        }

        size_t i = 0 ;
        for( ; i < _images.size(); ++i )
        {
            if( _images[ i ].tex_id == GLuint( -1 ) )
            {
                break ;
            }
        }

        if( i == _images.size() ) {
            _images.resize( i + 1 ) ;
        }

        // sampler
        if( _images[ i ].tex_id == GLuint( -1 ) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenTextures( 1, &id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGenTextures" ) ) ;

            _images[ i ].tex_id = id ;
        }

        {
            _images[ i ].name = name ;
            _images[ i ].type = motor::platform::gl3::convert( config.get_type() ) ;
        }

        {
            for( size_t j=0; j<(size_t)motor::graphics::texture_wrap_mode::size; ++j )
            {
                _images[ i ].wrap_types[ j ] = motor::platform::gl3::convert(
                    config.get_wrap( ( motor::graphics::texture_wrap_mode )j ) );
            }

            for( size_t j = 0; j < ( size_t ) motor::graphics::texture_filter_mode::size; ++j )
            {
                _images[ i ].filter_types[ j ] = motor::platform::gl3::convert(
                    config.get_filter( ( motor::graphics::texture_filter_mode )j ) );
            }
        }
        return i ;
    }

    //****************************************************************************************
    bool_t release_image_data( size_t const oid ) noexcept
    {
        auto & id = _images[ oid ] ;

        id.name = "released image" ;
        id.type = GL_NONE ;

        if( id.tex_id != GLuint( -1 ) )
        {
            glDeleteTextures( 1, &id.tex_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteTextures" ) ) ;

            id.tex_id = GLuint( -1 ) ;
        }
        
        id.sib = 0 ;

        return true ;
    }

    //****************************************************************************************
    size_t construct_msl_data( size_t oid, motor::graphics::msl_object_ref_t obj_ ) noexcept
    {
        // if the incoming msl shader is a library shader for example,
        // it does not need to have a associated background object
        oid = !obj_.name().empty() ? this_t::determine_oid( obj_.get_oid(_bid), obj_.name(), _msls ) : size_t(-1) ;

        motor::vector< motor::msl::symbol_t > config_symbols ;

        obj_.for_each_msl( motor::graphics::msl_api_type::msl_4_0, [&]( motor::string_in_t code )
        {
            motor::msl::post_parse::document_t doc = 
                motor::msl::parser_t( "gl4" ).process( code ) ;

            _mdb.insert( std::move( doc ), config_symbols ) ;
        } ) ; 

        motor::graphics::msl_object_t obj = obj_ ;

        for( auto const & c : config_symbols )
        {
            if( oid == size_t(-1) )
            {
                // this most likely came from a library dependency.
                // need to figure out the msl object associated
                // with this render configuration/object.
                // the msl object is required in order to reconstruct
                // the render_object and the shader_object.

                // 1. find the msl object associated to c
                // 2. use the found oid for further processing

                for( size_t i=0; i<_msls.size(); ++i )
                {
                    auto & d = _msls[i] ;

                    auto iter = std::find_if( d.ros.begin(), d.ros.end(), [&]( motor::graphics::render_object_cref_t di )
                    {
                        return di.name() == c.expand() ;
                    } ) ;
                    
                    if( iter == d.ros.end() ) continue ;
                    
                    oid = i ;
                    obj = _msls[oid].msl_obj ;
                    break ;
                }
            }

            // msl database contains render configuration 
            // which has not been configured by the user...
            if( oid == size_t(-1) ) 
            {
                motor::log::global_t::warning( "[gl4::construct_msl_data] : render configuration not found : " + c.expand() ) ;
                continue ;
            }

            motor::graphics::render_object_t ro( c.expand() ) ;
            motor::graphics::shader_object_t so( c.expand() ) ;

            motor::msl::generatable_t res = motor::msl::dependency_resolver_t().resolve( &_mdb, c ) ;
            if( res.missing.size() != 0 )
            {
                motor::log::global_t::warning( "We have missing symbols." ) ;
                for( auto const& s : res.missing )
                {
                    motor::log::global_t::status( s.expand() ) ;
                }
                continue ;
            }

            // inject default variable values into the 
            // variable sets
            {
                for ( auto & shd_ : res.config.shaders )
                {
                    for ( auto & var_ : shd_.variables )
                    {
                        if( var_.def_val == size_t(-1) ) continue ;

                        auto * df = res.config.def_values[ var_.def_val ] ;
                        if ( dynamic_cast<motor::msl::generic_default_value< motor::math::vec3f_t >*> ( df ) != nullptr )
                        {
                            using ptr_t = motor::msl::generic_default_value< motor::math::vec3f_t > * ;
                            ptr_t gdv = static_cast< ptr_t >( df ) ;
                            for ( auto & vs : obj.borrow_varibale_sets() )
                            {
                                vs->data_variable<motor::math::vec3f_t>( var_.name )->set( gdv->get() ) ;
                            }
                        }
                        else if ( dynamic_cast<motor::msl::texture_dv_ptr_t> ( df ) != nullptr )
                        {
                            using ptr_t = motor::msl::texture_dv_ptr_t ;
                            ptr_t gdv = static_cast<ptr_t>( df ) ;
                            for ( auto & vs : obj.borrow_varibale_sets() )
                            {
                                // have the type here
                                // gdv->get().t == motor::msl::texture_tag_dv::type::tex1d
                                vs->texture_variable( var_.name )->set( gdv->get().name ) ;
                            }
                        }
                    }
                }
            }

            motor::msl::generator_t gen( std::move( res ) ) ;

            {
                auto const code = gen.generate<motor::msl::glsl::glsl4_generator_t>() ;
                motor::graphics::msl_bridge::create_by_api_type( motor::graphics::shader_api_type::glsl_4_0, code, so ) ;
            }

            {
                size_t const sid = this_t::construct_shader_data( so.get_oid( _bid ), so ) ;
                if( sid == size_t(-1) )
                {
                    return oid ;
                }
                so.set_oid( _bid, sid ) ;
            }

            {
                if( obj.get_streamout().size() != 0 && obj.get_geometry().size() != 0)
                {
                    ro.link_geometry( obj.get_geometry()[0], obj.get_streamout()[0] ) ; 
                }
                else
                {
                    ro.link_geometry( obj.get_geometry() ) ;
                }
            }

            ro.link_shader( c.expand() ) ;
            ro.add_variable_sets( obj.get_varibale_sets() ) ;
            ro.set_oid( _bid, this_t::construct_render_data( ro.get_oid( _bid ), ro ) ) ;
            

            auto & msl = _msls[oid] ;

            // render object
            {
                auto iter = std::find_if( msl.ros.begin(), msl.ros.end(), [&]( motor::graphics::render_object_cref_t rol )
                {
                    return rol.name() == c.expand() ;
                } ) ;

                if( iter != msl.ros.end() )
                {
                    *iter = std::move( ro ) ;
                }
                else
                {
                    msl.ros.emplace_back( std::move( ro ) ) ;
                }
            }

            // shader object
            {
                // reflect compilation result to the user
                {
                    auto & shd = _shaders[ so.get_oid( _bid ) ] ;

                    obj.for_each( [&] ( motor::graphics::compilation_listener_mtr_t lst )
                    {
                        lst->set( shd.is_linkage_ok, so.shader_bindings() ) ;
                    } ) ;
                }

                auto iter = std::find_if( msl.sos.begin(), msl.sos.end(), [&]( motor::graphics::shader_object_cref_t rol )
                {
                    return rol.name() == c.expand() ;
                } ) ;

                if( iter != msl.sos.end() )
                {
                    *iter = std::move( so ) ;
                }
                else
                {
                    msl.sos.emplace_back( std::move( so ) ) ;
                }
            }
            
            msl.msl_obj = obj ;
        }
        return oid ;
    }

    //****************************************************************************************
    size_t construct_render_data( size_t oid, motor::graphics::render_object_ref_t obj ) noexcept
    {
        oid = determine_oid( obj.get_oid(_bid), obj.name(), _renders ) ;

        auto & rd = _renders[ oid ] ;

        {
            rd.name = obj.name() ;
            
            for( auto id : rd.geo_ids ) _geometries[id].remove_render_data_id( oid ) ;
            rd.geo_ids.clear();

            for( auto id : rd.tf_ids )
            {
                _feedbacks[id].remove_render_data_id( oid ) ;
            }
            rd.tf_ids.clear() ;

            rd.shd_id = size_t( -1 ) ;

            motor::memory::global_t::dealloc( _renders[ oid ].mem_block ) ;
            rd.mem_block = nullptr ;

            for( auto & d : rd.geo_to_vaos ) 
            {
                glDeleteVertexArrays( 1, &d.vao ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteVertexArrays" ) ) ;
            }
            rd.geo_to_vaos.clear() ;
        }

        if( !this_t::construct_render_data_ext( oid, obj ) )
        {
            motor::log::global_t::error("[gl4] : construct_render_data update failed") ;
        }

        return oid ;
    }

    //****************************************************************************************
    bool_t release_render_data( size_t const oid, bool_t const do_gl_release ) noexcept
    {
        auto & rd = _renders[ oid ] ;

        rd.valid = false ;
        rd.name = "released" ;

        for( auto id : rd.geo_ids ) _geometries[ id ].remove_render_data_id( oid ) ;
        rd.geo_ids.clear() ;
        
        rd.shd_id = GLuint( -1 ) ;
        rd.rss.clear() ;
        
        for( auto * v : rd.var_sets ) motor::memory::release_ptr( v ) ;

        // remember that rd.var_sets hold the ref count reminder!
        rd.var_sets.clear() ;
        rd.var_sets_array.clear() ;
        rd.var_sets_streamout.clear() ;
        rd.var_sets_data.clear() ;
        rd.var_sets_texture.clear() ;

        motor::memory::global_t::dealloc( rd.mem_block ) ;
        rd.mem_block = nullptr ;

        if( do_gl_release )
        {
            for( auto & d : rd.geo_to_vaos ) 
            {
                glDeleteVertexArrays( 1, &d.vao ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteVertexArrays" ) ) ;
            }
        }
        rd.geo_to_vaos.clear() ;

        return true ;
    }

    //****************************************************************************************
    bool_t construct_render_data_ext( size_t const id, motor::graphics::render_object_ref_t rc ) noexcept
    {
        auto& config = _renders[ id ] ;

        // handle geometry links
        {
            // find geometry
            for( size_t i=0; i<rc.get_num_geometry(); ++i )
            {
                auto const iter = std::find_if( _geometries.begin(), _geometries.end(),
                    [&] ( this_t::geo_data const& d )
                {
                    return d.name == rc.get_geometry(i) ;
                } ) ;

                if( iter == _geometries.end() )
                {
                    motor::log::global_t::warning( gl4_backend_log(
                        "no geometry with name [" + rc.get_geometry() + "] for render_data [" + rc.name() + "]" ) ) ;
                    continue ;
                }

                config.geo_ids.emplace_back( std::distance( _geometries.begin(), iter ) ) ;

                // add this render data id to the new geometry
                _geometries[ config.geo_ids.back() ].add_render_data_id( id ) ;
            }
        }

        // handle stream out links
        {
            // remove this render data id from the old feedback
            for( auto tfid : config.tf_ids ) _feedbacks[ tfid ].remove_render_data_id( id ) ;
            config.tf_ids.clear() ;

            // find stream out object
            for( size_t i=0; i<rc.get_num_streamout(); ++i )
            {
                auto const iter = std::find_if( _feedbacks.begin(), _feedbacks.end(),
                    [&] ( this_t::tf_data const& d )
                {
                    return d.name == rc.get_streamout(i) ;
                } ) ;

                if( iter == _feedbacks.end() )
                {
                    motor::log::global_t::warning( gl4_backend_log(
                        "no streamout object with name [" + rc.get_streamout() + "] for render_data [" + rc.name() + "]" ) ) ;
                    return false ;
                }

                config.tf_ids.emplace_back( std::distance( _feedbacks.begin(), iter ) ) ;
            }
        }

        // find shader
        {
            auto const iter = std::find_if( _shaders.begin(), _shaders.end(),
                [&] ( this_t::shader_data const& d )
            {
                return d.name == rc.get_shader() ;
            } ) ;
            if( iter == _shaders.end() )
            {
                motor::log::global_t::warning( gl4_backend_log(
                    "no shader with name [" + rc.get_shader() + "] for render_data [" + rc.name() + "]" ) ) ;
                return false ;
            }

            config.shd_id = std::distance( _shaders.begin(), iter ) ;
        }

        if( !_shaders[config.shd_id].is_linkage_ok )
        {
            int bp = 0 ;
        }

        {
            // remember that rd.var_sets hold the ref count reminder!
            // no need to release any managed pointer!
            auto vars = std::move( config.var_sets ) ;
            config.var_sets_data.clear() ;
            config.var_sets_texture.clear() ;
            config.var_sets_array.clear() ;
            config.var_sets_streamout.clear() ;
            config.var_sets.clear() ;

            rc.for_each( [&] ( size_t const i, motor::graphics::variable_set_mtr_t vs )
            {
                auto const res = this_t::connect( config, i, motor::memory::copy_ptr(vs) ) ;
                motor::log::global_t::warning( !res, gl4_backend_log( "connect" ) ) ;
            } ) ;

            for( auto * ptr : vars ) motor::memory::release_ptr( ptr ) ;
        }
        
        this_t::render_object_variable_memory( config, _shaders[ config.shd_id ] ) ;

        return true ;
    }

    bool_t bind_attributes( GLuint const vao, size_t const sid, size_t const gid ) noexcept
    {
        auto & sd = _shaders[sid] ;
        auto & gd = _geometries[ gid ] ;

        {
            glBindVertexArray( vao ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindVertexArray" ) ) )
                return false ;
        }

        return this_t::bind_attributes( sd, gd ) ;
    }

    //****************************************************************************************
    void_t render_object_variable_memory( this_t::render_data & rd, this_t::shader_data & shader )
    {
        auto& config = rd ;

        // construct memory block
        {
            size_t sib = 0 ;
            for( auto & link : config.var_sets_data )
            {
                auto & uv = shader.uniforms[ link.uniform_id ] ;
                sib += motor::ogl::uniform_size_of( uv.type ) ;
            }

            for ( auto & link : config.var_sets_texture )
            {
                auto & uv = shader.uniforms[ link.uniform_id ] ;
                sib += motor::ogl::uniform_size_of( uv.type ) ;
            }

            for ( auto & link : config.var_sets_array )
            {
                auto & uv = shader.uniforms[ link.uniform_id ] ;
                sib += motor::ogl::uniform_size_of( uv.type ) ;
            }

            for ( auto & link : config.var_sets_streamout )
            {
                auto & uv = shader.uniforms[ link.uniform_id ] ;
                sib += motor::ogl::uniform_size_of( uv.type ) ;
            }

            motor::memory::global_t::dealloc( config.mem_block ) ;
            config.mem_block = motor::memory::global_t::alloc( sib, "[gl3] : render config memory block" ) ;
        }

        // assign memory locations
        {
            void_ptr_t mem = config.mem_block ;
            for( auto & link : config.var_sets_data )
            {
                link.mem = mem ;

                auto & uv = shader.uniforms[ link.uniform_id ] ;
                mem = reinterpret_cast<void_ptr_t>( size_t( mem ) +
                    size_t( motor::ogl::uniform_size_of( uv.type ) ) ) ;
            }

            for ( auto & link : config.var_sets_texture )
            {
                link.mem = mem ;

                auto & uv = shader.uniforms[ link.uniform_id ] ;
                mem = reinterpret_cast<void_ptr_t>( size_t( mem ) +
                    size_t( motor::ogl::uniform_size_of( uv.type ) ) ) ;
            }

            for ( auto & link : config.var_sets_array )
            {
                link.mem = mem ;

                auto & uv = shader.uniforms[ link.uniform_id ] ;
                mem = reinterpret_cast<void_ptr_t>( size_t( mem ) +
                    size_t( motor::ogl::uniform_size_of( uv.type ) ) ) ;
            }

            for ( auto & link : config.var_sets_streamout )
            {
                link.mem = mem ;

                auto & uv = shader.uniforms[ link.uniform_id ] ;
                mem = reinterpret_cast<void_ptr_t>( size_t( mem ) +
                    size_t( motor::ogl::uniform_size_of( uv.type ) ) ) ;
            }
        }
    }

    //****************************************************************************************
    size_t construct_geo( size_t oid, motor::string_in_t name, motor::graphics::vertex_buffer_in_t vb ) noexcept
    {
        oid = determine_oid( oid, name, _geometries ) ;

        bool_t error = false ;
        auto& config = _geometries[ oid ] ;

        // vertex buffer
        if( config.vb_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenBuffers( 1, &id ) ;
            error = motor::ogl::error::check_and_log( 
                gl4_backend_log( "Vertex Buffer creation" ) ) ;

            config.vb_id = id ;
        }

        // index buffer
        if( config.ib_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenBuffers( 1, &id ) ;
            error = motor::ogl::error::check_and_log(
                gl4_backend_log( "Index Buffer creation" ) ) ;

            config.ib_id = id ;
        }

        {
            config.name = name ;
            config.stride = GLuint( vb.get_layout_sib() ) ;

            config.elements.clear() ;
            vb.for_each_layout_element( 
                [&]( motor::graphics::vertex_buffer_t::data_cref_t d )
            {
                this_t::geo_data::layout_element le ;
                le.va = d.va ;
                le.type = d.type ;
                le.type_struct = d.type_struct ;
                config.elements.push_back( le ) ;
            }) ;
        }

        motor::log::global_t::error( error, gl4_backend_log("Error ocurred for ["+ name +"]") ) ;

        return oid ;
    }

    //****************************************************************************************
    size_t construct_geo( size_t oid, motor::graphics::geometry_object_ref_t obj ) noexcept
    {
        return this_t::construct_geo( oid, obj.name(), obj.vertex_buffer() ) ;
    }

    //****************************************************************************************
    bool_t release_geometry( size_t const oid ) noexcept
    {
        auto & geo = _geometries[ oid ] ;

        geo.name = "released" ;
        geo.valid = false ;
        
        geo.elements.clear() ;
        geo.ib_elem_sib = 0 ;
        geo.stride = 0 ;

        if( geo.vb_id != GLuint( -1 ) )
        {
            glDeleteBuffers( 1, &geo.vb_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteBuffers" ) ) ;
            geo.vb_id = GLuint( -1 ) ;
        }

        if( geo.ib_id != GLuint( -1 ) )
        {
            glDeleteBuffers( 1, &geo.ib_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteBuffers" ) ) ;
            geo.ib_id = GLuint( -1 ) ;
        }

        geo.ib_elem_sib = 0 ;

        for( auto const & rd_id : geo.rd_ids )
        {
            if( rd_id == size_t( -1 ) ) continue ;
            auto & rd = _renders[ rd_id ] ;
            rd.remove_geometry_id( oid ) ;
        }
        geo.rd_ids.clear() ;

        return true ;
    }

    //****************************************************************************************
    bool_t update( size_t const id, motor::graphics::geometry_object_mtr_t geo, bool_t const is_config = false )
    {
        auto& config = _geometries[ id ] ;

        {
            config.num_elements_ib = geo->index_buffer().get_num_elements() ;
            config.num_elements_vb = geo->vertex_buffer().get_num_elements() ;
            config.ib_elem_sib = 0 ;
            config.ib_type = GL_UNSIGNED_INT ;
            config.pt = motor::platform::gl3::convert( geo->primitive_type() ) ;
        }

        // bind vertex buffer
        {
            glBindBuffer( GL_ARRAY_BUFFER, config.vb_id ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log("glBindBuffer - vertex buffer") ) )
                return false ;
        }

        // allocate buffer memory
        // what about mapped memory?
        {
            GLuint const sib = GLuint( geo->vertex_buffer().get_sib() ) ;
            if( is_config || sib > config.sib_vb )
            {
                glBufferData( GL_ARRAY_BUFFER, sib,
                    geo->vertex_buffer().data(), GL_STATIC_DRAW ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log( "glBufferData - vertex buffer" ) ) )
                    return false ;
                config.sib_vb = sib ;
            }
            else
            {
                glBufferSubData( GL_ARRAY_BUFFER, 0, sib, geo->vertex_buffer().data() ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glBufferSubData - vertex buffer" ) ) ;
            }
        }

        // allocate buffer memory
        // what about mapped memory?
        if( geo->index_buffer().get_num_elements() > 0 )
        {
            // bind index buffer
            {
                GLint const id_ = geo->index_buffer().get_num_elements() == 0 ? 0 : config.ib_id ;
                glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindBuffer - index buffer" ) ) )
                    return false ;
            }

            config.ib_elem_sib = geo->index_buffer().get_element_sib() ;

            GLuint const sib = GLuint( geo->index_buffer().get_sib() ) ;
            if( is_config || sib > config.sib_ib )
            {
                glBufferData( GL_ELEMENT_ARRAY_BUFFER, sib,
                    geo->index_buffer().data(), GL_STATIC_DRAW ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log( "glBufferData - index buffer" ) ) )
                    return false ;
                config.sib_ib = sib ;
            }
            else
            {
                glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, sib, geo->index_buffer().data() ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glBufferSubData - index buffer" ) ) ;
            }
        }

        if( is_config ) 
        {
            // force rebind of vertex attributes. Layout may have changed.
            for( auto const & rid : config.rd_ids )
            {
                if( rid == size_t( -1 ) ) continue ;
                if( _renders[ rid ].shd_id == size_t( -1 ) ) continue ;
                
                auto iter = std::find_if( _renders[ rid ].geo_to_vaos.begin(), _renders[ rid ].geo_to_vaos.end(), 
                    [&]( this_t::render_data::geo_to_vao const & d )
                    {
                        return d.gid == id ;
                    } ) ;

                // this may happen if the geometry is configured a second time
                // after the render object is configured but the render object
                // is not rendered yet.
                if( iter != _renders[ rid ].geo_to_vaos.end() )
                {
                    glDeleteVertexArrays( 1, &(iter->vao) ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteVertexArrays" ) ) ;

                    _renders[ rid ].geo_to_vaos.erase( iter ) ;
                }
            }
        }

        return true ;
    }

    //****************************************************************************************
    // @param is_config used to determine recreating the texture on the gpu side.
    //          This param should be true if an image is configured or re-configured.
    bool_t update( size_t const id, motor::graphics::image_object_ref_t confin, bool_t const is_config = false )
    {
        this_t::image_data_ref_t config = _images[ id ] ;

        glBindTexture( config.type, config.tex_id ) ;
        if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) )
            return false ;

        size_t const sib = confin.image().sib() ;
        GLenum const target = config.type ;
        GLint const level = 0 ;
        GLsizei const width = GLsizei( confin.image().get_dims().x() ) ;
        GLsizei const height = GLsizei( confin.image().get_dims().y() ) ;
        GLsizei const depth = GLsizei( confin.image().get_dims().z() ) ;
        GLenum const format = motor::platform::gl3::convert_to_gl_pixel_format( confin.image().get_image_format() ) ;
        GLenum const type = motor::platform::gl3::convert_to_gl_pixel_type( confin.image().get_image_element_type() ) ;
        void_cptr_t data = confin.image().get_image_ptr() ;

        // determine how to unpack the data
        // ideally, should be divideable by 4
        {
            GLint unpack = 0 ;

            if( width % 4 == 0 ) unpack = 4 ;
            else if( width % 3 == 0 ) unpack = 3 ;
            else if( width % 3 == 0 ) unpack = 2 ;
            else unpack = 1 ;

            glPixelStorei( GL_UNPACK_ALIGNMENT, unpack ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glPixelStorei" ) ) ;
        }

        if( is_config || ( sib == 0 || config.sib < sib ) )
        {
            GLint const border = 0 ;
            GLint const internal_format = motor::platform::gl3::convert_to_gl_format( confin.image().get_image_format(), confin.image().get_image_element_type() ) ;

            if( target == GL_TEXTURE_2D )
            {
                glTexImage2D( target, level, internal_format, width, height,
                    border, format, type, data ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glTexImage2D" ) ) ;
            }
            else if( target == GL_TEXTURE_2D_ARRAY )
            {
                glTexImage3D( target, level, internal_format, width, height, depth,
                    border, format, type, data ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glTexImage3D" ) ) ;
            }
        }
        else
        {
            GLint const xoffset = 0 ;
            GLint const yoffset = 0 ;
            GLint const zoffset = 0 ;

            if( target == GL_TEXTURE_2D )
            {
                glTexSubImage2D( target, level, xoffset, yoffset, width, height,
                    format, type, data ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glTexSubImage2D" ) ) ;
            }
            else if( target == GL_TEXTURE_2D_ARRAY )
            {
                glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth,
                    format, type, data ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glTexSubImage3D" ) ) ;
            }
            
        }

        config.sib = confin.image().sib() ;

        return false ;
    }

    //****************************************************************************************
    bool_t connect( this_t::render_data & config, size_t const var_set_idx, motor::graphics::variable_set_mtr_t vs )
    {
        this_t::shader_data_ref_t shd = _shaders[ config.shd_id ] ;

        size_t id = 0 ;
        for( auto& uv : shd.uniforms )
        {
            // is it a data uniform variable?
            if( motor::ogl::uniform_is_data( uv.type ) )
            {
                auto const types = motor::platform::gl3::to_type_type_struct( uv.type ) ;
                auto* var = vs->data_variable( uv.name, types.first, types.second ) ;
                if( var == nullptr )
                {
                    motor::log::global_t::error( gl4_backend_log( "can not claim variable " + uv.name ) ) ;
                    continue ;
                }

                this_t::render_data::uniform_variable_link link ;
                link.var_set_idx = var_set_idx ;
                link.uniform_id = id++ ;
                link.var = var ;

                config.var_sets_data.emplace_back( link ) ;
            }
            else if( motor::ogl::uniform_is_texture( uv.type ) )
            {
                //auto const types = motor::platform::gl3::to_type_type_struct( uv.type ) ;
                auto* var = vs->texture_variable( uv.name ) ;

                if( var == nullptr )
                {
                    motor::log::global_t::error( gl4_backend_log( "can not claim variable " + uv.name ) ) ;
                    continue ;
                }

                // looking for image
                {
                    size_t i = 0 ;
                    auto const& tx_name = var->get() ;
                    for( auto& cfg : _images )
                    {
                        if( tx_name == cfg.name ) break ;
                        ++i ;
                    }

                    if( i >= _images.size() )
                    {
                        motor::log::global_t::error( gl4_backend_log( "Could not find image [" +
                            tx_name.name() + "]" ) ) ;
                        continue ;
                    }

                    this_t::render_data::uniform_texture_link link ;
                    link.var_set_idx = var_set_idx ;
                    link.uniform_id = id++ ;
                    link.tex_id = _images[ i ].tex_id ;
                    link.img_id = i ;
                    config.var_sets_texture.emplace_back( link ) ;
                }
            }
            else if( motor::ogl::uniform_is_buffer( uv.type ) )
            {
                motor::string_t tx_name ;

                {
                    tx_name = vs->array_variable( uv.name )->get().name() ;

                    if ( tx_name.empty() )
                        tx_name = vs->array_variable_streamout( uv.name )->get().name() ;
                }

                // looking for data buffer
                auto handle_buffer_link = [&]( void_t )
                {
                    size_t const i = this_t::find_index_by_resource_name( tx_name, _arrays ) ;
                    if( i >= _arrays.size() ) return false ;

                    this_t::render_data::uniform_array_data_link link ;
                    link.var_set_idx = var_set_idx ;
                    link.uniform_id = id++ ;
                    link.tex_id = _arrays[ i ].tex_id ;
                    link.buf_id = i ;
                    config.var_sets_array.emplace_back( link ) ;
                    return true ;
                } ;

                // looking for streamout/transform feedback
                auto handle_feedback_link = [&]( void_t )
                {
                    size_t const i = this_t::find_index_by_resource_name( tx_name, _feedbacks ) ;
                    if( i >= _feedbacks.size() ) return false ;

                    this_t::render_data::uniform_streamout_link link ;
                    link.var_set_idx = var_set_idx ;
                    link.uniform_id = id++ ;
                    link.tex_id[0] = _feedbacks[ i ]._buffers[0].tids[0] ;
                    link.tex_id[1] = _feedbacks[ i ]._buffers[1].tids[0] ;
                    link.so_id = i ;
                    config.var_sets_streamout.emplace_back( link ) ;
                    return true ;
                } ;

                if( !handle_buffer_link() )
                {
                    if( !handle_feedback_link() )
                    {
                        motor::log::global_t::error( gl4_backend_log( 
                          "Could not find array nor streamout object [" + tx_name + "]" ) ) ;
                        continue ;
                    }
                }
            }
        }

        // ref count one copy here for all stored items
        config.var_sets.emplace_back( vs ) ;

        return true ;
    }

    //****************************************************************************************
    size_t construct_array_data( size_t oid, motor::graphics::array_object_ref_t obj ) noexcept
    {
        oid = determine_oid( oid, obj.name(), _arrays ) ;

        auto & data = _arrays[ oid ] ;

        // buffer
        if( data.buf_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenBuffers( 1, &id ) ;
            motor::ogl::error::check_and_log( 
                gl4_backend_log( "[construct_array_data] : glGenBuffers" ) ) ;

            data.buf_id = id ;
        }

        // texture
        if( data.tex_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenTextures( 1, &id ) ;
            motor::ogl::error::check_and_log( 
                gl4_backend_log( "[construct_array_data] : glGenTextures" ) ) ;

            data.tex_id = id ;
        }

        return oid ;
    }

    //****************************************************************************************
    bool_t release_array_data( size_t const oid ) noexcept
    {
        auto & data = _arrays[ oid ] ;

        if( data.buf_id == GLuint(-1) )
        {
            glDeleteBuffers( 1, &data.buf_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteBuffers" ) ) ;
            data.buf_id = GLuint( -1 ) ;
        }

        if( data.tex_id == GLuint(-1) )
        {
            glDeleteTextures( 1, &data.tex_id ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteTextures" ) ) ;
            data.tex_id = GLuint( -1 ) ;
        }

        data.valid = false ;
        data.name = "released" ;
        data.sib = 0 ;

        return true ;
    }

    //****************************************************************************************
    bool_t update( size_t oid, motor::graphics::array_object_ref_t obj, bool_t const is_config = false ) 
    {
        auto & data = _arrays[ oid ] ;

        // do buffer
        {
            // bind buffer
            {
                glBindBuffer( GL_TEXTURE_BUFFER, data.buf_id ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log("glBindBuffer") ) )
                    return false ;
            }

            // transfer data
            GLuint const sib = GLuint( obj.data_buffer().get_sib() ) ;
            if( is_config || sib > data.sib )
            {
                glBufferData( GL_TEXTURE_BUFFER, sib,
                        obj.data_buffer().data(), GL_DYNAMIC_DRAW ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log( "glBufferData" ) ) )
                    return false ;
                data.sib = sib ;
            }
            else
            {
                glBufferSubData( GL_TEXTURE_BUFFER, 0, sib, obj.data_buffer().data() ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glBufferSubData" ) ) ;
            }

            // bind buffer
            {
                glBindBuffer( GL_TEXTURE_BUFFER, 0 ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log("glBindBuffer") ) )
                    return false ;
            }
        }

        // do texture
        // glTexBuffer is required to be called after driver memory is aquired.
        {
            glBindTexture( GL_TEXTURE_BUFFER, data.tex_id ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) )
                return false ;

            auto const le = obj.data_buffer().get_layout_element(0) ;
            glTexBuffer( GL_TEXTURE_BUFFER, motor::platform::gl3::convert_for_texture_buffer(
                le.type, le.type_struct ), data.buf_id ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log( "glTexBuffer" ) ) )
                return false ;
        }

        return true ;
    }

    //****************************************************************************************
    size_t construct_feedback( size_t oid, motor::graphics::streamout_object_ref_t obj ) noexcept
    {
        oid = determine_oid( oid, obj.name(), _feedbacks ) ;

        //bool_t error = false ;
        auto & data = _feedbacks[ oid ] ;

        size_t const req_buffers = std::min( obj.num_buffers(), this_t::tf_data::buffer::max_buffers ) ;

        // rw : read/write - ping pong buffers
        for( size_t rw=0; rw<2; ++rw )
        {
            auto & buffer = data._buffers[rw] ;

            for( size_t i=0; i<req_buffers; ++i )
            {
                if( buffer.bids[i] != GLuint(-1) ) continue ;
                
                motor::string_t const is = motor::to_string( i ) ;
                motor::string_t const bs = motor::to_string( rw ) ;

                size_t const gid = this_t::construct_geo( size_t(-1), 
                    obj.name() + ".feedback."+is+"."+bs, obj.get_buffer(i) ) ;

                buffer.gids[i] = gid ;
                buffer.bids[i] = _geometries[ gid ].vb_id ;
                buffer.sibs[i] = 0 ; 
            }

            // we just always gen max buffers tex ids
            {
                glGenTextures( this_t::tf_data::buffer::max_buffers, buffer.tids ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glGenTextures" ) ) ;
            }

            for( size_t i=req_buffers; i<this_t::tf_data::buffer::max_buffers; ++i )
            {
                if( buffer.gids[i] == size_t(-1) ) break ;

                this_t::release_geometry( buffer.gids[i] ) ;

                buffer.bids[i] = GLuint( -1 ) ;
                buffer.gids[i] = size_t( -1 ) ;
            }

            if( buffer.qid == GLuint(-1) )
            {
                glGenQueries( 1, &buffer.qid ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glGenQueries" ) ) ;
            }

            if( buffer.tfid == GLuint(-1) )
            {
                glGenTransformFeedbacks( 1, &buffer.tfid ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glGenTransformFeedbacks" ) ) ;
            }
        }

        return oid ;
    }

    //****************************************************************************************
    bool_t release_tf_data( size_t const oid ) noexcept
    {
        auto & d = _feedbacks[ oid ] ;

        for( size_t rw=0; rw<2; ++rw )
        {
            auto & buffer = d._buffers[rw] ;

            for( GLuint i=0; i<tf_data_t::buffer::max_buffers; ++i )
            {
                if( buffer.gids[i] == size_t( -1 ) ) break ;

                this_t::release_geometry( buffer.gids[i] ) ;
            
                buffer.bids[i] = GLuint( -1 ) ; 
                buffer.sibs[i] = 0 ;
            }

            if( buffer.qid != GLuint(-1) )
            {
                glDeleteQueries( 1, &buffer.qid ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteBuffers" ) ) ;
                buffer.qid = GLuint( -1 ) ; 
            }

            if( buffer.tfid != GLuint(-1) )
            {
                glDeleteTransformFeedbacks( 1, &buffer.tfid ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteTransformFeedbacks" ) ) ;
                buffer.tfid = GLuint( -1 ) ; 
            }

            // release textures
            {
                glDeleteTextures( tf_data_t::buffer::max_buffers, buffer.tids ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDeleteTextures" ) ) ;
            }
        }

        d.valid = false ;
        d.name = "__released__" ;

        return true ;
    }

    //****************************************************************************************
    bool_t update( size_t oid, motor::graphics::streamout_object_ref_t obj, bool_t const is_config = false ) 
    {
        auto & data = _feedbacks[ oid ] ;

        for( size_t rw=0; rw<2; ++rw )
        {
            auto & buffer = data._buffers[rw] ;

            size_t i=0; 
            while( buffer.bids[i] != GLuint(-1) )
            {
                // bind buffer
                {
                    glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, buffer.bids[i] ) ;
                    if( motor::ogl::error::check_and_log( gl4_backend_log("glBindBuffer") ) )
                        continue ;
                }

                // allocate data
                GLuint const sib = GLuint( obj.get_buffer( i ).get_layout_sib() * obj.size() ) ;
                if( is_config || sib > buffer.sibs[i] )
                {
                    glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, sib, nullptr, GL_DYNAMIC_DRAW ) ;
                    if( motor::ogl::error::check_and_log( gl4_backend_log( "glBufferData" ) ) )
                        continue ;

                    buffer.sibs[i] = sib ;
                }
               
                // do texture
                // glTexBuffer is required to be called after driver memory is aquired.
                {
                    glBindTexture( GL_TEXTURE_BUFFER, buffer.tids[i] ) ;
                    if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) )
                        continue ;

                    auto const le = obj.get_buffer( i ).get_layout_element_zero() ;
                    glTexBuffer( GL_TEXTURE_BUFFER, motor::platform::gl3::convert_for_texture_buffer(
                        le.type, le.type_struct ), buffer.bids[i] ) ;
                    if( motor::ogl::error::check_and_log( gl4_backend_log( "glTexBuffer" ) ) )
                        continue ;

                    glBindTexture( GL_TEXTURE_BUFFER, 0 ) ;
                    if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) )
                        continue ;
                }

                ++i ;
            }
        }

        // unbind
        {
            glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, 0 ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log("glBindBuffer") ) )
                return false ;
        }

        for( size_t rw=0; rw<2; ++rw )
        {
            auto & buffer = data._buffers[rw] ;

            {
                glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, buffer.tfid ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log("glBindTransformFeedback") ) )
                    return false ;
            }

            // bind buffers
            {
                for( size_t i=0; i<tf_data::buffer::max_buffers; ++i )
                {
                    auto const bid = buffer.bids[ i ] ;
                    auto const sib = buffer.sibs[ i ] ;

                    if( bid == GLuint(-1) ) break ;

                    glBindBufferRange( GL_TRANSFORM_FEEDBACK_BUFFER, GLuint(i), bid, 0, sib ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glBindBufferRange" ) ) ;
                }
            }
        }

        return false ;
    }

    //****************************************************************************************
    bool_t use_transform_feedback( size_t const oid ) noexcept
    {
        if( oid == _tf_active_id ) return true ;
        _tf_active_id = oid ;

        return true ;
    }

    //****************************************************************************************
    void_t unuse_transform_feedback( void_t ) noexcept
    {
        _tf_active_id = size_t(-1) ;
    }

    //****************************************************************************************
    void_t activate_transform_feedback( this_t::geo_data & gdata ) noexcept
    {
        if( _tf_active_id == size_t(-1) ) return ;

        auto & data = _feedbacks[_tf_active_id] ;
        
        auto const wrt_idx = data.write_index() ;
        auto & buffer = data._buffers[wrt_idx] ;

        // EITHER just set prim type what was used for rendering.
        // OR use for overwriting the primitive type on render
        //data.pt = gdata.pt ;

        // bind transform feedback object and update primitive type for 
        // rendering the transform feedback data.
        {
            for( size_t i=0; i<tf_data::buffer::max_buffers; ++i )
            {
                if( buffer.bids[ i ] == GLuint(-1) ) break ;
                _geometries[buffer.gids[ i ]].pt = gdata.pt ;
            }

            glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, buffer.tfid ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glBindTransformFeedback" ) ) ;
        }

        // query written primitives
        {
            glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, buffer.qid ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glBeginQuery" ) ) ;
        }

        // begin 
        {
            // lets just use the primitive type of the used geometry
            GLenum const mode = gdata.pt ;
            glBeginTransformFeedback( mode ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glBeginTransformFeedback" ) ) ;
        }
    }

    //****************************************************************************************
    void_t deactivate_transform_feedback( void_t ) noexcept
    {
        if( _tf_active_id == size_t(-1) ) return ;

        {
            glEndTransformFeedback() ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glEndTransformFeedback" ) ) ;
        }

        // query written primitives
        {
            glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glEndQuery" ) ) ;
        }
        
        // swap the indices of the ping-pong buffers
        {
            _feedbacks[ _tf_active_id ].swap_index() ;
        }

        #if 0
        GLuint num_prims = 0 ;
        {
            auto & tfd = _feedbacks[ _tf_active_id] ;
            auto const ridx = tfd.read_index();
            glGetQueryObjectuiv( tfd.read_buffer().qid, GL_QUERY_RESULT, &num_prims ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glGetQueryObjectuiv" ) ) ;
        }
        #endif

        _tf_active_id = size_t(-1) ;
    }

    //****************************************************************************************
    bool_t update_variables( size_t const rd_id, size_t const varset_id ) noexcept
    {
        this_t::render_data & config = _renders[ rd_id ] ;
        this_t::shader_data & sconfig = _shaders[ config.shd_id ] ;

        {
            glUseProgram( sconfig.pg_id ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log( "glUseProgram" ) ) )
            {
                return false ;
            }
        }

        if( config.var_sets.size() > varset_id )
        {
            // data vars
            {
                for ( auto & link : config.var_sets_data )
                {
                    if ( link.var_set_idx > varset_id ) break ;
                    if ( link.var_set_idx < varset_id ) continue ;

                    auto & uv = sconfig.uniforms[ link.uniform_id ] ;
                    uv.do_copy_funk( link.mem, link.var ) ;
                }
            }

            // tex vars
            // this section must match with the section in the render function
            // because of the used texture unit
            {
                int_t tex_unit = 0 ;
                // textures
                {
                    for( auto& link : config.var_sets_texture )
                    {
                        if ( link.var_set_idx > varset_id ) break ;
                        if ( link.var_set_idx < varset_id ) continue ;

                        auto var = motor::graphics::data_variable< int_t >( tex_unit ) ;
                        auto & uv = sconfig.uniforms[ link.uniform_id ] ;

                        uv.do_copy_funk( link.mem, &var ) ;

                        ++tex_unit ;
                    }
                }

                // array data vars
                {
                    for( auto & link : config.var_sets_array )
                    {
                        if ( link.var_set_idx > varset_id ) break ;
                        if ( link.var_set_idx < varset_id ) continue ;

                        auto var = motor::graphics::data_variable< int_t >( tex_unit ) ;
                        auto & uv = sconfig.uniforms[ link.uniform_id ] ;

                        uv.do_copy_funk( link.mem, &var ) ;

                        ++tex_unit ;
                    }
                }

                // transform feedback bound vars
                {
                    for( auto & link : config.var_sets_streamout )
                    {
                        if ( link.var_set_idx > varset_id ) break ;
                        if ( link.var_set_idx < varset_id ) continue ;

                        auto var = motor::graphics::data_variable< int_t >( tex_unit ) ;
                        auto & uv = sconfig.uniforms[ link.uniform_id ] ;

                        uv.do_copy_funk( link.mem, &var ) ;

                        ++tex_unit ;
                    }
                }
            }
        }

        return true ;
    }

    //****************************************************************************************
    bool_t render( size_t const id, size_t const geo_idx = 0, bool_t feed_from_tf = false, 
                   bool_t const use_streamout_count = false, size_t const varset_id = size_t(0), 
                   GLsizei const start_element = GLsizei(0), GLsizei const num_elements = GLsizei(-1) )
    {
        MOTOR_PROBE( "Graphics", "[gl4] : render" ) ;

        this_t::render_data & config = _renders[ id ] ;
        this_t::shader_data & sconfig = _shaders[ config.shd_id ] ;

        size_t gid = size_t(-1) ;
        size_t tfid = size_t( -1 ) ;

        // #1 : check feedback geometry
        if( (feed_from_tf || use_streamout_count) && config.tf_ids.size() != 0 )
        {
            tfid = config.tf_ids[0] ;
            if( feed_from_tf )
                gid = _feedbacks[tfid].read_buffer().gids[0] ;
            else
                gid = config.geo_ids[ geo_idx ] ;
        }
        // #2 : check geometry 
        else if( config.geo_ids.size() > geo_idx )
        {
            gid = config.geo_ids[ geo_idx ] ;
        }
        // #3 : otherwise we can not render
        else
        {
            motor::log::global_t::error( "[gl3::render] : used geometry idx invalid because" 
                "exceeds array size for render object : " + config.name ) ;
                return false ;
        }

        this_t::geo_data & gconfig = _geometries[ gid ] ;

        // find vao
        {
            GLuint vao_id = GLuint(0) ;

            {
                auto iter = std::find_if( config.geo_to_vaos.begin(), config.geo_to_vaos.end(), 
                    [&]( this_t::render_data::geo_to_vao const & d )
                    {
                        return d.gid == gid ;
                    } ) ;
            
                if( iter == config.geo_to_vaos.end() )
                {                    
                    {
                        glGenVertexArrays( 1, &vao_id ) ;
                        if( motor::ogl::error::check_and_log( gl4_backend_log( "Vertex Array creation" ) ) )
                            return false ;
                    }

                    // bind vertex attributes to shader locations for vao
                    if( this_t::bind_attributes( vao_id, config.shd_id, gid ) )
                        config.geo_to_vaos.emplace_back( this_t::render_data::geo_to_vao{ gid, vao_id } ) ;
                }
                else vao_id = iter->vao ;
            }

            {
                glBindVertexArray( vao_id ) ;
                if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindVertexArray" ) ) )
                    return false ;
            }
        }

        if( !sconfig.is_linkage_ok ) return false ;

        {
            glUseProgram( sconfig.pg_id ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log( "glUseProgram" ) ) )
            {
                glBindVertexArray( 0 ) ;
                return false ;
            }
        }

        if( config.var_sets.size() > varset_id )
        {
            // data vars
            {
                for ( auto & link : config.var_sets_data )
                {
                    if ( link.var_set_idx > varset_id ) break ;
                    if ( link.var_set_idx < varset_id ) continue ;

                    auto & uv = sconfig.uniforms[ link.uniform_id ] ;
                    if ( !uv.do_uniform_funk( link.mem ) )
                    {
                        motor::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ;
                    }
                }
            }

            // tex vars
            // this section must match with the section in the update(render_object) function
            {
                int_t tex_unit = 0 ;
                // textures
                {
                    for( auto & link : config.var_sets_texture )
                    {
                        if ( link.var_set_idx > varset_id ) break ;
                        if ( link.var_set_idx < varset_id ) continue ;

                        glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) ) ;
                        motor::ogl::error::check_and_log( gl4_backend_log( "glActiveTexture" ) ) ;

                        {
                            auto const& ic = _images[ link.img_id ] ;  

                            glBindTexture( ic.type, link.tex_id ) ;
                            motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) ;

                            glTexParameteri( ic.type, GL_TEXTURE_WRAP_S, ic.wrap_types[0] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_WRAP_T, ic.wrap_types[1] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_WRAP_R, ic.wrap_types[2] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_MIN_FILTER, ic.filter_types[0] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_MAG_FILTER, ic.filter_types[1] ) ;
                            motor::ogl::error::check_and_log( gl4_backend_log( "glTexParameteri" ) ) ;
                        }

                        {
                            auto & uv = sconfig.uniforms[ link.uniform_id ] ;
                            if( !uv.do_uniform_funk( link.mem ) )
                            {
                                motor::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ;
                            }
                        }

                        ++tex_unit ;
                    }
                }

                // array data vars
                {
                    for( auto & link : config.var_sets_array )
                    {
                        if ( link.var_set_idx > varset_id ) break ;
                        if ( link.var_set_idx < varset_id ) continue ;

                        glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) ) ;
                        motor::ogl::error::check_and_log( gl4_backend_log( "glActiveTexture" ) ) ;
                        glBindTexture( GL_TEXTURE_BUFFER, link.tex_id ) ;
                        motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) ;

                        {
                            auto & uv = sconfig.uniforms[ link.uniform_id ] ;
                            if( !uv.do_uniform_funk( link.mem ) )
                            {
                                motor::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ;
                            }
                        }

                        ++tex_unit ;
                    }
                }

                // transform feedback as TBO
                {
                    for( auto & link : config.var_sets_streamout )
                    {
                        if ( link.var_set_idx > varset_id ) break ;
                        if ( link.var_set_idx < varset_id ) continue ;

                        auto const & tfd = _feedbacks[ link.so_id ] ;
                        GLuint const tid = link.tex_id[tfd.read_index()] ;

                        glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) ) ;
                        motor::ogl::error::check_and_log( gl4_backend_log( "glActiveTexture" ) ) ;
                        glBindTexture( GL_TEXTURE_BUFFER, tid ) ;
                        motor::ogl::error::check_and_log( gl4_backend_log( "glBindTexture" ) ) ;

                        {
                            auto & uv = sconfig.uniforms[ link.uniform_id ] ;
                            if( !uv.do_uniform_funk( link.mem ) )
                            {
                                motor::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ;
                            }
                        }

                        ++tex_unit ;
                    }
                }
            }
        }

        // disable raster if no pixel shader
        if( sconfig.ps_id == GLuint(-1) )
        {
            glEnable( GL_RASTERIZER_DISCARD ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glEnable( GL_RASTERIZER_DISCARD ) " ) ) ;
        }

        {
            this_t::activate_transform_feedback( gconfig ) ;
        }

        // render section
        {
            GLenum const pt = gconfig.pt ;
            //GLuint const ib = gconfig.ib_id ;
            //GLuint const vb = config.geo->vb_id ;

            if( tfid != size_t( -1 ) )
            {
                auto & tfd = _feedbacks[ tfid ] ;

                #if 0 // this is gl 3.x
                GLuint num_prims = 0 ;
                {
                    glGetQueryObjectuiv( tfd.read_buffer().qid, GL_QUERY_RESULT, &num_prims ) ;
                    motor::ogl::error::check_and_log( gl4_backend_log( "glGetQueryObjectuiv" ) ) ;
                }
                glDrawArrays( pt, start_element, num_prims * motor::platform::gl3::primitive_type_to_num_vertices( pt ) ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDrawArrays" ) ) ;
                #else
                glDrawTransformFeedback( pt, tfd.read_buffer().tfid ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDrawTransformFeedback" ) ) ;
                #endif

            }
            else if( gconfig.num_elements_ib > 0 )
            {
                GLsizei const max_elems = GLsizei( gconfig.num_elements_ib ) ;
                GLsizei const ne = std::min( num_elements>=0?num_elements:max_elems, max_elems ) ;

                GLenum const glt = gconfig.ib_type ;

                void_cptr_t offset = void_cptr_t( byte_cptr_t( nullptr ) + 
                    start_element * GLsizei( gconfig.ib_elem_sib ) ) ;

                glDrawElements( pt, ne, glt, offset ) ;

                motor::ogl::error::check_and_log( gl4_backend_log( "glDrawElements" ) ) ;
            }
            else
            {
                GLsizei const max_elems = GLsizei( gconfig.num_elements_vb ) ;
                GLsizei const ne = std::min( num_elements>=0?num_elements:max_elems, max_elems ) ;

                glDrawArrays( pt, start_element, ne ) ;
                motor::ogl::error::check_and_log( gl4_backend_log( "glDrawArrays" ) ) ;
            }
        }

        {
            this_t::deactivate_transform_feedback() ;
        }

        // enable raster back again.
        if( sconfig.ps_id == GLuint(-1) )
        {
            glDisable( GL_RASTERIZER_DISCARD ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glDisable( GL_RASTERIZER_DISCARD ) " ) ) ;
        }

        {
            glBindVertexArray( 0 ) ;
            if( motor::ogl::error::check_and_log( gl4_backend_log( "glBindVertexArray" ) ) )
                return false ;
        }

        return true ;
    }

    //****************************************************************************************
    void_t begin_frame( void_t ) 
    {
        // set default render states
        {
            auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) ) ;

            // set the viewport to the default new state, 
            // so the correct viewport is set automatically.
            {
                motor::math::vec4ui_t vp = _states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp ;
                vp.z( uint_t( vp_width ) ) ;
                vp.w( uint_t( vp_height ) ) ;
                _states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp = vp ;
            }

            _state_stack.push( _states[ ids_new.first ].states[ ids_new.second ] ) ;
        }
        
        // do clear the frame
        {
            motor::math::vec4f_t const clear_color = _state_stack.top().clear_s.ss.clear_color ;
            glClearColor( clear_color.x(), clear_color.y(), clear_color.z(), clear_color.w() ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glClearColor" ) ) ;

            GLbitfield const color = GL_COLOR_BUFFER_BIT ;
            GLbitfield const depth = GL_DEPTH_BUFFER_BIT ;

            glClear( color | depth ) ;
            motor::ogl::error::check_and_log( gl4_backend_log( "glEnable" ) ) ;
        }
    }

    //****************************************************************************************
    void_t end_frame( void_t ) 
    {
        glFlush() ;
        glFinish() ;

        _state_stack.pop() ;
    }
};

//************************************************************************************************
//
//
//************************************************************************************************

//******************************************************************************************************
gl4_backend::gl4_backend( motor::platform::opengl::rendering_context_ptr_t ctx ) noexcept 
{
    _pimpl = motor::memory::global_t::alloc( pimpl( this_t::get_bid(), ctx ), "gl4_backend::pimpl" ) ;
    _context = ctx ;
}

//******************************************************************************************************
gl4_backend::gl4_backend( this_rref_t rhv ) noexcept : backend( std::move( rhv ) )
{
    _pimpl = motor::move( rhv._pimpl ) ;
    _context = motor::move( rhv._context ) ;
}

//******************************************************************************************************
gl4_backend::~gl4_backend( void_t ) 
{
    motor::memory::global_t::dealloc( _pimpl ) ;
}

//******************************************************************************************************
void_t gl4_backend::set_window_info( window_info_cref_t wi ) noexcept 
{
    {
        //bool_t change = false ;
        if( wi.width != 0 )
        {
            _pimpl->vp_width = GLsizei( wi.width ) ;
            //change = true ;
        }
        if( wi.height != 0 )
        {
            _pimpl->vp_height = GLsizei( wi.height ) ;
            //change = true ;
        }
    }
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::msl_object_mtr_t obj ) noexcept
{
    if( obj == nullptr )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    size_t const oid = obj->set_oid( this_t::get_bid(), 
        _pimpl->construct_msl_data( obj->get_oid( this_t::get_bid() ), *obj ) ) ;

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::geometry_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->set_oid( this_t::get_bid(),
        _pimpl->construct_geo( obj->get_oid(this_t::get_bid() ), *obj ) ) ;

    if( !_pimpl->update( oid, obj, true ) )
    {
        return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::render_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->set_oid( this_t::get_bid(), _pimpl->construct_render_data( 
        obj->get_oid( this_t::get_bid() ), *obj ) ) ;

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    size_t const oid = obj->set_oid( this_t::get_bid(), _pimpl->construct_shader_data( 
        obj->get_oid( this_t::get_bid() ), *obj ) ) ;

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->set_oid( this_t::get_bid(), _pimpl->construct_image_config( 
        obj->get_oid( this_t::get_bid() ), obj->name(), *obj ) ) ;
    
    if( !_pimpl->update( oid, *obj, true ) )
    {
        return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::framebuffer_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        obj->set_oid( this_t::get_bid(), _pimpl->construct_framebuffer(
            obj->get_oid( this_t::get_bid() ), *obj ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        obj->set_oid( this_t::get_bid(), _pimpl->construct_state(
            obj->get_oid( this_t::get_bid() ), *obj ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::array_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->set_oid( this_t::get_bid(), _pimpl->construct_array_data( 
        obj->get_oid( this_t::get_bid() ), *obj ) ) ;

    if( !_pimpl->update( oid, *obj, true ) ) return motor::graphics::result::failed ;

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::streamout_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->set_oid( this_t::get_bid(), _pimpl->construct_feedback( 
        obj->get_oid( this_t::get_bid() ), *obj ) ) ;
    
    if( !_pimpl->update( oid, *obj, true ) ) return motor::graphics::result::failed ;

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::geometry_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_geometry( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::render_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_render_data( obj->get_oid( this_t::get_bid() ), true ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_shader_data( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_image_data( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::framebuffer_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_framebuffer( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        //_pimpl->relese( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::array_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_array_data( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::streamout_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_backend_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_tf_data( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
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
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        motor::log::global_t::error( gl4_backend_log( "invalid geometry configuration id" ) ) ;
        return motor::graphics::result::failed ;
    }

    auto const res = _pimpl->update( oid, obj ) ;
    motor::log::global_t::error( !res, gl4_backend_log( "update geometry" ) ) ;

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, false ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::array_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, false ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( !_pimpl->update( oid, *obj, false ) )
    {
        return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::render_object_mtr_t obj, size_t const varset ) noexcept 
{
    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update_variables( oid, varset ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::use( motor::graphics::framebuffer_object_mtr_t obj ) noexcept
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

//*******************************************************************************************
motor::graphics::result gl4_backend::use( motor::graphics::streamout_object_mtr_t obj ) noexcept 
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

    auto const res = _pimpl->use_transform_feedback( oid ) ;
    if( !res ) return motor::graphics::result::failed ;

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::unuse( motor::graphics::gen4::backend::unuse_type const t ) noexcept 
{
    switch( t ) 
    {
    case motor::graphics::gen4::backend::unuse_type::framebuffer: _pimpl->deactivate_framebuffer() ; break ;
    case motor::graphics::gen4::backend::unuse_type::streamout: _pimpl->unuse_transform_feedback() ; break ;
    }
    
    return motor::graphics::result::ok ;
}

//*************************************************************************************
motor::graphics::result gl4_backend::push( motor::graphics::state_object_mtr_t obj, 
               size_t const sid, bool_t const ) noexcept 
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

//*****************************************************************************************
motor::graphics::result gl4_backend::pop( motor::graphics::gen4::backend::pop_type const ) noexcept 
{
    _pimpl->handle_render_state( size_t( -1 ), size_t( -1 ) ) ;
    return motor::graphics::result::ok ;
}

//***************************************************************************************
motor::graphics::result gl4_backend::render( motor::graphics::render_object_mtr_t obj, 
          motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept 
{ 
    //motor::log::global_t::status( gl4_backend_log("render") ) ;

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;
    if( oid == size_t(-1)  )
    {
        motor::log::global_t::error( gl4_backend_log( "invalid id" ) ) ;
        return motor::graphics::result::failed ;
    }

    if( !_pimpl->check_link_status( oid ) )
    {
        motor::log::global_t::error( gl4_backend_log( "shader did not compile. Abort render." ) ) ;
        return motor::graphics::result::failed ;
    }

    // update variables before render
    {
        auto const res = _pimpl->update_variables( oid, detail.varset ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    // @todo
    // change per object render states here.
    _pimpl->render( oid, detail.geo, 
                    detail.feed_from_streamout, detail.use_streamout_count,
                    detail.varset, (GLsizei)detail.start, (GLsizei)detail.num_elems ) ;

    return motor::graphics::result::ok ;
}

//*************************************************************************************
motor::graphics::result gl4_backend::render( motor::graphics::msl_object_mtr_t obj, motor::graphics::gen4::backend::render_detail_cref_t detail ) noexcept 
{
    if( obj == nullptr ) return motor::graphics::result::invalid_argument ;

    size_t const oid = obj->get_oid( this_t::get_bid() ) ;

    if( oid == size_t(-1) )
    {
        motor::log::global_t::error( gl4_backend_log( "invalid id" ) ) ;
        return motor::graphics::result::failed ;
    }

    auto & msl = _pimpl->_msls[oid] ;
    
    motor::graphics::render_object_mtr_t ro = &msl.ros[detail.ro_idx] ;

    return this_t::render( ro, detail ) ;
}

//*************************************************************************************
void_t gl4_backend::render_begin( void_t ) noexcept 
{
    _pimpl->begin_frame() ;
}

//*************************************************************************************
void_t gl4_backend::render_end( void_t ) noexcept 
{
    _pimpl->end_frame() ;
}

//*************************************************************************************
void_t gl4_backend::clear_all_objects( void_t ) noexcept 
{
    _pimpl->clear_all_objects() ;
}