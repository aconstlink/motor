
#include "tri_render_3d.h"

#include <motor/concurrent/parallel_for.hpp>
#include <motor/math/utility/constants.hpp>

using namespace motor::gfx ;

//**********************************************************
tri_render_3d::tri_render_3d( void_t ) noexcept 
{
    _rs = motor::graphics::state_object_t() ;
    _ao = motor::graphics::array_object_t() ;
    _ro = motor::graphics::render_object_t() ;
    _go = motor::graphics::geometry_object_t() ;
    _so = motor::graphics::shader_object_t() ;

    {
        _proj = motor::math::mat4f_t().identity() ;
        _view = motor::math::mat4f_t().identity() ;
    }
}

//**********************************************************
tri_render_3d::tri_render_3d( this_rref_t rhv ) noexcept
{
    *this = std::move( rhv ) ;
}

//**********************************************************
tri_render_3d::this_ref_t tri_render_3d::operator = ( this_rref_t rhv ) noexcept 
{
    _rs = std::move( rhv._rs ) ;
    _ao = std::move( rhv._ao ) ;
    _ro = std::move( rhv._ro ) ;
    _go = std::move( rhv._go ) ;
    _so = std::move( rhv._so ) ;

    _circle_cache = std::move( rhv._circle_cache ) ;

    _proj = std::move( rhv._proj ) ;
    _view = std::move( rhv._view ) ;

    return *this ;
}

//**********************************************************
tri_render_3d::~tri_render_3d( void_t ) noexcept
{
}

