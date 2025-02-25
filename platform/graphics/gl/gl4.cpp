

#include "gl4.h"
#include "gl4_convert.h"
#include "../datas.hpp"

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


#define gl4_log( text ) "[GL4] : " text
#define gl4_log_error( text ) motor::ogl::error::check_and_log( "[GL4] : " text )
#define gl4_log_error2( text, name ) motor::ogl::error::check_and_log( "[GL4] : " text,  name )
#define gl4_log_error3( text, obj ) motor::ogl::error::check_and_log<decltype(obj)>( "[GL4] : " text,  obj )

using namespace motor::platform::gen4 ;
using namespace motor::ogl ;

namespace local_gl4
{
    template< typename T >
    struct work_item_deduction ;

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
        static work_item make_item( work_type const t, motor::core::mtr_safe<T> obj ) noexcept
        {
            return work_item{ t, work_item_deduction<T>::deduce( obj.mtr() ), 
                static_cast<void_ptr_t>( obj.mtr() ) } ;
        }
    };
    motor_typedef( work_item ) ;

    template< typename T >
    struct work_item_deduction
    {
        static work_item::obj_type deduce( T * ptr ) noexcept ;
    };

    template<>
    struct work_item_deduction< motor::graphics::msl_object >
    {
        static work_item::obj_type deduce( motor::graphics::msl_object * ptr ) noexcept 
        { return work_item::obj_type::msl ; }
    };
}

struct gl4_backend::pimpl
{
    motor_this_typedefs( pimpl ) ;

    //********************************************************************************
    // transform feedback buffers
    struct feedback_data
    {
        motor_this_typedefs( feedback_data ) ;

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

        void_t invalidate( motor::string_in_t name ) noexcept
        {
            for( size_t rw=0; rw<2; ++rw )
            {
                auto & buffer = _buffers[rw] ;

                if( buffer.qid != GLuint(-1) )
                {
                    glDeleteQueries( 1, &buffer.qid ) ;
                    gl4_log_error2( "glDeleteQueries", name.c_str() ) ;
                    buffer.qid = GLuint( -1 ) ; 
                }

                if( buffer.tfid != GLuint(-1) )
                {
                    glDeleteTransformFeedbacks( 1, &buffer.tfid ) ;
                    gl4_log_error2( "glDeleteTransformFeedbacks", name.c_str() ) ;
                    buffer.tfid = GLuint( -1 ) ; 
                }

                // release textures
                if( buffer.tids[0] != GLuint(-1) )
                {
                    glDeleteTextures( feedback_data::buffer::max_buffers, buffer.tids ) ;
                    gl4_log_error2( "glDeleteTextures", name.c_str() ) ;
                }
            }
        }
    };
    motor_typedef( feedback_data ) ;

    //typedef motor::vector< this_t::tf_data_t > tf_datas_t ;
    using feedbacks_t = motor::platform::datas< this_t::feedback_data_t > ;
    feedbacks_t _feedbacks ;

    //********************************************************************************
    struct geo_data
    {
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

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            elements.clear() ;
            ib_elem_sib = 0 ;
            stride = 0 ;

            if( vb_id != GLuint( -1 ) )
            {
                glDeleteBuffers( 1, &vb_id ) ;
                gl4_log_error( "glDeleteBuffers : vertex buffer" ) ;
                vb_id = GLuint( -1 ) ;
            }

            if( ib_id != GLuint( -1 ) )
            {
                glDeleteBuffers( 1, &ib_id ) ;
                gl4_log_error( "glDeleteBuffers : index buffer" ) ;
                ib_id = GLuint( -1 ) ;
            }

