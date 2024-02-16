
#include "text_render_2d.h"

using namespace motor::gfx ;

//*************************************************************************
text_render_2d::text_render_2d( void_t ) noexcept
{
    
}

//*************************************************************************
text_render_2d::text_render_2d( this_rref_t rhv ) noexcept
{
    _rc = motor::move( rhv._rc ) ;
    _sc = motor::move( rhv._sc ) ;
    _gc = motor::move( rhv._gc ) ;
    _vars = std::move( rhv._vars ) ;
    _ga = motor::move( rhv._ga ) ;
    _ic = motor::move( rhv._ic ) ;
    _sto = motor::move( rhv._sto ) ;

    _text_infos = motor::move( rhv._text_infos ) ;
    _glyph_infos = motor::move( rhv._glyph_infos ) ;

    _name = std::move( rhv._name ) ;
}

//*************************************************************************
text_render_2d::~text_render_2d( void_t ) noexcept
{
    motor::memory::release_ptr( _rc ) ;
    motor::memory::release_ptr( _sc ) ;
    motor::memory::release_ptr( _gc ) ;
    motor::memory::release_ptr( _ga ) ;
    motor::memory::release_ptr( _ic ) ;
    motor::memory::release_ptr( _sto ) ;

    motor::memory::release_ptr( _text_infos ) ;
    motor::memory::release_ptr( _glyph_infos ) ;
    for( auto * v : _vars ) motor::memory::release_ptr( v ) ;
}

//*************************************************************************
void_t text_render_2d::init( motor::string_cref_t name, size_t const ng ) noexcept 
{
    this_t::init( name, motor::font::glyph_atlas_mtr_safe_t(_ga), ng ) ;
}