//**********************************************************
void_t tri_render_3d::init( motor::string_cref_t name ) noexcept 
{
    _name = name ;

    // root render states
    {
        motor::graphics::state_object_t so = motor::graphics::state_object_t(
            name + ".render_states" ) ;

        {
            motor::graphics::render_state_sets_t rss ;

            rss.depth_s.do_change = true ;
            rss.depth_s.ss.do_activate = true ;
            rss.depth_s.ss.do_depth_write = true ;

            rss.polygon_s.do_change = true ;
            rss.polygon_s.ss.do_activate = false ;
            rss.polygon_s.ss.ff = motor::graphics::front_face::clock_wise ;
            rss.polygon_s.ss.cm = motor::graphics::cull_mode::back ;
            rss.polygon_s.ss.fm = motor::graphics::fill_mode::fill ;

            rss.blend_s.do_change = true ;
            rss.blend_s.ss.do_activate = true ;
            rss.blend_s.ss.src_blend_factor = motor::graphics::blend_factor::src_alpha ;
            rss.blend_s.ss.dst_blend_factor = motor::graphics::blend_factor::one_minus_src_alpha ;

            so.add_render_state_set( rss ) ;
        }

        _rs = std::move( so ) ;
    }

    // geometry configuration
    {
        auto vb = motor::graphics::vertex_buffer_t()
            .add_layout_element( motor::graphics::vertex_attribute::position, motor::graphics::type::tfloat, motor::graphics::type_struct::vec3 );

        auto ib = motor::graphics::index_buffer_t() ;

        _go = motor::graphics::geometry_object_t( name + ".geometry",
            motor::graphics::primitive_type::triangles, std::move( vb ), std::move( ib ) ) ;
    }

    // array
    {
        motor::graphics::data_buffer_t db = motor::graphics::data_buffer_t()
            .add_layout_element( motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 )
            .resize( 2 ).update< the_data >( [&]( the_data * array, size_t const ne )
            {
                for( size_t i=0; i<ne; ++i )
                {
                    array[i].color = motor::math::vec4f_t ( 0.0f, 0.5f, 1.0f, 1.0f) ;
                }
            });

        _ao = motor::graphics::array_object_t( name + ".per_tri_data", std::move( db ) ) ;
    }

    // shader configuration
    {
        motor::graphics::shader_object_t sc( name + ".shader" ) ;

        // shaders : ogl 3.1
        {
            motor::graphics::shader_set_t ss = motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 140

                    in vec3 in_pos ;
                    out vec4 var_col ;
                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;
                    uniform mat4 u_world ;
                    uniform samplerBuffer u_data ;
                    uniform int u_offset ;

                    void main()
                    {
                        // glDrawArrays adds starting index to gl_VertexID
                        int idx = (gl_VertexID) / 3 ;
                        vec4 color = texelFetch( u_data, idx ) ;

                        var_col = color ;
                        vec4 pos = vec4( in_pos, 1.0 )  ;
                        gl_Position = u_proj * u_view * u_world * pos ;

                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 140

                    in vec4 var_col ;
                    out vec4 out_color ;

                    void main()
                    {    
                        out_color = var_col ;
                    } )" ) ) ;

            sc.insert( motor::graphics::shader_api_type::glsl_4_0, std::move( ss ) ) ;
        }

        // shaders : es 3.0
        {
            motor::graphics::shader_set_t ss = motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 320 es
                    precision mediump samplerBuffer ;

                    in vec3 in_pos ;
                    out vec4 var_col ;
                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;
                    uniform mat4 u_world ;
                    uniform samplerBuffer u_data ;
                    uniform int u_offset ;

                    void main()
                    {
                        // glDrawArrays adds starting index to gl_VertexID
                        int idx = (gl_VertexID) / 3 ;
                        var_col = texelFetch( u_data, idx ) ;

                        vec4 pos = vec4( in_pos, 1.0 )  ;
                        gl_Position = u_proj * u_view * u_world * pos ;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 320 es
                    precision mediump float ;
                    in vec4 var_col ;
                    layout(location = 0 ) out vec4 out_color ;

                    void main()
                    {    
                        out_color = var_col ;
                    })" ) ) ;

            sc.insert( motor::graphics::shader_api_type::glsles_3_0, std::move( ss ) ) ;
        }

        // shaders : hlsl 11(5.0)
        {
            motor::graphics::shader_set_t ss = motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    cbuffer ConstantBuffer : register( b0 ) 
                    {
                        float4x4 u_proj ;
                        float4x4 u_view ;
                        float4x4 u_world ;
                        int u_offset ;
                    }

                    struct VS_INPUT
                    {
                        uint in_id: SV_VertexID ;
                        float3 in_pos : POSITION ; 
                    } ;
                    struct VS_OUTPUT
                    {
                        float4 pos : SV_POSITION;
                        float4 col : COLOR0;
                    };
                            
                    Buffer< float4 > u_data ;

                    VS_OUTPUT VS( VS_INPUT input )
                    {
                        VS_OUTPUT output = (VS_OUTPUT)0 ;

                        int idx = (input.in_id / 3) * 1 + (u_offset / 3) * 1 ;
                        float4 col = u_data.Load( idx + 0 ) ;

                        output.col = col ;
                        float4 pos = float4( input.in_pos, 1.0f )  ;
                        output.pos = mul( pos, u_world ) ;
                        output.pos = mul( output.pos, u_view ) ;
                        output.pos = mul( output.pos, u_proj ) ;
                        return output;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    
                    struct VS_OUTPUT
                    {
                        float4 pos : SV_POSITION;
                        float4 col : COLOR0;
                    };

                    float4 PS( VS_OUTPUT input ) : SV_Target0
                    {
                        return input.col ;
                    } )" ) ) ;

            sc.insert( motor::graphics::shader_api_type::hlsl_5_0, std::move( ss ) ) ;
        }

        // configure more details
        {
            sc
                .add_vertex_input_binding( motor::graphics::vertex_attribute::position, "in_pos" )
                .add_input_binding( motor::graphics::binding_point::view_matrix, "u_view" )
                .add_input_binding( motor::graphics::binding_point::projection_matrix, "u_proj" ) ;
        }

        _so = std::move( sc ) ;
    }

    // the render object
    {
        motor::graphics::render_object_t rc = motor::graphics::render_object_t( name + ".render_object" ) ;

        {
            rc.link_geometry( name + ".geometry" ) ;
            rc.link_shader( name + ".shader" ) ;
        }

        // add variable set 
        {
            this_t::add_variable_set( rc ) ;
        }

        _ro = std::move( rc ) ;
    }
}

