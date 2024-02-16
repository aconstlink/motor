
#include "quad.h"


using namespace motor::gfx ;


quad::quad( motor::string_cref_t name ) noexcept 
{
    _name = name ;
    this_t::init() ;
}

quad::quad( this_rref_t rhv ) noexcept 
{
    _ro = std::move( rhv._ro ) ;
    _go = std::move( rhv._go ) ;
    _name = std::move( rhv._name ) ;
}

quad::~quad( void_t ) noexcept 
{
}

void_t quad::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept 
{
    _ro->for_each( [&]( size_t const, motor::graphics::variable_set_mtr_t const & vars )
    {
        vars->data_variable< motor::math::mat4f_t >( "u_view" )->set( view ) ;
        vars->data_variable< motor::math::mat4f_t >( "u_proj" )->set( proj ) ;
    } ) ;
}

void_t quad::set_position( motor::math::vec2f_cref_t pos ) noexcept 
{
    _ro->for_each( [&]( size_t const, motor::graphics::variable_set_mtr_t const & vars )
    {
        auto * var = vars->data_variable< motor::math::mat4f_t >( "u_world" ) ;
        motor::math::mat4f_t world = var->get() ;
        world.set_column( 3, motor::math::vec3f_t( pos, 0.0f ) ) ;
        var->set( world ) ;
    } ) ;
}

void_t quad::set_position( size_t const vs, motor::math::vec2f_cref_t pos ) noexcept 
{
    if( vs >= _ro->get_num_variable_sets() ) return ; // could add new one

    auto * var = _ro->get_variable_set( vs )->data_variable< motor::math::mat4f_t >( "u_world" ) ;

    motor::math::mat4f_t world = var->get() ;
    world.set_column( 3, motor::math::vec3f_t( pos, 0.0f ) ) ;
    var->set( world ) ;
}

void_t quad::set_scale( motor::math::vec2f_cref_t s ) noexcept 
{
    _ro->for_each( [&]( size_t const, motor::graphics::variable_set_mtr_t const & vars )
    {
        auto * var = vars->data_variable< motor::math::mat4f_t >( "u_world" ) ;
        motor::math::mat4f_t world = var->get() ;
        world[0] = s.x() ;
        world[5] = s.y() ;
        var->set( world ) ;
    } ) ;
}

bool_t quad::set_texture( motor::string_cref_t name ) noexcept 
{
    _ro->for_each( [&]( size_t const, motor::graphics::variable_set_mtr_t const & vars )
    {
        vars->texture_variable( "u_tex" )->set( name ) ;
    } ) ;
    
    #if 0
    _asyncs.for_each( [&]( motor::graphics::async_view_t a )
    {
        a.configure( _ro ) ;
    } ) ;
    #endif
    return true ;
}

bool_t quad::set_texture( size_t const vs, motor::string_cref_t name ) noexcept 
{
    if( vs >= _ro->get_num_variable_sets() ) return false ; 
    _ro->get_variable_set( vs )->texture_variable( "u_tex" )->set( name ) ;
    return true ;
}

bool_t quad::set_texcoord( motor::math::vec4f_cref_t tc ) noexcept 
{
     _ro->for_each( [&]( size_t const, motor::graphics::variable_set_mtr_t const & vars )
    {
        vars->data_variable<motor::math::vec4f_t>( "u_tc" )->set( tc ) ;
    } ) ;
    return true ;
}

bool_t quad::set_texcoord( size_t const vs, motor::math::vec4f_cref_t tc ) noexcept 
{
    if( vs >= _ro->get_num_variable_sets() ) return false ;
    _ro->get_variable_set( vs )->data_variable<motor::math::vec4f_t>( "u_tc" )->set( tc ) ;
    return true ;
}
 
