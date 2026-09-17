#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/frontend/gen4/frontend.hpp>
#include <motor/graphics/variable/wire_variable_bridge.h>
#include <motor/property/property_sheet.hpp>

namespace motor
{
namespace gfx
{
class fxaa_stage
{
    motor_this_typedefs( fxaa_stage );

  private:

    motor::graphics::msl_object_mtr_t _msl = nullptr;
    motor::graphics::wire_variable_bridge_mtr_t _brg = nullptr;

    motor::property::property_sheet_mtr_t _prop_sheet = nullptr;

  public:

    fxaa_stage( void_t ) noexcept {}

    fxaa_stage( this_cref_t ) = delete;
    fxaa_stage( this_rref_t rhv ) noexcept
        : _msl( motor::move( rhv._msl ) ), _brg( motor::move( _brg ) )
    {
    }

    void_t init( motor::string_cref_t rt_name ) noexcept
    {
        // init msl
        // shaders for post process
        {
            // post quad object
            {
                motor::graphics::msl_object_t mslo( "gfx.postprocess.stage.fxaa" );

                mslo.add( motor::graphics::msl_api_type::msl_4_0, R"(
                library fxaa
                {
                    float_t saturate( float_t v ) 
                    {
                        return clamp( v, 0.0, 1.0 ) ;
                    }

                    float_t get_luma( vec3_t color ) 
                    {
                        return color.g ;
                    }

                    bool_t can_skip( float_t luma_range, float_t luma_max, float_t fixed_thres, float_t rel_thres )
                    {
                        return luma_range < max( fixed_thres, rel_thres * luma_max ) ;
                    }

                    // uses only the cross neighbors
                    float_t get_subpixel_blend_factor( float_t luma_range, float_t m, float_t s, float_t n, float_t w, float_t e )
                    {
                        float_t filter = (n+s+w+e)/4.0 ;
                        filter = abs( filter - m ) ;
                        filter = filter / luma_range ;
                        filter = smoothstep( 0.0, 1.0, filter ) ;
                        return filter * filter ;
                    }

                    // uses full 3x3 neighbors
                    float_t get_subpixel_blend_factor2( float_t luma_range, float_t m, 
                        float_t s, float_t n, float_t w, float_t e,
                        float_t nw, float_t ne, float_t sw, float_t se, float_t subpixel_blending )
                    {
                        float_t filter = 2.0 * (n+s+w+e) ;
                        filter += nw + ne + sw + se ;
                        filter *= 1.0 / 12.0 ;
                        filter = abs( filter - m ) ;
                        filter = msl.fxaa.saturate( filter / luma_range ) ;
                        filter = smoothstep( 0.0, 1.0, filter ) ;
                        return filter * filter * subpixel_blending ;
                    }

                    bool_t is_horizontal_edge( float_t m, 
                        float_t n, float_t s, float_t w, float_t e,
                        float_t ne, float_t se, float_t nw, float_t sw ) 
                    {
                        float_t horz = 2.0 *
                            abs( n + s - 2.0 * m ) +
                            abs( ne + se - 2.0 * e ) +
                            abs( nw + sw - 2.0 * w ) ;

                        float_t vert = 2.0 *
                            abs( w + e - 2.0 * m ) +
                            abs( ne + nw - 2.0 * n ) +
                            abs( se + sw - 2.0 * s ) ;

                        return horz >= vert ;
                    }

                    
                }

                // basically taken from here:
                // https://catlikecoding.com/unity/tutorials/custom-srp/fxaa/
                config gfx.postprocess.stage.fxaa
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

                        float_t fixed_threshold(0.013) ;
                        float_t relative_threshold(0.33) ;
                        float_t subpixel_blending(0.75) ;
                        
                        tex2d_t tx_map ;

                        void main()
                        {                            
                            vec2_t rdims = vec2_t(1.0,1.0) / texture_dims( tx_map ) ;

                            vec2_t uv = in.tx ;

                            vec2_t uv_n = uv + rdims ' vec2_t( 0.0,  1.0) ;
                            vec2_t uv_e = uv + rdims ' vec2_t( 1.0,  0.0) ;
                            vec2_t uv_s = uv + rdims ' vec2_t( 0.0,  -1.0) ;
                            vec2_t uv_w = uv + rdims ' vec2_t( -1.0, 0.0) ;

                            vec2_t uv_nw = uv + rdims ' vec2_t( -1.0,  1.0) ;
                            vec2_t uv_ne = uv + rdims ' vec2_t( 1.0,  1.0) ;
                            vec2_t uv_sw = uv + rdims ' vec2_t( -1.0,  -1.0) ;
                            vec2_t uv_se = uv + rdims ' vec2_t( 1.0, -1.0) ;

                            float_t m = rt_texture( tx_map, uv ).g ;
                            
                            float_t n = rt_texture( tx_map, uv_n ).g ;
                            float_t w = rt_texture( tx_map, uv_w ).g ;
                            float_t s = rt_texture( tx_map, uv_s ).g ;
                            float_t e = rt_texture( tx_map, uv_e ).g ;

                            float_t nw = rt_texture( tx_map, uv_nw ).g ;
                            float_t ne = rt_texture( tx_map, uv_ne ).g ;
                            float_t sw = rt_texture( tx_map, uv_sw ).g ;
                            float_t se = rt_texture( tx_map, uv_se ).g ;

                            // highest
                            float_t h = max( max( max ( max ( m, n ), e ), s ), w ) ;
                            // lowest
                            float_t l = min( min( min ( min ( m, n ), e ), s ), w ) ;
                            // range
                            float_t r = h - l ;

                            if( msl.fxaa.can_skip( r, h, fixed_threshold, relative_threshold ) )
                            {
                                out.color = vec4_t( rt_texture( tx_map, uv ).xyz, 1.0 ) ;
                                return ;
                            }

                            float_t blend_factor = msl.fxaa.get_subpixel_blend_factor2( 
                                r, m, s, n, w, e, nw, ne, sw, se, subpixel_blending ) ;
                            bool_t is_horz = msl.fxaa.is_horizontal_edge( m, n, s, w, e, ne, se, nw, sw ) ;
                            
                            // setup as if it is vertical
                            float_t pixel_step = rdims.x ;
                            float_t luma_p = e ;
                            float_t luma_n = w ;

                            if( is_horz )
                            {
                                pixel_step = rdims.y ;
                                luma_p = n ;
                                luma_n = s ;
                            }

                            float_t gradient_p = abs( luma_p - m ) ;
                            float_t gradient_n = abs( luma_n - m ) ;
                            float_t luma_grad ;
                            float_t luma_other ;

                            if( gradient_p < gradient_n ) 
                            {
                                pixel_step = -pixel_step ;
                                luma_grad = gradient_n ;
                                luma_other = luma_n ;
                            }
                            else
                            {
                                luma_grad = gradient_p ;
                                luma_other = luma_p ;
                            }

                            float_t edge_blend_factor ;

                            // function : get_edge_blend_factor(...)
                            {
                                vec2_t edge_uv = uv ;
                                vec2_t uv_step = vec2_t(0.0, 0.0) ;
                                if( is_horz )
                                {
                                    edge_uv.y += 0.5 * pixel_step ;
                                    uv_step.x = rdims.x ;
                                }
                                else
                                {
                                    edge_uv.x += 0.5 * pixel_step ;
                                    uv_step.y = rdims.y ;
                                }

                                float_t luma_edge = 0.5 * ( m + luma_other ) ;
                                float_t grad_thres = 0.25 * luma_grad ;

                                vec2_t uv_p = edge_uv + uv_step ;
                                vec2_t uv_n = edge_uv - uv_step ;

                                float_t luma_delta_p = rt_texture( tx_map, uv_p ).g - luma_edge ;
                                float_t luma_delta_n = rt_texture( tx_map, uv_n ).g - luma_edge ;

                                // search positive direction
                                {
                                    bool_t at_end_p = abs( luma_delta_p ) >= grad_thres ;

                                    for( int_t i=0; i<3 && !at_end_p; ++i )
                                    {
                                        uv_p += uv_step ;
                                        luma_delta_p = rt_texture( tx_map, uv_p ).g - luma_edge ;
                                        at_end_p = abs( luma_delta_p ) >= grad_thres ;
                                    }
                                    if( !at_end_p ) uv_p += uv_step ;
                                }

                                // search negative direction
                                {
                                    
                                    bool_t at_end_n = abs( luma_delta_n ) >= grad_thres ;

                                    for( int_t i=0; i<3 && !at_end_n; ++i )
                                    {
                                        uv_n -= uv_step ;
                                        luma_delta_n = rt_texture( tx_map, uv_n ).g - luma_edge ;
                                        at_end_n = abs( luma_delta_n ) >= grad_thres ;
                                    }
                                    if( !at_end_n ) uv_n -= uv_step ;
                                }

                                float_t dist_end_p ;
                                float_t dist_end_n ;

                                if( is_horz )
                                {
                                    dist_end_p = uv_p.x - uv.x ;
                                    dist_end_n = uv.x - uv_n.x ;
                                }
                                else
                                {
                                    dist_end_p = uv_p.y - uv.y ;
                                    dist_end_n = uv.y - uv_n.y ;
                                }

                                float_t dist_nearest_end ;
                                bool_t delta_sign ;
                                if( dist_end_p <= dist_end_n )
                                {
                                    dist_nearest_end = dist_end_p ;
                                    delta_sign = luma_delta_p >= 0.0 ;
                                }
                                else
                                {
                                    dist_nearest_end = dist_end_n ;
                                    delta_sign = luma_delta_n >= 0.0 ;
                                }

                                if( delta_sign == (m - luma_edge >= 0.0 ) )
                                {
                                    edge_blend_factor =  0.0 ;
                                }
                                else 
                                {
                                    edge_blend_factor = 0.5 - dist_nearest_end / (dist_end_p+dist_end_n) ;
                                }
                            }

                            blend_factor = max( blend_factor, edge_blend_factor ) ;


                            vec2_t blend_uv = uv  ;
                            if( is_horz )
                            {
                                blend_uv.y += pixel_step * blend_factor ;
                            }
                            else
                            {
                                blend_uv.x += pixel_step * blend_factor ;
                            }

                            

                            vec4_t tmp = pixel_step > 0.0 ? 
                                vec4_t( 1.0, 0.0, 0.0, 1.0 ) : vec4_t( 0.0, 1.0, 0.0, 1.0 ) ;
                            
                            vec4_t txl = rt_texture( tx_map, blend_uv ) ;
                            out.color = vec4_t( txl.xyz, 1.0 ) ;
                        }
                    }
                } )" );

                mslo.link_geometry( "gfx.postprocess.quad" );

                _msl = motor::shared( std::move( mslo ) );
            }