//**********************************************************
void_t tri_render_3d::release( void_t ) noexcept 
{
}

//**********************************************************
void_t tri_render_3d::draw( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1,
    motor::math::vec3f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept
{
    this_t::tri_t ln ;
    ln.pts.p0 = p0 ;
    ln.pts.p1 = p1 ;
    ln.pts.p2 = p2 ;
    ln.color = color ;

    {
        motor::concurrent::lock_guard_t lk( _tris_mtx ) ;
        if( _tris.capacity() == _tris.size() )
            _tris.reserve( _tris.size() + 100 ) ;
        
        _tris.emplace_back( std::move( ln ) ) ;
    }
}

//**********************************************************
void_t tri_render_3d::draw_rect( 
    motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, 
    motor::math::vec3f_cref_t p2, motor::math::vec3f_cref_t p3, motor::math::vec4f_cref_t color ) noexcept 
{
    this_t::draw( p0, p1, p2, color ) ;
    this_t::draw( p0, p2, p3, color ) ;
}

//**********************************************************
void_t tri_render_3d::draw_circle( motor::math::mat3f_cref_t o, motor::math::vec3f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color, size_t const s ) noexcept 
{
    auto const & points = this_t::lookup_circle_cache( s ) ;

    for( size_t i=0; i<points.size()-1; ++i )
    {
        this_t::draw( p0, p0+(o*points[i]*r), p0+(o*points[i+1]*r), color ) ;
    }
    this_t::draw( p0, p0+points.back()*r, p0+points[0]*r, color ) ;
}

//**********************************************************
void_t tri_render_3d::prepare_for_rendering( void_t ) noexcept 
{
    prepare_update pe = { false, false } ;

    // 1. copy data
    {
        size_t const vsib = _go.vertex_buffer().get_sib() ;
        size_t const bsib = _ao.data_buffer().get_sib() ;

        _go.vertex_buffer().resize( _tris.size() * 3 ) ;
        _ao.data_buffer().resize( _tris.size() ) ;

        // copy vertices
        {
            size_t const num_verts = _tris.size() * 3 ;
            _go.vertex_buffer().update<this_t::vertex>( 0, num_verts, 
                [&]( this_t::vertex * array, size_t const ne )
            {
                #if 1
                motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>(0, ne),
                    [&]( motor::concurrent::range_1d<size_t> const & r )
                {
                    for( size_t v=r.begin(); v<r.end(); ++v )
                    {
                        array[v].pos = _tris[ v / 3 ].array[ v % 3 ] ;
                    }
                } ) ;
                #else
                for( size_t v=0; v<ne; ++v )
                {
                    array[v].pos = _tris[ v / 3 ].array[ v % 3 ] ;
                }
                #endif
            } ) ;
        }

        // copy color data
        {
            #if 1
            motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>(0, _tris.size()),
                [&]( motor::concurrent::range_1d<size_t> const & r )
            {
                for( size_t l=r.begin(); l<r.end(); ++l )
                {
                    size_t const idx = l ;
                    _ao.data_buffer().update< motor::math::vec4f_t >( idx, _tris[l].color ) ;
                }
            } ) ;
            #else
            for( size_t i=0; i<_tris.size();++i)
            {
                size_t const idx = i ;
                _ao.data_buffer().update< motor::math::vec4f_t >( idx, _tris[i].color ) ;
            }
            #endif
        }

        pe.vertex_realloc = _go.vertex_buffer().get_sib() > vsib ;
        pe.data_realloc = _ao.data_buffer().get_sib() > bsib ;

        _num_tris = _tris.size() ;
        _tris.clear() ;
    }

    // 2. prepare variable sets 
    // one var set per layer
    {
        _ro.for_each( [&]( size_t const i, motor::graphics::variable_set_mtr_t vars )
        {
            {
                auto* var = vars->data_variable<motor::math::mat4f_t>( "u_view" ) ;
                var->set( _view ) ;
            }
            {
                auto* var = vars->data_variable<motor::math::mat4f_t>( "u_proj" ) ;
                var->set( _proj ) ;
            }
        } ) ;
    }

    _pe = pe ;
}