//*************************************************************************
void_t text_render_2d::init( motor::string_cref_t name, motor::font::glyph_atlas_mtr_safe_t ga_, size_t const ng ) noexcept
{
    _name = "natus.gfx.text_render_2d." + name ;

    if( ga_ == nullptr ) 
    {
        motor::log::global_t::error( motor_log_fn("invalid glyph atlas") ) ;
        return ;
    }

    // geometry
    {
        // @see struct vertex in the header
        auto vb = motor::graphics::vertex_buffer_t()
            .add_layout_element( motor::graphics::vertex_attribute::position, 
                motor::graphics::type::tfloat, motor::graphics::type_struct::vec2 )

            .resize( num_quads << 2 ).update<vertex>( [=] ( vertex* array, size_t const ne )
            {
                size_t const num_quads_ = ne >> 2 ;

                for( size_t v = 0 ; v <num_quads_ ; ++v )
                {
                    size_t const vid = v << 2 ;

                    array[ vid + 0 ].pos = motor::math::vec2f_t( -0.5f, -0.5f ) ;
                    array[ vid + 1 ].pos = motor::math::vec2f_t( -0.5f, +0.5f ) ;
                    array[ vid + 2 ].pos = motor::math::vec2f_t( +0.5f, +0.5f ) ;
                    array[ vid + 3 ].pos = motor::math::vec2f_t( +0.5f, -0.5f ) ;
                }
            } ); ;

        auto ib = motor::graphics::index_buffer_t().
            set_layout_element( motor::graphics::type::tuint ).resize( 6*num_quads ).
                update<uint_t>( [=] ( uint_t* array, size_t const ne )
                {
                    uint_t const num_quads_ = uint_t(ne) / 6 ;

                    for( uint_t i=0; i<num_quads_; ++i )
                    {
                        uint_t const idx = i * 6 ;
                        uint_t const vid = i << 2 ;

                        array[ idx + 0 ] = vid + 0 ;
                        array[ idx + 1 ] = vid + 1 ;
                        array[ idx + 2 ] = vid + 2 ;

                        array[ idx + 3 ] = vid + 0 ;
                        array[ idx + 4 ] = vid + 2 ;
                        array[ idx + 5 ] = vid + 3 ;
                    }
                } ) ; 

        motor::memory::release_ptr( _gc ) ;
        _gc = motor::shared( motor::graphics::geometry_object_t( this_t::name(),
            motor::graphics::primitive_type::triangles,
            std::move( vb ), std::move( ib ) ) ) ;
    }

    {
        motor::graphics::state_object_t so = motor::graphics::state_object_t( this_t::name() ) ;

        {
            motor::graphics::render_state_sets_t rss ;

            rss.blend_s.do_change = true ;
            rss.blend_s.ss.do_activate = true ;
            rss.blend_s.ss.src_blend_factor = motor::graphics::blend_factor::one ;
            rss.blend_s.ss.dst_blend_factor = motor::graphics::blend_factor::one_minus_src_alpha ;

            //rss.polygon_s.do_change = true ;
            rss.polygon_s.ss.do_activate = true ;
            rss.polygon_s.ss.ff = motor::graphics::front_face::counter_clock_wise ;
            rss.polygon_s.ss.cm = motor::graphics::cull_mode::back ;
            rss.polygon_s.ss.fm = motor::graphics::fill_mode::fill ;

            so.add_render_state_set( rss ) ;
        }

        motor::memory::release_ptr( _sto ) ;
        _sto = motor::shared( std::move( so ) ) ;
    }

    // shader config
    {
        motor::graphics::shader_object_t sc( this_t::name() ) ;

        // shaders : ogl 3.1
        {
            sc.insert( motor::graphics::shader_api_type::glsl_1_4, motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 140

                    in vec2 in_pos ;

                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;

                    // 1: vec4( vec2( tx start ), vec2( tx_dims ) )
                    // 2: vec4( img_id, bearing, vec2( aspect ) )
                    // aspect for geometric scaling
                    uniform samplerBuffer u_glyph_infos ;

                    // 1: vec4( pos.xy, offset, scale )
                    // 2: vec4( color.xyz, advancement_x )
                    uniform samplerBuffer u_text_infos ;

                    // group offset for separated group rendering
                    uniform uint u_offset ;

                    out vec3 var_uv ;
                    out vec3 var_color ;
                    flat out int var_layer ;

                    void main()
                    {
                        int idx_text = (gl_VertexID / 4) + int( u_offset ) ;
                        vec4 t1 = texelFetch( u_text_infos, idx_text * 2 + 0 ) ;
                        vec4 t2 = texelFetch( u_text_infos, idx_text * 2 + 1 ) ;

                        int idx_glyph = int( t1.z ) ;
                        vec4 g1 = texelFetch( u_glyph_infos, idx_glyph * 2 + 0 ) ;
                        vec4 g2 = texelFetch( u_glyph_infos, idx_glyph * 2 + 1 ) ;
                        
                        float z = floor( g2.x / 4.0 ) ;
                        var_layer = int( g2.x ) % 4 ;

                        // 
                        // calc texture coordinates
                        // 
                        var_uv = vec3( vec2( sign(in_pos.xy) * 0.5 + 0.5 ) * g1.zw + g1.xy, z ) ;
                        
                        // 
                        // calc position and scale
                        //
                        vec3 pos = vec3( sign( in_pos ) * 0.5 + 0.5, 0.0 ) ;
                        pos *= vec3( g1.zw * g2.zw * vec2(t1.w), 1.0 ) ;
                        pos += vec3( vec2( t2.w, g2.y*t1.w), 0.0 ) ;
                        gl_Position = u_proj * u_view * vec4( t1.xy, 0.0, 1.0 ) + vec4( pos, 0.0 ) ;

                        var_color = t2.xyz ;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 140

                    out vec4 out_color ;
                    uniform sampler2DArray u_atlas ;

                    in vec3 var_uv ;
                    in vec3 var_color ;
                    flat in int var_layer ;

                    void main()
                    {    
                        float mask = texture( u_atlas, var_uv )[var_layer] ;
                        out_color = vec4( var_color * mask, mask ) ;
                    } )" ) ) 
            ) ;
        }

        // shaders : es 3.0
        {
            sc.insert( motor::graphics::shader_api_type::glsles_3_0, motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    precision mediump float ;
                    layout( location = 0 ) in vec2 in_pos ;

                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;

                    // 1: vec4( vec2( tx start ), vec2( tx_dims ) )
                    // 2: vec4( img_id, bearing, vec2( aspect ) )
                    uniform sampler2D u_glyph_infos ;

                    // 1: vec4( pos.xy, offset, scale )
                    // 2: vec4( color.xyz, 0.0f )
                    uniform sampler2D u_text_infos ;

                    // group offset for separated group rendering
                    uniform uint u_offset ;

                    out vec3 var_uv ;
                    out vec3 var_color ;
                    flat out int var_layer ;

                    void main()
                    {
                        int idx_text = (gl_VertexID / 4) + int( u_offset ) ;
                        ivec2 wht = textureSize( u_text_infos, 0 ) ;
                        vec4 t1 = texelFetch( u_text_infos, 
                            ivec2( ((idx_text*2+0) % wht.x), ((idx_text*2+0) / wht.x) ), 0 ) ;
                        vec4 t2 = texelFetch( u_text_infos, 
                            ivec2( (idx_text*2+1) % wht.x, (idx_text*2+1) / wht.x ), 0 ) ;

                        int idx_glyph = int( t1.z ) ;
                        ivec2 whg = textureSize( u_glyph_infos, 0 ) ;
                        vec4 g1 = texelFetch( u_glyph_infos, 
                            ivec2( ((idx_glyph*2+0) % whg.x), ((idx_glyph*2+0) / whg.x) ), 0 ) ;
                        vec4 g2 = texelFetch( u_glyph_infos, 
                            ivec2( (idx_glyph*2+1) % whg.x, (idx_glyph*2+1) / whg.x ), 0 ) ;
                        
                        float z = floor( g2.x / 4.0 ) ;
                        var_layer = int( g2.x ) % 4 ;

                        // 
                        // calc texture coordinates
                        // 
                        var_uv = vec3( vec2( sign(in_pos.xy) * 0.5 + 0.5 ) * g1.zw + g1.xy, z ) ;
                        
                        // 
                        // calc position and scale
                        //
                        vec3 pos = vec3( sign( in_pos ) * 0.5 + 0.5, 0.0 ) ;
                        pos *= vec3( g1.zw * g2.zw * vec2(t1.w), 1.0 ) ;
                        pos += vec3( vec2( t2.w, g2.y*t1.w), 0.0 ) ;
                        gl_Position = u_proj * u_view * vec4( t1.xy, 0.0, 1.0 ) + vec4( pos, 0.0 ) ;

                        var_color = t2.xyz ;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    precision mediump float ;
                    precision mediump sampler2DArray ;
                    layout( location = 0 ) out vec4 out_color ;

                    uniform sampler2DArray u_atlas ;

                    in vec3 var_uv ;
                    in vec3 var_color ;
                    flat in int var_layer ;

                    void main()
                    {    
                        float mask = texture( u_atlas, var_uv )[var_layer] ;
                        out_color = vec4( var_color * mask, mask ) ;
                    } )" ) ) 
            ) ;
        }

        // shaders : hlsl 11
        {
            sc.insert( motor::graphics::shader_api_type::hlsl_5_0, motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    cbuffer Camera : register( b0 ) 
                    {
                        matrix u_proj ;
                        matrix u_view ;

                        // group offset for separated group rendering
                        uint u_offset ;
                    }

                    struct VS_OUTPUT
                    {
                        float4 pos : SV_POSITION ;
                        float3 tx : TEXCOORD0 ;
                        float3 color : COLOR0 ;
                        int layer : TEXCOORD1 ;
                    };

                    // 1: vec4( vec2( tx start ), vec2( tx_dims ) )
                    // 2: vec4( img_id, bearing, float2(aspect) )
                    Buffer< float4 > u_glyph_infos ;

                    // 1: vec4( pos.xy, offset, scale )
                    // 2: vec4( color.xyz, x advancement )
                    Buffer< float4 > u_text_infos ;

                    VS_OUTPUT VS( float2 in_pos : POSITION, uint in_id: SV_VertexID )
                    {
                        VS_OUTPUT output = (VS_OUTPUT)0;

                        int idx_text = (in_id / 4) + int( u_offset ) ;
                        float4 t1 = u_text_infos.Load( idx_text * 2 + 0 ) ;
                        float4 t2 = u_text_infos.Load( idx_text * 2 + 1 ) ;

                        int idx_glyph = int( t1.z ) ;
                        float4 g1 = u_glyph_infos.Load( idx_glyph * 2 + 0 ) ;
                        float4 g2 = u_glyph_infos.Load( idx_glyph * 2 + 1 ) ;
                        
                        float z = floor( g2.x / 4.0 ) ;
                        output.layer = int( g2.x % 4 ) ;

                        // 
                        // calc texture coordinates
                        // 
                        output.tx = float3( float2( (sign(in_pos.xy) * 0.5 + 0.5 ) * g1.zw + g1.xy ), z ) ;
                        
                        // 
                        // calc position and scale
                        //
                        float3 pos = float3( sign( in_pos ) * 0.5 + 0.5, 0.0 ) ;
                        pos *= float3( g1.zw * g2.zw * t1.w, 1.0 ) ;
                        pos += float3( float2( t2.w, g2.y*t1.w), 0.0 ) ;
                        
                        output.pos = mul( float4( t1.xy, 0.0, 1.0 ), u_view ) ;
                        output.pos = mul( output.pos, u_proj ) ;
                        output.pos += float4( pos, 0.0 ) ;

                        output.color = t2.xyz ;

                        return output ;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                        Texture2DArray u_atlas : register( t0 );
                        SamplerState smp_u_atlas : register( s0 );

                        struct VS_OUTPUT
                        {
                            float4 pos : SV_POSITION ;
                            float3 tx : TEXCOORD0 ;
                            float3 color : COLOR0 ;
                            int layer : TEXCOORD1 ;
                        };

                        float4 PS( VS_OUTPUT input ) : SV_Target
                        {
                            float mask = u_atlas.Sample( smp_u_atlas, input.tx )[input.layer] ;
                            return float4( input.color*mask, mask );
                        } )" 
                ) ) ) ;
        }

        // configure more details
        {
            sc
                .add_vertex_input_binding( motor::graphics::vertex_attribute::position, "in_pos" )
                .add_input_binding( motor::graphics::binding_point::view_matrix, "u_view" )
                .add_input_binding( motor::graphics::binding_point::projection_matrix, "u_proj" ) ;
        }

        motor::memory::release_ptr( _sc ) ;
        _sc = motor::shared( std::move( sc ) ) ;
    }

    // prepare glyph atlas planes as graphics images
    {
        _ga = std::move( ga_ ) ; 

        motor::font::glyph_atlas_ref_t ga = *_ga ;

        // squeeze one image in a color channel. 
        size_t const z = (ga.get_num_images() >> 2) + (ga.get_num_images() % 4 != 0 ? 1 : 0) ;
        motor::graphics::image_t img = motor::graphics::image_t( 
        motor::graphics::image_t::dims_t( ga.get_width(), ga.get_height(), z ) )
            .update( [&]( motor::graphics::image_ptr_t, motor::graphics::image_t::dims_in_t dims, void_ptr_t data_in )
        {
            typedef motor::math::vector4< uint8_t > rgba_t ;
            auto* data_ = reinterpret_cast< rgba_t * >( data_in ) ;

            for( size_t i=0; i<ga.get_num_images(); ++i )
            {
                auto * raw = ga.get_image(i) ;
                
                size_t const z = i >> 2 ;
                size_t const c = i % 4 ;

                for( size_t y=0; y<dims.y(); ++y )
                {
                    for( size_t x=0; x<dims.x() ; ++x )
                    {
                        *data_ = rgba_t(0) ;

                        size_t const idx_w = z * dims.x() * dims.y() + y * dims.x() + x ;
                        size_t const idx_r = y * dims.x() + x ;
                        
                        data_[idx_w][c] = raw->get_plane()[idx_r] ;
                    }
                }
            }
        } ) ;

        auto ic = motor::graphics::image_object_t( this_t::name() + ".atlas", std::move( img ) )
            .set_type( motor::graphics::texture_type::texture_2d_array )
            .set_wrap( motor::graphics::texture_wrap_mode::wrap_s, motor::graphics::texture_wrap_type::repeat )
            .set_wrap( motor::graphics::texture_wrap_mode::wrap_t, motor::graphics::texture_wrap_type::repeat )
            .set_filter( motor::graphics::texture_filter_mode::min_filter, motor::graphics::texture_filter_type::nearest )
            .set_filter( motor::graphics::texture_filter_mode::mag_filter, motor::graphics::texture_filter_type::nearest );

        motor::memory::release_ptr( _ic ) ;
        _ic = motor::shared( std::move( ic ) ) ;
    }

    // prepare glyph infos
    {
        motor::math::vec2f_t const aspect( 
            float_t( _ga->get_width() ) / float_t( _ga->get_height() ), 1.0f ) ;

        struct the_data
        {
            motor::math::vec4f_t v1 ;
            motor::math::vec4f_t v2 ;
        };

        motor::graphics::data_buffer_t db = motor::graphics::data_buffer_t()
            .add_layout_element( motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 )
            .add_layout_element( motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 )
            .resize( _ga->get_num_glyphs() )
            .update< the_data >( [&]( the_data * array, size_t const ne )
            {
                for( size_t i=0; i<ne; ++i )
                {
                    motor::font::glyph_atlas_t::glyph_info_t gi ;
                    auto const res = _ga->get_glyph_info( i, gi ) ;
                    if( !res ) continue ;

                    auto const v1 = motor::math::vec4f_t( gi.start, gi.dims ) ;
                    auto const v2 = motor::math::vec4f_t( float_t( gi.image_id ), gi.bearing, 
                        aspect.x(), aspect.y() ) ;

                    array[ i ] = { v1, v2 } ;
                }
            }
        );

        motor::memory::release_ptr( _glyph_infos ) ;
        _glyph_infos = motor::shared( motor::graphics::array_object_t( 
            this_t::name() + ".glyph_infos", std::move( db ) ) ) ;
    }

    // prepare text infos
    {
        struct the_data
        {
            // v1: ( pos.xy, offset, scale )
            // v2: ( color.xyz, 0.0f )
            motor::math::vec4f_t v1 ;
            motor::math::vec4f_t v2 ;
        };

        motor::graphics::data_buffer_t db = motor::graphics::data_buffer_t()
            .add_layout_element( motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 )
            .add_layout_element( motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 )
            .resize( 0 ) ;

        motor::memory::release_ptr( _text_infos ) ;
        _text_infos = motor::shared( motor::graphics::array_object_t( 
            this_t::name() + ".text_infos", std::move( db ) ) ) ;
    }

    // render configuration
    {
        motor::graphics::render_object_t rc( this_t::name() ) ;

        rc.link_geometry( this_t::name() ) ;
        rc.link_shader( this_t::name() ) ;
        
        for( auto * v : _vars ) motor::memory::release_ptr( v ) ;
        _vars.clear() ;

        // one variable set per group
        for( size_t i=0; i<ng; ++i )
        {
            motor::graphics::variable_set_t vs ;
        
            {
                auto* var = vs.texture_variable( "u_atlas" ) ;
                var->set( this_t::name() + ".atlas" ) ;
            }

            {
                auto* var = vs.array_variable( "u_glyph_infos" ) ;
                var->set( this_t::name() + ".glyph_infos" ) ;
            }

            {
                auto* var = vs.array_variable( "u_text_infos" ) ;
                var->set( this_t::name() + ".text_infos" ) ;
            }

            {
                auto* var = vs.data_variable<motor::math::mat4f_t>( "u_world" ) ;
                var->set( motor::math::mat4f_t().identity() ) ;
            }
            {
                auto* var = vs.data_variable<motor::math::mat4f_t>( "u_view" ) ;
                var->set( motor::math::mat4f_t().identity() ) ;
            }
            {
                auto* var = vs.data_variable<motor::math::mat4f_t>( "u_proj" ) ;
                var->set( motor::math::mat4f_t().identity() ) ;
            }

            _vars.emplace_back( motor::shared( std::move( vs ) ) ) ;
        }
        

        for( size_t i =0; i<_vars.size() ; ++i )
        {
            rc.add_variable_set( motor::share( _vars[i] ) ) ;
        }

        motor::memory::release_ptr( _rc ) ;
        _rc = motor::shared( std::move( rc ) ) ;
     }

    // group infos
    {
        _gis.resize( ng ) ;
    }
}