            ib_elem_sib = 0 ;
            rd_ids.clear() ;
        }
    };
    motor_typedef( geo_data ) ;
    using geo_datas_t = motor::platform::datas< this_t::geo_data_t > ;
    geo_datas_t _geometries ;

    //********************************************************************************
    struct shader_data
    {
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
            bool_t do_uniform_funk( void_ptr_t mem_ ) noexcept
            {
                uniform_funk( loc, 1, mem_ ) ;
                return !motor::ogl::error::check_and_log( gl4_log( "glUniform" ) ) ;
            }

            void_t do_copy_funk( void_ptr_t mem_, motor::graphics::ivariable_ptr_t var ) noexcept
            {
                std::memcpy( mem_, var->data_ptr(), motor::ogl::uniform_size_of( type ) ) ;
            }
        };
        motor_typedef( uniform_variable ) ;

        motor::vector< uniform_variable > uniforms ;

    public:

        //*******************************************************
        void_t delete_all_variables( void_t )
        {
            vertex_inputs.clear() ;

            //for( auto & v : config.attributes )
            {
                // delete memory
            }
            attributes.clear() ;
            uniforms.clear() ;
        }

        //*******************************************************
        // detach shaders from program
        void_t detach_shaders( void_t ) noexcept
        {
            if( pg_id == GLuint(-1) ) return ;

            GLsizei count = 0 ;
            GLuint shaders_[ 10 ] ;

            glGetAttachedShaders( pg_id, 10, &count, shaders_ ) ;
            gl4_log_error( "glGetAttachedShaders" ) ;

            for( GLsizei i = 0; i < count; ++i )
            {
                glDetachShader( pg_id, shaders_[ i ] ) ;
                gl4_log_error( "glDetachShader" ) ;
            }
        }

        //*******************************************************
        void_t delete_shaders( void_t ) noexcept
        {
            if( vs_id != GLuint(-1) )
            {
                glDeleteShader( vs_id ) ;
                gl4_log_error( "glDeleteShader : vertex shader" ) ;
                vs_id = GLuint(-1) ;
            }

            if( gs_id != GLuint(-1) )
            {
                glDeleteShader( gs_id ) ;
                gl4_log_error( "glDeleteShader : geometry shader" ) ;
                gs_id = GLuint(-1) ;
            }

            if( ps_id != GLuint(-1) )
            {
                glDeleteShader( ps_id ) ;
                gl4_log_error( "glDeleteShader : pixel shader" ) ;
                ps_id = GLuint(-1) ;
            }
        }

        void_t delete_program( void_t ) noexcept
        {
            if( pg_id != GLuint( -1 ) )
            {
                glDeleteProgram( pg_id ) ;
                gl4_log_error( "glDeleteProgram" ) ;
                pg_id = GLuint( -1 ) ;
            }
        }

    public:

        //*******************************************************
        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            is_linkage_ok = false ;

            detach_shaders() ;
            delete_shaders() ;
            delete_program() ;

            if( output_names != nullptr )
            {
                motor::memory::global_t::dealloc_raw( output_names ) ;
            }

            uniforms.clear() ;
            attributes.clear() ;
        }
    } ;
    motor_typedef( shader_data ) ;

    using shader_datas_t = motor::platform::datas< shader_data_t > ;
    shader_datas_t _shaders ;


    //********************************************************************************
    struct state_data
    {
        motor::vector< motor::graphics::render_state_sets_t > states ;

        void_t invalidate( motor::string_in_t ) {}
    } ;
    motor_typedef( state_data ) ;

    //typedef motor::vector< this_t::state_data_t > state_datas_t ;
    using states_t = motor::platform::datas< state_data > ;
    states_t _states ;
    motor::stack< motor::graphics::render_state_sets_t, 10 > _state_stack ;

    //********************************************************************************
    struct render_data
    {
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
            // the hash value of the images' name
            // so we can change the image linked
            // at run time. 
            size_t img_hash ;

            // the user variable holding the data.
            motor::graphics::texture_variable_t * var ;

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

        void_t invalidate( motor::string_in_t name ) noexcept
        {
            // need to clear geometry connections before!
            // I am trying to remove those connections.
            geo_ids.clear() ;
        
            // need to clear transform fb connections before!
            // I am trying to remove those connections.
            tf_ids.clear() ;

            shd_id = GLuint( -1 ) ;
            rss.clear() ;
        
            for( auto * v : var_sets ) motor::memory::release_ptr( v ) ;

            // remember that rd.var_sets hold the ref count reminder!
            var_sets.clear() ;
            var_sets_array.clear() ;
            var_sets_streamout.clear() ;
            var_sets_data.clear() ;
            var_sets_texture.clear() ;

            motor::memory::global_t::dealloc( mem_block ) ;
            mem_block = nullptr ;

            for( auto & d : geo_to_vaos ) 
            {
                glDeleteVertexArrays( 1, &d.vao ) ;
                gl4_log_error2( "glDeleteVertexArrays", name ) ;
            }

            geo_to_vaos.clear() ;
        }
    };
    motor_typedef( render_data ) ;
    using render_datas_t = motor::platform::datas< render_data > ;
    render_datas_t _renders ;

    //********************************************************************************
    struct image_data
    {
        GLenum type = GL_NONE ;

        GLuint tex_id = GLuint( -1 ) ;
        size_t sib = 0 ;

        GLenum wrap_types[ 3 ] ;
        GLenum filter_types[ 2 ] ;

        // sampler ids for gl>=3.3

        void_t invalidate( motor::string_in_t name ) noexcept
        {
            type = GL_NONE ;

            if( tex_id != GLuint( -1 ) )
            {
                glDeleteTextures( 1, &tex_id ) ;
                gl4_log_error2( "glDeleteTextures", name ) ;
                tex_id = GLuint( -1 ) ;
            }
        
            sib = 0 ;
        }
    };
    motor_typedef( image_data ) ;
    
    using image_datas_t = motor::platform::datas< image_data_t > ;
    image_datas_t _images ;

    //********************************************************************************
    struct array_data
    {
        GLuint tex_id = GLuint( -1 ) ;
        GLuint buf_id = GLuint( -1 ) ;

        GLuint sib = 0 ;

        void_t invalidate( motor::string_in_t name ) noexcept
        {
            if( buf_id != GLuint(-1) )
            {
                glDeleteBuffers( 1, &buf_id ) ;
                gl4_log_error2( "glDeleteBuffers", name ) ;
                buf_id = GLuint( -1 ) ;
            }

            if( tex_id != GLuint(-1) )
            {
                glDeleteTextures( 1, &tex_id ) ;
                gl4_log_error2( "glDeleteTextures", name ) ;
                tex_id = GLuint( -1 ) ;
            }

            sib = 0 ;
        }
    } ;
    motor_typedef( array_data ) ;

    //typedef motor::vector< this_t::array_data_t > array_datas_t ;
    using arrays_t = motor::platform::datas< array_data_t > ;
    arrays_t _arrays ;

    //********************************************************************************
    struct framebuffer_data
    {
        GLuint gl_id = GLuint( -1 ) ;

        size_t nt = 0 ;
        GLuint colors[ 8 ] = {
            GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ),
            GLuint( -1 ), GLuint( -1 ), GLuint( -1 ), GLuint( -1 ) } ;

        GLuint depth = GLuint( -1 ) ;

        motor::math::vec2ui_t dims ;

        // reset all gl ids. This is useful
        // if the context is already gone.
        void_t reset_gl_ids( void_t )
        {
            gl_id = GLuint(-1) ;
            std::memset( colors, GLuint(-1), sizeof(GLuint)*8 ) ;
            depth = GLuint(-1) ;
        }

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
            if( gl_id != GLuint( -1 ) )
            {
                glDeleteFramebuffers( 1, &gl_id ) ;
                gl4_log_error( "glDeleteFramebuffers" ) ;

                {
                    glDeleteTextures( 8, colors ) ;
                    gl4_log_error( "glDeleteTextures : colors" ) ;
                }
            }

            if( depth != GLuint(-1) )
            {
                glDeleteTextures( 1, &depth ) ;
                gl4_log_error( "glDeleteTextures : depth" ) ;
            }
            
            gl_id = GLuint( -1 ) ;
            depth = GLuint( -1 ) ;
            for( size_t i=0; i<8; ++i ) colors[i] = GLuint( -1 ) ;

            dims = motor::math::vec2ui_t() ;
        }
    };
    motor_typedef( framebuffer_data ) ;

    //typedef motor::vector< this_t::framebuffer_data_t > framebuffer_datas_t ;
    using framebuffer_datas_t = motor::platform::datas< framebuffer_data_t > ;
    framebuffer_datas_t _framebuffers ;

    //********************************************************************************
    struct msl_data
    {
        // purpose: keep track of the data within the msl object
        // if recompilation is triggered.
        motor::graphics::msl_object_t msl_obj ;

        motor::vector< motor::graphics::render_object_t > ros ; 
        motor::vector< motor::graphics::shader_object_t > sos ; 

        void_t invalidate( motor::string_in_t /*name*/ ) noexcept
        {
        }
    };
    motor_typedef( msl_data ) ;

    //typedef motor::vector< this_t::msl_data_t > msl_datas_t ;
    using msls_t = motor::platform::datas< msl_data_t > ;
    msls_t _msls ;

    // find render object by name
    static bool_t find_ro( msls_t & items, motor::string_in_t name, 
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
    static std::pair< size_t, motor::graphics::msl_object_t > find_pair_by_ro_name( motor::string_in_t name, msls_t & msls ) noexcept
    {
        auto ret = std::make_pair( size_t(-1), motor::graphics::msl_object_t() ) ;
        auto const res = this_t::find_ro( msls, name, [&]( size_t const id, pimpl::msl_data_ref_t d )
        {
            ret = std::make_pair( id, d.msl_obj ) ;
        } ) ;
        
        return ret ;
    }

    motor::msl::database_t _mdb ;

    GLsizei vp_width = 0 ;
    GLsizei vp_height = 0 ;

    motor::graphics::gen4::backend_type const bt = motor::graphics::gen4::backend_type::gl4 ;
    motor::graphics::shader_api_type const sapi = motor::graphics::shader_api_type::glsl_4_0 ;

    // the current render state set
    motor::graphics::render_state_sets_t render_states ;

    motor::platform::opengl::rendering_context_ptr_t _ctx ;
    motor::platform::opengl::rendering_context_mtr_t _shd_ctx ;

    size_t _tf_active_id = size_t( -1 ) ;

    size_t _bid = size_t(-1) ;

private: // support thread
    

    // for the support thread
    struct shared_data
    {
        bool_t running = false ;
        bool_t has_work = false ;
        this_ptr_t owner = nullptr ;
        motor::platform::opengl::rendering_context_ptr_t ctx ;

        std::mutex mtx ;
        std::condition_variable cv ;

        motor::vector< local_gl4::work_item > items ;

        void_t add_item( local_gl4::work_item_rref_t item ) noexcept
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
    std::thread _support_thread ;

    void_t start_support_thread( void_t ) noexcept 
    {
        assert( _ctsd == nullptr && "Just start/stop the thread." ) ;

        if ( _ctsd == nullptr )
        {
            _ctsd = motor::memory::global::alloc<shared_data>( 
                "support thread shared data" ) ;
            _ctsd->running = true ;
            _ctsd->owner = this ;
            _ctsd->ctx = _shd_ctx ;
        }

        auto * ctsd = _ctsd ;

        _support_thread = std::thread( [ctsd] ( void_t )
        {
            motor::log::global_t::status( "[gl4] : support thread started" ) ;

            motor::vector< local_gl4::work_item_t > items ;
            items.reserve( 50 ) ;

            // activate shared context
            {
                auto const res = ctsd->ctx->activate() ;
                if( res != motor::platform::result::ok ) 
                {
                    motor::log::global::error( "[gl4] : unable to activate shared context" ) ;
                    ctsd->running = false ;
                }
            }

            // just testing the shared context
            {
                glClearColor( 1.0f, 1.0f, 0.0f, 1.0f ) ;
                auto err = glGetError() ;
                assert( err == GL_NO_ERROR ) ;
                glClear( GL_COLOR_BUFFER_BIT ) ;
                err = glGetError() ;
                assert( err == GL_NO_ERROR ) ;
            }

            while ( ctsd->running )
            {
                // wait for some work and
                // swap the items buffer
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
                    case local_gl4::work_item::obj_type::msl: 
                    {
                        auto * msl = static_cast<motor::graphics::msl_object_ptr_t>(item.ptr) ;
                        switch( item.wt )
                        {
                        case local_gl4::work_item::work_type::configure: 
                            ctsd->owner->construct_msl_data_st( msl ) ;
                            break ;
                        case local_gl4::work_item::work_type::release: break ;
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
            ctsd->ctx->deactivate() ;

            motor_status( "[gl4] : support thread shut down" ) ;
        } ) ;
    }
    void_t stop_support_thread( void_t ) noexcept 
    {
        if ( _ctsd == nullptr ) return ;

        if ( _ctsd->running )
        {
            {
                std::unique_lock<std::mutex> lk( _ctsd->mtx ) ;
                _ctsd->running = false ;
            }
            _ctsd->cv.notify_one() ;
            if ( _support_thread.joinable() )
                _support_thread.join() ;
        }

        motor::memory::global::dealloc<shared_data>( motor::move( _ctsd ) ) ;
    }

public:

    //****************************************************************************************
    pimpl( size_t const bid, motor::platform::opengl::rendering_context_ptr_t ctx ) 
    {
        _ctx = ctx ;
        _shd_ctx = _ctx->create_shared() ;
        _bid = bid ;

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

            /*size_t const oid =*/ this_t::construct_state( obj ) ;
        }
        this_t::start_support_thread() ;
    }

    //****************************************************************************************
    pimpl( this_rref_t rhv ) noexcept :
        _shd_ctx( motor::move( rhv._shd_ctx ) )
    {
        rhv.stop_support_thread() ;

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

        this_t::start_support_thread() ;
    }

    //****************************************************************************************
    ~pimpl( void_t ) noexcept
    {
        this_t::stop_support_thread() ;
        motor::release( motor::move( _shd_ctx ) ) ;
        
        this_t::release_and_clear_all_geometry_data( false ) ; 
        this_t::release_and_clear_all_shader_data( false ) ;
        this_t::release_and_clear_all_render_data( false ) ;
        this_t::release_and_clear_all_image_data( false ) ;
        this_t::release_and_clear_all_framebuffer_data( false ) ;
        this_t::release_and_clear_all_array_data( false ) ;
        this_t::release_and_clear_all_feedback_data( false ) ;
        this_t::release_and_clear_all_msl_data( false ) ;
        this_t::release_and_clear_all_state_data( false ) ;
    }

    // silent clear. No gl release will be done.
    // the context probably is already gone.
    void_t on_context_destruction( void_t ) noexcept
    {
        this_t::stop_support_thread() ;
        motor::release( motor::move( _shd_ctx ) ) ;
        
        this_t::release_and_clear_all_shader_data( false ) ;
        this_t::release_and_clear_all_geometry_data( false ) ;
        this_t::release_and_clear_all_render_data( false ) ;
        this_t::release_and_clear_all_image_data( false ) ;
        this_t::release_and_clear_all_framebuffer_data( false ) ;
        this_t::release_and_clear_all_array_data( false ) ;
        this_t::release_and_clear_all_feedback_data( false ) ;
        this_t::release_and_clear_all_msl_data( false ) ;
        this_t::release_and_clear_all_state_data( false ) ;
    }

    //****************************************************************************************
    bool_t construct_state( motor::graphics::state_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid ) ;
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
        if( res ) obj.set_oid( _bid, oid ) ;
        return res ;
    }

    //******************************************************************************************************************************
    bool_t update_state( motor::graphics::state_object_ref_t obj ) noexcept
    {
        size_t const oid = obj.get_oid( _bid ) ;
        assert( oid < _states.size() ) ;
        if( !obj.check_and_reset_changed( _bid ) ) return false ;

        auto const res = _states.access( oid, [&]( this_t::state_data_ref_t s )
        {
            obj.for_each( [&]( size_t const i, motor::graphics::render_state_sets_cref_t rs )
            {
                if( i >= s.states.size() ) return ;
                s.states[i] = rs ;
            } ) ;
        } ) ;
        return res  ;
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
                    gl4_log_error( "glEnable : GL_DEPTH_TEST" ) ;

                    glDepthMask( new_states.depth_s.ss.do_depth_write ? GL_TRUE : GL_FALSE ) ;
                    gl4_log_error( "glDepthMask" ) ;

                    glDepthFunc( GL_LESS ) ;
                    gl4_log_error( "glDepthFunc" ) ;
                }
                else
                {
                    glDisable( GL_DEPTH_TEST ) ;
                    gl4_log_error( "glDisable" ) ;

                    glDepthMask( GL_FALSE ) ;
                    gl4_log_error( "glDepthMask" ) ;
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
                    motor::ogl::error::check_and_log( gl4_log( "glEnable" ) ) ;

                    GLenum const glsrc = motor::platform::gl3::convert( new_states.blend_s.ss.src_blend_factor ) ;
                    GLenum const gldst = motor::platform::gl3::convert( new_states.blend_s.ss.dst_blend_factor  );

                    glBlendFunc( glsrc, gldst ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glBlendFunc" ) ) ;
                }
                else
                {
                    glDisable( GL_BLEND ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glDisable" ) ) ;
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
                    motor::ogl::error::check_and_log( gl4_log( "glEnable" ) ) ;

                    glCullFace( motor::platform::gl3::convert( new_states.polygon_s.ss.cm ) ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glCullFace" ) ) ;

                    glFrontFace( motor::platform::gl3::convert(new_states.polygon_s.ss.ff ) ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glFrontFace" ) ) ;

                    glPolygonMode( GL_FRONT_AND_BACK, motor::platform::gl3::convert( new_states.polygon_s.ss.fm ) ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glPolygonMode" ) ) ;
                }
                else
                {
                    glDisable( GL_CULL_FACE ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glDisable" ) ) ;
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
                    motor::ogl::error::check_and_log( gl4_log( "glEnable" ) ) ;

                    glScissor(
                        GLint( new_states.scissor_s.ss.rect.x() ), GLint( new_states.scissor_s.ss.rect.y() ),
                        GLsizei( new_states.scissor_s.ss.rect.z() ), GLsizei( new_states.scissor_s.ss.rect.w() ) ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glScissor" ) ) ;
                }
                else
                {
                    glDisable( GL_SCISSOR_TEST ) ;
                    motor::ogl::error::check_and_log( gl4_log( "glDisable" ) ) ;
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
                    motor::ogl::error::check_and_log( gl4_log( "glViewport" ) ) ;
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
                motor::ogl::error::check_and_log( gl4_log( "glClearColor" ) ) ;

                GLbitfield const color_bit = clear_color ? GL_COLOR_BUFFER_BIT : 0 ;
                GLbitfield const depth_bit = clear_depth ? GL_DEPTH_BUFFER_BIT : 0 ;

                glClear( color_bit | depth_bit ) ;
                motor::ogl::error::check_and_log( gl4_log( "glEnable" ) ) ;
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
                motor_error( "no more render states to pop" ) ;
                return ;
            }
            auto const popped = _state_stack.pop() ;
            this_t::handle_render_state( popped - _state_stack.top(), true ) ;
        }
        else
        {
            auto new_id = std::make_pair( oid, rs_id ) ;

            _states.access( new_id.first, [&]( this_t::state_data_ref_t s )
            {
                _state_stack.push( _state_stack.top() + s.states[ new_id.second ] ) ;
            } ) ;
            this_t::handle_render_state( _state_stack.top(), false ) ;
        }
    }

    //****************************************************************************************
    size_t construct_framebuffer( motor::graphics::framebuffer_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid ) ;
        auto const res = _framebuffers.access( oid, obj.name(), [&]( motor::string_in_t fb_name, this_t::framebuffer_data_ref_t fb )
        {
            if( fb.gl_id == GLuint( -1 ) )
            {
                glGenFramebuffers( 1, &fb.gl_id ) ;
                if( gl4_log_error3( "glGenFramebuffers", obj ) )
                    return false ;
            }

            // bind
            {
                glBindFramebuffer( GL_FRAMEBUFFER, fb.gl_id ) ;
                if( gl4_log_error3( "glBindFramebuffer", obj ) ) 
                    return false ;
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
                    if( gl4_log_error3( "glGenTextures : color buffers", obj ) ) 
                        return false ;
                }

                if( fb.depth == GLuint(-1) )
                {
                    glGenTextures( GLsizei( 1 ), &fb.depth ) ;
                    if( gl4_log_error3( "glGenTextures : depth buffer", obj ) ) 
                        return false ;
                }
            }

            // construct color textures
            {
                for( size_t i=0; i<nt; ++i )
                {
                    GLuint const tid = fb.colors[i] ;

                    glBindTexture( GL_TEXTURE_2D, tid ) ;
                    if( gl4_log_error3( "glBindTexture : color buffer", obj ) ) 
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
                    gl4_log_error3( "glTexImage2D : color buffer", obj ) ;
                }

                // attach
                for( size_t i = 0; i < nt; ++i )
                {
                    GLuint const tid = fb.colors[ i ] ;
                    GLenum const att = GLenum( size_t( GL_COLOR_ATTACHMENT0 ) + i ) ;
                    glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 ) ;
                    gl4_log_error3( "glFramebufferTexture2D : color buffer", obj ) ;
                }
            }

            // depth/stencil
            if( dst != motor::graphics::depth_stencil_target_type::unknown )
            {
                {
                    GLuint const tid = fb.depth ;

                    glBindTexture( GL_TEXTURE_2D, tid ) ;
                    if( !gl4_log_error3( "glBindTexture : depth buffer", obj ) )
                    {
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
                        gl4_log_error3( "glTexImage2D : depth buffer", obj ) ;
                    }
                }

                // attach
                {
                    GLuint const tid = fb.depth ;
                    GLenum const att = motor::platform::gl3::to_gl_attachment(dst) ;
                    glFramebufferTexture2D( GL_FRAMEBUFFER, att, GL_TEXTURE_2D, tid, 0 ) ;
                    gl4_log_error3( "glFramebufferTexture2D : depth buffer", obj ) ;
                }
            }

            GLenum status = 0 ;
            // validate
            {
                status = glCheckFramebufferStatus( GL_FRAMEBUFFER ) ;
                gl4_log_error3( "glCheckFramebufferStatus", obj ) ;
                
                if( motor::log::global_t::warning_if<2048>( status != GL_FRAMEBUFFER_COMPLETE, 
                    "Incomplete framebuffer : [%s]", obj.name().c_str() ) ) return false ;
            }

            // unbind
            {
                glBindFramebuffer( GL_FRAMEBUFFER, 0 ) ;
                gl4_log_error3( "glBindFramebuffer", obj ) ;
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
                for( size_t i = 0; i < nt; ++i )
                {
                    size_t new_iid = size_t(-1) ;
                    motor::string_t name = fb_name + "." + motor::to_string( i ) ;
                    _images.access( new_iid, name, [&]( this_t::image_data_ref_t id )
                    {
                        id.tex_id = fb.colors[ i ] ;
                        id.type = GL_TEXTURE_2D ;
                        for( size_t j = 0; j < ( size_t ) motor::graphics::texture_wrap_mode::size; ++j )
                        {
                            id.wrap_types[ j ] = GL_CLAMP_TO_BORDER ;
                        }
                        for( size_t j = 0; j < ( size_t ) motor::graphics::texture_filter_mode::size; ++j )
                        {
                            id.filter_types[ j ] = GL_LINEAR ;
                        }
                        return true ;
                    } ) ;
                }
            }

            // store depth/stencil
            if( requires_store )
            {
                size_t new_iid = size_t(-1) ;
                motor::string_t name = fb_name + ".depth" ;
                _images.access( new_iid, name, [&]( this_t::image_data_ref_t id )
                {
                    id.tex_id = fb.depth ;
                    id.type = GL_TEXTURE_2D ; 

                    for( size_t j = 0; j < ( size_t ) motor::graphics::texture_wrap_mode::size; ++j )
                    {
                        id.wrap_types[ j ] = GL_REPEAT ;
                    }

                    for( size_t j = 0; j < ( size_t ) motor::graphics::texture_filter_mode::size; ++j )
                    {
                        id.filter_types[ j ] = GL_NEAREST ;
                    }

                    return true ;
                } ) ;
            }

            return true ;
        } ) ;

        if( res ) obj.set_oid( _bid, oid ) ;

        return res ;
    }

    //****************************************************************************************
    bool_t release_framebuffer( size_t const oid ) noexcept
    {
        // #1 clear the images buffers, i.e remove references
        _framebuffers.access( oid, [&]( this_t::framebuffer_data_ref_t fbd )
        {
            _images.for_each( [&]( this_t::image_data_ref_t id )
            {
                // try all colors
                for( size_t i=0; i<8; ++i )
                {
                    if( fbd.colors[i] == GLuint(-1) ) continue ;
                    if( id.tex_id != fbd.colors[i] ) continue ;

                    // remove id before invalidate
                    id.tex_id = GLuint(-1) ;
                    return ;
                }

                if( fbd.depth != GLuint(-1) )
                {
                    if( id.tex_id == fbd.depth ) 
                    {
                        // remove id before invalidate
                        id.tex_id = GLuint(-1) ;
                    }
                }
            } ) ;
        } ) ;

        // #2 now we can invalidate
        return _framebuffers.invalidate( oid ) ;
    }

    //****************************************************************************************
    bool_t activate_framebuffer( size_t const oid ) noexcept
    {
        auto const [a,b] = _framebuffers.access<bool_t>( oid, [&]( this_t::framebuffer_data_ref_t fb )
        {
            // bind
            {
                glBindFramebuffer( GL_FRAMEBUFFER, fb.gl_id ) ;
                if( gl4_log_error( "glBindFramebuffer" ) )
                {
                    return false ;
                }
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
                if( gl4_log_error( "glDrawBuffers" ) )
                {
                    return false ;
                }
            }
            return true ;
        } ) ;

        return a && b ;
    }

    //****************************************************************************************
    bool_t deactivate_framebuffer( void_t ) noexcept
    {
        // unbind
        {
            glBindFramebuffer( GL_FRAMEBUFFER, 0 ) ;
            if( gl4_log_error( "glGenFramebuffers" ) ) return false ;
        }

        return true ;
    }

    //****************************************************************************************
    bool_t construct_shader_data( motor::graphics::shader_object_ref_t obj ) noexcept
    {
        //
        // SECTION: Pre-Compilation
        //

        motor::graphics::shader_set_t ss ;
        {
            auto const res = obj.shader_set( this_t::sapi, ss ) ;
            if( !res )
            {
                motor::log::global::warning<2048>( gl4_log(
                    "config [%s] has no shaders for %s" ), obj.name().c_str(), 
                    motor::graphics::gen4::to_string( this_t::bt ) ) ;
                
                return false ;
            }
        }

        struct shader_compilation
        {
            GLuint pg_id = GLuint(-1) ;
            GLuint vs_id = GLuint(-1) ;
            GLuint gs_id = GLuint(-1) ;
            GLuint ps_id = GLuint(-1) ;
        } ;

        shader_compilation scomp ;

        // pre-compile all the shaders
        {
            motor_status2( 1024, "[gl4] : start shader compilation %s", obj.name().c_str() ) ;
            motor::profiling::scoped_timer time_this( "[gl4] : compilation done in" ) ;

            // Compile Vertex Shader
            {
                GLuint const id = glCreateShader( GL_VERTEX_SHADER ) ;
                gl4_log_error( "glCreateShader : vertex shader" ) ;

                if( !this_t::compile_shader( id, ss.vertex_shader().code() ) )
                {
                    glDeleteShader( id ) ;
                    gl4_log_error( "glDeleteShader : vertex shader" ) ;
                    return false ;
                }

                scomp.vs_id = id ;
            }

            // Compile Geometry Shader
            if( ss.has_geometry_shader() )
            {
                GLuint const id = glCreateShader( GL_GEOMETRY_SHADER ) ;
                gl4_log_error( "glCreateShader : geometry shader") ;

                if( !this_t::compile_shader( id, ss.geometry_shader().code() ) )
                {
                    glDeleteShader( scomp.vs_id ) ;
                    gl4_log_error( "glDeleteShader : vertex shader") ;

                    glDeleteShader( id ) ;
                    gl4_log_error( "glDeleteShader : geometry shader") ;

                    return false ;
                }

                scomp.gs_id = id ;
            }
            
            // Compile Pixel Shader
            if( ss.has_pixel_shader() )
            {
                GLuint const id = glCreateShader( GL_FRAGMENT_SHADER ) ;
                gl4_log_error( "glCreateShader : fragment shader") ;

                if( !this_t::compile_shader( id, ss.pixel_shader().code() ) )
                {
                    glDeleteShader( scomp.vs_id ) ;
                    gl4_log_error( "glDeleteShader : vertex shader") ;

                    if( ss.has_geometry_shader() )
                    {
                        glDeleteShader( scomp.gs_id ) ;
                        gl4_log_error( "glDeleteShader : geometry shader") ;
                    }
                    glDeleteShader( id ) ;
                    gl4_log_error( "glDeleteShader : fragment shader") ;

                    return false ;
                }
                scomp.ps_id = id ;
            }
        }

        //
        // SECTION : Shader/Program setup
        //

        {
            GLuint const id = glCreateProgram() ;
            gl4_log_error( "glCreateProgram : program" ) ;

            scomp.pg_id = id ;
        }

        // attach vertex
        {
            glAttachShader( scomp.pg_id, scomp.vs_id ) ;
            gl4_log_error( "glAttachShader : vertex shader" ) ;
        }

        // attach geometry shader
        if( scomp.gs_id != GLuint(-1) )
        {
            glAttachShader( scomp.pg_id, scomp.gs_id ) ;
            gl4_log_error( "glAttachShader : geometry shader" ) ;
        }

        // attach pixel shader
        if( scomp.ps_id != GLuint(-1) )
        {
            glAttachShader( scomp.pg_id, scomp.ps_id ) ;
            gl4_log_error( "glAttachShader : pixel shader" ) ;
        }

        //
        // SECTION : Shader/Program setup
        //

        size_t oid = obj.get_oid( _bid ) ;
        auto const shd_res = _shaders.access( oid, obj.name(), [&]( motor::string_in_t shd_name, this_t::shader_data_ref_t sd )
        {
            // handle old shader ids and stuff
            {
                sd.detach_shaders() ;
                sd.delete_shaders() ;
                sd.delete_all_variables() ;
            }

            sd.pg_id = scomp.pg_id ;
            sd.vs_id = scomp.vs_id ;
            sd.gs_id = scomp.gs_id ;
            sd.ps_id = scomp.ps_id ;

            // check max output vertices
            if( sd.gs_id != GLuint(-1) )
            {
                GLint max_out = 0 ;
                glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_VERTICES, &max_out ) ;
                gl4_log_error( "glGetIntegerv : GL_MAX_GEOMETRY_OUTPUT_VERTICES" ) ;
                ( void_t ) max_out ;
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

                gl4_log_error( "glTransformFeedbackVaryings" ) ;
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
                motor_status2( 1024, "[GL4] : Compilation Successful : [%s]", shd_name.c_str() ) ;
            }

            {
                this_t::post_link_attributes( sd ) ;
                this_t::post_link_uniforms( sd ) ;
                this_t::post_link_uniform_blocks( sd ) ;
            }
            return true ;
        } ) ;

        if( !shd_res ) return false ;
        obj.set_oid( _bid, oid ) ;

        // if the shader is redone, all render objects using
        // the shader need to be updated
        {
            _renders.for_each( [&]( this_t::render_data_ref_t rd )
            {
                if( rd.shd_id != oid ) return ;
                
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

                _shaders.access( rd.shd_id, [&]( this_t::shader_data_ref_t sd )
                {
                    this_t::render_object_variable_memory( rd, sd ) ;
                    return true ;
                } ) ;

                for( size_t vs_id=0; vs_id<rd.var_sets.size(); ++vs_id )
                {
                    this_t::update_variables( rd, vs_id ) ;
                }
            } ) ;
        }

        return true ;
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_msl_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _msls.for_each( [&]( this_t::msl_data_ref_t d )
            {
            } ) ;
        }

        _msls.invalidate_and_clear() ;
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_feedback_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _feedbacks.for_each( [&]( this_t::feedback_data_ref_t d )
            {
                for( size_t rw=0; rw<2; ++rw )
                {
                    auto & buffer = d._buffers[rw] ;
                    buffer.qid = GLuint(-1) ;
                    buffer.tfid =  GLuint(-1) ;
                    buffer.tids[0] = GLuint(-1) ;
                }
            } ) ;
        }

        _feedbacks.invalidate_and_clear() ;
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_array_data( bool_t const with_gl = false ) noexcept
    {
        // #1 first, reset the gl ids
        if( !with_gl )
        {
            _arrays.for_each( [&]( this_t::array_data_ref_t d )
            {
                d.buf_id = GLuint(-1) ;
                d.tex_id = GLuint(-1) ;
            } ) ;
        }

        // #2 now, the data can be invalidated
        _arrays.invalidate_and_clear() ;
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_framebuffer_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _framebuffers.for_each( [&]( this_t::framebuffer_data_ref_t d )
            {
                d.reset_gl_ids() ;
            } ) ;
        }

        _framebuffers.invalidate_and_clear() ;
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_image_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _images.for_each( [&]( this_t::image_data_ref_t d )
            {
                d.tex_id = GLuint( -1 ) ;
            } ) ;
        }

        _images.invalidate_and_clear() ;
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_render_data( bool_t const with_gl = false ) noexcept
    {
        if( !with_gl )
        {
            _renders.for_each( [&]( this_t::render_data_ref_t rd )
            {
                rd.geo_to_vaos.clear() ;
            } ) ;
        }

        _renders.invalidate_and_clear() ;
    }

    //****************************************************************************************
    // if with_gl, no gl calls will be made.
    void_t release_and_clear_all_shader_data( bool_t const with_gl = false ) noexcept
    {
        // if with_gl, just reset all gl ids.
        // => no gl calls will be made
        if( !with_gl )
        {
            _shaders.for_each( [&] ( this_t::shader_data_ref_t shd )
            {
                shd.pg_id = GLuint(-1) ;
                shd.vs_id = GLuint(-1) ;
                shd.gs_id = GLuint(-1) ;
                shd.ps_id = GLuint(-1) ;
            } ) ;
        }

        _shaders.invalidate_and_clear() ;
    }

    //****************************************************************************************
    void_t release_and_clear_all_geometry_data( bool_t const with_gl = false ) noexcept
    {
        // if with_gl, just reset all gl ids.
        // => no gl calls will be made
        if( !with_gl )
        {
            _geometries.for_each( [&] ( this_t::geo_data_ref_t gd )
            {
                gd.ib_id = GLuint(-1) ;
                gd.vb_id = GLuint(-1) ;
            } ) ;
        }

        _renders.for_each( [&]( this_t::render_data_ref_t rd ) 
        {
            rd.geo_ids.clear() ;
        } ) ;
        
        _geometries.invalidate_and_clear() ;
    }

    //****************************************************************************************
    void_t release_and_clear_all_state_data( bool_t const with_gl = false ) noexcept
    {
        _states.invalidate_and_clear() ;
    }

    //****************************************************************************************
    bool_t release_shader_data( size_t const oid ) noexcept
    {
        return _shaders.invalidate( oid ) ;
    }

    //****************************************************************************************
    void_t detach_shaders( GLuint const program_id )
    {
        GLsizei count = 0 ;
        GLuint shaders_[ 10 ] ;

        glGetAttachedShaders( program_id, 10, &count, shaders_ ) ;
        gl4_log_error( "glGetAttachedShaders" ) ;

        for( GLsizei i = 0; i < count; ++i )
        {
            glDetachShader( program_id, shaders_[ i ] ) ;
            gl4_log_error( "glDetachShader" ) ;
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
        if( gl4_log_error( "glShaderSource" ) ) return false ;
        
        glCompileShader( id ) ;
        if( gl4_log_error( "glCompileShader" ) ) return false ;

        GLint ret ;
        glGetShaderiv( id, GL_COMPILE_STATUS, &ret ) ;

        GLint length ;
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length ) ;

        if( ret == GL_TRUE && length <= 1 ) return true ;

        if( length == 0 )
        {
            gl4_log_error( "shader compilation failed, but info log length is 0." ) ;
            return false ;
        }

        // print first line for info
        // user can place the shader name or any info there.
        {
            size_t pos = code.find_first_of( '\n' ) ;
            motor::log::global::error( gl4_log( "First Line: " + code.substr( 0, pos ) ) ) ;
        }

        // get the error message it is and print it
        {
            motor::memory::malloc_guard<char> info_log( length ) ;

            glGetShaderInfoLog( id, length, 0, info_log ) ;
            gl4_log_error( "glGetShaderInfoLog" ) ;

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
        auto const [a,b] = _renders.access<bool_t>( rid, [&]( this_t::render_data_ref_t rd )
        {
            auto const [a, b] = _shaders.access<bool_t>( rd.shd_id, [&]( this_t::shader_data_ref_t sd )
            {
                return sd.is_linkage_ok ;
            } ) ;

            return a && b ;
        } ) ;
        return a && b ;
    }

    //****************************************************************************************
    bool_t link( GLuint const program_id )
    {
        glLinkProgram( program_id ) ;
        if( motor::ogl::error::check_and_log( gl4_log( "glLinkProgram" ) ) )
            return false ;

        {
            GLint ret ;
            glGetProgramiv( program_id, GL_LINK_STATUS, &ret ) ;

            GLint length ;
            glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &length ) ;

            if( ret == GL_TRUE && length <= 1 ) 
                return true ;

            if( motor::log::global_t::error( length == 0, gl4_log("unknown") ) )
                return false ;

            motor::memory::malloc_guard<char> info_log( length ) ;

            glGetProgramInfoLog( program_id, length, 0, info_log ) ;
            if( motor::ogl::error::check_and_log( gl4_log( "glGetProgramInfoLog" ) ) )
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
    void_t post_link_attributes( this_t::shader_data & config ) const noexcept
    {
        GLuint const program_id = config.pg_id ;

        GLint num_active_attributes = 0 ;
        GLint name_length = 0 ;

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTES, &num_active_attributes ) ;
        motor::ogl::error::check_and_log( gl4_log( "glGetProgramiv(GL_ACTIVE_ATTRIBUTES)" ) ) ;

        if( num_active_attributes == 0 ) return ;

        config.attributes.resize( num_active_attributes ) ;

        glGetProgramiv( program_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &name_length ) ;
        motor::ogl::error::check_and_log( gl4_log( 
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
                    gl4_log("can not find vertex attribute - " + vd.name ) ) ;
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
    bool_t bind_attributes( motor::string_in_t shd_name, this_t::shader_data & sconfig, this_t::geo_data & gconfig ) const noexcept
    {
        // bind vertex buffer
        {
            glBindBuffer( GL_ARRAY_BUFFER, gconfig.vb_id ) ;
            if( motor::ogl::error::check_and_log( 
                gl4_log("glBindBuffer(GL_ARRAY_BUFFER)") ) ) 
                return false ;
        }

        // bind index buffer
        {
            GLint const id_ = gconfig.ib_elem_sib == 0 ? 0 : gconfig.ib_id ;
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_ ) ;
            if( motor::ogl::error::check_and_log( 
                gl4_log( "glBindBuffer(GL_ELEMENT_ARRAY_BUFFER)" ) ) )
                return false ;
        }

        // disable old locations
        for( size_t i = 0; i < sconfig.attributes.size(); ++i )
        {
            glDisableVertexAttribArray( sconfig.attributes[ i ].loc ) ;
            motor::ogl::error::check_and_log( gl4_log( "glDisableVertexAttribArray" ) ) ;
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

                motor::log::global_t::warning( gl4_log( "Vertex attribute (" +
                    motor::graphics::to_string(e.va) + ") in shader (" + shd_name + ") not used."
                    "Will bind geometry (" + shd_name + ") layout attribute to custom location (" 
                    + motor::to_string( uint_t(loc) ) + ").") ) ;
            }
            else
            {
                loc = iter->loc ;
                type = iter->type ;
            }

            glEnableVertexAttribArray( loc ) ;
            motor::ogl::error::check_and_log(
                gl4_log( "glEnableVertexAttribArray" ) ) ;

            glVertexAttribPointer(
                loc,
                GLint( motor::graphics::size_of(e.type_struct) ),
                motor::ogl::complex_to_simple_type( type ),
                GL_FALSE,
                ( GLsizei ) uiStride,
                (const GLvoid*)(size_t)uiBegin 
                ) ;

            motor::ogl::error::check_and_log( gl4_log( "glVertexAttribPointer" ) ) ;
        }
      
        {
            glBindVertexArray( 0 ) ;
            if( motor::ogl::error::check_and_log(
                gl4_log( "glBindVertexArray" ) ) )
                return false ;
        }
        return true ;
    }

    //****************************************************************************************
    void_t post_link_uniforms( this_t::shader_data & config ) const noexcept
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
                gl4_log( "invalid uniform location id." ) ) ) continue ;

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
    void_t post_link_uniform_blocks( this_t::shader_data & config ) const noexcept
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
    bool_t construct_image_config( motor::graphics::image_object_ref_t config ) noexcept
    {
        size_t oid = config.get_oid( _bid ) ;
        auto const res = _images.access( oid, config.name(), [&]( this_t::image_data_ref_t img )
        {
            // sampler
            if( img.tex_id == GLuint( -1 ) )
            {
                GLuint id = GLuint( -1 ) ;
                glGenTextures( 1, &id ) ;
                if( gl4_log_error( "glGenTextures" ) ) return false ;

                img.tex_id = id ;
            }

            img.type = motor::platform::gl3::convert( config.get_type() ) ;
            for( size_t j=0; j<(size_t)motor::graphics::texture_wrap_mode::size; ++j )
            {
                img.wrap_types[ j ] = motor::platform::gl3::convert(
                    config.get_wrap( ( motor::graphics::texture_wrap_mode )j ) );
            }

            for( size_t j = 0; j < ( size_t ) motor::graphics::texture_filter_mode::size; ++j )
            {
                img.filter_types[ j ] = motor::platform::gl3::convert(
                    config.get_filter( ( motor::graphics::texture_filter_mode )j ) );
            }
            return true ;
        } ) ;

        if( res ) config.set_oid( _bid, oid ) ;
        return res ;
    }

    //****************************************************************************************
    bool_t release_image_data( size_t const oid ) noexcept
    {
        return _images.invalidate( oid ) ;
    }

    //****************************************************************************************
    void_t send_configure_to_st( motor::graphics::msl_object_mtr_t obj ) noexcept
    {
        obj->set_oid( _bid, size_t(-2) ) ;
        _ctsd->add_item( local_gl4::work_item::make_item( 
            local_gl4::work_item::work_type::configure, motor::share( obj ) ) ) ;
    }

    //****************************************************************************************
    // call from support thread
    bool_t construct_msl_data_st( motor::graphics::msl_object_ptr_t obj_in ) noexcept
    {
        auto obj = *obj_in;

        motor::vector< motor::msl::symbol_t > config_symbols ;

        obj.for_each_msl( motor::graphics::msl_api_type::msl_4_0,
            [&] ( motor::string_in_t code )
        {
            motor::msl::post_parse::document_t doc =
                motor::msl::parser_t( "gl4" ).process( code ) ;

            _mdb.insert( std::move( doc ), config_symbols ) ;
        } ) ;

        // cases:
        // obj.oid == -1 && obj.name.empty() : library/config shader
        // obj.oid == -1 && !obj.name.empty() : initial configuration

        // obj.oid != -1 && obj.name.empty() : invalid
        // obj.oid != -1 && !obj.name.empty() : was already configured

        // if the incoming msl shader is a library shader for example,
        // it does not need to have a associated background object
        size_t oid = obj.get_oid( _bid ) ;
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
                _msls.access( oid, [&]( this_t::msl_data_ref_t d )
                {
                   
                } ) ;
            }
            
            // msl database contains render configuration 
            // which has not been configured by the user...
            if ( oid == size_t( -1 ) )
            {
                motor_warning2( 1024,
                    "[gl4::construct_msl_data] : "
                    "render configuration not found : %s", c_exp.c_str() ) ;
                continue ;
            }

            motor::msl::generatable_t res = motor::msl::dependency_resolver_t().resolve( &_mdb, c ) ;
            if ( res.missing.size() != 0 )
            {
                motor::log::global_t::warning( "[gl4] : We have missing symbols for " + c_exp + " :" ) ;
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
                auto const code = gen.generate<motor::msl::glsl::glsl4_generator_t>() ;
                motor::graphics::msl_bridge::create_by_api_type( 
                    motor::graphics::shader_api_type::glsl_4_0, code, so ) ;

                {
                    size_t const milli = std::chrono::duration_cast<std::chrono::milliseconds>
                        ( std::chrono::high_resolution_clock::now() - tp_begin ).count() ;

                    char buffer[ 2048 ] ;
                    std::snprintf( buffer, 2048, "[gl4] : generating glsl shader took %zu ms", milli ) ;
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

                        
            {
                //so.set_oid( bid, this_t::construct_shader_config( so.get_oid( bid ), so ) ) ;
                //if ( !ctsd->owner->construct_shader_config( so ) )
                if( !this_t::construct_shader_data( so ) )
                {
                    // construction/compilation failed
                    // @todo return here.
                    continue ;
                }
                //if( !ctsd->owner->construct_render_config( ro ) )
                if( !this_t::construct_render_data( ro ) )
                {
                }
            }

            auto const access_res = _msls.access( oid, obj.name(), [&] ( this_t::msl_data_ref_t msl )
            {
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
                            auto const s = shd.is_linkage_ok ?
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

                msl.msl_obj = std::move( obj ) ;

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

    //****************************************************************************************
    // This is obsolete I think
    bool_t construct_msl_data( motor::graphics::msl_object_ref_t obj_ ) noexcept
    {
        return this_t::construct_msl_data_st( &obj_ ) ;
    }

    //****************************************************************************************
    bool_t construct_render_data( motor::graphics::render_object_ref_t obj ) noexcept
    {
        if( this_t::release_render_data( obj.get_oid(_bid) ) )
        {
            obj.set_oid( _bid, size_t(-1) ) ;
        }

        if( !this_t::construct_render_data_ext( obj ) )
        {
            motor::log::global_t::error("[gl4] : construct_render_data_ext") ;
        }

        return true ;
    }

    //****************************************************************************************
    bool_t release_render_data( size_t const oid ) noexcept
    {
        // #1 break connections BEFORE invalidating the item
        _renders.access( oid, [&]( this_t::render_data_ref_t rd )
        {
            for( auto id : rd.geo_ids )
            {
                _geometries.access( id, [&]( this_t::geo_data_ref_t d )
                {
                    d.remove_render_data_id( oid ) ;
                } ) ;
            }

            for( auto const id : rd.tf_ids )
            {
                _feedbacks.access( id, [&]( this_t::feedback_data_ref_t d )
                {
                    d.remove_render_data_id( oid ) ;
                } ) ;
            }
        } ) ;

        _renders.invalidate( oid ) ;

        return true ;
    }

    //****************************************************************************************
    bool_t construct_render_data_ext( motor::graphics::render_object_ref_t rc ) noexcept
    {
        size_t oid = rc.get_oid( _bid ) ;
        auto const res = _renders.access( oid, rc.name(), [&]( size_t const new_id, this_t::render_data_ref_t config )
        {
            // handle geometry links
            {
                config.geo_ids.reserve( rc.get_num_geometry() ) ;
                for( size_t i=0; i<rc.get_num_geometry(); ++i )
                {
                    auto const gid = _geometries.find_by_name( rc.get_geometry(i) ) ;
                    if( gid == size_t(-1) )
                    {
                        motor::log::global_t::warning<1024>( "[gl4] : no geometry with name [%s] for render_data [%s]",
                            rc.get_geometry().c_str(), rc.name().c_str() ) ;
                        continue ;
                    }
                    config.geo_ids.emplace_back( gid ) ;
                    _geometries.access( gid, [&]( this_t::geo_data_ref_t d ){ d.add_render_data_id( new_id ) ; } ) ;
                }
            }

            // handle stream out links
            {
                config.tf_ids.reserve( rc.get_num_streamout() ) ;

                // find stream out object
                for( size_t i=0; i<rc.get_num_streamout(); ++i )
                {
                    size_t const idx = _feedbacks.find_by_name( rc.get_streamout(i) ) ;
                    if( idx == size_t(-1) )
                    {
                        motor::log::global_t::warning<2048>( "[gl4] : no streamout object with name [%s] for render_data [%s]",
                            rc.get_streamout(i).c_str(), rc.name().c_str() ) ;
                        return false ;
                    }
                    config.tf_ids.emplace_back( idx ) ;
                }
            }

            // find shader
            {
                config.shd_id = _shaders.find_by_name( rc.get_shader() ) ;
                if( config.shd_id == size_t(-1) )
                {
                    motor::log::global::warning<1024>( "[gl4] : no shader with name [%s] for render_data [%s]", 
                        rc.get_shader().c_str(), rc.name().c_str() ) ;
                    return false ;
                }
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
                    motor::log::global_t::warning( !res, gl4_log( "connect" ) ) ;
                } ) ;

                for( auto * ptr : vars ) motor::memory::release_ptr( ptr ) ;
            }
        
            _shaders.access( config.shd_id, [&]( this_t::shader_data_ref_t shd )
            {
                this_t::render_object_variable_memory( config, shd ) ;
            } ) ;

            return true ;
        } ) ;

        if( res ) rc.set_oid( _bid, oid ) ;
        return res ;
    }

    bool_t bind_attributes( GLuint const vao, size_t const sid, size_t const gid ) noexcept
    {
        auto const [a,b] = _shaders.access<bool_t>( sid, [&]( this_t::shader_data_ref_t sd )
        {
            glBindVertexArray( vao ) ;
            if( gl4_log_error( "glBindVertexArray") )
            {
                return false;
            }

            //auto & gd = _geometries[ gid ] ;
            auto const [c,d] = _geometries.access<bool_t>( gid, [&]( motor::string_in_t name, this_t::geo_data_ref_t gd )
            {
                return this_t::bind_attributes( name, sd, gd ) ;
            } ) ;
            
            return c && d ;
            
        } ) ;
        return a && b ;
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
    bool_t construct_geo( size_t & oid, motor::string_in_t name, motor::graphics::vertex_buffer_in_t vb ) noexcept
    {
        motor_status2( 1024, "[gl4] : construct geometry %s", name.c_str() ) ;
        auto const res = _geometries.access( oid, name, [&]( this_t::geo_data_ref_t config )
        {
            // vertex buffer
            if( config.vb_id == GLuint(-1) )
            {
                GLuint id = GLuint( -1 ) ;
                glGenBuffers( 1, &id ) ;
                if( gl4_log_error( "glGenBuffers : vertex buffer") )
                    return false ;

                config.vb_id = id ;
            }

            // index buffer
            if( config.ib_id == GLuint(-1) )
            {
                GLuint id = GLuint( -1 ) ;
                glGenBuffers( 1, &id ) ;
                if( gl4_log_error( "glGenBuffers : index buffer") )
                    return false ;

                config.ib_id = id ;
            }

            {
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

            return true ;
        } ) ;

        return res ;
    }

    
    //****************************************************************************************
    bool_t construct_geo( motor::graphics::geometry_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid ) ;
        auto const res = this_t::construct_geo( oid, obj.name(), obj.vertex_buffer() ) ;
        if( res ) obj.set_oid( _bid, oid ) ;
        return res ;
    }

    //****************************************************************************************
    bool_t release_geometry( size_t const oid ) noexcept
    {
        // #1 have to deal with external references first
        _geometries.access( oid, [&]( this_t::geo_data_ref_t g )
        {
            for( auto const id : g.rd_ids )
            {
                if( id == size_t( -1 ) ) continue ;

                _renders.access( id, [&]( this_t::render_data_ref_t d )
                {
                    d.remove_geometry_id( oid ) ;
                } ) ;
            }
            g.rd_ids.clear() ;
        } ) ;

        // #2 now we can invalidate the data object itself
        return _geometries.invalidate( oid ) ;
    }

    //****************************************************************************************
    bool_t update( motor::graphics::geometry_object_mtr_t geo, bool_t const is_config = false )
    {
        size_t const id = geo->get_oid( _bid ) ;
        auto const [a,b] = _geometries.access<bool_t>( id, [&]( this_t::geo_data_ref_t config )
        {
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
                if( gl4_log_error( "glBindBuffer : vertex buffer" ) )
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
                    if( gl4_log_error( "glBufferData : vertex buffer" ) ) return false ;
                    config.sib_vb = sib ;
                }
                else
                {
                    glBufferSubData( GL_ARRAY_BUFFER, 0, sib, geo->vertex_buffer().data() ) ;
                    if( gl4_log_error( "glBufferSubData : vertex buffer" ) ) return false ;
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
                    if( gl4_log_error( "glBindBuffer : index buffer" ) )
                        return false ;
                }

                config.ib_elem_sib = geo->index_buffer().get_element_sib() ;

                GLuint const sib = GLuint( geo->index_buffer().get_sib() ) ;
                if( is_config || sib > config.sib_ib )
                {
                    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sib,
                        geo->index_buffer().data(), GL_STATIC_DRAW ) ;
                    if( gl4_log_error( "glBufferData : index buffer" ) )
                        return false ;
                    config.sib_ib = sib ;
                }
                else
                {
                    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, sib, geo->index_buffer().data() ) ;
                    if( gl4_log_error( "glBufferSubData : index buffer" ) ) return false ;
                }
            }

            if( is_config ) 
            {
                // force rebind of vertex attributes. Layout may have changed.
                for( auto const & rid : config.rd_ids )
                {
                    _renders.access( rid, [&]( this_t::render_data_ref_t rd )
                    {
                        if( rd.shd_id == size_t( -1 ) ) return false ;
                        
                        // this may happen if the geometry is configured a second time
                        // after the render object is configured but the render object
                        // is not rendered yet.
                        size_t i = size_t(-1) ;
                        while( ++i < rd.geo_to_vaos.size() && rd.geo_to_vaos[i].gid != id ) ;
                        if( i != rd.geo_to_vaos.size() )
                        {
                            glDeleteVertexArrays( 1, &(rd.geo_to_vaos[i].vao) ) ;
                            gl4_log_error( "glDeleteVertexArrays" ) ;

                            rd.geo_to_vaos[i].gid = GLuint(-1) ;
                            rd.geo_to_vaos[i].vao = GLuint(-1) ;
                        }
                        return true ;
                    } ) ;
                }
            }
            return true ;
        } ) ;

        return a && b ;
    }

    //****************************************************************************************
    // @param is_config used to determine recreating the texture on the gpu side.
    //          This param should be true if an image is configured or re-configured.
    bool_t update( motor::graphics::image_object_ref_t confin, bool_t const is_config = false )
    {
        //this_t::image_data_ref_t config = _images[ id ] ;
        size_t const id = confin.get_oid( _bid ) ;

        auto const [a,b] = _images.access<bool_t>( id, [&]( this_t::image_data_ref_t config )
        {
            glBindTexture( config.type, config.tex_id ) ;
            if( gl4_log_error( "glBindTexture" ) ) return false ;

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
                gl4_log_error( "glPixelStorei" ) ;
            }

            if( is_config || ( sib == 0 || config.sib < sib ) )
            {
                GLint const border = 0 ;
                GLint const internal_format = motor::platform::gl3::convert_to_gl_format( 
                    confin.image().get_image_format(), confin.image().get_image_element_type() ) ;

                if( target == GL_TEXTURE_2D )
                {
                    glTexImage2D( target, level, internal_format, width, height,
                        border, format, type, data ) ;
                    gl4_log_error( "glTexImage2D" ) ;
                }
                else if( target == GL_TEXTURE_2D_ARRAY )
                {
                    glTexImage3D( target, level, internal_format, width, height, depth,
                        border, format, type, data ) ;
                    gl4_log_error( "glTexImage3D" ) ;
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
                    gl4_log_error( "glTexSubImage2D" ) ;
                }
                else if( target == GL_TEXTURE_2D_ARRAY )
                {
                    glTexSubImage3D( target, level, xoffset, yoffset, zoffset, width, height, depth,
                        format, type, data ) ;
                    gl4_log_error( "glTexSubImage3D" ) ;
                }
            }

            config.sib = confin.image().sib() ;
            return true ;
        } ) ;
        

        return a && b ;
    }

    //****************************************************************************************
    bool_t connect( this_t::render_data & config, size_t const var_set_idx, motor::graphics::variable_set_mtr_t vs )
    {
        //this_t::shader_data_ref_t shd = _shaders[ config.shd_id ] ;
        _shaders.access( config.shd_id, [&]( this_t::shader_data_ref_t shd )
        {
            size_t id = 0 ;
            for( auto & uv : shd.uniforms )
            {
                // is it a data uniform variable?
                if( motor::ogl::uniform_is_data( uv.type ) )
                {
                    auto const types = motor::platform::gl3::to_type_type_struct( uv.type ) ;
                    auto* var = vs->data_variable( uv.name, types.first, types.second ) ;
                    if( var == nullptr )
                    {
                        motor::log::global_t::error( gl4_log( "can not claim variable " + uv.name ) ) ;
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
                        motor::log::global_t::error( gl4_log( "can not claim variable " + uv.name ) ) ;
                        continue ;
                    }

                    // looking for image
                    {
                        auto const& tx_name = var->get() ;

                        this_t::render_data::uniform_texture_link link ;
                        link.var_set_idx = var_set_idx ;
                        link.uniform_id = id++ ;
                        link.img_hash = var->get().hash() ;
                        link.var = var ;

                        auto const res = _images.access_by_name( tx_name.name(), [&]( size_t const img_id, this_t::image_data_ref_t d )
                        {
                            link.img_id = img_id ;
                            link.tex_id = d.tex_id ;
                        } ) ;

                        if( !res ) 
                        {
                            motor::log::global_t::error<1024>( "[gl4] : Could not find image [%s]", 
                                tx_name.name().c_str() ) ;
                            continue ;
                        }
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
                        this_t::render_data::uniform_array_data_link link ;
                        
                        auto const res = _arrays.access_by_name( tx_name, [&]( size_t const id_, this_t::array_data_ref_t d )
                        {
                            link.buf_id = id_ ;
                            link.tex_id = d.tex_id ;
                        } ) ;
                        if( !res ) return false ;

                        link.var_set_idx = var_set_idx ;
                        link.uniform_id = id++ ;

                        config.var_sets_array.emplace_back( link ) ;
                        return true ;
                    } ;

                    // looking for streamout/transform feedback
                    auto handle_feedback_link = [&]( void_t )
                    {
                        this_t::render_data::uniform_streamout_link link ;
                        
                        auto const res = _feedbacks.access_by_name( tx_name, [&]( size_t const id_, this_t::feedback_data_ref_t d )
                        {
                            link.tex_id[0] = d._buffers[0].tids[0] ;
                            link.tex_id[1] = d._buffers[1].tids[0] ;
                            link.so_id = id_ ;
                        } ) ;
                        if( !res ) return false ; 

                        link.var_set_idx = var_set_idx ;
                        link.uniform_id = id++ ;
                        config.var_sets_streamout.emplace_back( link ) ;
                        return true ;
                    } ;

                    if( !handle_buffer_link() )
                    {
                        if( !handle_feedback_link() )
                        {
                            motor::log::global_t::error<2048>(  
                              "[gl4] : Could not find array nor streamout object [%s]", 
                                tx_name.c_str() ) ;
                            continue ;
                        }
                    }
                }
            }

            // ref count one copy here for all stored items
            config.var_sets.emplace_back( vs ) ;
        } ) ;
        

        return true ;
    }

    //****************************************************************************************
    bool_t construct_array_data( motor::graphics::array_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid ) ;
        auto const res = _arrays.access( oid, obj.name(), [&]( this_t::array_data_ref_t data )
        {
            // buffer
            if( data.buf_id == GLuint(-1) )
            {
                GLuint id = GLuint( -1 ) ;
                glGenBuffers( 1, &id ) ;
                gl4_log_error3( "glGenBuffers", obj ) ;
                data.buf_id = id ;
            }

            // texture
            if( data.tex_id == GLuint(-1) )
            {
                GLuint id = GLuint( -1 ) ;
                glGenTextures( 1, &id ) ;
                gl4_log_error3( "glGenTextures", obj ) ;

                data.tex_id = id ;
            }
            return true ;
        } ) ;
        if( res ) obj.set_oid( _bid, oid ) ;
        return res ;
    }

    //****************************************************************************************
    bool_t release_array_data( size_t const oid ) noexcept
    {
        return _arrays.invalidate( oid ) ;
    }

    //****************************************************************************************
    bool_t update( motor::graphics::array_object_ref_t obj, bool_t const is_config = false ) 
    {
        auto const [a,b] = _arrays.access<bool_t>( obj.get_oid( _bid ), [&]( this_t::array_data_ref_t data )
        {
            // do buffer
            {
                // bind buffer
                glBindBuffer( GL_TEXTURE_BUFFER, data.buf_id ) ;
                if( gl4_log_error3( "glBindBuffer", obj ) ) 
                    return false ;

                // transfer data
                GLuint const sib = GLuint( obj.data_buffer().get_sib() ) ;
                if( is_config || sib > data.sib )
                {
                    glBufferData( GL_TEXTURE_BUFFER, sib,
                            obj.data_buffer().data(), GL_DYNAMIC_DRAW ) ;

                    if( gl4_log_error3( "glBufferData", obj ) ) 
                        return false ;

                    data.sib = sib ;
                }
                else
                {
                    glBufferSubData( GL_TEXTURE_BUFFER, 0, sib, obj.data_buffer().data() ) ;
                    if( gl4_log_error3( "glBufferSubData", obj ) ) 
                        return false ;
                }

                // bind buffer
                {
                    glBindBuffer( GL_TEXTURE_BUFFER, 0 ) ;
                    if( gl4_log_error3( "glBindBuffer", obj ) ) 
                        return false ;
                }
            }

            // do texture
            // glTexBuffer is required to be called after driver memory is aquired.
            {
                glBindTexture( GL_TEXTURE_BUFFER, data.tex_id ) ;
                if( gl4_log_error3( "glBindTexture", obj ) ) return false ;

                auto const le = obj.data_buffer().get_layout_element(0) ;
                glTexBuffer( GL_TEXTURE_BUFFER, motor::platform::gl3::convert_for_texture_buffer(
                    le.type, le.type_struct ), data.buf_id ) ;
                if( gl4_log_error3( "glTexBuffer", obj ) ) return false ;
            }
            return true ;
        } ) ;
        return a && b ;
    }

    //****************************************************************************************
    bool_t construct_feedback( motor::graphics::streamout_object_ref_t obj ) noexcept
    {
        size_t oid = obj.get_oid( _bid ) ;
        auto const res = _feedbacks.access( oid, obj.name(), [&]( this_t::feedback_data_ref_t data )
        {
            size_t const req_buffers = std::min( obj.num_buffers(), this_t::feedback_data::buffer::max_buffers ) ;

            // rw : read/write - ping pong buffers
            for( size_t rw=0; rw<2; ++rw )
            {
                auto & buffer = data._buffers[rw] ;

                for( size_t i=0; i<req_buffers; ++i )
                {
                    if( buffer.bids[i] != GLuint(-1) ) continue ;
                
                    motor::string_t const is = motor::to_string( i ) ;
                    motor::string_t const bs = motor::to_string( rw ) ;

                    size_t gid = size_t(-1) ;
                    auto const res = this_t::construct_geo( gid, 
                        obj.name() + ".feedback."+is+"."+bs, obj.get_buffer(i) ) ;
                
                    if( !res ) 
                    {
                        motor_warning2( 2048, "[gl4] : unable to create geometry for transform feedback [%s]", 
                            obj.name().c_str() ) ;
                        continue ;
                    }

                    buffer.gids[i] = gid ;
                    buffer.sibs[i] = 0 ; 

                    _geometries.access( gid, [&]( this_t::geo_data_ref_t d )
                    { 
                        buffer.bids[i] =  d.vb_id ;
                    } ) ;
                }

                // we just always gen max buffers tex ids
                {
                    glGenTextures( this_t::feedback_data::buffer::max_buffers, buffer.tids ) ;
                    if( gl4_log_error3( "glGenTextures", obj ) ) return false ;
                }

                for( size_t i=req_buffers; i<this_t::feedback_data::buffer::max_buffers; ++i )
                {
                    if( buffer.gids[i] == size_t(-1) ) break ;

                    this_t::release_geometry( buffer.gids[i] ) ;

                    buffer.bids[i] = GLuint( -1 ) ;
                    buffer.gids[i] = size_t( -1 ) ;
                }

                if( buffer.qid == GLuint(-1) )
                {
                    glGenQueries( 1, &buffer.qid ) ;
                    if( gl4_log_error3( "glGenQueries", obj ) ) return false ;
                }

                if( buffer.tfid == GLuint(-1) )
                {
                    glGenTransformFeedbacks( 1, &buffer.tfid ) ;
                    if( gl4_log_error3( "glGenTransformFeedbacks", obj ) ) return false ;
                }
            }
            return true ;
        } ) ;
        if( res ) obj.set_oid( _bid, oid ) ;
        return res ;
    }

    //****************************************************************************************
    bool_t release_tf_data( size_t const oid ) noexcept
    {
        _feedbacks.access( oid, [&]( this_t::feedback_data_ref_t d )
        {
            for( size_t rw=0; rw<2; ++rw )
            {
                auto & buffer = d._buffers[rw] ;

                for( GLuint i=0; i<feedback_data_t::buffer::max_buffers; ++i )
                {
                    if( buffer.gids[i] == size_t( -1 ) ) break ;

                    this_t::release_geometry( buffer.gids[i] ) ;
            
                    buffer.bids[i] = GLuint( -1 ) ; 
                    buffer.sibs[i] = 0 ;
                }
            }
        } ) ;
        return _feedbacks.invalidate( oid ) ;
    }

    //****************************************************************************************
    bool_t update( motor::graphics::streamout_object_ref_t obj, bool_t const is_config = false ) 
    {
        auto const [a,b] = _feedbacks.access<bool_t>( obj.get_oid( _bid ), [&]( this_t::feedback_data_ref_t data )
        {
            for( size_t rw=0; rw<2; ++rw )
            {
                auto & buffer = data._buffers[rw] ;

                size_t i=0; 
                while( buffer.bids[i] != GLuint(-1) )
                {
                    // bind buffer
                    {
                        glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, buffer.bids[i] ) ;
                        if( gl4_log_error3( "glBindBuffer", obj ) ) continue ;
                    }

                    // allocate data
                    GLuint const sib = GLuint( obj.get_buffer( i ).get_layout_sib() * obj.size() ) ;
                    if( is_config || sib > buffer.sibs[i] )
                    {
                        glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, sib, nullptr, GL_DYNAMIC_DRAW ) ;
                        if( gl4_log_error3( "glBufferData", obj ) ) continue ;
                        buffer.sibs[i] = sib ;
                    }
               
                    // do texture
                    // glTexBuffer is required to be called after driver memory is aquired.
                    {
                        glBindTexture( GL_TEXTURE_BUFFER, buffer.tids[i] ) ;
                        if( gl4_log_error3( "glBindTexture", obj ) ) continue ;

                        auto const le = obj.get_buffer( i ).get_layout_element_zero() ;
                        glTexBuffer( GL_TEXTURE_BUFFER, motor::platform::gl3::convert_for_texture_buffer(
                            le.type, le.type_struct ), buffer.bids[i] ) ;
                        if( gl4_log_error3( "glTexBuffer", obj ) ) continue ;

                        glBindTexture( GL_TEXTURE_BUFFER, 0 ) ;
                        if( gl4_log_error3( "glBindTexture", obj ) ) continue ;
                    }

                    ++i ;
                }
            }

            // unbind
            {
                glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, 0 ) ;
                if( gl4_log_error3( "glBindBuffer", obj ) ) return false ;
            }

            for( size_t rw=0; rw<2; ++rw )
            {
                auto & buffer = data._buffers[rw] ;

                {
                    glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, buffer.tfid ) ;
                    if( gl4_log_error3( "glBindTransformFeedback", obj ) ) return false ;
                }

                // bind buffers
                {
                    for( size_t i=0; i<feedback_data::buffer::max_buffers; ++i )
                    {
                        auto const bid = buffer.bids[ i ] ;
                        auto const sib = buffer.sibs[ i ] ;

                        if( bid == GLuint(-1) ) break ;

                        glBindBufferRange( GL_TRANSFORM_FEEDBACK_BUFFER, GLuint(i), bid, 0, sib ) ;
                        if( gl4_log_error3( "glBindTransformFeedback", obj ) ) return false ;
                    }
                }
            }
            return true ;
        } ) ;
        return a && b ;
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
    void_t activate_transform_feedback( GLenum const gpt ) noexcept
    {
        if( _tf_active_id == size_t(-1) ) return ;

        auto const res = _feedbacks.access( _tf_active_id, [&]( this_t::feedback_data_ref_t data )
        {
            auto const wrt_idx = data.write_index() ;
            auto & buffer = data._buffers[wrt_idx] ;

            // EITHER just set prim type what was used for rendering.
            // OR use for overwriting the primitive type on render
            //data.pt = gdata.pt ;

            // bind transform feedback object and update primitive type for 
            // rendering the transform feedback data.
            {
                for( size_t i=0; i<feedback_data::buffer::max_buffers; ++i )
                {
                    if( buffer.bids[ i ] == GLuint(-1) ) break ;

                    _geometries.access( buffer.gids[ i ], [&]( this_t::geo_data_ref_t d )
                    {
                        d.pt = gpt ;
                    } ) ;
                }

                glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, buffer.tfid ) ;
                gl4_log_error( "glBindTransformFeedback" ) ;
            }

            // query written primitives
            {
                glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, buffer.qid ) ;
                gl4_log_error( "glBeginQuery" ) ;
            }

            // begin 
            {
                // lets just use the primitive type of the used geometry
                GLenum const mode = gpt ;
                glBeginTransformFeedback( mode ) ;
                gl4_log_error( "glBeginTransformFeedback" ) ;
            }
        } ) ;
    }

    //****************************************************************************************
    void_t deactivate_transform_feedback( void_t ) noexcept
    {
        if( _tf_active_id == size_t(-1) ) return ;

        {
            glEndTransformFeedback() ;
            gl4_log_error( "glEndTransformFeedback" ) ;
        }

        // query written primitives
        {
            glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN ) ;
            gl4_log_error( "glEndQuery : GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN" ) ;
        }
        
        // swap the indices of the ping-pong buffers
        {
            _feedbacks.access( _tf_active_id, [&]( this_t::feedback_data_ref_t d )
            {
                d.swap_index() ;
            } ) ;
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

        _tf_active_id = size_t(-1) ;
    }

    //****************************************************************************************
    bool_t update_variables( size_t const oid, size_t const varset_id ) noexcept
    {
        auto const [a,b] = _renders.access<bool_t>( oid, [&]( this_t::render_data_ref_t rd )
        {
            return this_t::update_variables( rd, varset_id ) ;
        } ) ;
        return a && b ;
    }

    //****************************************************************************************
    bool_t update_variables( this_t::render_data & config, size_t const varset_id ) noexcept
    {
        _shaders.access( config.shd_id, [&]( this_t::shader_data_ref_t sconfig )
        {
            {
                glUseProgram( sconfig.pg_id ) ;
                if( gl4_log_error("glUseProgram") ) return false ;
            }

            if( config.var_sets.size() <= varset_id ) return false ;
            
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

                        // automatic image update
                        if( link.img_hash != link.var->get().hash() )
                        {
                            size_t const i = _images.find_by_name( link.var->get().name() ) ;
                            if( i == size_t(-1) )
                            {
                                motor::log::global::warning("[gl4] : image variable value not found : " 
                                    "%s : Resetting to old name.", link.var->get().name().c_str() ) ;

                                _images.access( link.img_id, [&]( motor::string_in_t id_name, this_t::image_data_ref_t )
                                {
                                    link.var->set( id_name ) ;
                                } ) ;
                            }
                            else
                            {
                                _images.access( i, [&]( this_t::image_data_ref_t id )
                                {
                                    link.img_hash = link.var->get().hash() ;
                                    link.img_id = i ;
                                    link.tex_id = id.tex_id ;
                                } ) ;
                            }
                        }
                        
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
            return true ;
        } ) ;

        return true ;
    }

    //****************************************************************************************
    bool_t render( size_t const id, size_t const geo_idx = 0, bool_t feed_from_tf = false, 
                   bool_t const use_streamout_count = false, size_t const varset_id = size_t(0), 
                   GLsizei const start_element = GLsizei(0), GLsizei const num_elements = GLsizei(-1) )
    {
        MOTOR_PROBE( "Graphics", "[gl4] : render" ) ;

        _renders.access( id, [&]( motor::string_in_t name, this_t::render_data_ref_t config )
        {
            _shaders.access( config.shd_id, [&]( this_t::shader_data_ref_t sconfig )
            {
                size_t gid = size_t(-1) ;
                size_t tfid = size_t( -1 ) ;

                // #1 : check feedback geometry
                if( (feed_from_tf || use_streamout_count) && config.tf_ids.size() != 0 )
                {
                    tfid = config.tf_ids[0] ;
                    if( feed_from_tf )
                    {
                        _feedbacks.access( tfid, [&]( this_t::feedback_data_ref_t d )
                        {
                            gid = d.read_buffer().gids[0] ;
                        } ) ;
                    }
                    else gid = config.geo_ids[ geo_idx ] ;
                }
                // #2 : check geometry 
                else if( config.geo_ids.size() > geo_idx )
                {
                    gid = config.geo_ids[ geo_idx ] ;
                }
                // #3 : otherwise we can not render
                else
                {
                    motor::log::global::error<1024>( "used geometry idx invalid because" 
                        "exceeds array size for render object : %s", name.c_str() ) ;

                    return false ;
                }

            
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
                                if( motor::ogl::error::check_and_log( gl4_log( "Vertex Array creation" ) ) )
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
                        if( motor::ogl::error::check_and_log( gl4_log( "glBindVertexArray" ) ) )
                            return false ;
                    }
                }

                if( !sconfig.is_linkage_ok ) return false ;

                {
                    glUseProgram( sconfig.pg_id ) ;
                    if( motor::ogl::error::check_and_log( gl4_log( "glUseProgram" ) ) )
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
                                gl4_log_error( "glActiveTexture" ) ;

                                _images.access( link.img_id, [&]( this_t::image_data_ref_t ic )
                                {
                                    glBindTexture( ic.type, link.tex_id ) ;
                                    gl4_log_error( "glBindTexture" ) ;

                                    glTexParameteri( ic.type, GL_TEXTURE_WRAP_S, ic.wrap_types[0] ) ;
                                    glTexParameteri( ic.type, GL_TEXTURE_WRAP_T, ic.wrap_types[1] ) ;
                                    glTexParameteri( ic.type, GL_TEXTURE_WRAP_R, ic.wrap_types[2] ) ;
                                    glTexParameteri( ic.type, GL_TEXTURE_MIN_FILTER, ic.filter_types[0] ) ;
                                    glTexParameteri( ic.type, GL_TEXTURE_MAG_FILTER, ic.filter_types[1] ) ;
                                    gl4_log_error( "glTexParameteri" ) ;
                                } ) ;

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
                                motor::ogl::error::check_and_log( gl4_log( "glActiveTexture" ) ) ;
                                glBindTexture( GL_TEXTURE_BUFFER, link.tex_id ) ;
                                motor::ogl::error::check_and_log( gl4_log( "glBindTexture" ) ) ;

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

                                //auto const & tfd = _feedbacks[ link.so_id ] ;
                                auto const [a, b] = _feedbacks.access<bool_t>( link.so_id, [&]( motor::string_in_t name, 
                                    this_t::feedback_data_ref_t tfd )
                                {
                                    GLuint const tid = link.tex_id[tfd.read_index()] ;

                                    glActiveTexture( GLenum( GL_TEXTURE0 + tex_unit ) ) ;
                                    if( gl4_log_error2( "glActiveTexture", name.c_str() ) ) return false ;
                                    glBindTexture( GL_TEXTURE_BUFFER, tid ) ;
                                    if( gl4_log_error2( "glBindTexture", name.c_str() ) ) return false ;

                                    return true ;
                                } ) ;

                                if( !(a&&b )) return false ;
                                {
                                    auto & uv = sconfig.uniforms[ link.uniform_id ] ;
                                    if( motor::log::global_t::error_if<2048>( !uv.do_uniform_funk( link.mem ),
                                        "[gl4] : uniform %s failed.", uv.name.c_str() ) )
                                    {
                                        return false ;
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
                    gl4_log_error( "glEnable : GL_RASTERIZER_DISCARD" ) ;
                }

                //this_t::geo_data & gconfig = _geometries[ gid ] ;
           
                auto const [gres, g_pt] = _geometries.access<GLenum>( gid, [&]( this_t::geo_data_ref_t d )
                {
                    return d.pt ;
                } ) ;

                {
                    this_t::activate_transform_feedback( g_pt ) ;
                }

                // render section
                {
                    GLenum const pt = g_pt ;
                    //GLuint const ib = gconfig.ib_id ;
                    //GLuint const vb = config.geo->vb_id ;

                    if( tfid != size_t( -1 ) )
                    {
                        _feedbacks.access( tfid, [&]( this_t::feedback_data_ref_t tfd )
                        {
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
                            gl4_log_error( "glDrawTransformFeedback" ) ;
                            #endif
                        } ) ;
                    }
                    else 
                    {
                        _geometries.access( gid, [&]( this_t::geo_data_ref_t gconfig )
                        {
                            if( gconfig.num_elements_ib > 0 )
                            {
                                GLsizei const max_elems = GLsizei( gconfig.num_elements_ib ) ;
                                GLsizei const ne = std::min( num_elements>=0?num_elements:max_elems, max_elems ) ;

                                GLenum const glt = gconfig.ib_type ;

                                void_cptr_t offset = void_cptr_t( byte_cptr_t( nullptr ) + 
                                    start_element * GLsizei( gconfig.ib_elem_sib ) ) ;

                                glDrawElements( pt, ne, glt, offset ) ;
                                gl4_log_error( "glDrawElements" ) ;
                            }
                            else
                            {
                                GLsizei const max_elems = GLsizei( gconfig.num_elements_vb ) ;
                                GLsizei const ne = std::min( num_elements>=0?num_elements:max_elems, max_elems ) ;

                                glDrawArrays( pt, start_element, ne ) ;
                                gl4_log_error( "glDrawArrays" ) ;
                            }
                        } ) ;
                    }
                }

                {
                    this_t::deactivate_transform_feedback() ;
                }

                // enable raster back again.
                if( sconfig.ps_id == GLuint(-1) )
                {
                    glDisable( GL_RASTERIZER_DISCARD ) ;
                    gl4_log_error( "glDisable : GL_RASTERIZER_DISCARD" ) ;
                }

                {
                    glBindVertexArray( 0 ) ;
                    gl4_log_error( "glBindVertexArray" ) ;
                }

                return true ;
            } ) ;
        } ) ;

        //this_t::shader_data & sconfig = _shaders[ config.shd_id ] ;
        

        return true ;
    }

    //****************************************************************************************
    void_t begin_frame( void_t ) 
    {
        // set default render states
        {
            auto const ids_new = std::make_pair( size_t( 0 ), size_t( 0 ) ) ;

            _states.access( ids_new.first, [&]( this_t::state_data_ref_t s )
            {
                // set the viewport to the default new state, 
                // so the correct viewport is set automatically.
                {
                    motor::math::vec4ui_t vp = s.states[ ids_new.second ].view_s.ss.vp ;
                    vp.z( uint_t( vp_width ) ) ;
                    vp.w( uint_t( vp_height ) ) ;
                    s.states[ ids_new.second ].view_s.ss.vp = vp ;
                }

                _state_stack.push( s.states[ ids_new.second ] ) ;
            } ) ;
        }
        
        // do clear the frame
        {
            motor::math::vec4f_t const clear_color = _state_stack.top().clear_s.ss.clear_color ;
            glClearColor( clear_color.x(), clear_color.y(), clear_color.z(), clear_color.w() ) ;
            motor::ogl::error::check_and_log( gl4_log( "glClearColor" ) ) ;

            GLbitfield const color = GL_COLOR_BUFFER_BIT ;
            GLbitfield const depth = GL_DEPTH_BUFFER_BIT ;

            glClear( color | depth ) ;
            motor::ogl::error::check_and_log( gl4_log( "glEnable" ) ) ;
        }
    }

    //****************************************************************************************
    void_t end_frame( void_t ) 
    {
        glFlush() ;
        glFinish() ;

        _state_stack.pop() ;

        _ctsd->notify_thread() ;
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
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::geometry_object_mtr_t obj ) noexcept 
{
    if( !_pimpl->construct_geo( *obj ) ) 
        return motor::graphics::result::failed ;

    if( !_pimpl->update( obj, true ) )
    {
        return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::render_object_mtr_t obj ) noexcept 
{
    #if 0
    size_t const oid = obj->set_oid( this_t::get_bid(), _pimpl->construct_render_data( 
        obj->get_oid( this_t::get_bid() ), *obj ) ) ;
    #endif

    auto const res = _pimpl->construct_render_data( *obj ) ;
    return res ? motor::graphics::result::ok : motor::graphics::result::failed ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    auto const res = _pimpl->construct_shader_data( *obj ) ;
    return res ? motor::graphics::result::ok : motor::graphics::result::failed ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    auto const res = _pimpl->construct_image_config( *obj ) ;
    if( !res )
    {
        return motor::graphics::result::failed ;
    }

    if( !_pimpl->update( *obj, true ) )
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
        gl4_log_error( "[configure] : Object must be valid and requires a name" ) ;
        return motor::graphics::result::invalid_argument ;
    }
    
    auto const res = _pimpl->construct_framebuffer( *obj ) ;

    return res ? motor::graphics::result::ok : motor::graphics::result::failed ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::state_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }
    
    auto const res = _pimpl->construct_state( *obj ) ;
    return res ? motor::graphics::result::ok : motor::graphics::result::failed ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::array_object_mtr_t obj ) noexcept 
{
    auto const res = _pimpl->construct_array_data( *obj ) ;
    if( !res ) return motor::graphics::result::failed ;

    if( !_pimpl->update( *obj, true ) ) return motor::graphics::result::failed ;

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::configure( motor::graphics::streamout_object_mtr_t obj ) noexcept 
{
    auto const res = _pimpl->construct_feedback( *obj ) ;
    if( !res ) return motor::graphics::result::failed ;

    if( !_pimpl->update( *obj, true ) ) return motor::graphics::result::failed ;

    return motor::graphics::result::ok ;
}

//************************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::msl_object_mtr_t ) noexcept 
{
    return motor::graphics::result::failed ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::geometry_object_mtr_t obj ) noexcept 
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
        return motor::graphics::result::invalid_argument ;
    }

    {
        _pimpl->release_render_data( obj->get_oid( this_t::get_bid() ) ) ;
        obj->set_oid( this_t::get_bid(), size_t( -1 ) ) ;
    }

    return motor::graphics::result::ok ;
}

