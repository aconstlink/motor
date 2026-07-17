
#include "bloom_stage.h"

using namespace motor::gfx;

//********************************************************
bloom_stage::bloom_stage( void_t ) noexcept {}

//********************************************************
bloom_stage::bloom_stage( this_rref_t rhv ) noexcept
    : _msl_down( motor::move( rhv._msl_down ) ), _msl_up( motor::move( rhv._msl_up ) ),
      _brg( motor::move( rhv._brg ) )
{
}

//********************************************************
void_t bloom_stage::change_resolution( uint_t const w, uint_t const h ) noexcept
{
    for( size_t i = 0; i < size_t( level_type::num_level ); ++i )
    {
        uint_t const lw_down = w >> i;
        uint_t const lh_down = h >> i;

        uint_t const lw_up = w >> ( i - 1 );
        uint_t const lh_up = h >> ( i - 1 );

        if( _per_level[ i ].so_down == nullptr )
        {
            auto so = motor::graphics::state_object_t(
                "gfx.postprocess.bloom.downsample." + motor::to_string( i ) );

            motor::graphics::render_state_sets_t rss;
            rss.view_s.do_change = true;
            rss.view_s.ss.do_activate = true;
            rss.view_s.ss.vp = motor::math::vec4ui_t( 0, 0, lw_down, lh_down );
            so.add_render_state_set( rss );

            _per_level[ i ].so_down = motor::shared( std::move( so ) );
        }
        else
        {
            _per_level[ i ].so_down->access_render_state( 0,
                [ & ]( motor::graphics::render_state_sets_ref_t rss )
            {
                rss.view_s.ss.vp = motor::math::vec4ui_t( 0, 0, lw_down, lh_down );

                return true;
            } );
        }

        if( _per_level[ i ].so_up == nullptr )
        {
            auto so = motor::graphics::state_object_t(
                "gfx.postprocess.bloom.upsample." + motor::to_string( i ) );

            motor::graphics::render_state_sets_t rss;
            rss.view_s.do_change = true;
            rss.view_s.ss.do_activate = true;
            rss.view_s.ss.vp = motor::math::vec4ui_t( 0, 0, lw_up, lh_up );
            so.add_render_state_set( rss );

            _per_level[ i ].so_up = motor::shared( std::move( so ) );
        }
        else
        {
            _per_level[ i ].so_up->access_render_state( 0,
                [ & ]( motor::graphics::render_state_sets_ref_t rss )
            {
                rss.view_s.ss.vp = motor::math::vec4ui_t( 0, 0, lw_up, lh_up );

                return true;
            } );
        }

        if( _per_level[ i ].vs == nullptr )
        {
            motor::graphics::variable_set_t vs;

            {
                auto * var = vs.texture_variable( "tx_for_downsample" );
                var->set( "" );
            }

            {
                auto * var = vs.texture_variable( "tx_for_upsample" );
                var->set( "" );
            }

            auto vs_ptr = motor::shared( std::move( vs ), "a variable set in bloom stage" );
            _msl_down->add_variable_set( motor::share( vs_ptr ) );
            _msl_up->add_variable_set( motor::share( vs_ptr ) );

            _per_level[ i ].vs = motor::move( vs_ptr );
        }
        else
        {
        }
    }
}