void_t text_render_2d::on_frame_init( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->configure<motor::graphics::geometry_object_t>( _gc ) ;
    fe->configure<motor::graphics::shader_object_t>( _sc ) ;
    fe->configure<motor::graphics::image_object_t>( _ic ) ;
    fe->configure<motor::graphics::array_object_t>( _glyph_infos ) ;
    fe->configure<motor::graphics::array_object_t>( _text_infos ) ;
    fe->configure<motor::graphics::render_object_t>( _rc ) ;
    fe->configure<motor::graphics::state_object_t>( _sto ) ;
}

void_t text_render_2d::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) 
{
    for( auto & g : _gis )
    {
        g.proj = proj ;
        g.view = view ;
    }

    for( auto & vs : _vars )
    {
        {
            auto* var = vs->data_variable<motor::math::mat4f_t>( "u_view" ) ;
            var->set( view ) ;
        }
        {
            auto* var = vs->data_variable<motor::math::mat4f_t>( "u_proj" ) ;
            var->set( proj ) ;
        }
    }
}

void_t text_render_2d::set_view_proj( size_t const i, motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) 
{
    if( i >= _gis.size() ) return ;

    _gis[i].proj = proj ;
    _gis[i].view = view ;

    {
        auto & vs = _vars[i] ;
        {
            auto* var = vs->data_variable<motor::math::mat4f_t>( "u_view" ) ;
            var->set( view ) ;
        }
        {
            auto* var = vs->data_variable<motor::math::mat4f_t>( "u_proj" ) ;
            var->set( proj ) ;
        }
    }
}

