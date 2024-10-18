
#include "sprite_render_2d.h"

#include <motor/math/utility/constants.hpp>

using namespace motor::gfx ;

//**********************************************************************************************************
sprite_render_2d::sprite_render_2d( void_t ) noexcept
{
    _rs = motor::graphics::state_object_t() ;
    _ao = motor::graphics::array_object_t() ;
    _ro = motor::graphics::render_object_t() ;
    _go = motor::graphics::geometry_object_t() ;
    _so = motor::graphics::shader_object_t() ;
}

//**********************************************************************************************************
sprite_render_2d::sprite_render_2d( this_rref_t rhv ) noexcept
{
    _rs = std::move( rhv._rs ) ;
    _ao = std::move( rhv._ao ) ;
    _ro = std::move( rhv._ro ) ;
    _go = std::move( rhv._go ) ;
    _so = std::move( rhv._so ) ;
    _layers = std::move( rhv._layers ) ;

    _name = std::move( rhv._name ) ;
    _image_name = std::move( rhv._image_name ) ;
}

//**********************************************************************************************************
sprite_render_2d::this_ref_t sprite_render_2d::operator = ( this_rref_t rhv ) noexcept
{
    _rs = std::move( rhv._rs ) ;
    _ao = std::move( rhv._ao ) ;
    _ro = std::move( rhv._ro ) ;
    _go = std::move( rhv._go ) ;
    _so = std::move( rhv._so ) ;
    _layers = std::move( rhv._layers ) ;

    _name = std::move( rhv._name ) ;
    _image_name = std::move( rhv._image_name ) ;

    return *this ;
}

//**********************************************************************************************************
sprite_render_2d::~sprite_render_2d( void_t ) noexcept
{
    for ( auto * layer : _layers )
    {
        motor::memory::global_t::dealloc( layer ) ;
    }
}

