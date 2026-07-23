
#include "hdr_postprocess_pipeline.h"

using namespace motor::gfx;

//***************************************************
hdr_postprocess_pipeline::hdr_postprocess_pipeline( uint_t const w, uint_t const h ) noexcept
    : _post_fb_dims( w, h )
{
}

//***************************************************
hdr_postprocess_pipeline::hdr_postprocess_pipeline( this_rref_t rhv ) noexcept
    : _post_so( motor::move( rhv._post_so ) ), _post_quad( motor::move( rhv._post_quad ) ),
      _post_fbs( std::move( rhv._post_fbs ) ), _mts_so( motor::move( rhv._mts_so ) ),
      _hdr_so( motor::move( rhv._hdr_so ) ), _msl( motor::move( rhv._msl ) )
{
    _hdr_fbs[ 0 ] = motor::move( rhv._hdr_fbs[ 0 ] );
    _hdr_fbs[ 1 ] = motor::move( rhv._hdr_fbs[ 1 ] );
}

//***************************************************
hdr_postprocess_pipeline::~hdr_postprocess_pipeline( void_t ) noexcept {}

//***************************************************
motor::graphics::framebuffer_object_mtr_t hdr_postprocess_pipeline::borrow_hdr_fb(
    size_t const idx ) noexcept
{
    if( idx >= 2 ) return nullptr;
    return _hdr_fbs[ idx ];
}

//***************************************************
motor::graphics::state_object_mtr_t hdr_postprocess_pipeline::borrow_hdr_states( void_t ) noexcept
{
    return _hdr_so;
}

//***************************************************
void_t hdr_postprocess_pipeline::on_resize( uint_t const w, uint_t const h ) noexcept
{
    _dims = motor::math::vec2ui_t( w, h );
    _size_changed = true;
}