            // variable sets
            {
                motor::graphics::variable_set_t vars;

                {
                    auto * var = vars.texture_variable( "tx_map" );
                    var->set( rt_name );
                }

                // Trims the algorithm from processing darks.
                //   0.0833 - upper limit (default, the start of visible unfiltered edges)
                //   0.0625 - high quality (faster)
                //   0.0312 - visible limit (slower)
                {
                    auto * var = vars.data_variable< float_t >( "fixed_threshold" );
                    var->set( 0.031f );
                }

                // The minimum amount of local contrast required to apply algorithm.
                //   0.333 - too little (faster)
                //   0.250 - low quality
                //   0.166 - default
                //   0.125 - high quality 
                //   0.063 - overkill (slower)
                {
                    auto * var = vars.data_variable< float_t >( "relative_threshold" );
                    var->set( 0.166f );
                }

                // Choose the amount of sub-pixel aliasing removal.
                // This can effect sharpness.
                //   1.00 - upper limit (softer)
                //   0.75 - default amount of filtering
                //   0.50 - lower limit (sharper, less sub-pixel aliasing removal)
                //   0.25 - almost off
                //   0.00 - completely off
                {
                    auto * var = vars.data_variable< float_t >( "subpixel_blending" );
                    var->set( 0.75f );
                }

#if 0
                {
                    auto * var = vars.data_variable< float_t >( "brightness_threshold" );
                    var->set( 10.0f );
                }

                {
                    auto * var = vars.data_variable< float_t >( "brightness_knee_percent" );
                    var->set( 0.3f );
                }
#endif

                auto vs_ptr =
                    motor::shared( std::move( vars ), "a variable set in the fxaa stage" );
                _msl->add_variable_set( motor::share( vs_ptr ) );

                _brg = motor::shared(
                    motor::graphics::wire_variable_bridge_t( motor::move( vs_ptr ) ) );
                _brg->update_bindings();

                {
                    motor::property::property_sheet_t ps;

                    using is_float_t = motor::wire::input_slot< float_t >;

                    {
                        motor::property::add_is_property< float_t >( "fixed_threshold",
                            _brg->borrow_inputs()->borrow( "fixed_threshold" ), ps );

                        {
                            auto * prop = ps.borrow_property< is_float_t >( "fixed_threshold" );
                            prop->set_min_max(
                                motor::property::min_max< float_t >( 0.0312f, 0.0833f ) );
                        }
                    }

                    {
                        motor::property::add_is_property< float_t >( "relative_threshold",
                            _brg->borrow_inputs()->borrow( "relative_threshold" ), ps );

                        {
                            auto * prop = ps.borrow_property< is_float_t >( "relative_threshold" );
                            prop->set_min_max(
                                motor::property::min_max< float_t >( 0.333f, 0.063f ) );
                        }
                    }

                    {
                        motor::property::add_is_property< float_t >( "subpixel_blending",
                            _brg->borrow_inputs()->borrow( "subpixel_blending" ), ps );

                        {
                            auto * prop = ps.borrow_property< is_float_t >( "subpixel_blending" );
                            prop->set_min_max(
                                motor::property::min_max< float_t >( 0.0f, 1.0f ) );
                        }
                    }

                    _prop_sheet = motor::shared( std::move( ps ) );
                }
            }
        }
    }

    void_t release( void_t ) noexcept
    {
        motor::release( motor::move( _msl ) );
        motor::release( motor::move( _brg ) );
        motor::release( motor::move( _prop_sheet ) );
    }

    void_t init_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        fe->configure< motor::graphics::msl_object_t >( _msl );
    }

    void_t release_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        fe->release< motor::graphics::msl_object_t >( _msl );
    }

    void_t render( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        _brg->pull_data();
        motor::graphics::gen4::backend::render_detail det;
        fe->render( _msl, det );
    }

    motor::wire::inputs_mtr_t borrow_inputs( void_t ) noexcept
    {
        return _brg->borrow_inputs();
    }

    motor::property::property_sheet_mtr_t borrow_properties( void_t ) noexcept
    {
        return _prop_sheet;
    }
};
motor_typedef( fxaa_stage );

} // namespace gfx
} // namespace motor