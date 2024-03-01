
#include "line_render_3d.h"

#include <motor/concurrent/parallel_for.hpp>

using namespace motor::gfx ;

//**********************************************************
line_render_3d::line_render_3d( void_t ) noexcept 
{
}
            
//**********************************************************
line_render_3d::line_render_3d( this_rref_t rhv ) noexcept 
{
    *this = std::move( rhv ) ;
}

//**********************************************************
line_render_3d::this_ref_t line_render_3d::operator = ( this_rref_t rhv ) noexcept 
{
    _rs = std::move( rhv._rs ) ;
    _ao = std::move( rhv._ao ) ;
    _so = std::move( rhv._so ) ;
    _ro = std::move( rhv._ro ) ;
    _go = std::move( rhv._go ) ;

    _lines = std::move( rhv._lines ) ;
    _num_lines = rhv._num_lines ;

    return *this ;
}

//**********************************************************
line_render_3d::~line_render_3d( void_t ) noexcept 
{
}
            
//**********************************************************
void_t line_render_3d::init( motor::string_cref_t name ) noexcept 
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
            .add_layout_element( motor::graphics::vertex_attribute::position, motor::graphics::type::tfloat, motor::graphics::type_struct::vec3 )
            .resize( 4 ).update<this_t::vertex>( [=] ( this_t::vertex* array, size_t const ne )
        {
            array[ 0 ].pos = motor::math::vec3f_t( -0.5f, -0.5f ) ;
            array[ 1 ].pos = motor::math::vec3f_t( -0.5f, +0.5f ) ;
            array[ 2 ].pos = motor::math::vec3f_t( +0.5f, +0.5f ) ;
            array[ 3 ].pos = motor::math::vec3f_t( +0.5f, -0.5f ) ;
        } );

        auto ib = motor::graphics::index_buffer_t() ;

        _go = motor::graphics::geometry_object_t( name + ".lines",
            motor::graphics::primitive_type::lines, std::move( vb ), std::move( ib ) ) ;
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

        _ao = motor::graphics::array_object_t( name + ".per_line_data", std::move( db ) ) ;
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
                        int idx = gl_VertexID / 2 ;
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
                        int idx = gl_VertexID / 2 ;
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
                        int idx = input.in_id / 2 ;
                        float4 col = u_data.Load( idx ) ;
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
                        float4 Pos : SV_POSITION;
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
            rc.link_geometry( name + ".lines" ) ;
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
void_t line_render_3d::release( void_t ) noexcept 
{
}

//**********************************************************
void_t line_render_3d::draw( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec4f_cref_t color ) noexcept
{
    this_t::line_t ln ;
    ln.points.p0 = p0 ;
    ln.points.p1 = p1 ;
    ln.color = color ;

    {
        motor::concurrent::lock_guard_t lk( _lines_mtx ) ;
        if( _lines.capacity() == _lines.size() )
            _lines.reserve( _lines.size() + 100 ) ;
        
        _lines.emplace_back( std::move( ln ) ) ;
    }
}

//**********************************************************
void_t line_render_3d::configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->configure<motor::graphics::geometry_object_t>( &_go ) ;
    fe->configure<motor::graphics::shader_object_t>( &_so ) ;
    fe->configure<motor::graphics::array_object_t>( &_ao) ;
    fe->configure<motor::graphics::render_object_t>( &_ro ) ;
    fe->configure<motor::graphics::state_object_t>( &_rs ) ;
}

//**********************************************************
void_t line_render_3d::prepare_for_rendering( void_t ) noexcept 
{
    prepare_update pe = { false, false } ;

    // 1. copy data
    {
        size_t const vsib = _go.vertex_buffer().get_sib() ;
        size_t const bsib = _ao.data_buffer().get_sib() ;

        _go.vertex_buffer().resize( _lines.size() << 1 ) ;
        _ao.data_buffer().resize( _lines.size() ) ;

        // copy vertices
        {
            size_t const num_verts = _lines.size() << 1 ;
            _go.vertex_buffer().update<this_t::vertex>( 0, num_verts, 
                [&]( this_t::vertex * array, size_t const ne )
            {
                motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>(0, ne>>1),
                    [&]( motor::concurrent::range_1d<size_t> const & r )
                {
                    for( size_t l=r.begin(); l<r.end(); ++l )
                    {
                        size_t const w = l << 1 ;
                        array[ w + 0 ].pos = _lines[ l ].pa[0] ;
                        array[ w + 1 ].pos = _lines[ l ].pa[1] ;
                    }
                } ) ;
            } ) ;
        }

        // copy data
        {
            motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>(0, _lines.size()),
                [&]( motor::concurrent::range_1d<size_t> const & r )
            {
                for( size_t l=r.begin(); l<r.end(); ++l )
                {
                    size_t const idx = l ;
                    _ao.data_buffer().update< motor::math::vec4f_t >( idx, _lines[l].color ) ;
                }
            } ) ;
        }

        pe.vertex_realloc = _go.vertex_buffer().get_sib() > vsib ;
        pe.data_realloc = _ao.data_buffer().get_sib() > bsib ;

        _num_lines = _lines.size() ;
        _lines.clear() ;
    }

    // 2. prepare variable set
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
}

//**********************************************************
void_t line_render_3d::prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    if( _pe.vertex_realloc ) fe->configure<motor::graphics::geometry_object_t>( &_go ) ;
    else fe->update( &_go ) ;

    if( _pe.data_realloc ) fe->configure<motor::graphics::array_object_t>( &_ao ) ;
    else fe->update( &_ao ) ;
}

//**********************************************************
void_t line_render_3d::render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->push( &_rs ) ;
    {
        motor::graphics::gen4::backend::render_detail rd ;
        rd.num_elems = _num_lines << 1 ;
        rd.start = 0 ;
        rd.varset = 0 ;
        fe->render( &_ro, rd ) ;
    }
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
}
            
//**********************************************************
void_t line_render_3d::add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept 
{
    motor::graphics::variable_set_t vars ;
            
    {
        auto* var = vars.array_variable( "u_data" ) ;
        var->set( _name + ".per_line_data" ) ;
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
void_t line_render_3d::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept 
{
    _view = view ;
    _proj = proj ;
}