//**********************************************************************************************************
sprite_render_2d::this_ref_t sprite_render_2d::init( motor::string_cref_t name, motor::string_cref_t image_name ) noexcept
{
    _name = name ;
    _image_name = image_name ;

    // root render states
    {
        motor::graphics::state_object_t so = motor::graphics::state_object_t(
            name + ".render_states" ) ;

        {
            motor::graphics::render_state_sets_t rss ;

            rss.depth_s.do_change = true ;
            rss.depth_s.ss.do_activate = false ;
            rss.depth_s.ss.do_depth_write = false ;

            rss.polygon_s.do_change = true ;
            rss.polygon_s.ss.do_activate = true ;
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
            .add_layout_element( motor::graphics::vertex_attribute::position, motor::graphics::type::tfloat, motor::graphics::type_struct::vec2 )
            .resize( _max_quads * 4 ).update<this_t::vertex>( [&] ( this_t::vertex* array, size_t const ne )
        {
            for( size_t i=0; i<_max_quads; ++i )
            {
                size_t const base = i << 2 ;
                array[ base + 0 ].pos = motor::math::vec2f_t( -0.5f, -0.5f ) ;
                array[ base + 1 ].pos = motor::math::vec2f_t( -0.5f, +0.5f ) ;
                array[ base + 2 ].pos = motor::math::vec2f_t( +0.5f, +0.5f ) ;
                array[ base + 3 ].pos = motor::math::vec2f_t( +0.5f, -0.5f ) ;
            }
        } );

        auto ib = motor::graphics::index_buffer_t().
            set_layout_element( motor::graphics::type::tuint ).resize( _max_quads * 6 ).
            update<uint_t>( [&] ( uint_t* array, size_t const ne )
        {
            for( uint_t i=0; i<uint_t(_max_quads); ++i )
            {
                uint_t const bi = i * 6 ;
                uint_t const bv = i * 4 ;

                array[ bi + 0 ] = bv + 0 ;
                array[ bi + 1 ] = bv + 1 ;
                array[ bi + 2 ] = bv + 2 ;

                array[ bi + 3 ] = bv + 0 ;
                array[ bi + 4 ] = bv + 2 ;
                array[ bi + 5 ] = bv + 3 ;
            }
        } ) ;

        _go = motor::graphics::geometry_object_t( name + ".geometry",
            motor::graphics::primitive_type::triangles, std::move( vb ), std::move( ib ) ) ;
    }

    // array
    {
        motor::graphics::data_buffer_t db = motor::graphics::data_buffer_t()
            .add_layout_element( motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 )
            .resize( 5 ) ;

        _ao = motor::graphics::array_object_t( name + ".per_sprite_data", std::move( db ) ) ;
    }

    // shader configuration
    {
        motor::graphics::shader_object_t sc( name + ".shader" ) ;

        // shaders : ogl 3.1
        {
            motor::graphics::shader_set_t ss = motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 140

                    in vec2 in_pos ;
                    out vec4 var_col ;
                    out vec3 var_uv ;
                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;
                    uniform mat4 u_world ;
                    uniform int u_offset ;
                    uniform samplerBuffer u_data ;

                    void main()
                    {
                        int idx = (gl_VertexID / 4) * 6 + u_offset * 6 ;
                        vec4 d0 = texelFetch( u_data, idx + 0 ) ; // pos scale
                        vec4 d1 = texelFetch( u_data, idx + 1 ) ; // frame
                        vec4 d2 = texelFetch( u_data, idx + 2 ) ; // uv rect
                        vec4 d3 = texelFetch( u_data, idx + 3 ) ; // additional
                        vec4 d4 = texelFetch( u_data, idx + 4 ) ; // uv animation
                        vec4 d5 = texelFetch( u_data, idx + 5 ) ; // color
                        
                        vec2 pivot = d3.xy * d0.zw * vec2( 0.5 ) ;
                        vec2 scale = d0.zw * (d2.zw - d2.xy) ;
                        vec2 pos_p = d0.xy + pivot ;

                        mat2 scaling = mat2( scale.x, 0.0, 0.0, scale.y ) ;
                        mat2 frame = mat2( d1.xy, d1.zw ) ;
    
                        vec2 uvs[4] = vec2[4]( d2.xy, d2.xw, d2.zw, d2.zy ) ;
                        var_uv.xy = uvs[gl_VertexID%4] ;
                        var_uv.z = d3.w ; // which texture layer

                        var_col = d5 ;
                        vec4 pos = vec4( pos_p + frame * scaling * in_pos, 0.0, 1.0 )  ;
                        gl_Position = u_proj * u_view * u_world * pos ;

                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 140

                    in vec4 var_col ;
                    in vec3 var_uv ;
                    out vec4 out_color ;
                    
                    uniform sampler2DArray u_tex ;

                    void main()
                    {    
                        out_color = texture( u_tex, var_uv ) * var_col ;
                    } )" ) ) ;

            sc.insert( motor::graphics::shader_api_type::glsl_4_0, std::move( ss ) ) ;
        }

        // shaders : es 3.0
        {
            motor::graphics::shader_set_t ss = motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    precision mediump float ;
                    layout( location = 0 ) in vec2 in_pos ;
                    out vec4 var_col ;
                    out vec3 var_uv ;
                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;
                    uniform mat4 u_world ;
                    uniform int u_offset ;
                    uniform samplerBuffer u_data ;

                    void main()
                    {
                        ivec2 wh = textureSize( u_data, 0 ) ;

                        int idx = (gl_VertexID / 4) * 6 + u_offset * 6 ;
                        vec4 d0 = texelFetch( u_data, idx + 0 ) ; // pos scale
                        vec4 d1 = texelFetch( u_data, idx + 1 ) ; // frame
                        vec4 d2 = texelFetch( u_data, idx + 2 ) ; // uv rect
                        vec4 d3 = texelFetch( u_data, idx + 3 ) ; // additional
                        vec4 d4 = texelFetch( u_data, idx + 4 ) ; // uv animation
                        vec4 d5 = texelFetch( u_data, idx + 5 ) ; // color

                        vec2 pivot = d3.xy * d0.zw * vec2( 0.5 ) ;
                        vec2 scale = d0.zw * (d2.zw - d2.xy) ;
                        vec2 pos_p = d0.xy + pivot ;

                        mat2 scaling = mat2( scale.x, 0.0, 0.0, scale.y ) ;
                        mat2 frame = mat2( d1.xy, d1.zw ) ;
    
                        vec2 uvs[4] = vec2[4]( d2.xy, d2.xw, d2.zw, d2.zy ) ;
                        var_uv.xy = uvs[gl_VertexID%4] ;
                        var_uv.z = d3.w ; // which texture layer
                        //var_uv.xy = sign( in_pos.xy ) *0.5 + 0.5 ;

                        var_col = d5 ;
                        vec4 pos = vec4( pos_p + frame * scaling * in_pos, 0.0, 1.0 )  ;
                        gl_Position = u_proj * u_view * u_world * pos ;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    precision mediump float ;
                    precision mediump sampler2DArray ;

                    in vec4 var_col ;
                    in vec3 var_uv ;
                    layout( location = 0 )out vec4 out_color ;

                    uniform sampler2DArray u_tex ;

                    void main()
                    {
                        out_color = texture( u_tex, var_uv ) * var_col ;
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
                        float2 in_pos : POSITION ; 
                    } ;
                    struct VS_OUTPUT
                    {
                        float4 pos : SV_POSITION;
                        float4 col : COLOR0 ;
                        float3 uv : TEXCOORD0 ;
                    };
                            
                    Buffer< float4 > u_data ;

                    VS_OUTPUT VS( VS_INPUT input )
                    {
                        VS_OUTPUT output = (VS_OUTPUT)0 ;
                        int idx = (input.in_id / 4) * 6 + u_offset * 6 ;
                        float4 d0 = u_data.Load( idx + 0 ) ; // pos scale
                        float4 d1 = u_data.Load( idx + 1 ) ; // frame
                        float4 d2 = u_data.Load( idx + 2 ) ; // uv rect
                        float4 d3 = u_data.Load( idx + 3 ) ; // additional
                        float4 d4 = u_data.Load( idx + 4 ) ; // uv animation
                        float4 d5 = u_data.Load( idx + 5 ) ; // color
                        
                        float2 pivot = d3.xy * d0.zw * float2( 0.5, 0.5 ) ;
                        float2 scale = d0.zw * (d2.zw - d2.xy) ;
                        float2 pos_p = d0.xy + pivot ;

                        float2x2 frame = { d1.x, d1.y, 
                                           d1.z, d1.w } ;

                        float2 uvs[4] = { d2.xy, d2.xw, d2.zw, d2.zy } ;
                        output.uv.xy = uvs[input.in_id % 4] ;
                        output.uv.z = d3.w ;

                        output.col = d5 ;

                        float2 pos = mul( input.in_pos * scale, frame ) ; 
                        output.pos = float4( pos_p + pos, 0.0f, 1.0f )  ;
                        output.pos = mul( output.pos, u_world ) ;
                        output.pos = mul( output.pos, u_view ) ;
                        output.pos = mul( output.pos, u_proj ) ;
                        
                        // test with full screen quad
                        //output.pos = float4( sign( input.in_pos ), 0.0, 1.0 ) ;
                        //output.uv.xy = float2( sign( input.in_pos ) * 0.5 + 0.5 ) ;

                        return output;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    
                    Texture2DArray u_tex : register( t0 ) ;
                    SamplerState smp_u_tex : register( s0 ) ;

                    struct VS_OUTPUT
                    {
                        float4 Pos : SV_POSITION ;
                        float4 col : COLOR0 ;
                        float3 uv : TEXCOORD0 ;
                    };

                    float4 PS( VS_OUTPUT input ) : SV_Target0
                    {
                        return u_tex.Sample( smp_u_tex, input.uv ) * input.col ;
                    } )" ) ) ;

            sc.insert( motor::graphics::shader_api_type::hlsl_5_0, std::move( ss ) ) ;
        }

        // configure more details
        {
            sc.shader_bindings()
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

    return *this ;
}

//**********************************************************************************************************
void_t sprite_render_2d::release( void_t ) noexcept 
{
}

//**********************************************************************************************************
void_t sprite_render_2d::set_texture( motor::string_cref_t name ) noexcept 
{
    _image_name = name ;
}

//**********************************************************************************************************
void_t sprite_render_2d::draw( size_t const l, motor::math::vec2f_cref_t pos, motor::math::mat2f_cref_t frame, 
    motor::math::vec2f_cref_t scale, motor::math::vec4f_cref_t uv_rect, 
    size_t const slot, motor::math::vec2f_cref_t pivot, motor::math::vec4f_cref_t color ) noexcept 
{
    auto * layer = this_t::add_layer( l ) ;

    {
        motor::concurrent::lock_guard_t lk( _num_sprites_mtx ) ;
        ++_num_sprites ;
    }

    {
        motor::concurrent::mrsw_t::writer_lock_t lk( layer->mtx ) ;
        layer->sprites[ layer->sprites.resize_by( 1, 1000 ) ] = this_t::sprite_t 
        { 
            pos, frame, scale, uv_rect, pivot, color, slot
        } ;
    }
}

//**********************************************************************************************************
sprite_render_2d::this_ref_t sprite_render_2d::configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept
{
    fe->configure<motor::graphics::geometry_object_t>( &_go ) ;
    fe->configure<motor::graphics::shader_object_t>( &_so ) ;
    fe->configure<motor::graphics::array_object_t>( &_ao) ;
    fe->configure<motor::graphics::render_object_t>( &_ro ) ;
    fe->configure<motor::graphics::state_object_t>( &_rs ) ;

    return *this ;
}

//**********************************************************************************************************
void_t sprite_render_2d::prepare_for_rendering( void_t ) noexcept 
{
    this_t::prepare_update pe ;

    if( _num_sprites == 0 ) return ;

    // 1. prepare render data
    {
        size_t const num_vs = (_num_sprites / _max_quads) + _layers.size() ;
        _render_data.resize( num_vs ) ;
        _render_layer_infos.resize( _layers.size() ) ;

        size_t rd_idx = 0 ;
        
        size_t rd_start = 0 ;

        for( size_t i=0; i<_layers.size(); ++i )
        {
            auto const & sprites = _layers[i]->sprites ;
            
            // number of render calls for this layer
            size_t const num_render = sprites.size() / _max_quads ; 

            for( size_t r=0; r< num_render; ++r, ++rd_idx )
            {
                _render_data[rd_idx].num_quads = _max_quads ;
                _render_data[rd_idx].num_elems = _max_quads * 6 ;
            }

            size_t const residuals = sprites.size() % _max_quads ;
            if( residuals != 0 )
            {
                _render_data[rd_idx].num_quads = residuals ;
                _render_data[rd_idx].num_elems = residuals * 6 ;
                ++rd_idx ;
            }

            this_t::render_layer_info rli = { rd_start, rd_idx } ;
            _render_layer_infos[i] = std::move( rli ) ;

            rd_start = rd_idx ;
        }
    }

    // 2. prepare variable sets 
    // one var set per render pass per layer
    {
        for( size_t i=_ro.get_num_variable_sets(); i<_render_data.size(); ++i )
        {
            this_t::add_variable_set( _ro ) ;
            pe.reconfig_ro = true ;
        }

        int_t offset = 0 ;
        for( size_t i=0; i<_render_data.size(); ++i )
        {
            _ro.borrow_variable_set(i)->data_variable<int32_t>( "u_offset" )->set( offset ) ;
            offset += int32_t( _render_data[i].num_quads ) ;
        }

        if( _image_name_changed || pe.reconfig_ro )
        {
            for( size_t i=0; i<_render_data.size(); ++i )
            {
                _ro.borrow_variable_set(i)->texture_variable( "u_tex" )->set( _image_name ) ;
            }
            _image_name_changed = false ;
        }

        if( _reset_view_proj )
        {
            _reset_view_proj = false ;
            for( size_t i=0; i<_render_data.size(); ++i )
            {
                {
                    auto* var = _ro.borrow_variable_set(i)->data_variable<motor::math::mat4f_t>( "u_view" ) ;
                    var->set( _view ) ;
                }
                {
                    auto* var = _ro.borrow_variable_set(i)->data_variable<motor::math::mat4f_t>( "u_proj" ) ;
                    var->set( _proj ) ;
                }
            }
        }
    }

    // 3. copy data
    {
        size_t const bsib = _ao.data_buffer().get_sib() ;

        size_t const sizeof_data = sizeof(this_t::the_data) / sizeof( motor::math::vec4f_t ) ;
        if( _ao.data_buffer().get_num_elements() < (_num_sprites * sizeof_data) )
            _ao.data_buffer().resize( _num_sprites * sizeof_data ) ;

        size_t lstart = 0 ;

        for( size_t l=0; l<_layers.size(); ++l )
        {
            auto const & sprites = _layers[l]->sprites ;
            
            for( size_t i=0; i<sprites.size(); ++i )
            {
                size_t const idx = lstart + i * sizeof_data ;
                _ao.data_buffer().update< motor::math::vec4f_t >( idx + 0, 
                    motor::math::vec4f_t( sprites[i].pos, sprites[i].scale ) ) ;

                _ao.data_buffer().update< motor::math::vec4f_t >( idx + 1, 
                    motor::math::vec4f_t( sprites[i].frame.column(0), sprites[i].frame.column(1) ) ) ;

                _ao.data_buffer().update< motor::math::vec4f_t >( idx + 2, 
                    sprites[i].uv_rect ) ;

                // additional infos
                _ao.data_buffer().update< motor::math::vec4f_t >( idx + 3, 
                    motor::math::vec4f_t( sprites[i].pivot.x(), sprites[i].pivot.y(), 0.0f, float_t( sprites[i].slot ) ) ) ;

                // uv animation
                _ao.data_buffer().update< motor::math::vec4f_t >( idx + 4, 
                    motor::math::vec4f_t(1.0f) ) ;

                // color
                _ao.data_buffer().update< motor::math::vec4f_t >( idx + 5, 
                    sprites[i].color ) ;

                //_ao.data_buffer().update< motor::math::vec4f_t >( idx + 3, 
                    //  motor::math::vec4f_t( sprites[i].pos, sprites[i].scale ) ) ;
            }
            lstart += sprites.size() * sizeof_data ;
            
            _layers[l]->sprites.clear() ;
        }
        
        pe.data_realloc = _ao.data_buffer().get_sib() > bsib ;
    }

    _num_sprites = 0 ;
    _pe = pe ;
}

//**********************************************************************************************************
void_t sprite_render_2d::prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    if( _pe.data_realloc ) fe->configure<motor::graphics::array_object_t>( &_ao ) ;
    else fe->update( &_ao ) ;

    if( _pe.reconfig_ro ) fe->configure<motor::graphics::render_object_t>( &_ro ) ;
}

//**********************************************************************************************************
void_t sprite_render_2d::render( motor::graphics::gen4::frontend_mtr_t fe, size_t const l ) noexcept 
{
    fe->push( &_rs ) ;
    if( _render_layer_infos.size() > l ) 
    {
        auto const & rli = _render_layer_infos[l] ;

        for( size_t idx= rli.start; idx < rli.end; ++idx )
        {
            auto const & plrd = _render_data[idx] ;

            motor::graphics::gen4::backend::render_detail rd ;
            rd.start = 0 ;
            rd.num_elems = plrd.num_elems ;
            rd.varset = idx ;

            fe->render( &_ro, rd ) ;
        }
    }
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
}

//**********************************************************************************************************
void_t sprite_render_2d::add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept 
{
    motor::graphics::variable_set_t vars ;
            
    {
        auto* var = vars.array_variable( "u_data" ) ;
        var->set( _name + ".per_sprite_data" ) ;
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
    {
        auto* var = vars.texture_variable( "u_tex" ) ;
        var->set( _image_name ) ;
    }
    rc.add_variable_set( motor::shared( std::move( vars ) ) ) ;
}

//**********************************************************************************************************
void_t sprite_render_2d::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept 
{
    _view = view ;
    _proj = proj ;
    _reset_view_proj = true ;
}

//**********************************************************************************************************
sprite_render_2d::layer_ptr_t sprite_render_2d::add_layer( size_t const l ) noexcept
{
    motor::concurrent::lock_guard_t lk( _layers_mtx ) ;
    if ( _layers.size() <= l )
    {
        size_t const old_size = _layers.size() ;

        _layers.resize( l + 1 ) ;

        for ( size_t i = old_size; i < _layers.size(); ++i )
        {
            _layers[ i ] = motor::memory::global_t::alloc( this_t::layer() ) ;
        }
    }
    return _layers[ l ] ;
}