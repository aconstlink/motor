

#include "es3.h"
#include "es3_convert.h"

#include "../../buffer/vertex_buffer.hpp"
#include "../../buffer/index_buffer.hpp"

#include <natus/ogl/es/error.hpp>
#include <natus/memory/global.h>
#include <natus/memory/guards/malloc_guard.hpp>
#include <natus/ntd/vector.hpp>
#include <natus/ntd/stack.hpp>
#include <natus/ntd/string/split.hpp>

using namespace natus::graphics ;

struct es3_backend::pimpl
{
    natus_this_typedefs( pimpl ) ;

    //********************************************************************************
    // transform feedback buffers
    struct tf_data
    {
        natus_this_typedefs( tf_data ) ;

        bool_t valid = false ;
        natus::ntd::string_t name ;

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
        natus::ntd::vector< size_t > rd_ids ;
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
    natus_typedef( tf_data ) ;

    //********************************************************************************
    struct geo_data
    {
        bool_t valid = false ;
        natus::ntd::string_t name ;

        GLuint vb_id = GLuint( -1 ) ;
        GLuint ib_id = GLuint( -1 ) ;
        GLuint va_id = GLuint( -1 ) ;

        void_ptr_t vb_mem = nullptr ;
        void_ptr_t ib_mem = nullptr ;

        size_t num_elements_vb = 0 ;
        size_t num_elements_ib = 0 ;

        size_t sib_vb = 0 ;
        size_t sib_ib = 0 ;

        struct layout_element
        {
            natus::graphics::vertex_attribute va ;
            natus::graphics::type type ;
            natus::graphics::type_struct type_struct ;

            GLuint sib( void_t ) const noexcept
            {
                return GLuint( natus::graphics::size_of( type ) *
                    natus::graphics::size_of( type_struct ) ) ;
            }
        };
        natus::ntd::vector< layout_element > elements ;

        // per vertex sib
        GLuint stride = 0 ;

        GLenum ib_type ;
        size_t ib_elem_sib = 0  ;
        GLenum pt ;

        // for geo reconfig - render data ids
        natus::ntd::vector< size_t > rd_ids ;
        void_t remove_render_data_id( size_t const rid ) noexcept
        {
            if( rid == size_t( -1 ) ) return ;
            for( auto& id : rd_ids ) if( id == rid ) { id = size_t( -1 ) ; break ; }
        }

        void_t add_render_data_id( size_t const rid ) noexcept
        {
            if( rid == size_t( -1 ) ) return ;
            for( auto& id : rd_ids ) if( id == size_t( -1 ) ) { id = rid ; return ; }
            rd_ids.push_back( rid ) ;
        }
    };
    natus_typedef( geo_data ) ;

    //********************************************************************************
    struct shader_data
    {
        bool_t valid = false ;
        natus::ntd::string_t name ;

        GLuint vs_id = GLuint( -1 ) ;
        GLuint gs_id = GLuint( -1 ) ;
        GLuint ps_id = GLuint( -1 ) ;
        GLuint pg_id = GLuint( -1 ) ;

        bool_t is_compilation_ok = false ;

        struct vertex_input_binding
        {
            natus::graphics::vertex_attribute va ;
            natus::ntd::string_t name ;
        };
        natus::ntd::vector< vertex_input_binding > vertex_inputs ;

        struct vertex_output_binding
        {
            natus::graphics::vertex_attribute va ;
            natus::ntd::string_t name ;
        };
        natus::ntd::vector< vertex_output_binding > vertex_outputs ;
        char const ** output_names = nullptr ;

        bool_t find_vertex_input_binding_by_name( natus::ntd::string_cref_t name_,
            natus::graphics::vertex_attribute& va ) const noexcept
        {
            auto iter = ::std::find_if( vertex_inputs.begin(), vertex_inputs.end(),
                [&] ( vertex_input_binding const& b )
            {
                return b.name == name_ ;
            } ) ;
            if( iter == vertex_inputs.end() ) return false ;

            va = iter->va ;

            return true ;
        }

        bool_t find_vertex_output_binding_by_name( natus::ntd::string_cref_t name_,
            natus::graphics::vertex_attribute& va ) const noexcept
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
            natus::graphics::vertex_attribute va ;
            natus::ntd::string_t name ;
            GLuint loc ;
            GLenum type ;
        };
        natus_typedef( attribute_variable ) ;

        natus::ntd::vector< attribute_variable_t > attributes ;

        struct uniform_variable
        {
            natus::ntd::string_t name ;
            GLuint loc ;
            GLenum type ;

            // the GL uniform function
            natus::ogl::uniform_funk_t uniform_funk ;

            bool_t do_uniform_funk( void_ptr_t mem_ )
            {
                uniform_funk( loc, 1, mem_ ) ;
                return !natus::es::error::check_and_log( natus_log_fn( "glUniform" ) ) ;
            }

            void_t do_copy_funk( void_ptr_t mem_, natus::graphics::ivariable_ptr_t var )
            {
                std::memcpy( mem_, var->data_ptr(), natus::ogl::uniform_size_of( type ) ) ;
            }
        };
        natus_typedef( uniform_variable ) ;

        natus::ntd::vector< uniform_variable > uniforms ;

        void_ptr_t uniform_mem = nullptr ;
    } ;
    natus_typedef( shader_data ) ;

    //********************************************************************************
    struct state_data
    {
        bool_t valid = false ;
        natus::ntd::string_t name ;
        natus::ntd::vector< natus::graphics::render_state_sets_t > states ;
    } ;
    natus_typedef( state_data ) ;

    //********************************************************************************
    struct render_data
    {
        bool_t valid = false ;
        natus::ntd::string_t name ;

        natus::ntd::vector< size_t > geo_ids ;
        natus::ntd::vector< size_t > tf_ids ; // feed from for geometry
        size_t shd_id = size_t( -1 ) ;

        struct uniform_variable_link
        {
            // the index into the shader_data::uniforms array
            size_t uniform_id ;
            // the user variable holding the data.
            natus::graphics::ivariable_ptr_t var ;

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };

        natus::ntd::vector< natus::graphics::variable_set_res_t > var_sets ;

        // user provided variable set
        natus::ntd::vector< ::std::pair<
            natus::graphics::variable_set_res_t,
            natus::ntd::vector< uniform_variable_link > > > var_sets_data ;

        struct uniform_texture_link
        {
            // the index into the shader_data::uniforms array
            size_t uniform_id ;
            GLint tex_id ;
            size_t img_id ;

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };
        natus::ntd::vector< ::std::pair<
            natus::graphics::variable_set_res_t,
            natus::ntd::vector< uniform_texture_link > > > var_sets_texture ;

        struct uniform_array_data_link
        {
            // the index into the shader_config::uniforms array
            size_t uniform_id ;
            GLint tex_id ;
            size_t buf_id ;

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };
        natus::ntd::vector< std::pair<
            natus::graphics::variable_set_res_t,
            natus::ntd::vector< uniform_array_data_link > > > var_sets_array ;

        struct uniform_streamout_link
        {
            // the index into the shader_config::uniforms array
            size_t uniform_id ;
            GLint tex_id[2] ; // streamout object do double buffer
            size_t so_id ; // link into _feedbacks

            // pointing into the mem_block
            void_ptr_t mem = nullptr ;
        };
        natus::ntd::vector< std::pair<
            natus::graphics::variable_set_res_t,
            natus::ntd::vector< uniform_streamout_link > > > var_sets_streamout ;

        // memory block for all variables in all variable sets.
        void_ptr_t mem_block = nullptr ;

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
    natus_typedef( render_data ) ;

    //********************************************************************************
    struct image_data
    {
        bool_t valid = false ;
        natus::ntd::string_t name ;

        GLenum type = GL_NONE ;

        GLuint tex_id = GLuint( -1 ) ;
        size_t sib = 0 ;

        GLenum wrap_types[ 3 ] ;
        GLenum filter_types[ 2 ] ;

        // sampler ids for gl>=3.3
    };
    natus_typedef( image_data ) ;

    //********************************************************************************
    struct array_data
    {
        bool_t valid = false ;
        natus::ntd::string_t name ;

        GLuint tex_id = GLuint( -1 ) ;
        GLuint buf_id = GLuint( -1 ) ;

        GLuint sib = 0 ;
    } ;
    natus_typedef( array_data ) ;

    //********************************************************************************
    struct framebuffer_data
    {
        bool_t valid = false ;

        natus::ntd::string_t name ;

        GLuint gl_id = GLuint( -1 ) ;

        size_t nt = 0 ;
        GLuint colors[ 8 ] = {
            GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ),
            GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ) } ;

        GLuint depth = GLuint(-1) ;
        void_ptr_t mem_ptr = nullptr ;