motor::gfx::result text_render_2d::draw_text( size_t const group, size_t const font_id, size_t const point_size, motor::math::vec2f_cref_t pos, motor::math::vec4f_cref_t color, motor::string_cref_t text ) 
{
    if( group >= _gis.size() )
        return motor::gfx::result::invalid_argument ;

    motor::math::vec2f_t const aspect( float_t( _ga->get_width() ) / float_t( _ga->get_height() ), 1.0f ) ;

    motor::concurrent::lock_t lk( _gis[group].mtx ) ;

    float_t adv_x = 0.0f ;
    for( auto const c : text )
    {
        size_t idx ;
        motor::font::glyph_atlas_t::glyph_info_t gi ;
        if( c != ' ' )        
        {
            auto const res = _ga->find_glyph( font_id, motor::font::utf32_t( c ), idx, gi ) ;
            if( !res )
            {
                auto const res2 = _ga->find_glyph( font_id, motor::font::utf32_t( '?' ), idx, gi ) ;
                if( !res2 )
                {
                    motor::log::global_t::error( motor_log_fn( 
                        "glyph atlas requires ? glyph for unknown glyphs" ) ) ;
                    continue ;
                }
            }
        }
        else 
        {
            auto const res2 = _ga->find_glyph( font_id, motor::font::utf32_t( '?' ), idx, gi ) ;
            if( !res2 )
            {
                motor::log::global_t::error( motor_log_fn( 
                    "glyph atlas requires ? glyph for unknown glyphs" ) ) ;
                continue ;
            }

            adv_x += gi.adv.x() ;
            continue ;
        }

        
        float_t const point_size_scale = float_t(point_size) / float_t(gi.point_size) ;

        motor::math::vec2f_t adv = motor::math::vec2f_t( adv_x * point_size_scale, 0.0f ) * aspect ;
        adv_x += gi.adv.x()  ;

        motor::math::vec2f_t pos_ = pos + adv ;

        {
            this_t::glyph_info_t tgi ;
            tgi.color = color ;
            tgi.pos = pos_ ;
            tgi.point_size_scale = point_size_scale ;
            tgi.offset = idx ;
            tgi.adv_x = adv.x() ;

            _gis[group].glyph_infos.emplace_back( tgi ) ;
        }
    }

    return motor::gfx::result::ok ;
}