//**********************************************************
void_t tri_render_3d::configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->configure<motor::graphics::geometry_object_t>( &_go ) ;
    fe->configure<motor::graphics::shader_object_t>( &_so ) ;
    fe->configure<motor::graphics::array_object_t>( &_ao) ;
    fe->configure<motor::graphics::render_object_t>( &_ro ) ;
    fe->configure<motor::graphics::state_object_t>( &_rs ) ;
}

//**********************************************************
void_t tri_render_3d::prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    if( _pe.vertex_realloc ) fe->configure<motor::graphics::geometry_object_t>( &_go ) ;
    else fe->update( &_go ) ;

    if( _pe.data_realloc ) fe->configure<motor::graphics::array_object_t>( &_ao ) ;
    else fe->update( &_ao ) ;
}

//**********************************************************
void_t tri_render_3d::render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->push( &_rs ) ;
    {
        motor::graphics::gen4::backend::render_detail rd ;
        rd.num_elems = _num_tris * 3 ;
        rd.start = 0 ;
        rd.varset = 0 ;
        fe->render( &_ro, rd ) ;
    }
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
}

//**********************************************************
tri_render_3d::circle_cref_t tri_render_3d::lookup_circle_cache( size_t const s ) noexcept 
{
    // segments required
    size_t const segs_reg = s * 3 ;

    auto iter = std::find_if( _circle_cache.begin(), _circle_cache.end(), [&]( circle_cref_t c )
    {
        return c.size() == segs_reg ;
    } ) ;

    if( iter == _circle_cache.end() )
    {
        size_t const segs = std::max( size_t(5), s ) ;
        motor::vector< motor::math::vec3f_t > points( segs_reg  ) ;

        float_t a = 0.0f ;
        float_t const del = 2.0f * motor::math::constants<float_t>::pi() / float_t(segs) ;
        for( size_t i=0; i<segs; ++i, a += del )
        {
            size_t const idx = i * 3 ;
            points[idx+0] = motor::math::vec3f_t( 0.0f ) ;
            points[idx+1] = motor::math::vec3f_t( std::cos( a ), std::sin( a ), 0.0f ) ;
            points[idx+2] = motor::math::vec3f_t( std::cos( a+del ), std::sin( a+del ), 0.0f ) ;
        }
        iter = _circle_cache.insert( iter, points ) ;
    }
    return *iter ;
}

//**********************************************************
void_t tri_render_3d::add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept 
{
    motor::graphics::variable_set_t vars ;
            
    {
        auto* var = vars.array_variable( "u_data" ) ;
        var->set( _name + ".per_tri_data" ) ;
    }
    {
        auto* var = vars.data_variable<int32_t>( "u_offset" ) ;
        var->set( 0 ) ;
    }
    {
        auto* var = vars.data_variable<motor::math::mat4f_t>( "u_world" ) ;
        var->set( motor::math::mat4f_t().identity() ) ;
    }
    {
        auto* var = vars.data_variable<motor::math::mat4f_t>( "u_view" ) ;
        var->set( motor::math::mat4f_t().identity() ) ;
    }
    {
        auto* var = vars.data_variable<motor::math::mat4f_t>( "u_proj" ) ;
        var->set( motor::math::mat4f_t().identity() ) ;
    }

    rc.add_variable_set( motor::shared( std::move( vars ) ) ) ;
}

//**********************************************************
void_t tri_render_3d::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept 
{
    _view = view ;
    _proj = proj ;
}