//******************************************************************************************************
motor::graphics::result gl4_backend::release( motor::graphics::shader_object_mtr_t obj ) noexcept
{
    if( obj == nullptr || obj->name().empty() )
    {
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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
        motor::log::global_t::error( gl4_log( "Object must be valid and requires a name" ) ) ;
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
        motor::log::global_t::error( gl4_log( "invalid geometry configuration id" ) ) ;
        return motor::graphics::result::failed ;
    }

    auto const res = _pimpl->update( obj ) ;
    motor::log::global_t::error( !res, gl4_log( "update geometry" ) ) ;

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::streamout_object_mtr_t obj ) noexcept
{
    {
        auto const res = _pimpl->update( *obj, false ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::array_object_mtr_t obj ) noexcept 
{
    {
        auto const res = _pimpl->update( *obj, false ) ;
        if( !res ) return motor::graphics::result::failed ;
    }

    return motor::graphics::result::ok ;
}

//*******************************************************************************************
motor::graphics::result gl4_backend::update( motor::graphics::image_object_mtr_t obj ) noexcept 
{
    if( !_pimpl->update( *obj, false ) )
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
    
    _pimpl->update_state( *obj ) ;
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
        motor::log::global_t::error( gl4_log( "invalid id" ) ) ;
        return motor::graphics::result::failed ;
    }

    if( !_pimpl->check_link_status( oid ) )
    {
        motor::log::global_t::error( gl4_log( "shader did not compile. Abort render." ) ) ;
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
        motor::log::global_t::error( gl4_log( "invalid id" ) ) ;
        return motor::graphics::result::failed ;
    }

    if ( oid == size_t(-2) )
    {
        return motor::graphics::result::in_transit ;
    }

    auto const [a,b] = _pimpl->_msls.access<motor::graphics::result>( oid, [&]( pimpl::msl_data_ref_t msl )
    {
        motor::graphics::render_object_mtr_t ro = &msl.ros[detail.ro_idx] ;
        return this_t::render( ro, detail ) ;
    } ) ;
    
    return (a && b == motor::graphics::result::ok) ? b : motor::graphics::result::failed ;
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
void_t gl4_backend::on_context_destruction( void_t ) noexcept 
{
    _pimpl->on_context_destruction() ;
}