//***************************************************
void_t hdr_postprocess_pipeline::init( void_t ) noexcept
{
    // init quad
    {
        struct vertex
        {
            motor::math::vec2f_t pos;
        };

        auto vb = motor::graphics::vertex_buffer_t()
                      .add_layout_element( motor::graphics::vertex_attribute::position,
                          motor::graphics::type::tfloat, motor::graphics::type_struct::vec2 )
                      .resize( 4 )
                      .update< vertex >( [ = ]( vertex * array, size_t const ne )
        {
            array[ 0 ].pos = motor::math::vec2f_t( -0.5f, -0.5f );
            array[ 1 ].pos = motor::math::vec2f_t( -0.5f, +0.5f );
            array[ 2 ].pos = motor::math::vec2f_t( +0.5f, +0.5f );
            array[ 3 ].pos = motor::math::vec2f_t( +0.5f, -0.5f );
        } );

        auto ib = motor::graphics::index_buffer_t()
                      .set_layout_element( motor::graphics::type::tuint )
                      .resize( 6 )
                      .update< uint_t >( []( uint_t * array, size_t const ne )
        {
            array[ 0 ] = 0;
            array[ 1 ] = 1;
            array[ 2 ] = 2;

            array[ 3 ] = 0;
            array[ 4 ] = 2;
            array[ 5 ] = 3;
        } );

        _post_quad = motor::shared( motor::graphics::geometry_object_t( "gfx.postprocess.quad",
            motor::graphics::primitive_type::triangles, std::move( vb ), std::move( ib ) ) );
    }

    {
        motor::graphics::state_object_t so =
            motor::graphics::state_object_t( "gfx.postprocess.render_state" );

        {
            motor::graphics::render_state_sets_t rss;
            rss.depth_s.do_change = false;
            rss.depth_s.ss.do_activate = false;
            rss.depth_s.ss.do_depth_write = false;
            rss.polygon_s.do_change = false;
            rss.polygon_s.ss.do_activate = true;
            rss.polygon_s.ss.fm = motor::graphics::fill_mode::fill;
            rss.polygon_s.ss.ff = motor::graphics::front_face::clock_wise;
            rss.polygon_s.ss.cm = motor::graphics::cull_mode::back;
            rss.clear_s.do_change = false;
            rss.clear_s.ss.clear_color = motor::math::vec4f_t( 0.5f, 0.5f, 0.5f, 1.0f );
            rss.clear_s.ss.do_activate = false;
            rss.clear_s.ss.do_color_clear = true;
            rss.clear_s.ss.do_depth_clear = true;
            rss.view_s.do_change = true;
            rss.view_s.ss.do_activate = true;
            rss.view_s.ss.vp = motor::math::vec4ui_t( 0, 0, _post_fb_dims.x(), _post_fb_dims.y() );
            so.add_render_state_set( rss );
        }

        _post_so = motor::shared( std::move( so ) );
    }

    // map to screen
    {
        motor::graphics::state_object_t so =
            motor::graphics::state_object_t( "gfx.postprocess.map_to_screen" );

        {
            motor::graphics::render_state_sets_t rss;
            rss.depth_s.do_change = true;
            rss.depth_s.ss.do_activate = false;
            rss.depth_s.ss.do_depth_write = false;
            rss.polygon_s.do_change = true;
            rss.polygon_s.ss.do_activate = true;
            rss.polygon_s.ss.fm = motor::graphics::fill_mode::fill;
            rss.polygon_s.ss.ff = motor::graphics::front_face::clock_wise;
            rss.polygon_s.ss.cm = motor::graphics::cull_mode::back;
            rss.clear_s.do_change = true;
            rss.clear_s.ss.clear_color = motor::math::vec4f_t( 0.5f, 0.5f, 0.5f, 1.0f );
            rss.clear_s.ss.do_activate = true;
            rss.clear_s.ss.do_color_clear = true;
            rss.clear_s.ss.do_depth_clear = true;
            rss.view_s.do_change = false;
            rss.view_s.ss.do_activate = false;
            rss.view_s.ss.vp = motor::math::vec4ui_t( 0, 0, 50, 50 );
            so.add_render_state_set( rss );
        }

        _mts_so = motor::shared( std::move( so ) );
    }

    // init msl
    // shaders for post process
    {
        // post quad object
        {
            motor::graphics::msl_object_t mslo( "gfx.postprocess.color_to_screen" );

            mslo.add( motor::graphics::msl_api_type::msl_4_0, R"(
            
            config gfx.postprocess.color_to_screen
            {
                vertex_shader
                {
                    in vec2_t pos : position ;

                    out vec2_t tx : texcoord0 ;
                    out vec4_t pos : position ;

                    void main()
                    {
                        out.tx = sign( in.pos.xy ) *0.5 + 0.5 ;
                        out.pos = vec4_t( sign( in.pos.xy ), 0.0, 1.0 ) ; 
                    }
                }

                pixel_shader
                {
                    in vec2_t tx : texcoord0 ;
                    out vec4_t color : color ;

                    tex2d_t tx_map ;

                    void main()
                    {
                        vec4_t col = rt_texture( tx_map, in.tx ) ;
                        out.color = col ;
                        
                        float_t d = col.r ;
                        //out.color = vec4_t( as_vec3(pow(d,10)), 1.0 ) ;
                        //out.color = vec4_t( col.r, col.r, col.r ,1.0) ;
                        //out.color = vec4_t( in.tx, 0.0, 1.0) ;
                        //if( in.tx.x > 0.5 )
                          //  out.color = vec4_t( 0.0,0.0, 0.0, 1.0) ;
                    }
                }
            } )" );

            mslo.link_geometry( "gfx.postprocess.quad" );

            _msl = motor::shared( std::move( mslo ) );
        }

        // variable sets
        {
            {
                motor::graphics::variable_set_t vars;

                {
                    auto * var = vars.texture_variable( "tx_map" );
                    // var->set( "gfx.postprocess.framebuffer.0" );
                    var->set( "gfx.postprocess.fb.full.hdr.0.0" );
                }

                _msl->add_variable_set(
                    motor::memory::create_ptr( std::move( vars ), "a variable set" ) );
            }

            // use this varset for temporary display
            {
                motor::graphics::variable_set_t vars;

                {
                    auto * var = vars.texture_variable( "tx_map" );
                    // set as dummy, so texture variable will be found in the backend
                    // if not found during configure, the variable can not be set 
                    // at the moment. @see motor #154
                    var->set( "gfx.postprocess.fb.full.hdr.0.0" );
                }

                _msl->add_variable_set(
                    motor::memory::create_ptr( std::move( vars ), "a variable set" ) );
            }
        }
    }

    // init framebuffer
    // the framebuffers we do the post processing in.
    {
        size_t const w = _post_fb_dims.x();
        size_t const h = _post_fb_dims.y();

        // full hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.full.hdr.0" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 ).resize( w, h );

            _post_fbs[ size_t( framebuffer_type::full_hdr_0 ) ] = motor::shared( std::move( fb ) );
        }

        // full hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.full.hdr.1" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 ).resize( w, h );

            _post_fbs[ size_t( framebuffer_type::full_hdr_1 ) ] = motor::shared( std::move( fb ) );
        }

        // full hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.full.hdr.2" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 ).resize( w, h );

            _post_fbs[ size_t( framebuffer_type::full_hdr_2 ) ] = motor::shared( std::move( fb ) );
        }

        // full hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.full.hdr.3" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 ).resize( w, h );

            _post_fbs[ size_t( framebuffer_type::full_hdr_3 ) ] = motor::shared( std::move( fb ) );
        }

        // half hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.half.hdr.0" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 )
                .resize( w >> 1, h >> 1 );

            _post_fbs[ size_t( framebuffer_type::half_hdr ) ] = motor::shared( std::move( fb ) );
        }

        // quater hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.quater.hdr.0" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 )
                .resize( w >> 2, h >> 2 );

            _post_fbs[ size_t( framebuffer_type::quater_hdr ) ] = motor::shared( std::move( fb ) );
        }

        // eigth hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.eigth.hdr.0" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 )
                .resize( w >> 3, h >> 3 );

            _post_fbs[ size_t( framebuffer_type::eigth_hdr ) ] = motor::shared( std::move( fb ) );
        }

        // sixteenth hdr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.sixteenth.hdr.0" );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 1 )
                .resize( w >> 4, h >> 4 );

            _post_fbs[ size_t( framebuffer_type::sixteenth_hdr ) ] =
                motor::shared( std::move( fb ) );
        }

        // full ldr framebuffer
        {
            auto fb = motor::graphics::framebuffer_object_t( "gfx.postprocess.fb.full.ldr.0" );
            fb.set_target( motor::graphics::color_target_type::rgba_uint_8, 1 ).resize( w, h );

            _post_fbs[ size_t( framebuffer_type::full_ldr ) ] = motor::shared( std::move( fb ) );
        }
    }

    // hdr render states
    {
        motor::graphics::state_object_t so =
            motor::graphics::state_object_t( "gfx.postprocess.hdr" );

        {
            motor::graphics::render_state_sets_t rss;
            rss.depth_s.do_change = false;
            rss.depth_s.ss.do_activate = true;
            rss.depth_s.ss.do_depth_write = true;
            rss.polygon_s.do_change = false;
            rss.polygon_s.ss.do_activate = true;
            rss.polygon_s.ss.fm = motor::graphics::fill_mode::fill;
            rss.polygon_s.ss.ff = motor::graphics::front_face::counter_clock_wise;
            rss.polygon_s.ss.cm = motor::graphics::cull_mode::back;
            rss.clear_s.do_change = true;
            rss.clear_s.ss.clear_color = motor::math::vec4f_t( 0.0f, 0.0f, 0.0f, 1.0f );
            rss.clear_s.ss.do_activate = true;
            rss.clear_s.ss.do_color_clear = true;
            rss.clear_s.ss.do_depth_clear = true;
            rss.view_s.do_change = true;
            rss.view_s.ss.do_activate = true;
            rss.view_s.ss.vp = motor::math::vec4ui_t( 0, 0, _post_fb_dims.x(), _post_fb_dims.y() );
            so.add_render_state_set( rss );
        }

        _hdr_so = motor::shared( std::move( so ) );
    }

    // init framebuffer
    // the framebuffers we can render the scene to.
    // the content of those framebuffers are used in post processing
    {
        size_t const w = _post_fb_dims.x();
        size_t const h = _post_fb_dims.y();

        for( size_t i = 0; i < 2; ++i )
        {
            motor::string_t const n = motor::to_string( i );
            auto fb =
                motor::graphics::framebuffer_object_t( "gfx.postprocess.hdr.framebuffer." + n );
            fb.set_target( motor::graphics::color_target_type::rgba_float_32, 4 )
                .set_target( motor::graphics::depth_stencil_target_type::depth32 )
                .resize( w, h );

            _hdr_fbs[ i ] = motor::shared( std::move( fb ) );
        }
    }

    // init tone mapping
    {
        _tone_map = motor::shared( motor::gfx::tone_map_stage() );
        _tone_map->init( "gfx.postprocess.fb.full.hdr.1.0" );
    }

    // init brightpass
    {
        _brightpass = motor::shared( motor::gfx::bright_pass_stage() );
        _brightpass->init( "gfx.postprocess.hdr.framebuffer.0.0" );
    }

    // init bloom
    {
        uint_t const w = _post_fb_dims.x();
        uint_t const h = _post_fb_dims.y();

        _bloom = motor::shared( motor::gfx::bloom_stage() );
        _bloom->init( w, h );

        // level 1 downsample should read the render target the bright pass wrote into
        _bloom->set_read_render_target_for_down(
            motor::gfx::bloom_stage_t::level_type::level_1, "gfx.postprocess.fb.full.hdr.1.0" );
        _bloom->set_read_render_target_for_down(
            motor::gfx::bloom_stage_t::level_type::level_2, "gfx.postprocess.fb.half.hdr.0.0" );
        _bloom->set_read_render_target_for_down(
            motor::gfx::bloom_stage_t::level_type::level_3, "gfx.postprocess.fb.quater.hdr.0.0" );
        _bloom->set_read_render_target_for_down(
            motor::gfx::bloom_stage_t::level_type::level_4, "gfx.postprocess.fb.eigth.hdr.0.0" );
        _bloom->set_read_render_target_for_down( motor::gfx::bloom_stage_t::level_type::level_5,
            "gfx.postprocess.fb.sixteenth.hdr.0.0" );

        // upsampling working the other way around.
        _bloom->set_read_render_target_for_up(
            motor::gfx::bloom_stage_t::level_type::level_1, "gfx.postprocess.fb.half.hdr.0.0" );
        _bloom->set_read_render_target_for_up(
            motor::gfx::bloom_stage_t::level_type::level_2, "gfx.postprocess.fb.quater.hdr.0.0" );
        _bloom->set_read_render_target_for_up(
            motor::gfx::bloom_stage_t::level_type::level_3, "gfx.postprocess.fb.eigth.hdr.0.0" );
        _bloom->set_read_render_target_for_up( motor::gfx::bloom_stage_t::level_type::level_4,
            "gfx.postprocess.fb.sixteenth.hdr.0.0" );
        _bloom->set_read_render_target_for_up(
            motor::gfx::bloom_stage_t::level_type::level_5, "gfx.postprocess.fb.5.0" ); // not used.
    }

    // init merge
    {
        _merge = motor::shared( motor::gfx::merge_stage() );
        // merge hdr scene and bloom
        _merge->init( "gfx.postprocess.hdr.framebuffer.0.0", "gfx.postprocess.fb.full.hdr.2.0" );
    }
}