motor::gfx::result text_render_2d::prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) 
{
    struct the_data
    {
        // v1: ( pos.xy, offset, scale )
        // v2: ( color.xyz, 0.0f )
        motor::math::vec4f_t v1 ;
        motor::math::vec4f_t v2 ;
    };

    // resize text infos
    {
        size_t num_elems = 0 ;
        for( size_t i=0; i<_gis.size(); ++i ) 
        {
            _gis[i].ri = {} ;
            num_elems += _gis[i].glyph_infos.size() ;
        }
        _text_infos->data_buffer().resize( num_elems ) ;
    }

    // encode text infos
    {
        size_t start = 0 ;
        size_t num_glyphs = 0 ;
        for( size_t i=0; i<_gis.size(); ++i )
        {
            if( _gis[i].glyph_infos.size() == 0 ) continue ;

            num_glyphs = _gis[i].glyph_infos.size() ;
            for( size_t g=0; g<num_glyphs; ++g )
            {
                auto const & gi = _gis[i].glyph_infos[g] ;

                motor::math::vec4f_t const v1( gi.pos,
                    motor::math::vec2f_t( float_t( gi.offset ), gi.point_size_scale ) ) ;
                motor::math::vec4f_t const v2( gi.color, gi.adv_x ) ;

                _text_infos->data_buffer().update<the_data>( start + g, { v1, v2 } ) ;
            }
        
            _gis[i].ri.start = start ;
            _gis[i].ri.num_elems = num_glyphs ;

            start += num_glyphs ;

            _gis[i].glyph_infos.clear() ;
        }
    }

    fe->update( _text_infos ) ;

    return motor::gfx::result::ok ;
}

bool_t text_render_2d::need_to_render( size_t const i ) const noexcept
{
    return _gis[i].ri.num_elems != 0 ;
}

motor::gfx::result text_render_2d::render( motor::graphics::gen4::frontend_mtr_t fe, size_t const i ) 
{
    if( i >= _gis.size() ) return motor::gfx::result::invalid_argument ;
    if( !this_t::need_to_render(i) ) return motor::gfx::result::ok ;

    fe->push( _sto ) ;

    // set the offset to the first glyph of this group
    {
        _vars[i]->data_variable< uint32_t >( "u_offset" )->set( uint32_t(_gis[i].ri.start) ) ;
    }

    motor::graphics::gen4::backend_t::render_detail_t detail ;

    detail.start = 0 ;
    detail.num_elems = _gis[i].ri.num_elems * 6 ;
    detail.varset = i ;

    fe->render( _rc, detail ) ;
        
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;

    return motor::gfx::result::ok ;
}

motor::gfx::result text_render_2d::release( void_t ) 
{
    return motor::gfx::result::ok ;
}