//********************************************************
void_t bloom_stage::init( uint_t const w, uint_t const h ) noexcept
{

    // init msl
    // shaders for post process

    // post quad object
    {
        motor::graphics::msl_object_t mslo( "gfx.postprocess.stage.bloom.downsample" );

        mslo.add( motor::graphics::msl_api_type::msl_4_0, R"(
            
                config gfx.postprocess.stage.bloom.downsample
                {
                    vertex_shader
                    {
                        in vec2_t pos : position ;

                        out vec2_t tx : texcoord0 ;
                        out vec4_t pos : position ;

                        void main()
                        {
                            out.tx = sign( in.pos.xy ) * 0.5 + 0.5 ;
                            out.pos = vec4_t( sign( in.pos.xy ), 0.0, 1.0 ) ; 
                        }
                    }

                    pixel_shader
                    {
                        in vec2_t tx : texcoord0 ;
                        out vec4_t color : color ;

                        tex2d_t tx_for_downsample ;

                        void main()
                        {
                            vec2_t texel = vec2_t(1.0,1.0) / texture_dims( tx_for_downsample ) ;
                            vec2_t uv = in.tx ;

                            vec3_t c = vec3_t(0.0,0.0,0.0);
                            vec2_t uv0 = uv + texel ' vec2_t(-1.0, -1.0) ;
                            vec2_t uv1 = uv + texel ' vec2_t( 1.0, -1.0) ;
                            vec2_t uv2 = uv + texel ' vec2_t(-1.0,  1.0) ;
                            vec2_t uv3 = uv + texel ' vec2_t( 1.0,  1.0) ;

                            c += rt_texture(tx_for_downsample, uv0 ).xyz;
                            c += rt_texture(tx_for_downsample, uv1 ).xyz;
                            c += rt_texture(tx_for_downsample, uv2 ).xyz;
                            c += rt_texture(tx_for_downsample, uv3 ).xyz;
                            c *= 0.25;

                            out.color = vec4_t( c, 1.0 ) ;
                            //out.color = rt_texture(tx_for_downsample, in.tx ) ;
                            //out.color = vec4_t( in.tx, 0.0, 1.0) ;
                        }
                    }
                } )" );

        mslo.link_geometry( "gfx.postprocess.quad" );

        _msl_down = motor::shared( std::move( mslo ) );
    }

    // post quad object
    {
        motor::graphics::msl_object_t mslo( "gfx.postprocess.stage.bloom.upsample" );

        mslo.add( motor::graphics::msl_api_type::msl_4_0, R"(
            
                config gfx.postprocess.stage.bloom.upsample
                {
                    vertex_shader
                    {
                        in vec2_t pos : position ;

                        out vec2_t tx : texcoord0 ;
                        out vec4_t pos : position ;

                        void main()
                        {
                            out.tx = sign( in.pos.xy ) * 0.5 + 0.5 ;
                            out.pos = vec4_t( sign( in.pos.xy ), 0.0, 1.0 ) ; 
                        }
                    }

                    pixel_shader
                    {
                        in vec2_t tx : texcoord0 ;
                        out vec4_t color : color ;

                        tex2d_t tx_for_upsample ;

                        void main()
                        {
                            vec2_t texel = vec2_t(1.0,1.0) / texture_dims( tx_for_upsample ) ;
                            vec2_t uv = in.tx ;
                            float_t r = 3.0 ;

                            vec3_t c = vec3_t(0.0,0.0,0.0);

                            vec2_t uv0 = uv + texel ' vec2_t(-1.0, -1.0) * r ;
                            vec2_t uv1 = uv + texel ' vec2_t( 0.0, -1.0) * r ;
                            vec2_t uv2 = uv + texel ' vec2_t( 1.0, -1.0) * r ;

                            vec2_t uv3 = uv + texel ' vec2_t(-1.0, 0.0) * r ;
                            vec2_t uv4 = uv + texel ' vec2_t( 0.0, 0.0) ;
                            vec2_t uv5 = uv + texel ' vec2_t( 1.0, 0.0) * r ;

                            vec2_t uv6 = uv + texel ' vec2_t(-1.0, 1.0) * r ;
                            vec2_t uv7 = uv + texel ' vec2_t( 0.0, 1.0) * r ;
                            vec2_t uv8 = uv + texel ' vec2_t( 1.0, 1.0) * r ;

                            c += rt_texture(tx_for_upsample, uv0 ).xyz ' vec3_t(1.0,1.0,1.0);
                            c += rt_texture(tx_for_upsample, uv1 ).xyz ' vec3_t(2.0,2.0,2.0);
                            c += rt_texture(tx_for_upsample, uv2 ).xyz ' vec3_t(1.0,1.0,1.0);

                            c += rt_texture(tx_for_upsample, uv3 ).xyz ' vec3_t(2.0,2.0,2.0);
                            c += rt_texture(tx_for_upsample, uv4 ).xyz ' vec3_t(4.0,4.0,4.0);
                            c += rt_texture(tx_for_upsample, uv5 ).xyz ' vec3_t(2.0,2.0,2.0);

                            c += rt_texture(tx_for_upsample, uv6 ).xyz ' vec3_t(1.0,1.0,1.0);
                            c += rt_texture(tx_for_upsample, uv7 ).xyz ' vec3_t(2.0,2.0,2.0);
                            c += rt_texture(tx_for_upsample, uv8 ).xyz ' vec3_t(1.0,1.0,1.0);
                            c *= 1.0/16.0;

                            out.color = vec4_t( c, 1.0 ) ;
                            //out.color = rt_texture(tx_for_downsample, in.tx ) ;
                            //out.color = vec4_t( in.tx, 0.0, 1.0) ;
                        }
                    }
                } )" );

        mslo.link_geometry( "gfx.postprocess.quad" );

        _msl_up = motor::shared( std::move( mslo ) );
    }

    this_t::change_resolution( w, h );

#if 0
    // variable sets
    {
        motor::graphics::variable_set_t vars;

        {
            auto * var = vars.texture_variable( "tx_map" );
            var->set( rt_name );
        }

        {
            auto * var = vars.data_variable< float_t >( "brightness_threshold" );
            var->set( 10.0f );
        }

        {
            auto * var = vars.data_variable< float_t >( "brightness_knee_percent" );
            var->set( 0.3f );
        }

        auto vs_ptr = motor::shared( std::move( vars ), "a variable set" );
        _msl_down->add_variable_set( motor::share( vs_ptr ) );

        _brg = motor::shared( motor::graphics::wire_variable_bridge_t( motor::move( vs_ptr ) ) );
        _brg->update_bindings();

        {
            motor::property::property_sheet_t ps;

            using is_float_t = motor::wire::input_slot< float_t >;

            {
                motor::property::add_is_property< float_t >( "brightness_threshold",
                    _brg->borrow_inputs()->borrow( "brightness_threshold" ), ps );

                {
                    auto * prop = ps.borrow_property< is_float_t >( "brightness_threshold" );
                    prop->set_min_max( motor::property::min_max< float_t >( 1.0f, 20.0f ) );
                }
            }

            {
                motor::property::add_is_property< float_t >( "brightness_knee_percent",
                    _brg->borrow_inputs()->borrow( "brightness_knee_percent" ), ps );

                {
                    auto * prop = ps.borrow_property< is_float_t >( "brightness_knee_percent" );
                    prop->set_min_max( motor::property::min_max< float_t >( 0.1f, 0.7f ) );
                }
            }

            _prop_sheet = motor::shared( std::move( ps ) );
        }
    }
#endif
}