//***************************************************
void_t hdr_postprocess_pipeline::release( void_t ) noexcept
{
    motor::release( motor::move( _post_quad ) );
    motor::release( motor::move( _post_so ) );
    motor::release( motor::move( _hdr_fbs[ 0 ] ) );
    motor::release( motor::move( _hdr_fbs[ 1 ] ) );
    motor::release( motor::move( _hdr_so ) );
    motor::release( motor::move( _mts_so ) );
    motor::release( motor::move( _msl ) );

    for( size_t i = 0; i < _post_fbs.size(); ++i )
    {
        motor::release( motor::move( _post_fbs[ i ] ) );
    }

    _tone_map->release();
    motor::release( motor::move( _tone_map ) );

    _brightpass->release();
    motor::release( motor::move( _brightpass ) );

    _bloom->release();
    motor::release( motor::move( _bloom ) );

    _merge->release();
    motor::release( motor::move( _merge ) );
}

//***************************************************
void_t hdr_postprocess_pipeline::init_render( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    for( size_t i = 0; i < _post_fbs.size(); ++i )
    {
        fe->configure< motor::graphics::framebuffer_object_t >( _post_fbs[ i ] );
    }

    fe->configure< motor::graphics::framebuffer_object_t >( _hdr_fbs[ 0 ] );
    fe->configure< motor::graphics::framebuffer_object_t >( _hdr_fbs[ 1 ] );
    fe->configure< motor::graphics::geometry_object_t >( _post_quad );
    fe->configure< motor::graphics::msl_object_t >( _msl );
    fe->configure< motor::graphics::state_object_t >( _post_so );
    fe->configure< motor::graphics::state_object_t >( _mts_so );
    fe->configure< motor::graphics::state_object_t >( _hdr_so );

    _tone_map->init_graphics( fe );
    _brightpass->init_graphics( fe );
    _bloom->init_graphics( fe );
    _merge->init_graphics( fe );
}