        natus::math::vec2ui_t dims ;
    };
    natus_typedef( framebuffer_data ) ;

    typedef natus::ntd::vector< this_t::shader_data > shaders_t ;
    shaders_t _shaders ;

    typedef natus::ntd::vector< this_t::render_data > render_datas_t ;
    render_datas_t _renders ;

    typedef natus::ntd::vector< this_t::geo_data > geo_datas_t ;
    geo_datas_t _geometries ;

    typedef natus::ntd::vector< this_t::image_data > image_datas_t ;
    image_datas_t _images ;

    typedef natus::ntd::vector< this_t::framebuffer_data_t > framebuffers_t ;
    framebuffers_t _framebuffers ;

    typedef natus::ntd::vector< this_t::state_data_t > states_t ;
    states_t _states ;
    natus::ntd::stack< natus::graphics::render_state_sets_t, 10 > _state_stack ;

    typedef natus::ntd::vector< this_t::array_data_t > arrays_t ;
    arrays_t _arrays ;

    typedef natus::ntd::vector< this_t::tf_data_t > tf_datas_t ;
    tf_datas_t _feedbacks ;

    GLsizei vp_width = 0 ;
    GLsizei vp_height = 0 ;

    natus::graphics::backend_type const bt = natus::graphics::backend_type::es3 ;
    natus::graphics::shader_api_type const sapi = natus::graphics::shader_api_type::glsles_3_0 ;

    // the current render state set
    natus::graphics::render_state_sets_t render_states ;

    natus::graphics::es_context_ptr_t _ctx = nullptr ;

    size_t _tf_active_id = size_t( -1 ) ;

    //****************************************************************************************
    pimpl( natus::graphics::es_context_ptr_t ctx ) : _ctx( ctx )
    {
        {
            natus::graphics::state_object_t obj( "es3_default_states" ) ;

            auto new_states = natus::graphics::backend_t::default_render_states() ;

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
            this_t::release_render_data( i ) ;
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

        _geometries.clear() ;
        _shaders.clear() ;
        _renders.clear() ;
        _framebuffers.clear() ;
        _arrays.clear() ;
        _images.clear() ;
        _feedbacks.clear() ;
    }

    template< typename T >
    static size_t determine_oid( natus::ntd::string_cref_t name, natus::ntd::vector< T >& v ) noexcept
    {
        size_t oid = size_t( -1 ) ;

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
    static size_t find_index_by_resource_name( natus::ntd::string_in_t name, natus::ntd::vector< T > const & resources ) noexcept
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
    size_t construct_state( size_t oid, natus::graphics::state_object_ref_t obj ) noexcept
    {
        oid = determine_oid( obj.name(), _states ) ;

        auto& states = _states[ oid ] ;

        states.states.resize( obj.size() ) ;

        // @note
        // set all enables. Enable/Disable only possible during construction
        // values are assigned in the update function for the render states
        obj.for_each( [&] ( size_t const i, natus::graphics::render_state_sets_cref_t rs )
        {
            states.states[ i ] = rs ;
        } ) ;

        return oid ;
    }

    //****************************************************************************************
    void_t handle_render_state( render_state_sets const& new_states, bool_t const popped = false )
    {
        // depth test
        {
            if( new_states.depth_s.do_change )
            {
                if( new_states.depth_s.ss.do_activate )
                {
                    glEnable( GL_DEPTH_TEST );
                    natus::es::error::check_and_log( natus_log_fn( "glEnable" ) ) ;

                    glDepthMask( new_states.depth_s.ss.do_depth_write ? GL_TRUE : GL_FALSE ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glDepthMask" ) ) ;

                    glDepthFunc( GL_LESS ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glDepthFunc" ) ) ;
                }
                else
                {
                    glDisable( GL_DEPTH_TEST ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glDisable( GL_DEPTH_TEST )" ) ) ;

                    glDepthMask( GL_FALSE ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glDepthMask" ) ) ;
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
                    natus::es::error::check_and_log( natus_log_fn( "glEnable" ) ) ;

                    GLenum const glsrc = natus::graphics::es3::convert( new_states.blend_s.ss.src_blend_factor ) ;
                    GLenum const gldst = natus::graphics::es3::convert( new_states.blend_s.ss.dst_blend_factor );

                    glBlendFunc( glsrc, gldst ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glBlendFunc" ) ) ;
                }
                else
                {
                    glDisable( GL_BLEND ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glDisable" ) ) ;
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
                    natus::es::error::check_and_log( natus_log_fn( "glEnable" ) ) ;

                    glCullFace( natus::graphics::es3::convert( new_states.polygon_s.ss.cm ) ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glCullFace" ) ) ;

                    glFrontFace( natus::graphics::es3::convert( new_states.polygon_s.ss.ff ) ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glFrontFace" ) ) ;
                }
                else
                {
                    glDisable( GL_CULL_FACE ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glDisable" ) ) ;
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
                    natus::es::error::check_and_log( natus_log_fn( "glEnable" ) ) ;

                    glScissor(
                        GLint( new_states.scissor_s.ss.rect.x() ), GLint( new_states.scissor_s.ss.rect.y() ),
                        GLsizei( new_states.scissor_s.ss.rect.z() ), GLsizei( new_states.scissor_s.ss.rect.w() ) ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glScissor" ) ) ;
                }
                else
                {
                    glDisable( GL_SCISSOR_TEST ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glDisable" ) ) ;
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
                    natus::es::error::check_and_log( natus_log_fn( "glViewport" ) ) ;
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

                natus::math::vec4f_t const color = new_states.clear_s.ss.clear_color ;
                glClearColor( color.x(), color.y(), color.z(), color.w() ) ;
                natus::es::error::check_and_log( natus_log_fn( "glClearColor" ) ) ;

                GLbitfield const color_bit = clear_color ? GL_COLOR_BUFFER_BIT : 0 ;
                GLbitfield const depth_bit = clear_depth ? GL_DEPTH_BUFFER_BIT : 0 ;

                glClear( color_bit | depth_bit ) ;
                natus::es::error::check_and_log( natus_log_fn( "glEnable" ) ) ;
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
                natus::log::global_t::error( natus_log_fn( "no more render states to pop" ) ) ;
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
    size_t construct_framebuffer( size_t oid, natus::graphics::framebuffer_object_ref_t obj ) noexcept
    {
        oid = determine_oid( obj.name(), _framebuffers ) ;

        framebuffer_data_ref_t fb = _framebuffers[ oid ] ;

        if( fb.gl_id == GLuint( -1 ) )
        {
            glGenFramebuffers( 1, &fb.gl_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glGenFramebuffers" ) ) ;
        }

        if( fb.gl_id == GLuint( -1 ) )return oid ;

        // bind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fb.gl_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glGenFramebuffers" ) ) ;
        }

        size_t const nt = obj.get_num_color_targets() ;
        auto const ctt = obj.get_color_target() ;
        auto const dst = obj.get_depth_target() ;
        natus::math::vec2ui_t dims = obj.get_dims() ;

        // fix dims
        {
            dims.x( dims.x() + dims.x() % 2 ) ;
            dims.y( dims.y() + dims.y() % 2 ) ;
        }

        bool_t const requires_store = fb.colors[0]== GLuint(-1) ;

        {
            if( fb.colors[0] == GLuint(-1) )
            {
                glGenTextures( GLsizei( 8 ), fb.colors ) ;
                natus::es::error::check_and_log( natus_log_fn( "glGenTextures" ) ) ;
            }

            if( fb.depth == GLuint(-1) )
            {
                glGenTextures( GLsizei( 1 ), &fb.depth ) ;
                natus::es::error::check_and_log( natus_log_fn( "glGenTextures" ) ) ;
            }
        }

        // construct textures
        {
            for( size_t i = 0; i < nt; ++i )
            {
                GLuint const tid = fb.colors[ i ] ;

                glBindTexture( GL_TEXTURE_2D, tid ) ;
                if( natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) )
                    continue ;

                GLenum const target = GL_TEXTURE_2D ;
                GLint const level = 0 ;
                GLsizei const width = dims.x() ;
                GLsizei const height = dims.y() ;
                GLenum const format = GL_RGBA ;
                GLenum const type = natus::graphics::es3::to_pixel_type( ctt ) ;
                GLint const border = 0 ;
                GLint const internal_format = natus::graphics::es3::to_gl_format( ctt ) ;

                // maybe required for memory allocation
                // at the moment, render targets do not have system memory.
                #if 0
                size_t const sib = natus::graphics::es3::calc_sib( dims.x(), dims.y(), ctt ) ;
                #endif
                void_cptr_t data = 0;//nullptr ;

                glTexImage2D( target, level, internal_format, width, height, border, format, type, data ) ;
                natus::es::error::check_and_log( natus_log_fn( "glTexImage2D" ) ) ;
            }

            // Attach
            for( size_t i = 0; i < nt; ++i )
            {
                GLuint const tid = fb.colors[ i ] ;
                GLenum const att = GLenum( size_t( GL_COLOR_ATTACHMENT0 ) + i ) ;
                glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 ) ;
                natus::es::error::check_and_log( natus_log_fn( "glFramebufferTexture2D" ) ) ;
            }

            // setup color
            {
                GLenum attachments[ 15 ] ;
                size_t const num_color = nt ;

                for( size_t i = 0; i < num_color; ++i )
                {
                    attachments[ i ] = GLenum( size_t( GL_COLOR_ATTACHMENT0 ) + i ) ;
                }

                glDrawBuffers( GLsizei( num_color ), attachments ) ;
                natus::es::error::check_and_log( natus_log_fn( "glGenFramebuffers" ) ) ;
            }
        }

        // depth/stencil
        if( dst != natus::graphics::depth_stencil_target_type::unknown )
        {
            {
                GLuint const tid = fb.depth ;

                glBindTexture( GL_TEXTURE_2D, tid ) ;
                natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) ;

                GLenum const target = GL_TEXTURE_2D ;
                GLint const level = 0 ;
                GLsizei const width = dims.x() ;
                GLsizei const height = dims.y() ;
                GLenum const format = natus::graphics::es3::to_gl_format( dst ) ;
                GLenum const type = natus::graphics::es3::to_gl_type( dst ) ;
                GLint const border = 0 ;
                GLint const internal_format = natus::graphics::es3::to_gl_format( dst ) ;

                // maybe required for memory allocation
                // at the moment, render targets do not have system memory.
                #if 0
                size_t const sib = natus::graphics::es3::calc_sib( dims.x(), dims.y(), ctt ) ;
                #endif
                void_cptr_t data = nullptr ;

                glTexImage2D( target, level, internal_format, width, height, border, format, type, data ) ;
                natus::es::error::check_and_log( natus_log_fn( "glTexImage2D" ) ) ;
            }

            // attach
            {
                GLuint const tid = fb.depth ;
                GLenum const att = natus::graphics::es3::to_gl_attachment( dst ) ;
                glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 ) ;
                natus::es::error::check_and_log( natus_log_fn( "glFramebufferTexture2D" ) ) ;
            }
        }

        GLenum status = 0 ;
        // validate
        {
            status = glCheckFramebufferStatus( GL_FRAMEBUFFER ) ;
            natus::es::error::check_and_log( natus_log_fn( "glCheckFramebufferStatus" ) ) ;

            natus::log::global_t::warning( status != GL_FRAMEBUFFER_COMPLETE,
                "Incomplete framebuffer : [" + obj.name() + "]" ) ;
        }

        // unbind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 ) ;
            natus::es::error::check_and_log( natus_log_fn( "glGenFramebuffers" ) ) ;
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
                _images[ idx ].valid = true ;
                _images[ idx ].name = fb.name + "." + std::to_string( i ) ;
                _images[ idx ].tex_id = fb.colors[ i ] ;
                _images[ idx ].type = GL_TEXTURE_2D ; 

                for( size_t j = 0; j < ( size_t ) natus::graphics::texture_wrap_mode::size; ++j )
                {
                    _images[ idx ].wrap_types[ j ] = GL_CLAMP_TO_EDGE ;
                }

                for( size_t j = 0; j < ( size_t ) natus::graphics::texture_filter_mode::size; ++j )
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
                _images[ idx ].valid = true ;
                _images[ idx ].name = fb.name + ".depth" ;
                _images[ idx ].tex_id = fb.depth ;
                _images[ idx ].type = GL_TEXTURE_2D ; 

                for( size_t j = 0; j < ( size_t ) natus::graphics::texture_wrap_mode::size; ++j )
                {
                    _images[ idx ].wrap_types[ j ] = GL_REPEAT ;
                }

                for( size_t j = 0; j < ( size_t ) natus::graphics::texture_filter_mode::size; ++j )
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
            natus::es::error::check_and_log( natus_log_fn( "glDeleteFramebuffers" ) ) ;
        }

        {
            glDeleteTextures( 8, fbd.colors ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteTextures" ) ) ;
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
                natus::es::error::check_and_log( natus_log_fn( "glDeleteTextures" ) ) ;
            }
        }

        fbd.name = "released" ;
        fbd.valid = false ;
        fbd.gl_id = GLuint( -1 ) ;
        fbd.depth = GLuint( -1 ) ;
        for( size_t i=0; i<8; ++i ) fbd.colors[i] = GLuint( -1 ) ;

        fbd.dims = natus::math::vec2ui_t() ;

        return true ;
    }

    //****************************************************************************************
    bool_t activate_framebuffer( size_t const oid  )
    {
        framebuffer_data_ref_t fb = _framebuffers[ oid ] ;

        // bind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, fb.gl_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glGenFramebuffers" ) ) ;
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
            natus::es::error::check_and_log( natus_log_fn( "glGenFramebuffers" ) ) ;
        }

        return true ;
    }

    //****************************************************************************************
    void_t deactivate_framebuffer( void_t )
    {
        // unbind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 ) ;
            natus::es::error::check_and_log( natus_log_fn( "glGenFramebuffers" ) ) ;
        }
    }

    //****************************************************************************************
    size_t construct_shader_data( size_t oid, natus::graphics::shader_object_ref_t obj )
    {
        oid = determine_oid( obj.name(), _shaders ) ;
        _shaders[ oid ].name = obj.name() ;

        // program
        if( _shaders[ oid ].pg_id == GLuint( -1 ) )
        {
            GLuint const id = glCreateProgram() ;
            natus::es::error::check_and_log(
                natus_log_fn( "Shader Program creation" ) ) ;

            _shaders[ oid ].pg_id = id ;
        }
        {
            this_t::detach_shaders( _shaders[ oid ].pg_id ) ;
            this_t::delete_all_variables( _shaders[ oid ] ) ;
        }

        // vertex shader
        if( _shaders[oid].vs_id == GLuint(-1) )
        {
            GLuint const id = glCreateShader( GL_VERTEX_SHADER ) ;
            natus::es::error::check_and_log(
                natus_log_fn( "Vertex Shader creation" ) ) ;

            _shaders[ oid ].vs_id = id ;
        }
        {
            glAttachShader( _shaders[ oid ].pg_id, _shaders[oid].vs_id ) ;
            natus::es::error::check_and_log(
                natus_log_fn( "Attaching vertex shader" ) ) ;
        }

        natus::graphics::shader_set_t ss ;
        {
            auto const res = obj.shader_set( this_t::sapi, ss ) ;
            if( natus::core::is_not(res) )
            {
                natus::log::global_t::warning( natus_log_fn(
                    "config [" + obj.name() + "] has no shaders for " + 
                    natus::graphics::to_string( this_t::bt ) ) ) ;
                return oid ;
            }
        }

        // geometry shader
        if( ss.has_geometry_shader() )
        {
            GLuint id = _shaders[ oid ].gs_id ;

            if( id == GLuint(-1) )
            {
                id = glCreateShader( GL_GEOMETRY_SHADER ) ;
                natus::es::error::check_and_log(
                    natus_log_fn( "Geometry Shader creation" ) ) ;

                _shaders[ oid ].gs_id = id ;
            }

            GLuint const pid = _shaders[ oid ].pg_id ;

            glAttachShader( pid, id ) ;
            natus::es::error::check_and_log(
                natus_log_fn( "Attaching geometry shader" ) ) ;

            // check max output vertices
            {
                GLint max_out = 0 ;
                glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_out ) ;
                natus::es::error::check_and_log(
                    natus_log_fn( "Geometry Shader Max Output Vertices" ) ) ;
                ( void_t ) max_out ;
            }
        }
        else if( _shaders[ oid ].gs_id != GLuint(-1) )
        {
            glDeleteShader( _shaders[ oid ].gs_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteShader" ) ) ;
            _shaders[ oid ].gs_id = GLuint( -1 ) ;
        }

        // pixel shader
        if( ss.has_pixel_shader() )
        {
            GLuint id = _shaders[ oid ].ps_id ;
            if( id == GLuint(-1) )
            {
                id = glCreateShader( GL_FRAGMENT_SHADER ) ;
                natus::es::error::check_and_log(
                    natus_log_fn( "Fragment Shader creation" ) ) ;
            }

            glAttachShader( _shaders[ oid ].pg_id, id ) ;
            natus::es::error::check_and_log( natus_log_fn( "Attaching pixel shader" ) ) ;

            _shaders[ oid ].ps_id = id ;
        }
        else if( _shaders[ oid ].ps_id != GLuint( -1 ) )
        {
            glDeleteShader( _shaders[ oid ].ps_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteShader" ) ) ;
            _shaders[ oid ].ps_id = GLuint( -1 ) ;
        }

        return oid ;
    }

    //****************************************************************************************
    bool_t release_shader_data( size_t const oid ) noexcept
    {
        auto & shd = _shaders[ oid ] ;

        if( !shd.valid ) return true ;

        // delete shaders
        {
            glDetachShader( shd.pg_id, shd.vs_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDetachShader" ) ) ;

            glDeleteShader( shd.vs_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteShader" ) ) ;
        }

        if( shd.gs_id != GLuint(-1) )
        {
            glDetachShader( shd.pg_id, shd.gs_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDetachShader" ) ) ;

            glDeleteShader( shd.gs_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteShader" ) ) ;
        }

        if( shd.ps_id != GLuint(-1) )
        {
            glDetachShader( shd.pg_id, shd.ps_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDetachShader" ) ) ;

            glDeleteShader( shd.ps_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteShader" ) ) ;
        }

        // delete program
        {
            glDeleteProgram( shd.pg_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteProgram" ) ) ;
        }

        shd.valid = false ;
        shd.pg_id = GLuint( -1 ) ;
        shd.gs_id = GLuint( -1 ) ;
        shd.vs_id = GLuint( -1 ) ;
        shd.ps_id = GLuint( -1 ) ;
        shd.name = "released shader" ;

        shd.is_compilation_ok = false ;
        shd.uniforms.clear() ;
        shd.attributes.clear() ;

        return true ;
    }

    //*******************************************************************************
    void_t detach_shaders( GLuint const program_id )
    {
        GLsizei count = 0 ;
        GLuint shaders_[ 10 ] ;

        glGetAttachedShaders( program_id, 10, &count, shaders_ ) ;
        natus::es::error::check_and_log( natus_log_fn( "glGetAttachedShaders" ) ) ;

        for( GLsizei i = 0; i < count; ++i )
        {
            glDetachShader( program_id, shaders_[ i ] ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDetachShader" ) ) ;
        }
    }

    //*********************************************************************************
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

    //*********************************************************************************
    bool_t compile_shader( GLuint const id, natus::ntd::string_cref_t code )
    {
        if( code.empty() ) return true ;

        GLchar const* source_string = ( GLchar const* ) ( code.c_str() ) ;

        glShaderSource( id, 1, &source_string, 0 ) ;
        if( natus::es::error::check_and_log( natus_log_fn( "glShaderSource" ) ) )
            return false ;

        glCompileShader( id ) ;
        if( natus::es::error::check_and_log( natus_log_fn( "glCompileShader" ) ) )
            return false ;

        GLint ret ;
        glGetShaderiv( id, GL_COMPILE_STATUS, &ret ) ;

        GLint length ;
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length ) ;

        if( ret == GL_TRUE && length <= 1 ) return true ;

        if( natus::log::global::error( length == 0, 
            natus_log_fn( "shader compilation failed, but info log length is 0." ) ) )
            return false ;

        // print first line for info
        // user can place the shader name or any info there.
        {
            size_t pos = code.find_first_of( '\n' ) ;
            natus::log::global::error( natus_log_fn( "First Line: " + code.substr( 0, pos ) ) ) ;
        }

        // get the error message it is and print it
        {
            natus::memory::malloc_guard<char> info_log( length ) ;

            glGetShaderInfoLog( id, length, 0, info_log ) ;

            natus::ntd::vector< natus::ntd::string_t > tokens ;
            natus::ntd::string_ops::split( natus::ntd::string_t( info_log ), '\n', tokens ) ;

            for( auto const & msg : tokens )
            {
                natus::log::global::error( msg ) ;
            }
        }
        return true ;
    }

    //******************************************************************************
    bool_t link( GLuint const program_id )
    {
        glLinkProgram( program_id ) ;
        if( natus::es::error::check_and_log( natus_log_fn( "glLinkProgram" ) ) )
            return false ;

        {
            GLint ret ;
            glGetProgramiv( program_id, GL_LINK_STATUS, &ret ) ;

            GLint length ;
            glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &length ) ;

            if( ret == GL_TRUE && length <= 1 ) 
                return true ;

            if( natus::log::global_t::error( length == 0, natus_log_fn("unknown") ) )
                return false ;

            natus::memory::malloc_guard<char> info_log( length ) ;

            glGetProgramInfoLog( program_id, length, 0, info_log ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glGetProgramInfoLog" ) ) )
                return false ;

            std::string info_log_string = std::string( ( const char* ) info_log ) ;

            {
                natus::ntd::vector< natus::ntd::string_t > tokens ;
                natus::ntd::string_ops::split( natus::ntd::string_t( info_log ), '\n', tokens ) ;

                for( auto token : tokens )
                {
                    natus::log::global_t::error( "[es]" + token ) ;
                }
            }
        }
        return false ;
    }

    //*******************************************************************************
    void_t post_link_attributes( this_t::shader_data & config )
    {
        GLuint const program_id = config.pg_id ;

        GLint num_active_attributes = 0 ;
        GLint name_length = 0 ;

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTES, &num_active_attributes ) ;
        natus::es::error::check_and_log( natus_log_fn( "glGetProgramiv(GL_ACTIVE_ATTRIBUTES)" ) ) ;

        if( num_active_attributes == 0 ) return ;

        config.attributes.resize( num_active_attributes ) ;

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_length ) ;
        natus::es::error::check_and_log( natus_log_fn( 
            "glGetProgramiv(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)" ) ) ;

        GLint size ;
        GLenum gl_attrib_type ;

        natus::memory::malloc_guard<char> buffer( name_length ) ;
        
        for( GLint i = 0; i < num_active_attributes; ++i )
        {
            glGetActiveAttrib( program_id, i, name_length, 0, 
                &size, &gl_attrib_type, buffer ) ;

            if( natus::es::error::check_and_log( "glGetActiveAttrib failed. continue loop." ) ) continue ;

            // build-ins are not needed in the shader variables container.
            if( name_length >= 3 && buffer.equals( 0, 'g' ) && buffer.equals( 1, 'l' ) && buffer.equals( 2, '_' ) ) continue ;

            GLuint const location_id = glGetAttribLocation( program_id, buffer ) ;
            if( natus::es::error::check_and_log( "glGetAttribLocation failed. continue loop." ) ) continue ;

            natus::ntd::string_t const variable_name = natus::ntd::string_t( ( const char* ) buffer ) ;

            this_t::shader_data::attribute_variable_t vd ;
            vd.name = ::std::move( variable_name ) ;
            vd.loc = location_id ;
            vd.type = gl_attrib_type ;
            
            {
                natus::graphics::vertex_attribute va = natus::graphics::vertex_attribute::undefined ;
                auto const res = config.find_vertex_input_binding_by_name( vd.name, va ) ;
                natus::log::global_t::error( natus::core::is_not( res ), 
                    natus_log_fn("can not find vertex attribute - " + vd.name ) ) ;
                vd.va = va ;
            }
            config.attributes[i] = vd ;
        }
    }

    //**************************************************************************************
    bool_t bind_attributes( this_t::shader_data & sconfig, this_t::geo_data & gconfig )
    {
        // bind vertex array object
        {
            glBindVertexArray( gconfig.va_id ) ;
            if( natus::es::error::check_and_log(
                natus_log_fn( "glBindVertexArray" ) ) )
                return false ;
        }

        // bind vertex buffer
        {
            glBindBuffer( GL_ARRAY_BUFFER, gconfig.vb_id ) ;
            if( natus::es::error::check_and_log( 
                natus_log_fn("glBindBuffer(GL_ARRAY_BUFFER)") ) ) 
                return false ;
        }

        // bind index buffer
        {
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gconfig.ib_id ) ;
            if( natus::es::error::check_and_log( 
                natus_log_fn( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)" ) ) )
                return false ;
        }

        // disable old locations
        for( size_t i = 0; i < sconfig.attributes.size(); ++i )
        {
            glDisableVertexAttribArray( sconfig.attributes[ i ].loc ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDisableVertexAttribArray" ) ) ;
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

            auto iter = ::std::find_if( sconfig.attributes.begin(), sconfig.attributes.end(), 
                [&]( this_t::shader_data::attribute_variable_cref_t av )
            {
                return av.va == e.va ;
            } ) ;

            GLuint loc = 0 ;
            GLenum type = GL_NONE ;

            if( iter == sconfig.attributes.end() ) 
            {
                loc = ++max_loc ;
                type = natus::ogl::convert( e.type ) ;

                natus::log::global_t::warning( natus_log_fn( "Vertex attribute (" +
                    natus::graphics::to_string(e.va) + ") in shader (" + sconfig.name + ") not used."
                    "Will bind geometry (" +sconfig.name+ ") layout attribute to custom location (" 
                    + std::to_string( uint_t(loc) ) + ").") ) ;
            }
            else
            {
                loc = iter->loc ;
                type = iter->type ;
            }

            glEnableVertexAttribArray( loc ) ;
            natus::es::error::check_and_log(
                natus_log_fn( "glEnableVertexAttribArray" ) ) ;

            glVertexAttribPointer(
                loc,
                GLint( natus::graphics::size_of(e.type_struct) ),
                natus::ogl::complex_to_simple_type( type ),
                GL_FALSE,
                ( GLsizei ) uiStride,
                (const GLvoid*)(size_t)uiBegin 
                ) ;

            natus::es::error::check_and_log( natus_log_fn( "glVertexAttribPointer" ) ) ;
        }
        
        return true ;
    }

    //*************************************************************************************
    void_t post_link_uniforms( this_t::shader_data & config )
    {
        GLuint const program_id = config.pg_id ;

        GLint num_active_uniforms = 0 ;
        GLint name_length = 0 ;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORMS, &num_active_uniforms ) ;
        natus::es::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORMS" ) ;

        if( num_active_uniforms == 0 ) return ;

        config.uniforms.resize( num_active_uniforms ) ;

        glGetProgramiv( program_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_length ) ;
        natus::es::error::check_and_log( "[glGetProgramiv] : GL_ACTIVE_UNIFORM_MAX_LENGTH" ) ;

        GLint size ;
        GLenum gl_uniform_type ;

        natus::memory::malloc_guard<char> buffer( name_length ) ;

        for( GLint i = 0; i < num_active_uniforms; ++i )
        {
            glGetActiveUniform( program_id, i, name_length, 0, 
                &size, &gl_uniform_type, buffer ) ;
            if( natus::es::error::check_and_log( "[gl_33_api] : glGetActiveUniform" ) ) continue ;

            // build-ins are not needed in the shader variables container.
            if( name_length >= 3 && buffer.equals( 0, 'g' ) && buffer.equals( 1, 'l' ) && buffer.equals( 2, '_' ) ) continue ;

            GLuint const location_id = glGetUniformLocation( program_id, buffer ) ;
            if( natus::es::error::check_and_log( "[glGetUniformLocation]" ) ) continue ;

            if( natus::log::global_t::error( location_id == GLuint( -1 ), 
                natus_log_fn( "invalid uniform location id." ) ) ) continue ;

            natus::ntd::string const variable_name = natus::ntd::string( char_cptr_t( buffer ) ) ;

            this_t::shader_data::uniform_variable_t vd ;
            vd.name = ::std::move( variable_name ) ;
            vd.loc = location_id ;
            vd.type = gl_uniform_type ;
            vd.uniform_funk = natus::ogl::uniform_funk( gl_uniform_type ) ;

            config.uniforms[ i ] = vd ;
        }
    }

    //***************************************************************************************
    size_t construct_image_config( size_t oid, natus::graphics::image_object_ref_t obj )
    {
        oid = determine_oid( obj.name(), _images ) ;

        auto & config = _images[ oid ] ;
        config.name = obj.name() ;
        config.type = natus::graphics::es3::convert( obj.get_type() ) ;

        // sampler
        if( config.tex_id == GLuint( -1 ) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenTextures( 1, &id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glGenTextures" ) ) ;

            config.tex_id = id ;
        }

        {
            for( size_t j=0; j<(size_t)natus::graphics::texture_wrap_mode::size; ++j )
            {
                config.wrap_types[ j ] = natus::graphics::es3::convert(
                    obj.get_wrap( ( natus::graphics::texture_wrap_mode )j ) );
            }

            for( size_t j = 0; j < ( size_t ) natus::graphics::texture_filter_mode::size; ++j )
            {
                config.filter_types[ j ] = natus::graphics::es3::convert(
                    obj.get_filter( ( natus::graphics::texture_filter_mode )j ) );
            }
        }
        return oid ;
    }

    //*************************************************************************************
    bool_t release_image_data( size_t const oid ) noexcept
    {
        auto & id = _images[ oid ] ;

        id.name = "released image" ;
        id.type = GL_NONE ;

        if( id.tex_id != GLuint( -1 ) )
        {
            glDeleteTextures( 1, &id.tex_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteTextures" ) ) ;

            id.tex_id = GLuint( -1 ) ;
        }

        id.sib = 0 ;

        return true ;
    }

    //***********************************************************************************
    size_t construct_render_config( size_t oid, natus::graphics::render_object_ref_t obj )
    {
        oid = determine_oid( obj.name(), _renders ) ;
        
        {
            _renders[ oid ].name = obj.name() ;
            _renders[ oid ].var_sets_data.clear() ;
            _renders[ oid ].var_sets_texture.clear() ;
            _renders[ oid ].var_sets_array.clear() ;
            _renders[ oid ].var_sets_streamout.clear() ;
            _renders[ oid ].var_sets.clear() ;

            _renders[ oid ].geo_ids.clear();
            _renders[ oid ].tf_ids.clear() ;
            _renders[ oid ].shd_id = size_t( -1 ) ;

            natus::memory::global_t::dealloc( _renders[ oid ].mem_block ) ;
            _renders[ oid ].mem_block = nullptr ;
        }

        return oid ;
    }

    //******************************************************************************
    bool_t release_render_data( size_t const oid ) noexcept
    {
        auto & rd = _renders[ oid ] ;

        rd.valid = false ;
        rd.name = "released" ;

        for( auto id : rd.geo_ids ) _geometries[ id ].remove_render_data_id( oid ) ;
        rd.geo_ids.clear() ;

        rd.shd_id = GLuint( -1 ) ;
        rd.var_sets.clear() ;
        rd.var_sets_array.clear() ;
        rd.var_sets_streamout.clear() ;
        rd.var_sets_data.clear() ;
        rd.var_sets_texture.clear() ;

        natus::memory::global_t::dealloc( rd.mem_block ) ;
        rd.mem_block = nullptr ;

        return true ;
    }

    //**********************************************************************************
    bool_t update( size_t const id, natus::graphics::shader_object_cref_t sc )
    {
        auto& sconfig = _shaders[ id ] ;

        {
            sc.for_each_vertex_input_binding( [&]( size_t const,
                natus::graphics::vertex_attribute const va, natus::ntd::string_cref_t name )
            {
                sconfig.vertex_inputs.emplace_back( this_t::shader_data::vertex_input_binding 
                    { va, name } ) ;
            } ) ;
        }

        // !!! must be done pre-link !!!
        // set transform feedback varyings
        if( (sc.get_num_output_bindings() != 0) && 
            (sc.get_streamout_mode() != natus::graphics::streamout_mode::unknown) )
        {
            sconfig.output_names = (char const **)natus::memory::global_t::
                alloc_raw<char *>( sc.get_num_output_bindings() ) ;

            sc.for_each_vertex_output_binding( [&]( size_t const i,
                natus::graphics::vertex_attribute const va, 
                natus::graphics::ctype const, natus::ntd::string_cref_t name )
            {
                sconfig.vertex_outputs.emplace_back( 
                    this_t::shader_data::vertex_output_binding { va, name } ) ;
                sconfig.output_names[i] = name.c_str() ;
            } ) ;

            // the mode interleaved or separate depends on the number of 
            // buffers in the streamout object. So if a streamout
            // object is used, the engine needs to relink this shader 
            // based on the number of buffers attached to the streamout
            // object.
            GLenum const mode = natus::graphics::es3::convert( sc.get_streamout_mode() ) ;
            glTransformFeedbackVaryings( sconfig.pg_id, GLsizei( sc.get_num_output_bindings() ), 
                                         sconfig.output_names, mode ) ;

            natus::es::error::check_and_log( natus_log_fn( "glTransformFeedbackVaryings" ) ) ;
            natus::log::global_t::status( mode == GL_NONE, 
                            "Did you miss to set the streamout mode in the shader object?" ) ;

            natus::memory::global_t::dealloc( sconfig.output_names ) ;
            sconfig.output_names = nullptr ;
        }

        // compile
        {
            natus::graphics::shader_set_t ss ;
            {
                auto const res = sc.shader_set( this_t::sapi, ss ) ;
                if( res )
                {
                    auto const r1 = this_t::compile_shader( sconfig.vs_id, ss.vertex_shader().code() ) ;
                    auto const r2 = this_t::compile_shader( sconfig.gs_id, ss.geometry_shader().code() ) ;
                    auto const r3 = this_t::compile_shader( sconfig.ps_id, ss.pixel_shader().code() ) ;
                    auto const r4 = this_t::link( sconfig.pg_id ) ;
                    if( !( r1 && r2 && r3 && r4 ) )
                    {
                        sconfig.is_compilation_ok = false ;
                        return false ;
                    }
                    sconfig.is_compilation_ok = true ;
                    natus::log::global_t::status( "[ES3] : Compilation Successful : [" + sconfig.name + "]" ) ;
                }
            }
        }

        {
            this_t::post_link_attributes( sconfig ) ;
            this_t::post_link_uniforms( sconfig ) ;
        }

        // if the shader is redone, all render objects using
        // the shader need to be updated
        {
            for( size_t i = 0; i < _renders.size(); ++i )
            {
                auto& rd = _renders[ i ] ;

                if( rd.shd_id != id ) continue ;

                rd.var_sets_data.clear() ;
                rd.var_sets_texture.clear() ;
                rd.var_sets_array.clear() ;
                rd.var_sets_streamout.clear() ;

                auto vs = std::move( rd.var_sets ) ;
                for( auto& item : vs )
                    this_t::connect( rd, item ) ;

                this_t::render_object_variable_memory( rd, _shaders[ rd.shd_id ] ) ;
                for( size_t vs_id=0; vs_id<rd.var_sets.size(); ++vs_id )
                {
                    this_t::update_variables( i, vs_id ) ;
                }
            }
        }

        return true ;
    }

    //**************************************************************************************
    bool_t update( size_t const id, natus::graphics::render_object_ref_t rc )
    {
        auto& config = _renders[ id ] ;

        // handle geometry links
        {
            // remove this render data id from the old geometry
            for( auto gid : config.geo_ids ) _geometries[ gid ].remove_render_data_id( id ) ;
            config.geo_ids.clear() ;

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
                    natus::log::global_t::warning( natus_log_fn(
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
                    natus::log::global_t::warning( natus_log_fn(
                        "no streamout object with name [" + rc.get_streamout() + "] for render_data [" + rc.name() + "]" ) ) ;
                    return false ;
                }

                config.tf_ids.emplace_back( std::distance( _feedbacks.begin(), iter ) ) ;
            }
        }

        // find shader
        {
            auto const iter = ::std::find_if( _shaders.begin(), _shaders.end(),
                [&] ( this_t::shader_data const& d )
            {
                return d.name == rc.get_shader() ;
            } ) ;
            if( iter == _shaders.end() )
            {
                natus::log::global_t::warning( natus_log_fn(
                    "no shader with name [" + rc.get_shader() + "] for render_config [" + rc.name() + "]" ) ) ;
                return false ;
            }

            config.shd_id = std::distance( _shaders.begin(), iter ) ;
        }

        {
            this_t::shader_data_ref_t shd = _shaders[ config.shd_id ] ;

            // for binding attributes, the shader and the geometry is required.
            for( size_t i=0; i<config.geo_ids.size(); ++i )
            {
                this_t::geo_data_ref_t geo = _geometries[ config.geo_ids[i] ] ;
                this_t::bind_attributes( shd, geo ) ;
            }

            // do for feedback objects
            // for binding attributes, the shader and the geometry is required.
            for( size_t i=0; i<config.tf_ids.size(); ++i )
            {
                auto & tfd = _feedbacks[ config.tf_ids[i] ] ;
                for( size_t j=0; j<tf_data::buffer::max_buffers; ++j )
                {
                    if( tfd._buffers[0].gids[j] == size_t(-1) ) break ;
                    this_t::bind_attributes( shd, _geometries[ tfd._buffers[0].gids[j] ] ) ;
                    this_t::bind_attributes( shd, _geometries[ tfd._buffers[1].gids[j] ] ) ;
                }
            }
        }

        {
            rc.for_each( [&] ( size_t const /*i*/, natus::graphics::variable_set_res_t vs )
            {
                auto const res = this_t::connect( id, vs ) ;
                natus::log::global_t::warning( natus::core::is_not( res ),
                    natus_log_fn( "connect" ) ) ;
            } ) ;
        }
        
        this_t::render_object_variable_memory( config, _shaders[ config.shd_id ] ) ;

        return true ;
    }

    //***************************************************************************************
    void_t render_object_variable_memory( this_t::render_data & rd, this_t::shader_data & shader )
    {
        auto& config = rd ;

        // construct memory block
        {
            size_t sib = 0 ;
            for( auto & vs : config.var_sets_data )
            {
                for( auto & link : vs.second )
                {
                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    sib += natus::ogl::uniform_size_of( uv.type ) ;
                }
            }

            for( auto & vs : config.var_sets_texture )
            {
                for( auto & link : vs.second )
                {
                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    sib += natus::ogl::uniform_size_of( uv.type ) ;
                }
            }

            for( auto & vs : config.var_sets_array )
            {
                for( auto & link : vs.second )
                {
                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    sib += natus::ogl::uniform_size_of( uv.type ) ;
                }
            }

            for( auto & vs : config.var_sets_streamout )
            {
                for( auto & link : vs.second )
                {
                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    sib += natus::ogl::uniform_size_of( uv.type ) ;
                }
            }

            natus::memory::global_t::dealloc( config.mem_block ) ;
            config.mem_block = natus::memory::global_t::alloc( sib, "[gles3] : render config memory block" ) ;
        }

        // assign memory locations
        {
            void_ptr_t mem = config.mem_block ;
            for( auto & vs : config.var_sets_data )
            {
                for( auto & link : vs.second )
                {
                    link.mem = mem ;

                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    mem = reinterpret_cast<void_ptr_t>( size_t(mem) + 
                         size_t( natus::ogl::uniform_size_of( uv.type ) )) ;
                }
            }

            for( auto & vs : config.var_sets_texture )
            {
                for( auto & link : vs.second )
                {
                    link.mem = mem ;

                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    mem = reinterpret_cast<void_ptr_t>( size_t(mem) + 
                          size_t( natus::ogl::uniform_size_of( uv.type ) )) ;
                }
            }

            for( auto & vs : config.var_sets_array )
            {
                for( auto & link : vs.second )
                {
                    link.mem = mem ;

                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    mem = reinterpret_cast<void_ptr_t>( size_t(mem) + 
                         size_t( natus::ogl::uniform_size_of( uv.type ) )) ;
                }
            }

            for( auto & vs : config.var_sets_streamout )
            {
                for( auto & link : vs.second )
                {
                    link.mem = mem ;

                    auto & uv = shader.uniforms[ link.uniform_id ] ;
                    mem = reinterpret_cast<void_ptr_t>( size_t(mem) + 
                         size_t( natus::ogl::uniform_size_of( uv.type ) )) ;
                }
            }
        }
    }

    //****************************************************************************************
    size_t construct_geo( size_t oid, natus::ntd::string_in_t name, natus::graphics::vertex_buffer_in_t vb ) noexcept
    {
        oid = determine_oid( name, _geometries ) ;

        bool_t error = false ;
        auto& config = _geometries[ oid ] ;

        // vertex array object
        if( config.va_id == GLuint( -1 ) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenVertexArrays( 1, &id ) ;
            natus::es::error::check_and_log(
                natus_log_fn( "Vertex Array creation" ) ) ;

            config.va_id = id ;
        }

        // vertex buffer
        if( config.vb_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenBuffers( 1, &id ) ;
            error = natus::es::error::check_and_log( 
                natus_log_fn( "Vertex Buffer creation" ) ) ;

            config.vb_id = id ;
        }

        // index buffer
        if( config.ib_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenBuffers( 1, &id ) ;
            error = natus::es::error::check_and_log(
                natus_log_fn( "Index Buffer creation" ) ) ;

            config.ib_id = id ;
        }

        {
            config.name = name ;
            config.stride = GLuint( vb.get_layout_sib() ) ;

            config.elements.clear() ;
            vb.for_each_layout_element( 
                [&]( natus::graphics::vertex_buffer_t::data_cref_t d )
            {
                this_t::geo_data::layout_element le ;
                le.va = d.va ;
                le.type = d.type ;
                le.type_struct = d.type_struct ;
                config.elements.push_back( le ) ;
            }) ;
        }

        natus::log::global_t::error( error, natus_log_fn("Error ocurred for ["+ name +"]") ) ;

        return oid ;
    }

    //****************************************************************************************
    size_t construct_geo( size_t oid, natus::graphics::geometry_object_ref_t obj ) noexcept
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
            natus::es::error::check_and_log( natus_log_fn( "glDeleteBuffers" ) ) ;
            geo.vb_id = GLuint( -1 ) ;
        }

        if( geo.ib_id != GLuint( -1 ) )
        {
            glDeleteBuffers( 1, &geo.ib_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteBuffers" ) ) ;
            geo.ib_id = GLuint( -1 ) ;
        }

        if( geo.va_id != GLuint( -1 ) )
        {
            glDeleteVertexArrays( 1, &geo.va_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteVertexArrays" ) ) ;
            geo.va_id = GLuint( -1 ) ;
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

    //************************************************************************************
    bool_t update( size_t const id, natus::graphics::geometry_object_res_t geo, 
                   bool_t const is_config = false )
    {
        auto& config = _geometries[ id ] ;

        // disable vertex array so the buffer ids do not get overwritten
        {
            glBindVertexArray( 0 ) ;
            natus::es::error::check_and_log( natus_log_fn( "glBindVertexArray" ) ) ;
        }

        {
            config.num_elements_ib = geo->index_buffer().get_num_elements() ;
            config.num_elements_vb = geo->vertex_buffer().get_num_elements() ;
            config.ib_elem_sib = 0 ;
            config.ib_type = GL_UNSIGNED_INT ;
            config.pt = natus::graphics::es3::convert( geo->primitive_type() ) ;
        }

        // bind vertex buffer
        {
            glBindBuffer( GL_ARRAY_BUFFER, config.vb_id ) ;
            if( natus::es::error::check_and_log( natus_log_fn("glBindBuffer - vertex buffer") ) )
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
                if( natus::es::error::check_and_log( natus_log_fn( "glBufferData - vertex buffer" ) ) )
                    return false ;
                config.sib_vb = sib ;
            }
            else
            {
                glBufferSubData( GL_ARRAY_BUFFER, 0, sib, geo->vertex_buffer().data() ) ;
                natus::es::error::check_and_log( natus_log_fn( "glBufferSubData - vertex buffer" ) ) ;
            }
        }

        // bind index buffer
        {
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, config.ib_id ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glBindBuffer - index buffer" ) ) )
                return false ;
        }

        // allocate buffer memory
        // what about mapped memory?
        if( geo->index_buffer().get_num_elements() > 0 )
        {
            config.ib_elem_sib = geo->index_buffer().get_element_sib() ;

            GLuint const sib = GLuint( geo->index_buffer().get_sib() ) ;
            if( is_config || sib > config.sib_ib )
            {
                glBufferData( GL_ELEMENT_ARRAY_BUFFER, sib,
                    geo->index_buffer().data(), GL_STATIC_DRAW ) ;
                if( natus::es::error::check_and_log( natus_log_fn( "glBufferData - index buffer" ) ) )
                    return false ;
                config.sib_ib = sib ;
            }
            else
            {
                glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, sib, geo->index_buffer().data() ) ;
                natus::es::error::check_and_log( natus_log_fn( "glBufferSubData - index buffer" ) ) ;
            }
        }

        if( is_config )
        {
            for( auto const& rid : config.rd_ids )
            {
                if( rid == size_t( -1 ) ) continue ;
                if( _renders[ rid ].shd_id == size_t( -1 ) ) continue ;
                // @todo it should be required to rebind vertex data to attributes
                // if the vertex layout changes.
                //this_t::bind_attributes( _shaders[ _renders[ rid ].shd_id ], config ) ;
            }
        }

        return true ;
    }

    //********************************************************************************************
    bool_t update( size_t const id, natus::graphics::image_object_ref_t confin, bool_t const do_config )
    {
        this_t::image_data_ref_t config = _images[ id ] ;

        glBindTexture( config.type, config.tex_id ) ;
        if( natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) )
            return false ;

        size_t const sib = confin.image().sib() ;
        GLenum const target = config.type ;
        GLint const level = 0 ;
        GLsizei const width = GLsizei( confin.image().get_dims().x() ) ;
        GLsizei const height = GLsizei( confin.image().get_dims().y() ) ;
        GLsizei const depth = GLsizei( confin.image().get_dims().z() ) ;
        GLenum const format = natus::graphics::es3::convert_to_gl_pixel_format( confin.image().get_image_format() ) ;
        GLenum const type = natus::graphics::es3::convert_to_gl_pixel_type( confin.image().get_image_element_type() ) ;
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
            natus::es::error::check_and_log( natus_log_fn( "glPixelStorei" ) ) ;
        }

        if( do_config || (sib == 0 || config.sib < sib) )
        {
            GLint const border = 0 ;
            GLint const internal_format = natus::graphics::es3::convert_to_gl_format( confin.image().get_image_format(), confin.image().get_image_element_type() ) ;

            if( target == GL_TEXTURE_2D )
            {
                glTexImage2D( target, level, internal_format, width, height,
                              border, format, type, data ) ;
                natus::es::error::check_and_log( natus_log_fn( "glTexImage2D" ) ) ;
            }
            else if( target == GL_TEXTURE_2D_ARRAY )
            {
                glTexImage3D( target, level, internal_format, width, height, depth,
                    border, format, type, data ) ;
                natus::es::error::check_and_log( natus_log_fn( "glTexImage3D" ) ) ;
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
                natus::es::error::check_and_log( natus_log_fn( "glTexSubImage2D" ) ) ;
            }
            else if( target == GL_TEXTURE_2D_ARRAY )
            {
                glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth,
                    format, type, data ) ;
                natus::es::error::check_and_log( natus_log_fn( "glTexSubImage3D" ) ) ;
            }
        }

        config.sib = confin.image().sib() ;

        return true ;
    }

    //****************************************************************************************
    bool_t connect( size_t const id, natus::graphics::variable_set_res_t vs )
    {
        auto& config = _renders[ id ] ;

        this_t::connect( config, vs ) ;

        return true ;
    }

    //*********************************************************************************************
    bool_t connect( this_t::render_data & config, natus::graphics::variable_set_res_t vs )
    {
        auto item_data = ::std::make_pair( vs,
            natus::ntd::vector< this_t::render_data::uniform_variable_link >() ) ;

        auto item_tex = ::std::make_pair( vs,
            natus::ntd::vector< this_t::render_data::uniform_texture_link >() ) ;

        auto item_buf = ::std::make_pair( vs,
            natus::ntd::vector< this_t::render_data::uniform_array_data_link >() ) ;

        auto item_tfb = std::make_pair( vs,
            natus::ntd::vector< this_t::render_data::uniform_streamout_link >() ) ;

        this_t::shader_data_ref_t shd = _shaders[ config.shd_id ] ;

        size_t id = 0 ;
        for( auto& uv : shd.uniforms )
        {
            // is it a data uniform variable?
            if( natus::ogl::uniform_is_data( uv.type ) )
            {
                auto const types = natus::graphics::es3::to_type_type_struct( uv.type ) ;
                auto* var = vs->data_variable( uv.name, types.first, types.second ) ;
                if( natus::core::is_nullptr( var ) )
                {
                    natus::log::global_t::error( natus_log_fn( "can not claim variable " + uv.name ) ) ;
                    continue ;
                }

                this_t::render_data::uniform_variable_link link ;
                link.uniform_id = id++ ;
                link.var = var ;

                item_data.second.emplace_back( link ) ;
            }
            else if( natus::ogl::uniform_is_texture( uv.type ) )
            {
                //auto const types = natus::graphics::es3::to_type_type_struct( uv.type ) ;
                auto* var = vs->texture_variable( uv.name ) ;
                var = var->get().empty() ? vs->array_variable( uv.name ) : var ;

                if( natus::core::is_nullptr( var ) )
                {
                    natus::log::global_t::error( natus_log_fn( "can not claim variable " + uv.name ) ) ;
                    continue ;
                }

                size_t i = 0 ;
                auto const & tx_name = var->get() ;
                for( auto & cfg : _images )
                {
                    if( cfg.name == tx_name ) break ;
                    ++i ;
                }

                if( i >= _images.size() ) 
                {
                    natus::log::global_t::warning( natus_log_fn("image not found : " + tx_name ) ) ;
                    continue ;
                }

                this_t::render_data::uniform_texture_link link ;
                link.uniform_id = id++ ;
                link.tex_id = _images[ i ].tex_id ;
                link.img_id = i ;
                item_tex.second.emplace_back( link ) ;
            }
            else if( natus::ogl::uniform_is_buffer( uv.type ) )
            {
                auto* var = vs->array_variable( uv.name ) ;

                if( natus::core::is_nullptr( var ) )
                {
                    natus::log::global_t::error( natus_log_fn( "can not claim variable " + uv.name ) ) ;
                    continue ;
                }

                if( var->get().empty() )
                    var = vs->array_variable_streamout( uv.name ) ;

                auto const & tx_name = var->get() ;

                // looking for data buffer
                auto handle_buffer_link = [&]( void_t )
                {
                    size_t const i = this_t::find_index_by_resource_name( tx_name, _arrays ) ;
                    if( i >= _arrays.size() ) return false ;

                    this_t::render_data::uniform_array_data_link link ;
                    link.uniform_id = id++ ;
                    link.tex_id = _arrays[ i ].tex_id ;
                    link.buf_id = i ;
                    item_buf.second.emplace_back( link ) ;
                    return true ;
                } ;

                // looking for streamout/transform feedback
                auto handle_feedback_link = [&]( void_t )
                {
                    size_t const i = this_t::find_index_by_resource_name( tx_name, _feedbacks ) ;
                    if( i >= _feedbacks.size() ) return false ;

                    this_t::render_data::uniform_streamout_link link ;
                    link.uniform_id = id++ ;
                    link.tex_id[0] = _feedbacks[ i ]._buffers[0].tids[0] ;
                    link.tex_id[1] = _feedbacks[ i ]._buffers[1].tids[0] ;
                    link.so_id = i ;
                    item_tfb.second.emplace_back( link ) ;
                    return true ;
                } ;

                if( !handle_buffer_link() )
                {
                    if( !handle_feedback_link() )
                    {
                        natus::log::global_t::error( natus_log_fn( 
                            "Could not find array nor streamout object [" + tx_name + "]" ) ) ;
                        continue ;
                    }
                }
            }
        }

        config.var_sets.emplace_back( vs ) ;
        config.var_sets_data.emplace_back( item_data ) ;
        config.var_sets_texture.emplace_back( item_tex ) ;
        config.var_sets_array.emplace_back( item_buf ) ;
        config.var_sets_streamout.emplace_back( item_tfb ) ;

        return true ;
    }

    //************************************************************************************
    size_t construct_array_data( size_t oid, natus::graphics::array_object_ref_t obj ) 
    {
        oid = determine_oid( obj.name(), _arrays ) ;

        auto & data = _arrays[ oid ] ;

        // buffer
        if( data.buf_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenBuffers( 1, &id ) ;
            natus::es::error::check_and_log( 
                natus_log_fn( "[construct_array_data] : glGenBuffers" ) ) ;

            data.buf_id = id ;
        }

        // texture
        if( data.tex_id == GLuint(-1) )
        {
            GLuint id = GLuint( -1 ) ;
            glGenTextures( 1, &id ) ;
            natus::es::error::check_and_log( 
                natus_log_fn( "[construct_array_data] : glGenTextures" ) ) ;

            data.tex_id = id ;
        }

        return oid ;
    }

    //**************************************************************************************
    bool_t release_array_data( size_t const oid ) noexcept
    {
        auto & data = _arrays[ oid ] ;

        if( data.buf_id == GLuint(-1) )
        {
            glDeleteBuffers( 1, &data.buf_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteBuffers" ) ) ;
            data.buf_id = GLuint( -1 ) ;
        }

        if( data.tex_id == GLuint(-1) )
        {
            glDeleteTextures( 1, &data.tex_id ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDeleteTextures" ) ) ;
            data.tex_id = GLuint( -1 ) ;
        }

        data.valid = false ;
        data.name = "released" ;
        data.sib = 0 ;

        return true ;
    }

    //***********************************************************************************
    size_t update( size_t oid, natus::graphics::array_object_ref_t obj, bool_t const is_config = false ) 
    {
        auto & data = _arrays[ oid ] ;

        // do buffer
        {
            // bind buffer
            {
                glBindBuffer( GL_TEXTURE_BUFFER, data.buf_id ) ;
                if( natus::es::error::check_and_log( natus_log_fn("glBindBuffer") ) )
                    return false ;
            }

            // transfer data
            GLuint const sib = GLuint( obj.data_buffer().get_sib() ) ;
            if( is_config || sib > data.sib )
            {
                glBufferData( GL_TEXTURE_BUFFER, sib,
                        obj.data_buffer().data(), GL_DYNAMIC_DRAW ) ;
                if( natus::es::error::check_and_log( natus_log_fn( "glBufferData" ) ) )
                    return false ;
                data.sib = sib ;
            }
            else
            {
                glBufferSubData( GL_TEXTURE_BUFFER, 0, sib, obj.data_buffer().data() ) ;
                natus::es::error::check_and_log( natus_log_fn( "glBufferSubData" ) ) ;
            }

            // bind buffer
            {
                glBindBuffer( GL_TEXTURE_BUFFER, 0 ) ;
                if( natus::es::error::check_and_log( natus_log_fn("glBindBuffer") ) )
                    return false ;
            }
        }

        // do texture
        // glTexBuffer is required to be called after driver memory is aquired.
        {
            glBindTexture( GL_TEXTURE_BUFFER, data.tex_id ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) )
                return false ;

            auto const le = obj.data_buffer().get_layout_element(0) ;
            glTexBuffer( GL_TEXTURE_BUFFER, natus::graphics::es3::convert_for_texture_buffer(
                le.type, le.type_struct ), data.buf_id ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glTexBuffer" ) ) )
                return false ;
        }

        return true ;
    }

    //****************************************************************************************
    size_t construct_feedback( size_t /*oid_*/, natus::graphics::streamout_object_ref_t obj ) noexcept
    {
        size_t const oid = determine_oid( obj.name(), _feedbacks ) ;

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

                natus::ntd::string_t const is = std::to_string( i ) ;
                natus::ntd::string_t const bs = std::to_string( rw ) ;

                size_t const gid = this_t::construct_geo( size_t(-1), 
                    obj.name() + ".feedback."+is+"."+bs, obj.get_buffer(i) ) ;

                buffer.gids[i] = gid ;
                buffer.bids[i] = _geometries[ gid ].vb_id ;
                buffer.sibs[i] = 0 ; 
            }

            // we just allways gen max buffers tex ids
            {
                glGenTextures( this_t::tf_data::buffer::max_buffers, buffer.tids ) ;
                natus::es::error::check_and_log( natus_log_fn( "glGenTextures" ) ) ;
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
                natus::es::error::check_and_log( natus_log_fn( "glGenQueries" ) ) ;
            }

            if( buffer.tfid == GLuint(-1) )
            {
                glGenTransformFeedbacks( 1, &buffer.tfid ) ;
                natus::es::error::check_and_log( natus_log_fn( "glGenTransformFeedbacks" ) ) ;
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
                natus::es::error::check_and_log( natus_log_fn( "glDeleteBuffers" ) ) ;
                buffer.qid = GLuint( -1 ) ; 
            }

            if( buffer.tfid != GLuint(-1) )
            {
                glDeleteTransformFeedbacks( 1, &buffer.tfid ) ;
                natus::es::error::check_and_log( natus_log_fn( "glDeleteTransformFeedbacks" ) ) ;
                buffer.tfid = GLuint( -1 ) ; 
            }

            // release textures
            {
                glDeleteTextures( tf_data_t::buffer::max_buffers, buffer.tids ) ;
                natus::es::error::check_and_log( natus_log_fn( "glDeleteTextures" ) ) ;
            }
        }

        d.valid = false ;
        d.name = "__released__" ;

        return true ;
    }

    //****************************************************************************************
    bool_t update( size_t oid, natus::graphics::streamout_object_ref_t obj, bool_t const is_config = false ) 
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
                    if( natus::es::error::check_and_log( natus_log_fn("glBindBuffer") ) )
                        continue ;
                }

                // allocate data
                GLuint const sib = GLuint( obj.get_buffer( i ).get_layout_sib() * obj.size() ) ;
                if( is_config || sib > buffer.sibs[i] )
                {
                    glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, sib, nullptr, GL_DYNAMIC_DRAW ) ;
                    if( natus::es::error::check_and_log( natus_log_fn( "glBufferData" ) ) )
                        continue ;

                    buffer.sibs[i] = sib ;
                }
               
                // do texture
                // glTexBuffer is required to be called after driver memory is aquired.
                {
                    glBindTexture( GL_TEXTURE_BUFFER, buffer.tids[i] ) ;
                    if( natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) )
                        continue ;

                    auto const le = obj.get_buffer( i ).get_layout_element_zero() ;
                    glTexBuffer( GL_TEXTURE_BUFFER, natus::graphics::es3::convert_for_texture_buffer(
                        le.type, le.type_struct ), buffer.bids[i] ) ;
                    if( natus::es::error::check_and_log( natus_log_fn( "glTexBuffer" ) ) )
                        continue ;

                    glBindTexture( GL_TEXTURE_BUFFER, 0 ) ;
                    if( natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) )
                        continue ;
                }

                ++i ;
            }
        }

        // unbind
        {
            glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, 0 ) ;
            if( natus::es::error::check_and_log( natus_log_fn("glBindBuffer") ) )
                return false ;
        }

        for( size_t rw=0; rw<2; ++rw )
        {
            auto & buffer = data._buffers[rw] ;

            {
                glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, buffer.tfid ) ;
                if( natus::es::error::check_and_log( natus_log_fn("glBindTransformFeedback") ) )
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
                    natus::es::error::check_and_log( natus_log_fn( "glBindBufferRange" ) ) ;
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
            natus::es::error::check_and_log( natus_log_fn( "glBindTransformFeedback" ) ) ;
        }

        // query written primitives
        {
            glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, buffer.qid ) ;
            natus::es::error::check_and_log( natus_log_fn( 
                        "glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN )" ) ) ;
        }

        // begin 
        {
            // lets just use the primitive type of the used geometry
            GLenum const mode = gdata.pt ;
            glBeginTransformFeedback( mode ) ;
            natus::es::error::check_and_log( natus_log_fn( "glBeginTransformFeedback" ) ) ;
        }
    }

    //****************************************************************************************
    void_t deactivate_transform_feedback( void_t ) noexcept
    {
        if( _tf_active_id == size_t(-1) ) return ;

        {
            glEndTransformFeedback() ;
            natus::es::error::check_and_log( natus_log_fn( "glEndTransformFeedback" ) ) ;
        }

        // query written primitives
        {
            glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN ) ;
            natus::es::error::check_and_log( natus_log_fn( "glEndQuery" ) ) ;
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
            natus::es::error::check_and_log( natus_log_fn( "glGetQueryObjectuiv" ) ) ;
        }
        #endif

        _tf_active_id = size_t(-1) ;
    }

    //***************************************************************************
    bool_t update_variables( size_t const rd_id, size_t const varset_id )
    {
        this_t::render_data & config = _renders[ rd_id ] ;
        this_t::shader_data & sconfig = _shaders[ config.shd_id ] ;

        {
            glUseProgram( sconfig.pg_id ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glUseProgram" ) ) )
            {
                return false ;
            }
        }

        if( config.var_sets_data.size() > varset_id )
        {
            // data vars
            {
                auto& varset = config.var_sets_data[ varset_id ] ;
                for( auto& item : varset.second )
                {
                    auto& uv = sconfig.uniforms[ item.uniform_id ] ;
                    uv.do_copy_funk( item.mem, item.var ) ;
                }
            }

            // tex vars
            // this section must match with the section in the render function
            // because of the used texture unit
            {
                int_t tex_unit = 0 ;
                // textures
                {
                    auto& varset = config.var_sets_texture[ varset_id ] ;
                    for( auto& item : varset.second )
                    {
                        auto var = natus::graphics::data_variable< int_t >( tex_unit ) ;
                        auto & uv = sconfig.uniforms[ item.uniform_id ] ;

                        uv.do_copy_funk( item.mem, &var ) ;

                        ++tex_unit ;
                    }
                }

                // array data vars
                {
                    auto & varset = config.var_sets_array[ varset_id ] ;
                    for( auto & item : varset.second )
                    {
                        auto var = natus::graphics::data_variable< int_t >( tex_unit ) ;
                        auto & uv = sconfig.uniforms[ item.uniform_id ] ;

                        uv.do_copy_funk( item.mem, &var ) ;

                        ++tex_unit ;
                    }
                }

                // transform feedback bound vars
                {
                    auto & varset = config.var_sets_streamout[ varset_id ] ;
                    for( auto & item : varset.second )
                    {
                        auto var = natus::graphics::data_variable< int_t >( tex_unit ) ;
                        auto & uv = sconfig.uniforms[ item.uniform_id ] ;

                        uv.do_copy_funk( item.mem, &var ) ;

                        ++tex_unit ;
                    }
                }
            }
        }

        return true ;
    }

    //************************************************************************************
    bool_t render( size_t const id, size_t const geo_idx = 0, bool_t feed_from_tf = false,
                   bool_t const use_streamout_count = false, size_t const varset_id = size_t(0), 
                   GLsizei const start_element = GLsizei(0), GLsizei const num_elements = GLsizei(-1) )
    {
        this_t::render_data & config = _renders[ id ] ;
        this_t::shader_data& sconfig = _shaders[ config.shd_id ] ;

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
            natus::log::global_t::error( "[gl3::render] : used geometry idx invalid because" 
                "exceeds array size for render object : " + config.name ) ;
                return false ;
        }

        this_t::geo_data& gconfig = _geometries[ gid ] ;

        if( !sconfig.is_compilation_ok ) return false ;

        {
            glBindVertexArray( gconfig.va_id ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glBindVertexArray" ) ) )
                return false ;
        }

        {
            glUseProgram( sconfig.pg_id ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glUseProgram" ) ) )
                return false ;
        }

        if( config.var_sets_data.size() > varset_id )
        {
            // data vars
            {
                auto& varset = config.var_sets_data[ varset_id ] ;
                for( auto& item : varset.second )
                {
                    auto& uv = sconfig.uniforms[ item.uniform_id ] ;
                    if( !uv.do_uniform_funk( item.mem ) )
                    {
                        natus::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ; 
                    }
                }
            }

            // tex vars
            {
                int_t tex_unit = 0 ;

                // textures
                {
                    auto& varset = config.var_sets_texture[ varset_id ] ;
                    for( auto& item : varset.second )
                    {
                        glActiveTexture( GLenum(GL_TEXTURE0 + tex_unit) ) ;
                        natus::es::error::check_and_log( natus_log_fn( "glActiveTexture" ) ) ;

                        {
                            auto const& ic = _images[ item.img_id ] ;

                            glBindTexture( ic.type, item.tex_id ) ;
                            natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) ;

                            glTexParameteri( ic.type, GL_TEXTURE_WRAP_S, ic.wrap_types[0] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_WRAP_T, ic.wrap_types[1] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_WRAP_R, ic.wrap_types[2] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_MIN_FILTER, ic.filter_types[0] ) ;
                            glTexParameteri( ic.type, GL_TEXTURE_MAG_FILTER, ic.filter_types[1] ) ;
                            natus::es::error::check_and_log( natus_log_fn( "glTexParameteri" ) ) ;
                        }

                        {
                            auto & uv = sconfig.uniforms[ item.uniform_id ] ;
                            if( !uv.do_uniform_funk( item.mem ) )
                            {
                                natus::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ; 
                            }
                        }
                        ++tex_unit ;
                    }
                }

                // array data vars
                {
                    auto & varset = config.var_sets_array[ varset_id ] ;
                    for( auto & item : varset.second )
                    {
                        glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) ) ;
                        natus::es::error::check_and_log( natus_log_fn( "glActiveTexture" ) ) ;
                        glBindTexture( GL_TEXTURE_BUFFER, item.tex_id ) ;
                        natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) ;

                        {
                            auto & uv = sconfig.uniforms[ item.uniform_id ] ;
                            if( !uv.do_uniform_funk( item.mem ) )
                            {
                                natus::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ; 
                            }
                        }

                        ++tex_unit ;
                    }
                }

                // transform feedback as TBO
                {
                    auto & varset = config.var_sets_streamout[ varset_id ] ;
                    for( auto & item : varset.second )
                    {
                        auto const & tfd = _feedbacks[ item.so_id ] ;
                        GLuint const tid = item.tex_id[tfd.read_index()] ;

                        glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) ) ;
                        natus::es::error::check_and_log( natus_log_fn( "glActiveTexture" ) ) ;
                        glBindTexture( GL_TEXTURE_BUFFER, tid ) ;
                        natus::es::error::check_and_log( natus_log_fn( "glBindTexture" ) ) ;

                        {
                            auto & uv = sconfig.uniforms[ item.uniform_id ] ;
                            if( !uv.do_uniform_funk( item.mem ) )
                            {
                                natus::log::global_t::error( "[gl4] : uniform " + uv.name + " failed." ) ; 
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
            natus::es::error::check_and_log( natus_log_fn( "glEnable( GL_RASTERIZER_DISCARD ) " ) ) ;
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

                #if 1 // this is gl 3.x
                GLuint num_prims = 0 ;
                {
                    glGetQueryObjectuiv( tfd.read_buffer().qid, GL_QUERY_RESULT, &num_prims ) ;
                    natus::es::error::check_and_log( natus_log_fn( "glGetQueryObjectuiv" ) ) ;
                }
                glDrawArrays( pt, start_element, num_prims * 
                              natus::graphics::es3::primitive_type_to_num_vertices( pt ) ) ;

                natus::es::error::check_and_log( natus_log_fn( "glDrawArrays" ) ) ;
                #else
                glDrawTransformFeedback( pt, tfd.read_buffer().tfid ) ;
                natus::es::error::check_and_log( natus_log_fn( "glDrawTransformFeedback" ) ) ;
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

                natus::es::error::check_and_log( natus_log_fn( "glDrawElements" ) ) ;
            }
            else
            {
                GLsizei const max_elems = GLsizei( gconfig.num_elements_vb ) ;
                GLsizei const ne = std::min( num_elements>=0?num_elements:max_elems, max_elems ) ;

                glDrawArrays( pt, start_element, ne ) ;

                natus::es::error::check_and_log( natus_log_fn( "glDrawArrays" ) ) ;
            }
        }

        {
            this_t::deactivate_transform_feedback() ;
        }

        // enable raster back again.
        if( sconfig.ps_id == GLuint(-1) )
        {
            glDisable( GL_RASTERIZER_DISCARD ) ;
            natus::es::error::check_and_log( natus_log_fn( "glDisable( GL_RASTERIZER_DISCARD ) " ) ) ;
        }

        {
            glBindVertexArray( 0 ) ;
            if( natus::es::error::check_and_log( natus_log_fn( "glBindVertexArray" ) ) )
                return false ;
        }

        return true ;
    }

    //**************************************************************************************
    void_t begin_frame( void_t ) 
    {
        // set default render states
        {
            auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) ) ;

            // set the viewport to the default new state, 
            // so the correct viewport is set automatically.
            {
                natus::math::vec4ui_t vp = _states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp ;
                vp.z( uint_t( vp_width ) ) ;
                vp.w( uint_t( vp_height ) ) ;
                _states[ ids_new.first ].states[ ids_new.second ].view_s.ss.vp = vp ;
            }

            _state_stack.push( _states[ ids_new.first ].states[ ids_new.second ] ) ;
        }

        // do clear the frame
        {
            natus::math::vec4f_t const clear_color = _state_stack.top().clear_s.ss.clear_color ;
            glClearColor( clear_color.x(), clear_color.y(), clear_color.z(), clear_color.w() ) ;
            natus::es::error::check_and_log( natus_log_fn( "glClearColor" ) ) ;

            GLbitfield const color = GL_COLOR_BUFFER_BIT ;
            GLbitfield const depth = GL_DEPTH_BUFFER_BIT ;

            glClear( color | depth ) ;
            natus::es::error::check_and_log( natus_log_fn( "glEnable" ) ) ;
        }
    }

    void_t end_frame( void_t ) 
    {
        glFlush() ;
        glFinish() ;

        _state_stack.pop() ;
    }
};

//****************************************************************************************************************************************************************************************************************
//
//
//****************************************************************************************************************************************************************************************************************

//********************************************************************************************************************
es3_backend::es3_backend( natus::graphics::es_context_ptr_t ctx ) noexcept : 
    backend( natus::graphics::backend_type::es3 )
{
    _pimpl = natus::memory::global_t::alloc( pimpl( ctx ), 
        natus_log_fn("es3_backend::pimpl") ) ;

    _context = ctx ;
}

//********************************************************************************************************************
es3_backend::es3_backend( this_rref_t rhv ) noexcept : backend( ::std::move( rhv ) )
{
    natus_move_member_ptr( _pimpl, rhv ) ;
    natus_move_member_ptr( _context, rhv ) ;
}

//********************************************************************************************************************
es3_backend::~es3_backend( void_t ) 
{
    natus::memory::global_t::dealloc( _pimpl ) ;
}

//********************************************************************************************************************
void_t es3_backend::set_window_info( window_info_cref_t wi ) noexcept 
{
    {
        if( wi.width != 0 )
        {
            _pimpl->vp_width = GLsizei( wi.width ) ;
        }
        if( wi.height != 0 )
        {
            _pimpl->vp_height = GLsizei( wi.height ) ;
        }
    }
}

//*****************************************************************************************
natus::graphics::result es3_backend::configure( natus::graphics::geometry_object_res_t gconf ) noexcept 
{
    natus::graphics::id_res_t id = gconf->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_geo( 
            id->get_oid( this_t::get_bid() ), *gconf ) ) ;
    }

    {
        auto const res = _pimpl->update( id->get_oid( this_t::get_bid() ), gconf, true ) ;
        if( natus::core::is_not( res ) )
        {
            return natus::graphics::result::failed ;
        }
    }

    return natus::graphics::result::ok ;
}

//***************************************************************************************
natus::graphics::result es3_backend::configure( natus::graphics::render_object_res_t config ) noexcept 
{
    natus::graphics::id_res_t id = config->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_render_config( 
            id->get_oid( this_t::get_bid() ), *config ) ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *config ) ;
        if( natus::core::is_not( res ) )
        {
            return natus::graphics::result::failed ;
        }
    }

    return natus::graphics::result::ok ;
}