void_t quad::init( size_t const nvs ) noexcept 
{
    // root render states
    {
        motor::graphics::state_object_t so = motor::graphics::state_object_t(
            _name + ".render_states" ) ;

        {
            motor::graphics::render_state_sets_t rss ;
            rss.depth_s.do_change = true ;
            rss.depth_s.ss.do_activate = false ;
            rss.depth_s.ss.do_depth_write = false ;

            rss.polygon_s.do_change = true ;
            rss.polygon_s.ss.do_activate = false ;
            rss.polygon_s.ss.ff = motor::graphics::front_face::clock_wise ;
            so.add_render_state_set( rss ) ;
        }

        motor::memory::release_ptr( _rs ) ;
        _rs = motor::shared( std::move( so ) ) ;
    }

    // geometry
    {
        auto vb = motor::graphics::vertex_buffer_t()
            .add_layout_element( motor::graphics::vertex_attribute::position, motor::graphics::type::tfloat, motor::graphics::type_struct::vec3 )
            .resize( 4 ).update<vertex>( [=] ( vertex* array, size_t const ne )
        {
            array[ 0 ].pos = motor::math::vec3f_t( -0.5f, -0.5f, 0.0f ) ;
            array[ 1 ].pos = motor::math::vec3f_t( -0.5f, +0.5f, 0.0f ) ;
            array[ 2 ].pos = motor::math::vec3f_t( +0.5f, +0.5f, 0.0f ) ;
            array[ 3 ].pos = motor::math::vec3f_t( +0.5f, -0.5f, 0.0f ) ;
        } );

        auto ib = motor::graphics::index_buffer_t().
            set_layout_element( motor::graphics::type::tuint ).resize( 6 ).
            update<uint_t>( [] ( uint_t* array, size_t const ne )
        {
            array[ 0 ] = 0 ;
            array[ 1 ] = 1 ;
            array[ 2 ] = 2 ;

            array[ 3 ] = 0 ;
            array[ 4 ] = 2 ;
            array[ 5 ] = 3 ;
        } ) ;

        motor::memory::release_ptr( _rs ) ;
        _go = motor::shared( motor::graphics::geometry_object_t( _name + ".geometry",
            motor::graphics::primitive_type::triangles, std::move( vb ), std::move( ib ) ) ) ;

        
    }

    // blit framebuffer render object
    {
        motor::graphics::render_object_t rc = motor::graphics::render_object_t( _name + ".render_object" ) ;

        // shader configuration
        {
            motor::graphics::shader_object_t sc( _name + ".shader" ) ;

            // shaders : ogl 3.0
            {
                motor::graphics::shader_set_t ss = motor::graphics::shader_set_t().

                    set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 140
                    in vec3 in_pos ;
                    out vec2 var_tx ;
                    uniform mat4 u_world ;
                    uniform mat4 u_view ;
                    uniform mat4 u_proj ;
                    uniform vec4 u_tc ;
                    void main()
                    {
                        vec2 tc[4] = vec2[](
                            vec2( u_tc.x, u_tc.y ),
                            vec2( u_tc.x, u_tc.w ),
                            vec2( u_tc.z, u_tc.w ),
                            vec2( u_tc.z, u_tc.y )
                        ) ;
                        var_tx = tc[ gl_VertexID ] ;
                        //var_tx = sign( in_pos.xy ) * vec2( 0.5, 0.5 ) + vec2( 0.5, 0.5 ) ;
                        gl_Position =  u_proj * u_view * u_world * vec4( sign( in_pos ), 1.0 ) ;
                    } )" ) ).

                    set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 140
                    in vec2 var_tx ;
                    out vec4 out_color ;
                    uniform sampler2D u_tex ;

                    void main()
                    {
                        out_color = texture( u_tex, var_tx ) ;
                    } )" ) ) ;

                sc.insert( motor::graphics::shader_api_type::glsl_1_4, std::move( ss ) ) ;
            }

            // shaders : es 3.0
            {
                motor::graphics::shader_set_t ss = motor::graphics::shader_set_t().

                    set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    in vec3 in_pos ;
                    out vec2 var_tx ;
                    uniform mat4 u_world ;
                    uniform mat4 u_view ;
                    uniform mat4 u_proj ;
                    uniform vec4 u_tc ;
                    void main()
                    {
                        vec2 tc[4] = vec2[](
                            vec2( u_tc.x, u_tc.y ),
                            vec2( u_tc.x, u_tc.w ),
                            vec2( u_tc.z, u_tc.w ),
                            vec2( u_tc.z, u_tc.y )
                        ) ;
                        var_tx = tc[ gl_VertexID ] ;
                        gl_Position = u_proj * u_view * u_world * vec4( sign( in_pos ), 1.0 ) ;
                    } )" ) ).

                    set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    precision mediump float ;
                    in vec2 var_tx ;
                    out vec4 out_color ;
                    uniform sampler2D u_tex ;

                    void main()
                    {
                        out_color = texture( u_tex, var_tx ) ;
                    } )" ) ) ;

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
                        float4 u_tc ;
                    }

                    struct VS_OUTPUT
                    {
                        float4 pos : SV_POSITION ;
                        float2 tx : TEXCOORD0 ;
                    };

                    VS_OUTPUT VS( float4 in_pos : POSITION, uint in_id : SV_VertexID )
                    {
                        float2 tc[4] = {
                            float2( u_tc.x, u_tc.y ),
                            float2( u_tc.x, u_tc.w ),
                            float2( u_tc.z, u_tc.w ),
                            float2( u_tc.z, u_tc.y )
                        } ;

                        VS_OUTPUT output = (VS_OUTPUT)0;
                        output.pos = float4( sign( in_pos.xyz ), 1.0f ) ;
                        output.pos = mul( output.pos, u_world ) ;
                        output.pos = mul( output.pos, u_view ) ;
                        output.pos = mul( output.pos, u_proj ) ;
                        output.tx = tc[in_id] ; 
                        //output.tx = sign( in_pos.xy ) * float2( 0.5, 0.5 ) + float2( 0.5, 0.5 ) ;
                        return output;
                    } )" ) ).

                    set_pixel_shader( motor::graphics::shader_t( R"(
                            
                    Texture2D u_tex : register( t0 );
                    SamplerState smp_u_tex : register( s0 );
                    float sys_flipv_u_tex ;

                    struct VS_OUTPUT
                    {
                        float4 pos : SV_POSITION ;
                        float2 tx : TEXCOORD0 ;
                    };

                    float4 PS( VS_OUTPUT input ) : SV_Target
                    {
                        float2 uv = input.tx ;
                        uv.y = lerp( uv.y, 1.0 - uv.y, sys_flipv_u_tex ) ;
                        return u_tex.Sample( smp_u_tex, uv );
                    } )" ) ) ;

                sc.insert( motor::graphics::shader_api_type::hlsl_5_0, std::move( ss ) ) ;
            }

            // configure more details
            {
                sc.add_vertex_input_binding( motor::graphics::vertex_attribute::position, "in_pos" ) ;
            }

            motor::memory::release_ptr( _so ) ;
            _so = motor::shared( std::move( sc ) ) ;
        }

        {
            rc.link_geometry( _name + ".geometry" ) ;
            rc.link_shader( _name + ".shader" ) ;
        }

        _ro->remove_variable_sets() ;

        for( size_t i = 0 ; i<nvs; ++i )
        {
            motor::graphics::variable_set_t vars = motor::graphics::variable_set_t() ; 

            {
                auto * var = vars.data_variable< motor::math::mat4f_t >( "u_world" ) ;
                var->set( _world ) ;
            }
            {
                auto * var = vars.data_variable< motor::math::mat4f_t >( "u_view" ) ;
                var->set( _view ) ;
            }
            {
                auto * var = vars.data_variable< motor::math::mat4f_t >( "u_proj" ) ;
                var->set( _proj ) ;
            }
            {
                auto * var = vars.data_variable< motor::math::vec4f_t >( "u_tc" ) ;
                var->set( motor::math::vec4f_t(0.0f,0.0f,1.0f,1.0f) ) ;
            }
            rc.add_variable_set( motor::shared( std::move(vars)  ) ) ;
        }

        motor::memory::release_ptr( _ro ) ;
        _ro = motor::shared( std::move( rc ) ) ;
    }
}