//***************************************************
void_t hdr_postprocess_pipeline::release_render( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    _tone_map->release_graphics( fe );
    _brightpass->release_graphics( fe );
    _bloom->release_graphics( fe );
    _merge->release_graphics( fe );

    for( size_t i = 0; i < _post_fbs.size(); ++i )
    {
        fe->release< motor::graphics::framebuffer_object_t >( _post_fbs[ i ] );
    }

    fe->release< motor::graphics::geometry_object_t >( _post_quad );
    fe->release< motor::graphics::msl_object_t >( _msl );
    fe->release< motor::graphics::state_object_t >( _post_so );
    fe->release< motor::graphics::state_object_t >( _mts_so );
    fe->release< motor::graphics::state_object_t >( _hdr_so );
    fe->release< motor::graphics::framebuffer_object_t >( _hdr_fbs[ 0 ] );
    fe->release< motor::graphics::framebuffer_object_t >( _hdr_fbs[ 1 ] );
}

//***************************************************
void_t hdr_postprocess_pipeline::render( motor::graphics::gen4::frontend_ptr_t fe, bool_t const temp ) noexcept
{
    if( _size_changed )
    {
        uint_t const w = _dims.x();
        uint_t const h = _dims.y();

        // change post render state
        {
            _post_so->access_render_state( 0, [ & ]( motor::graphics::render_state_sets_ref_t ss )
            {
                ss.view_s.ss.vp.z( w );
                ss.view_s.ss.vp.w( h );

                return true;
            } );
        }

        {
            for( size_t i = 0; i < _post_fbs.size(); ++i )
            {
                _post_fbs[ i ]->resize( w, h );
            }

            _hdr_fbs[ 0 ]->resize( w, h );
            _hdr_fbs[ 1 ]->resize( w, h );
        }

        fe->configure< motor::graphics::state_object_t >( _post_so );
        fe->configure< motor::graphics::framebuffer_object_t >( _hdr_fbs[ 0 ] );
        fe->configure< motor::graphics::framebuffer_object_t >( _hdr_fbs[ 1 ] );
        _size_changed = false;
    }

    // stages
    {
        {
            fe->use( _post_fbs[ 1 ] );
            fe->push( _post_so );
            _brightpass->render( fe );
            fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
            fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
        }

        fe->push( _post_so );

        // downsample
        {
            // level 1
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::half_hdr ) ] );
                _bloom->render_down( motor::gfx::bloom_stage_t::level_type::level_1, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }
            // level 2
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::quater_hdr ) ] );
                _bloom->render_down( motor::gfx::bloom_stage_t::level_type::level_2, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }
            // level 3
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::eigth_hdr ) ] );
                _bloom->render_down( motor::gfx::bloom_stage_t::level_type::level_3, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }
            // level 4
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::sixteenth_hdr ) ] );
                _bloom->render_down( motor::gfx::bloom_stage_t::level_type::level_4, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }
        }

        // upsample
        {
            // level 4
            // bind eigth but read from sixteenth
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::eigth_hdr ) ] );
                _bloom->render_up( motor::gfx::bloom_stage_t::level_type::level_4, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }

            // level 3
            // bind quater but read from eigth
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::quater_hdr ) ] );
                _bloom->render_up( motor::gfx::bloom_stage_t::level_type::level_3, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }

            // level 2
            // bind half but read from quater
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::half_hdr ) ] );
                _bloom->render_up( motor::gfx::bloom_stage_t::level_type::level_2, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }

            // level 1
            // bind full but read from half
            {
                fe->use( _post_fbs[ this_t::to_idx( this_t::framebuffer_type::full_hdr_2 ) ] );
                _bloom->render_up( motor::gfx::bloom_stage_t::level_type::level_1, fe );
                fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
            }
        }

        fe->pop( motor::graphics::gen4::backend::pop_type::render_state );

        {
            fe->use( _post_fbs[ 1 ] );
            fe->push( _post_so );
            _merge->render( fe );
            fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
            fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
        }

        {
            fe->use( _post_fbs[ 0 ] );
            fe->push( _post_so );
            _tone_map->render( fe );
            fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
            fe->unuse( motor::graphics::gen4::backend::unuse_type::framebuffer );
        }
    }

    // map to screen
    {
        fe->push( _mts_so );
        motor::graphics::gen4::backend::render_detail det;
        det.varset = temp ? 1 : 0 ;
        fe->render( _msl, det );
        fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
    }
}

