
#pragma once

#include "../typedefs.h"
#include "../object/shader_object.h"

#include <motor/msl/generator_structs.hpp>
#include <motor/log/global.h>

namespace motor
{
    namespace graphics
    {
        // generates a shader configuration from generated code
        class msl_bridge
        {
            motor_this_typedefs( msl_bridge ) ;

        public:

            //**************************************************************************************************
            static motor::graphics::shader_object_t create_unknown_api_type( motor::msl::generated_code_cref_t code ) noexcept
            {
                motor::graphics::shader_object_t ret ;

                this_t::do_bindings( ret, code ) ;
                
                // code
                {
                    motor::vector< motor::msl::api_type > const types = 
                    {
                        motor::msl::api_type::es3, motor::msl::api_type::gl4,
                        motor::msl::api_type::d3d11 
                    } ;

                    for( auto const & t : types )
                    {
                        motor::graphics::shader_api_type bt = motor::graphics::shader_api_type::unknown ;
                        switch( t )
                        {
                        case motor::msl::api_type::gl4:
                            bt = motor::graphics::shader_api_type::glsl_1_4 ;
                            break ;
                        case motor::msl::api_type::es3:
                            bt = motor::graphics::shader_api_type::glsles_3_0 ;
                            break ;
                        case motor::msl::api_type::d3d11:
                            bt = motor::graphics::shader_api_type::hlsl_5_0 ;
                            break ;
                        default:
                            break;
                        }

                        if( bt == motor::graphics::shader_api_type::unknown )
                        {
                            motor::log::global_t::warning( "[nsl_bridge] : unknown/unmappable api type" ) ;
                            continue ;
                        }

                        motor::graphics::shader_set_t ss ;

                        code.sorted_by_api_type( t, [&] ( motor::msl::shader_type st, motor::msl::generated_code_t::code_cref_t c )
                        {
                            if( st == motor::msl::shader_type::vertex_shader )
                                ss.set_vertex_shader( c.shader ) ;
                            else if( st == motor::msl::shader_type::geometry_shader )
                                ss.set_geometry_shader( c.shader ) ;
                            else if( st == motor::msl::shader_type::pixel_shader )
                                ss.set_pixel_shader( c.shader ) ;
                        } ) ;

                        ret.insert( bt, std::move( ss ) ) ;
                    }
                }

                return std::move( ret ) ;
            }

            //**************************************************************************************************
            static void_t create_by_api_type( motor::graphics::shader_api_type const bt,
                motor::msl::generated_code_cref_t code,
                motor::graphics::shader_object_inout_t ret ) noexcept
            {
                this_t::do_bindings( ret, code ) ;
                
                // code
                {
                    motor::graphics::shader_set_t ss ;

                    code.sorted_by_shader_type( [&] ( motor::msl::shader_type st, motor::msl::generated_code_t::code_cref_t c )
                    {
                        if( st == motor::msl::shader_type::vertex_shader )
                            ss.set_vertex_shader( c.shader ) ;
                        else if( st == motor::msl::shader_type::geometry_shader )
                            ss.set_geometry_shader( c.shader ) ;
                        else if( st == motor::msl::shader_type::pixel_shader )
                            ss.set_pixel_shader( c.shader ) ;
                    } ) ;

                    ret.insert( bt, std::move( ss ) ) ;
                }
            }

        private:

            //**************************************************************************************************
            static void_t do_bindings( motor::graphics::shader_object_inout_t ret, 
                motor::msl::generated_code_cref_t code ) noexcept
            {
                // vertex input bindings
                {
                    for( auto const & v : code.geometry_ins )
                    {
                        // check texcoord
                        {
                            if( motor::msl::is_texcoord( v.binding ) )
                            {
                                motor::graphics::vertex_attribute va
                                    = motor::graphics::vertex_attribute::undefined ;

                                switch( v.binding )
                                {
                                case motor::msl::binding::texcoord0: 
                                    va = motor::graphics::vertex_attribute::texcoord0 ; break ;
                                case motor::msl::binding::texcoord1: 
                                    va = motor::graphics::vertex_attribute::texcoord1 ; break ;
                                case motor::msl::binding::texcoord2: 
                                    va = motor::graphics::vertex_attribute::texcoord2 ; break ;
                                case motor::msl::binding::texcoord3: 
                                    va = motor::graphics::vertex_attribute::texcoord3 ; break ;
                                case motor::msl::binding::texcoord4: 
                                    va = motor::graphics::vertex_attribute::texcoord4 ; break ;
                                case motor::msl::binding::texcoord5: 
                                    va = motor::graphics::vertex_attribute::texcoord5 ; break ;
                                case motor::msl::binding::texcoord6: 
                                    va = motor::graphics::vertex_attribute::texcoord6 ; break ;
                                case motor::msl::binding::texcoord7: 
                                    va = motor::graphics::vertex_attribute::texcoord7 ; break ;
                                default: break;
                                }
                                ret.add_vertex_input_binding( va, v.name ) ;

                                continue ;
                            }
                        }

                        // check color
                        {
                            if( motor::msl::is_color( v.binding ) )
                            {
                                motor::graphics::vertex_attribute va
                                    = motor::graphics::vertex_attribute::undefined ;

                                switch( v.binding )
                                {
                                case motor::msl::binding::color0: 
                                    va = motor::graphics::vertex_attribute::color0 ; break ;
                                case motor::msl::binding::color1: 
                                    va = motor::graphics::vertex_attribute::color1 ; break ;
                                case motor::msl::binding::color2: 
                                    va = motor::graphics::vertex_attribute::color2 ; break ;
                                case motor::msl::binding::color3: 
                                    va = motor::graphics::vertex_attribute::color3 ; break ;
                                case motor::msl::binding::color4: 
                                    va = motor::graphics::vertex_attribute::color4 ; break ;
                                case motor::msl::binding::color5: 
                                    va = motor::graphics::vertex_attribute::color5 ; break ;
                                default: break;
                                }
                                ret.add_vertex_input_binding( va, v.name ) ;
                                continue ;
                            }
                        }

                        if( v.binding == motor::msl::binding::position )
                        {
                            ret.add_vertex_input_binding( motor::graphics::vertex_attribute::position, v.name ) ;
                        }
                        else if( v.binding == motor::msl::binding::normal )
                        {
                            ret.add_vertex_input_binding( motor::graphics::vertex_attribute::normal, v.name ) ;
                        }
                        else if( v.binding == motor::msl::binding::tangent )
                        {
                            ret.add_vertex_input_binding( motor::graphics::vertex_attribute::tangent, v.name ) ;
                        }
                    }
                }

                // vertex output bindings
                {
                    for( auto const & v : code.geometry_outs )
                    {
                        // check texcoord
                        {
                            if( motor::msl::is_texcoord( v.binding ) )
                            {
                                motor::graphics::vertex_attribute va
                                    = motor::graphics::vertex_attribute::undefined ;

                                switch( v.binding )
                                {
                                case motor::msl::binding::texcoord0: 
                                    va = motor::graphics::vertex_attribute::texcoord0 ; break ;
                                case motor::msl::binding::texcoord1: 
                                    va = motor::graphics::vertex_attribute::texcoord1 ; break ;
                                case motor::msl::binding::texcoord2: 
                                    va = motor::graphics::vertex_attribute::texcoord2 ; break ;
                                case motor::msl::binding::texcoord3: 
                                    va = motor::graphics::vertex_attribute::texcoord3 ; break ;
                                case motor::msl::binding::texcoord4: 
                                    va = motor::graphics::vertex_attribute::texcoord4 ; break ;
                                case motor::msl::binding::texcoord5: 
                                    va = motor::graphics::vertex_attribute::texcoord5 ; break ;
                                case motor::msl::binding::texcoord6: 
                                    va = motor::graphics::vertex_attribute::texcoord6 ; break ;
                                case motor::msl::binding::texcoord7: 
                                    va = motor::graphics::vertex_attribute::texcoord7 ; break ;
                                default: break;
                                }
                                ret.add_vertex_output_binding( va, v.name ) ;

                                continue ;
                            }
                        }

                        // check color
                        {
                            if( motor::msl::is_color( v.binding ) )
                            {
                                motor::graphics::vertex_attribute va
                                    = motor::graphics::vertex_attribute::undefined ;

                                switch( v.binding )
                                {
                                case motor::msl::binding::color0: 
                                    va = motor::graphics::vertex_attribute::color0 ; break ;
                                case motor::msl::binding::color1: 
                                    va = motor::graphics::vertex_attribute::color1 ; break ;
                                case motor::msl::binding::color2: 
                                    va = motor::graphics::vertex_attribute::color2 ; break ;
                                case motor::msl::binding::color3: 
                                    va = motor::graphics::vertex_attribute::color3 ; break ;
                                case motor::msl::binding::color4: 
                                    va = motor::graphics::vertex_attribute::color4 ; break ;
                                case motor::msl::binding::color5: 
                                    va = motor::graphics::vertex_attribute::color5 ; break ;
                                default: break;
                                }
                                ret.add_vertex_output_binding( va, v.name ) ;
                                continue ;
                            }
                        }

                        if( v.binding == motor::msl::binding::position )
                        {
                            ret.add_vertex_output_binding( motor::graphics::vertex_attribute::position, v.name ) ;
                        }
                        else if( v.binding == motor::msl::binding::normal )
                        {
                            ret.add_vertex_output_binding( motor::graphics::vertex_attribute::normal, v.name ) ;
                        }
                        else if( v.binding == motor::msl::binding::tangent )
                        {
                            ret.add_vertex_output_binding( motor::graphics::vertex_attribute::tangent, v.name ) ;
                        }
                    }
                }

                // streamout type
                if( code.streamout != motor::msl::streamout_type::none )
                {
                    motor::graphics::streamout_mode sm = motor::graphics::streamout_mode::unknown ;
                    switch( code.streamout )
                    {
                    case motor::msl::streamout_type::interleaved: sm = motor::graphics::streamout_mode::interleaved ; break ;
                    case motor::msl::streamout_type::separate: sm = motor::graphics::streamout_mode::separate ; break ;
                    default: break ;
                    }

                    ret.set_streamout_mode( sm ) ;
                }

                // variable bindings
                {
                    // global and general purpose variables 
                    for( auto const& s : code.shaders )
                    {
                        {
                            for( auto const& v : s.variables )
                            {
                                motor::graphics::binding_point bp = motor::graphics::binding_point::undefined ;
                                
                                if( v.binding == motor::msl::binding::object )
                                {
                                    bp = motor::graphics::binding_point::object_matrix ;
                                }
                                else if( v.binding == motor::msl::binding::world )
                                {
                                    bp = motor::graphics::binding_point::world_matrix ;
                                }
                                else if( v.binding == motor::msl::binding::view )
                                {
                                    bp = motor::graphics::binding_point::view_matrix ;
                                }
                                else if( v.binding == motor::msl::binding::projection )
                                {
                                    bp = motor::graphics::binding_point::projection_matrix ;
                                }
                                else if( v.binding == motor::msl::binding::camera )
                                {
                                    bp = motor::graphics::binding_point::camera_matrix ;
                                }
                                else if( v.binding == motor::msl::binding::camera_position )
                                {
                                    bp = motor::graphics::binding_point::camera_position ;
                                }
                                else if( v.binding == motor::msl::binding::viewport )
                                {
                                    bp = motor::graphics::binding_point::viewport_dimension ;
                                }

                                if( !ret.has_input_binding( bp ) && bp != motor::graphics::binding_point::undefined ) 
                                    ret.add_input_binding( bp, v.name ) ;
                            }
                        }
                    }
                }
            }
        };
        motor_typedef( msl_bridge ) ;
    }
}