//***
natus::graphics::result es3_backend::configure( natus::graphics::shader_object_res_t config ) noexcept
{
    natus::graphics::id_res_t id = config->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_shader_data( 
            id->get_oid( this_t::get_bid() ), *config ) ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *config ) ;
        if( natus::core::is_not( res ) )
        {
            return natus::graphics::result::failed ;
        }
    }

    return natus::graphics::result::ok ;
}

//***
natus::graphics::result es3_backend::configure( natus::graphics::image_object_res_t config ) noexcept 
{
    natus::graphics::id_res_t id = config->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_image_config( 
            id->get_oid( this_t::get_bid() ), *config ) ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *config, true ) ;
        if( natus::core::is_not( res ) )
        {
            return natus::graphics::result::failed ;
        }
    }

    return natus::graphics::result::ok ;
}

//***
natus::graphics::result es3_backend::configure( natus::graphics::framebuffer_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    natus::graphics::id_res_t id = obj->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_framebuffer(
            id->get_oid( this_t::get_bid() ), *obj ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::configure( natus::graphics::state_object_res_t obj ) noexcept
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    natus::graphics::id_res_t id = obj->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_state(
            id->get_oid( this_t::get_bid() ), *obj ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::configure( natus::graphics::array_object_res_t obj ) noexcept 
{
    natus::graphics::id_res_t id = obj->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_array_data( 
            id->get_oid( this_t::get_bid() ), *obj ) ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, true ) ;
        if( natus::core::is_not( res ) ) return natus::graphics::result::failed ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::configure( natus::graphics::streamout_object_res_t obj ) noexcept 
{
    natus::graphics::id_res_t id = obj->get_id() ;

    {
        id->set_oid( this_t::get_bid(), _pimpl->construct_feedback( 
            id->get_oid( this_t::get_bid() ), *obj ) ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, true ) ;
        if( natus::core::is_not( res ) ) return natus::graphics::result::failed ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::geometry_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        _pimpl->release_geometry( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::render_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        _pimpl->release_render_data( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::shader_object_res_t obj ) noexcept
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        _pimpl->release_shader_data( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::image_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        _pimpl->release_image_data( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::framebuffer_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        _pimpl->release_framebuffer( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::state_object_res_t obj ) noexcept
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        //_pimpl->relese( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::array_object_res_t obj ) noexcept
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        _pimpl->release_array_data( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::release( natus::graphics::streamout_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() || obj->name().empty() )
    {
        natus::log::global_t::error( natus_log_fn( "Object must be valid and requires a name" ) ) ;
        return natus::graphics::result::invalid_argument ;
    }

    {
        natus::graphics::id_res_t id = obj->get_id() ;
        _pimpl->release_tf_data( id->get_oid( this_t::get_bid() ) ) ;
        id->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::connect( natus::graphics::render_object_res_t config, 
                natus::graphics::variable_set_res_t vs ) noexcept
{
    natus::graphics::id_res_t id = config->get_id() ;

    if( id->is_not_valid( this_t::get_bid() ) )
    {
        natus::log::global_t::error( natus_log_fn( "invalid render configuration id" ) ) ;
        return natus::graphics::result::failed ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    auto const res = _pimpl->connect( oid, vs ) ;
    natus::log::global_t::error( natus::graphics::is_not( res ), 
        natus_log_fn( "connect variable set" ) ) ;
   
    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::update( natus::graphics::geometry_object_res_t config ) noexcept 
{
    natus::graphics::id_res_t id = config->get_id() ;

    if( id->is_not_valid( this_t::get_bid() ) )
    {
        natus::log::global_t::error( natus_log_fn( "invalid geometry configuration id" ) ) ;
        return natus::graphics::result::failed ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    auto const res = _pimpl->update( oid, config ) ;
    natus::log::global_t::error( natus::graphics::is_not( res ),
        natus_log_fn( "update geometry" ) ) ;

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::update( natus::graphics::streamout_object_res_t obj ) noexcept
{
    natus::graphics::id_res_t id = obj->get_id() ;
    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, false ) ;
        if( natus::core::is_not( res ) ) return natus::graphics::result::failed ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::update( natus::graphics::array_object_res_t obj ) noexcept 
{
    natus::graphics::id_res_t id = obj->get_id() ;
    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update( oid, *obj, false ) ;
        if( natus::core::is_not( res ) ) return natus::graphics::result::failed ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::update( natus::graphics::image_object_res_t ) noexcept 
{
    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::update( natus::graphics::render_object_res_t obj, size_t const varset ) noexcept 
{
    natus::graphics::id_res_t id = obj->get_id() ;
    size_t const oid = id->get_oid( this_t::get_bid() ) ;

    {
        auto const res = _pimpl->update_variables( oid, varset ) ;
        if( natus::core::is_not( res ) ) return natus::graphics::result::failed ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::use( natus::graphics::framebuffer_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() )
    {
        return this_t::unuse( natus::graphics::backend::unuse_type::framebuffer ) ;
    }

    natus::graphics::id_res_t id = obj->get_id() ;

    if( id->is_not_valid( this_t::get_bid() ) )
    {
        return this_t::unuse( natus::graphics::backend::unuse_type::framebuffer ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;
    auto const res = _pimpl->activate_framebuffer( oid ) ;
    if( !res ) return natus::graphics::result::failed ;

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::use( natus::graphics::streamout_object_res_t obj ) noexcept 
{
    if( !obj.is_valid() )
    {
        return this_t::unuse( natus::graphics::backend::unuse_type::streamout ) ;
    }

    natus::graphics::id_res_t id = obj->get_id() ;

    if( id->is_not_valid( this_t::get_bid() ) )
    {
        return this_t::unuse( natus::graphics::backend::unuse_type::streamout ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;
    auto const res = _pimpl->use_transform_feedback( oid ) ;
    if( !res ) return natus::graphics::result::failed ;

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::unuse( natus::graphics::backend::unuse_type const t ) noexcept 
{
    switch( t ) 
    {
    case natus::graphics::backend::unuse_type::framebuffer: _pimpl->deactivate_framebuffer() ; break ;
    case natus::graphics::backend::unuse_type::streamout: _pimpl->unuse_transform_feedback() ; break ;
    }

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::push( natus::graphics::state_object_res_t obj, size_t const sid, bool_t const ) noexcept 
{
    if( !obj.is_valid() )
    {
        return this_t::pop( natus::graphics::backend::pop_type::render_state ) ;
    }

    natus::graphics::id_res_t id = obj->get_id() ;

    if( id->is_not_valid( this_t::get_bid() ) )
    {
        return this_t::pop( natus::graphics::backend::pop_type::render_state ) ;
    }

    size_t const oid = id->get_oid( this_t::get_bid() ) ;
    _pimpl->handle_render_state( oid, sid ) ;

    return natus::graphics::result::ok ;
}

//**************************************************************************************************
natus::graphics::result es3_backend::pop( natus::graphics::backend::pop_type const ) noexcept 
{
    _pimpl->handle_render_state( size_t( -1 ), size_t( -1 ) ) ;
    return natus::graphics::result::ok ;
}

//***********************************************************************************************
natus::graphics::result es3_backend::render( natus::graphics::render_object_res_t config, 
         natus::graphics::backend::render_detail_cref_t detail ) noexcept 
{ 
    natus::graphics::id_res_t id = config->get_id() ;

    //natus::log::global_t::status( natus_log_fn("render") ) ;

    if( id->is_not_valid( this_t::get_bid()) )
    {
        natus::log::global_t::error( natus_log_fn( "invalid id" ) ) ;
        return natus::graphics::result::failed ;
    }

    _pimpl->render( id->get_oid( this_t::get_bid()), detail.geo,
                    detail.feed_from_streamout, detail.use_streamout_count,
                    detail.varset, (GLsizei)detail.start, (GLsizei)detail.num_elems ) ;

    return natus::graphics::result::ok ;
}

//********************************************************************************************************************
void_t es3_backend::render_begin( void_t ) noexcept 
{
    _pimpl->begin_frame() ;
}

void_t es3_backend::render_end( void_t ) noexcept 
{
    _pimpl->end_frame() ;
}