//***************************************************
motor::hash_map< motor::string_t, motor::wire::inputs_mtr_t > hdr_postprocess_pipeline::inputs(
    void_t ) noexcept
{
    inputs_map_t ret;
    ret[ "tone_map" ] = _tone_map->borrow_inputs();

    return ret;
}

//***************************************************
hdr_postprocess_pipeline::property_sheets_t hdr_postprocess_pipeline::property_sheets(
    void_t ) noexcept
{
    this_t::property_sheets_t ret;

    {
        ret[ "tone_map" ] = _tone_map->borrow_properties();
        ret[ "brightpass" ] = _brightpass->borrow_properties();
        ret[ "bloom" ] = _bloom->borrow_properties();
        ret[ "merge" ] = _merge->borrow_properties();
    }
    return ret;
}

//***************************************************
void_t hdr_postprocess_pipeline::update_properies( void_t ) noexcept
{
    _bloom->update_properties();
}

//***************************************************
void_t hdr_postprocess_pipeline::set_map_to_screen_texture_temp( motor::string_in_t name ) noexcept
{
    auto * vs = _msl->borrow_varibale_set( 1 );
    if( vs )
    {
        auto * var = vs->texture_variable( "tx_map" );
        if( var ) var->set( name );
    }
}

//***************************************************
motor::graphics::framebuffer_object_mtr_t hdr_postprocess_pipeline::borrow_post_framebuffer(
    this_t::framebuffer_type const t ) noexcept
{
    return _post_fbs[ size_t( t ) ];
}