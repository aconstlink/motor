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
class bright_pass_stage
{
    motor_this_typedefs( bright_pass_stage );

  private:

    motor::graphics::msl_object_mtr_t _msl = nullptr;
    motor::graphics::wire_variable_bridge_mtr_t _brg = nullptr;

    motor::property::property_sheet_mtr_t _prop_sheet = nullptr;

  public:

    bright_pass_stage( void_t ) noexcept {}

    bright_pass_stage( this_cref_t ) = delete;
    bright_pass_stage( this_rref_t rhv ) noexcept
        : _msl( motor::move( rhv._msl ) ), _brg( motor::move( _brg ) )
    {
    }

  public:

    void_t init( motor::string_cref_t rt_name ) noexcept
    {
        // init msl
        // shaders for post process
        {
            // post quad object
            {
                motor::graphics::msl_object_t mslo( "gfx.postprocess.stage.tone_map" );

                mslo.add( motor::graphics::msl_api_type::msl_4_0, R"(
            
                config gfx.postprocess.stage.tone_map.reinhardt
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

                        vec3_t reinhard_offset ;

                        tex2d_t tx_map ;

                        void main()
                        {
                            vec4_t col = rt_texture( tx_map, in.tx ) ;
                            vec3_t tone_mapped =  col.xyz / ( col.xyz + reinhard_offset ) ; 
                            tone_mapped = pow( tone_mapped, 1.0/2.2) ;
                            out.color = vec4_t( tone_mapped, 1.0 ) ;

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
                motor::graphics::variable_set_t vars;

                {
                    auto * var = vars.texture_variable( "tx_map" );
                    var->set( rt_name );
                }

                {
                    auto * var = vars.data_variable< motor::math::vec3f_t >( "reinhard_offset" );
                    var->set( motor::math::vec3f_t( 6.0f, 3.0f, 6.0f ) );
                }

                auto vs_ptr = motor::shared( std::move( vars ), "a variable set" );
                _msl->add_variable_set( motor::share( vs_ptr ) );

                _brg = motor::shared(
                    motor::graphics::wire_variable_bridge_t( motor::move( vs_ptr ) ) );
                _brg->update_bindings();

                {
                    motor::property::property_sheet_t ps;

                    using is_vec3f_t = motor::wire::input_slot< motor::math::vec3f_t >;
                    motor::property::add_is_property< motor::math::vec3f_t >( "reinhard_offset",
                        _brg->borrow_inputs()->borrow( "reinhard_offset" ), ps );

                    {
                        auto * prop = ps.borrow_property< is_vec3f_t >( "reinhard_offset" );
                        prop->set_min_max( motor::property::min_max< motor::math::vec3f_t >(
                            motor::math::vec3f_t( 0.1f ), motor::math::vec3f_t( 20.0f ) ) );
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
        motor::release( motor::move( _prop_sheet ) ) ;
    }

    void_t init_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        fe->configure< motor::graphics::msl_object_t >( _msl );
    }

    void_t release_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        fe->release< motor::graphics::msl_object_t >( motor::move( _msl ) );
    }

    void_t render( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        _brg->update_bindings();
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
motor_typedef( bright_pass_stage );

} // namespace gfx
} // namespace motor