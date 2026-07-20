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
class merge_stage
{
    motor_this_typedefs( merge_stage );

  private:

    motor::graphics::msl_object_mtr_t _msl = nullptr;
    motor::graphics::wire_variable_bridge_mtr_t _brg = nullptr;

    motor::property::property_sheet_mtr_t _prop_sheet = nullptr;

    motor::graphics::compilation_listener_mtr_t _comp_list =
        motor::shared( motor::graphics::compilation_listener_t() );

  public:

    merge_stage( void_t ) noexcept {}

    merge_stage( this_cref_t ) = delete;
    merge_stage( this_rref_t rhv ) noexcept
        : _msl( motor::move( rhv._msl ) ), _brg( motor::move( rhv._brg ) ),
          _prop_sheet( motor::move( rhv._prop_sheet ) ), _comp_list( motor::move( rhv._comp_list ) )
    {
    }

    ~merge_stage( void_t ) noexcept 
    {
        this_t::release() ;
    }

  public:

    void_t init( motor::string_cref_t rt_name_a, motor::string_cref_t rt_name_b ) noexcept
    {
        // init msl
        // shaders for post process
        {
            // post quad object
            {
                motor::graphics::msl_object_t mslo( "gfx.postprocess.stage.hdr.merge" );

                mslo.add( motor::graphics::msl_api_type::msl_4_0, R"(
            
                config gfx.postprocess.stage.hdr.merge
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

                        tex2d_t tx_a ;
                        tex2d_t tx_b ;

                        float_t strength_a(1.0) ;
                        float_t strength_b(0.8) ;

                        void main()
                        {

                            vec4_t color_a = rt_texture( tx_a, in.tx ) * strength_a ;
                            vec4_t color_b = rt_texture( tx_b, in.tx ) * strength_b ;
                            
                            out.color = vec4_t( color_a.xyz + color_b.xyz, 1.0 ) ;
                        }
                    }
                } )" );

                mslo.link_geometry( "gfx.postprocess.quad" );

                _msl = motor::shared( std::move( mslo ) );
                _msl->register_listener( motor::share( _comp_list ) );
            }

            // variable sets
            {
                motor::graphics::variable_set_t vars;

                {
                    auto * var = vars.texture_variable( "tx_a" );
                    var->set( rt_name_a );
                }

                {
                    auto * var = vars.texture_variable( "tx_b" );
                    var->set( rt_name_b );
                }
#if 0
                {
                    auto * var = vars.data_variable< float_t >( "strength_a" );
                    var->set( 1.0f );
                }

                {
                    auto * var = vars.data_variable< float_t >( "strength_b" );
                    var->set( 0.8f );
                }
#endif
                auto vs_ptr = motor::shared( std::move( vars ), "a variable set" );
                _msl->add_variable_set( motor::move( vs_ptr ) );

                {
                    _brg = motor::shared( motor::graphics::wire_variable_bridge_t() );
                }

#if 0
                {
                    motor::property::property_sheet_t ps;

                    using is_float_t = motor::wire::input_slot< float_t >;

                    {
                        motor::property::add_is_property< float_t >(
                            "strength_a", _brg->borrow_inputs()->borrow( "strength_a" ), ps );

                        {
                            auto * prop = ps.borrow_property< is_float_t >( "strength_a" );
                            prop->set_min_max( motor::property::min_max< float_t >( 0.1f, 1.0f ) );
                        }
                    }

                    {
                        motor::property::add_is_property< float_t >(
                            "strength_b", _brg->borrow_inputs()->borrow( "strength_b" ), ps );

                        {
                            auto * prop = ps.borrow_property< is_float_t >( "strength_b" );
                            prop->set_min_max( motor::property::min_max< float_t >( 0.1f, 1.0f ) );
                        }
                    }

                    _prop_sheet = motor::shared( std::move( ps ) );
                }
#else
                {
                    _prop_sheet = motor::shared( motor::property::property_sheet_t() );
                }
#endif
            }
        }
    }

    void_t release( void_t ) noexcept
    {
        motor::release( motor::move( _msl ) );
        motor::release( motor::move( _brg ) );
        motor::release( motor::move( _prop_sheet ) );
        motor::release( motor::move( _comp_list ) );
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
        // the point of doing it here in the render function is
        // so that the shader variable default values can be used.
        if( _comp_list->has_changed() )
        {
            motor::graphics::shader_bindings_t sb;
            if( _comp_list->reset_and_successful( sb ) )
            {
                auto vs = _msl->get_varibale_set( 0 );

                // update wire slot to shader variable bridge
                {
                    _brg->update_bindings( motor::move( vs ) );
                }

                {
                    _prop_sheet->clear();

                    {
                        motor::string_t const name = "strength_a";
                        using is_t = motor::wire::input_slot< float_t >;
                        auto * is = _brg->borrow_inputs()->borrow( name );
                        motor::property::add_is_property< is_t::value_t >( name, is, *_prop_sheet );
                        {
                            auto * prop = _prop_sheet->borrow_property< is_t >( name );
                            prop->set_min_max( motor::property::min_max< is_t::value_t >(
                                is_t::value_t( 0.1f ), is_t::value_t( 1.0f ) ) );
                        }
                    }

                    {
                        motor::string_t const name = "strength_b";
                        using is_t = motor::wire::input_slot< float_t >;
                        auto * is = _brg->borrow_inputs()->borrow( name );
                        motor::property::add_is_property< is_t::value_t >( name, is, *_prop_sheet );
                        {
                            auto * prop = _prop_sheet->borrow_property< is_t >( name );
                            prop->set_min_max( motor::property::min_max< is_t::value_t >(
                                is_t::value_t( 0.1f ), is_t::value_t( 1.0f ) ) );
                        }
                    }
                }
            }
        }

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
motor_typedef( merge_stage );

} // namespace gfx
} // namespace motor