//********************************************************
void_t bloom_stage::release( void_t ) noexcept
{
    motor::release( motor::move( _msl_down ) );
    motor::release( motor::move( _msl_up ) );
    motor::release( motor::move( _brg ) );
    motor::release( motor::move( _prop_sheet ) );

    for( auto & pld : _per_level )
    {
        motor::release( motor::move( pld.so_down ) );
        motor::release( motor::move( pld.so_up ) );
        motor::release( motor::move( pld.vs ) ) ;
    }
}

//********************************************************
void_t bloom_stage::init_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    fe->configure< motor::graphics::msl_object_t >( _msl_down );
    fe->configure< motor::graphics::msl_object_t >( _msl_up );

    for( auto & pld : _per_level )
    {
        fe->configure< motor::graphics::state_object_t >( pld.so_down );
        fe->configure< motor::graphics::state_object_t >( pld.so_up );
    }
}

//********************************************************
void_t bloom_stage::release_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    fe->release< motor::graphics::msl_object_t >( motor::move( _msl_down ) );
    fe->release< motor::graphics::msl_object_t >( motor::move( _msl_up ) );

    for( auto & pld : _per_level )
    {
        fe->release< motor::graphics::state_object_t >( motor::move( pld.so_down ) );
        fe->release< motor::graphics::state_object_t >( motor::move( pld.so_up ) );
    }
}

//********************************************************
void_t bloom_stage::set_read_render_target_for_down(
    this_t::level_type const lt, motor::string_in_t name ) noexcept
{
    size_t const idx = this_t::to_idx( lt );
    auto * var = _per_level[ idx ].vs->texture_variable( "tx_for_downsample" );
    var->set( name );
}

//********************************************************
void_t bloom_stage::set_read_render_target_for_up(
    this_t::level_type const lt, motor::string_in_t name ) noexcept
{
    size_t const idx = this_t::to_idx( lt );
    auto * var = _per_level[ idx ].vs->texture_variable( "tx_for_upsample" );
    var->set( name );
}

//********************************************************
void_t bloom_stage::render_down(
    this_t::level_type const lt, motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    //_brg->update_bindings();

    auto & pld = _per_level[ size_t( lt ) ];

    fe->push( pld.so_down );
    motor::graphics::gen4::backend::render_detail det;
    det.varset = this_t::to_idx( lt );
    fe->render( _msl_down, det );
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
}

//********************************************************
void_t bloom_stage::render_up(
    this_t::level_type const lt, motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    //_brg->update_bindings();

    auto & pld = _per_level[ size_t( lt ) ];

    fe->push( pld.so_up );
    motor::graphics::gen4::backend::render_detail det;
    det.varset = this_t::to_idx( lt );
    fe->render( _msl_up, det );
    fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
}

//********************************************************
motor::wire::inputs_mtr_t bloom_stage::borrow_inputs( void_t ) noexcept
{
    return _brg->borrow_inputs();
}

//********************************************************
motor::property::property_sheet_mtr_t bloom_stage::borrow_properties( void_t ) noexcept
{
    return _prop_sheet;
}