void_t quad::on_frame_init( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->configure<motor::graphics::state_object_t>( _rs ) ;
    fe->configure<motor::graphics::geometry_object_t>( _go ) ;
    fe->configure<motor::graphics::shader_object_t>( _so ) ;
    fe->configure<motor::graphics::render_object_t>( _ro ) ;
}

void_t quad::on_frame_release( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->release( motor::move( _rs ) ) ;
    fe->release( motor::move( _go ) ) ;
    fe->release( motor::move( _so ) ) ;
    fe->release( motor::move( _ro ) ) ;
}

void_t quad::on_frame_render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->push( _rs ) ;

    _ro->for_each( [&]( size_t const i, motor::graphics::variable_set_mtr_t const & )
    {
        motor::graphics::gen4::backend::render_detail rd ;
        rd.varset = i ;

        fe->render( _ro, rd ) ;
    } ) ;
        
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
}

void_t quad::on_frame_render( size_t const i, motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->push( _rs ) ;
    {
        motor::graphics::gen4::backend::render_detail rd ;
        rd.varset = i ;

        fe->render( _ro, rd ) ;
    }
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
}

void_t quad::add_variable_sets( size_t const nvs ) noexcept
{
    for( size_t i = 0 ; i<nvs; ++i )
    {
        motor::graphics::variable_set_t vars = motor::graphics::variable_set_t() ; 

        {
            auto * var = vars.data_variable< motor::math::mat4f_t >( "u_world" ) ;
            var->set( _world ) ;
        }
        {
            auto * var = vars.data_variable< motor::math::mat4f_t >( "u_view" ) ;
            var->set( _view ) ;
        }
        {
            auto * var = vars.data_variable< motor::math::mat4f_t >( "u_proj" ) ;
            var->set( _proj ) ;
        }
        {
            auto * var = vars.data_variable< motor::math::vec4f_t >( "u_tc" ) ;
            var->set( motor::math::vec4f_t(0.0f,0.0f,1.0f,1.0f) ) ;
        }
        _ro->add_variable_set( motor::shared( std::move( vars ) ) ) ;
    }

    //_asyncs.for_each( [&]( motor::graphics::async_view_t a ) { a.configure( _ro ) ; } ) ;
    assert( false ) ;
}

size_t quad::get_num_variable_sets( void_t ) const noexcept 
{
    return _ro->get_num_variable_sets() ;
}
