

#include "generator.h"

#include <motor/log/global.h>
#include <motor/std/sstream>
#include <regex>

using namespace motor::msl::hlsl ;

namespace this_file
{
    motor::core::types::bool_t fragment_by_opcode( motor::msl::buildin_type bit, motor::msl::post_parse::library_t::fragment_out_t ret ) noexcept
    {
        #if 0
        // texture offset is available in hlsl but kept this as an example.
        if( bit == motor::msl::buildin_type::texture_offset )
        {
            motor::msl::signature_t::arg_t a { motor::msl::type_t::as_tex2d(), "tex" } ;
            motor::msl::signature_t::arg_t b { motor::msl::type_t::as_vec2(motor::msl::type_base::tfloat), "uv" } ;
            motor::msl::signature_t::arg_t c { motor::msl::type_t::as_vec2(motor::msl::type_base::tint), "off" } ;

            motor::msl::signature_t sig = motor::msl::signature_t
            { 
                motor::msl::type_t::as_vec4(), "__d3d11_texture_offset__", { a, b, c } 
            } ;

            motor::vector< motor::string_t > lines 
            {                 
                "return :texture_( tex, uv + uvr ) ;"
            } ;

            motor::msl::post_parse::library_t::fragment_t frg ;
            frg.sym_long = motor::msl::symbol_t("__d3d11_texture_offset__") ;
            frg.sig = std::move( sig ) ;
            frg.fragments = std::move( lines ) ;
            frg.buildins.emplace_back( motor::msl::post_parse::used_buildin{ 0, 0, 
                motor::msl::get_build_in( motor::msl::buildin_type::texture_dims ) }) ;

            ret = std::move( frg ) ;

            return true ;
        }
        else 
        #endif

        if( bit == motor::msl::buildin_type::texture_dims )
        {
            motor::msl::signature_t::arg_t a { motor::msl::type_t::as_tex2d(), "tex" } ;
            motor::msl::signature_t::arg_t b { motor::msl::type_t::as_int(), "lod" } ;

            motor::msl::signature_t sig = motor::msl::signature_t
            { 
                motor::msl::type_t::as_vec2(motor::msl::type_base::tuint), "__d3d11_texture_dims__", { a, b } 
            } ;

            motor::vector< motor::string_t > lines 
            { 
                "uint width = 0 ; uint height = 0 ; int elements = 0 ; int depth = 0 ; int num_levels = 0 ; int num_samples = 0 ;",
                "tex.GetDimensions( lod, width, height, num_levels ) ;",
                "return uint2( width, height ) ;"
            } ;

            motor::msl::post_parse::library_t::fragment_t frg ;
            frg.sym_long = motor::msl::symbol_t("__d3d11_texture_dims__") ;
            frg.sig = std::move( sig ) ;
            frg.fragments = std::move( lines ) ;

            ret = std::move( frg ) ;

            return true ;
        }

        return false ;
    }

    //******************************************************************************************************************************************
    static motor::string_t determine_input_structure_name( motor::msl::shader_type const cur, motor::msl::shader_type const before ) noexcept 
    {
        // input assembler to vertex shader
        if( cur == motor::msl::shader_type::vertex_shader )
            return "ia_to_" + motor::msl::short_name( cur ) ;

        return motor::msl::short_name( before ) + "_to_" + motor::msl::short_name( cur ) ;
    }

    //******************************************************************************************************************************************
    static motor::string_t determine_output_structure_name( motor::msl::shader_type const cur, motor::msl::shader_type const after ) noexcept 
    {
        if( cur == motor::msl::shader_type::pixel_shader )
            return "into_the_pixel_pot" ;
    
        if( cur != motor::msl::shader_type::pixel_shader && 
            after == motor::msl::shader_type::unknown )
            return "streamout" ; // must be streamout!!

        return motor::msl::short_name( cur ) + "_to_" + motor::msl::short_name( after ) ;
    }
}


motor::string_t generator::replace_buildin_symbols( motor::string_rref_t code ) noexcept
{
    motor::msl::repl_syms_t repls =
    {
        {
            motor::string_t( ":cmul:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "mul ( INVALID_ARGS ) " ;
                return args[ 0 ] + " * " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":mmul:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "mmul ( INVALID_ARGS ) " ;
                return "mul( " + args[ 1 ] + " , " + args[ 0 ] + " )" ;
            }
        },
        {
            motor::string_t( ":add_asg:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "add_asg ( INVALID_ARGS ) " ;
                return args[ 0 ] + " += " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":sub_asg:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "sub_asg ( INVALID_ARGS ) " ;
                return args[ 0 ] + " -= " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":mul_asg:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "mul_asg ( INVALID_ARGS ) " ;
                return args[ 0 ] + " *= " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":lor:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "lor ( INVALID_ARGS ) " ;
                return args[ 0 ] + " || " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":land:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "land ( INVALID_ARGS ) " ;
                return args[ 0 ] + " && " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":add:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() == 0 || args.size() > 2 ) return "add ( INVALID_ARGS ) " ;
                return args.size() == 1 ? " + " + args[ 0 ] : args[ 0 ] + " + " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":sub:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() == 0 || args.size() > 2 ) return "sub ( INVALID_ARGS ) " ;
                return args.size() == 1 ? " - " + args[ 0 ] : args[ 0 ] + " - " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":plus:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "plus ( INVALID_ARGS ) " ;
                return " + " + args[ 0 ] ;
            }
        },
        {
            motor::string_t( ":minus:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "minus ( INVALID_ARGS ) " ;
                return " - " + args[ 0 ] ;
            }
        },
        {
            motor::string_t( ":div:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "div ( INVALID_ARGS ) " ;
                return args[ 0 ] + " / " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":pulse:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "pulse ( INVALID_ARGS ) " ;
                return  "( step ( " + args[ 0 ] + " , " + args[ 2 ] + " ) - " +
                    "step ( " + args[ 1 ] + " , " + args[ 2 ] + " ) )" ;
            }
        },
        {
            motor::string_t( ":smoothstep:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "smoothstep ( INVALID_ARGS ) " ;
                return  "smoothstep ( " + args[ 0 ] + " , " + args[ 1 ] + " , " + args[ 2 ] + " )" ;
            }
        },
        {
            motor::string_t( ":smoothpulse:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "smoothpulse ( INVALID_ARGS ) " ;
                return  "( smoothstep ( " + args[ 0 ] + " , " + args[ 1 ] + " , " + args[ 4 ] + " ) - " +
                    "smoothstep ( " + args[ 2 ] + " , " + args[ 3 ] + " , " + args[ 4 ] + " ) )" ;
            }
        },
        {
            motor::string_t( ":clamp:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "clamp ( INVALID_ARGS ) " ;
                return  "clamp ( " + args[ 0 ] + " , " + args[ 1 ] + " , " + args[ 2 ] + " )" ;
            }
        },
        {
            motor::string_t( ":texture:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "texture ( INVALID_ARGS ) " ;
                return  args[ 0 ] + ".Sample ( smp_" + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":rt_texcoords:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "rt_texcoords ( INVALID_ARGS ) " ;
                return  "float2 ( " + args[ 0 ] + ".x , 1.0f - " + args[ 0 ] + ".y ) " ;
            }
        },
        {
            motor::string_t( ":rt_texture:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "texture ( INVALID_ARGS ) " ;
                return  args[ 0 ] + ".Sample ( smp_" + args[ 0 ] + " , "
                    "float2 ( " + args[ 1 ] + ".x , 1.0f - " + args[ 1 ] + ".y ) ) " ;
            }
        },
        {
            motor::string_t( ":rt_texture_offset:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "rt_texture_offset ( INVALID_ARGS ) " ;
                return  args[ 0 ] + ".Sample ( smp_" + args[ 0 ] + " , "
                    "float2 ( " + args[ 1 ] + ".x , 1.0f - " + args[ 1 ] + ".y ) , int2( 1, -1 ) * " + args[2] + " ) " ;
            }
        },
        {
            motor::string_t( ":ls:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return " << ( INVALID_ARGS ) " ;
                return args[ 0 ] + " << " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":rs:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return ">> ( INVALID_ARGS ) " ;
                return args[ 0 ] + " >> " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":leq:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "leq ( INVALID_ARGS ) " ;
                return args[ 0 ] + " <= " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":geq:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "geq ( INVALID_ARGS ) " ;
                return args[ 0 ] + " >= " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":neq:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "neq ( INVALID_ARGS ) " ;
                return args[ 0 ] + " != " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":eqeq:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "eqeq ( INVALID_ARGS ) " ;
                return args[ 0 ] + " == " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":lt:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "lt ( INVALID_ARGS ) " ;
                return args[ 0 ] + " < " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":gt:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "gt ( INVALID_ARGS ) " ;
                return args[ 0 ] + " > " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":lt_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "lt_vec ( INVALID_ARGS ) " ;
                return args[ 0 ] + " < " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":gt_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "gt_vec ( INVALID_ARGS ) " ;
                return args[ 0 ] + " > " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":lte_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "lte_vec ( INVALID_ARGS ) " ;
                return args[ 0 ] + " <= " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":gte_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "gte_vec ( INVALID_ARGS ) " ;
                return args[ 0 ] + " >= " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":eqeq_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "eqeq_vec ( INVALID_ARGS ) " ;
                return args[ 0 ] + " == " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":neq_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "neq_vec ( INVALID_ARGS ) " ;
                return args[ 0 ] + " != " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":any:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "any ( INVALID_ARGS ) " ;
                return "any( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":all:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "all ( INVALID_ARGS ) " ;
                return "all( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":ret:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "ret ( INVALID_ARGS ) " ;
                return args[0].empty() ? "return" : "return " + args[ 0 ] ;
            }
        },
        {
            motor::string_t( ":mix:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "mix ( INVALID_ARGS ) " ;
                return "lerp ( " + args[ 0 ] + " , " + args[ 1 ] + " , " + args[ 2 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":inc:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "++ ( INVALID_ARGS ) " ;
                return "++ ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":dec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "-- ( INVALID_ARGS ) " ;
                return "-- ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":inc_post:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "++ ( INVALID_ARGS ) " ;
                return "( " + args[ 0 ] + " ) ++ " ;
            }
        },
        {
            motor::string_t( ":dec_post:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "-- ( INVALID_ARGS ) " ;
                return "( " + args[ 0 ] + " ) -- " ;
            }
        },
        {
            motor::string_t( ":fract:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "frac ( INVALID_ARGS ) " ;
                return "frac ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":ceil:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "ceil ( INVALID_ARGS ) " ;
                return "ceil ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":floor:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "floor ( INVALID_ARGS ) " ;
                return "floor ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":abs:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "abs ( INVALID_ARGS ) " ;
                return "abs ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":dot:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "dot ( INVALID_ARGS ) " ;
                return "dot ( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":pow:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "pow ( INVALID_ARGS ) " ;
                return "pow ( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":min:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "min ( INVALID_ARGS ) " ;
                return "min ( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":max:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "max ( INVALID_ARGS ) " ;
                return "max ( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":make_array:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() < 3 ) return "make_array ( INVALID_ARGS ) " ;
                motor::string_t tmp ;
                for( size_t i=0; i<args.size()-3; ++i ) tmp += args[3+i] + " , " ;
                tmp = tmp.substr( 0, tmp.size() - 3 ) ;
                return args[0] + " " + args[1] + "[ " + args[2] + " ] = { " + tmp + " } " ;
            }
        },
        {
            motor::string_t( ":as_vec2:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "as_vec2 ( INVALID_ARGS ) " ;
                return "float2 ( " + args[ 0 ] + " , " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":as_vec3:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "as_vec3 ( INVALID_ARGS ) " ;
                return "float3 ( " + args[ 0 ] + " , " + args[ 0 ] + " , " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":as_vec4:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "as_vec4 ( INVALID_ARGS ) " ;
                return "float4 ( " + args[ 0 ] + " , " + args[ 0 ] + " , " + args[ 0 ] + " , " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":fetch_data:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "fetch_data ( INVALID_ARGS ) " ;
                return args[ 0 ] + ".Load( " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":texture_offset:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "texture_offset ( INVALID_ARGS ) " ;
                return  args[ 0 ] + ".Sample ( smp_" + args[ 0 ] + " , " + args[ 1 ] + " , " + args[2] + " ) " ;
            }
        },
        {
            motor::string_t( ":texture_dims:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() == 1 ) return "__d3d11_texture_dims__( " + args[ 0 ] + " , 0 ) " ;
                if( args.size() == 2 ) return "__d3d11_texture_dims__( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;

                return "texture_dims ( INVALID_ARGS ) " ;
            }
        },
        {
            motor::string_t( ":emit_vertex:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 0 ) return "emit_vertex( INVALID_ARGS ) " ;
                return "prim_stream.Append( __output__ )"  ;
            }
        },
        {
            motor::string_t( ":end_primitive:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 0 ) return "end_primitive( INVALID_ARGS ) " ;
                return "prim_stream.RestartStrip() ; " ;
            }
        }
    } ;

    return motor::msl::perform_repl( std::move( code ), repls ) ;
}

motor::string_t generator::map_variable_type( motor::msl::type_cref_t type ) noexcept
{
    typedef std::pair< motor::msl::type_t, motor::string_t > mapping_t ;
    static mapping_t const __mappings[] =
    {
        mapping_t( motor::msl::type_t(), "unknown" ),
        mapping_t( motor::msl::type_t::as_void(), "void" ),
        mapping_t( motor::msl::type_t::as_bool(), "bool" ),
        mapping_t( motor::msl::type_t::as_int(), "int" ),
        mapping_t( motor::msl::type_t::as_vec2(motor::msl::type_base::tint), "int2" ),
        mapping_t( motor::msl::type_t::as_vec3(motor::msl::type_base::tint), "int3" ),
        mapping_t( motor::msl::type_t::as_vec4(motor::msl::type_base::tint), "int4" ),
        mapping_t( motor::msl::type_t::as_uint(), "uint" ),
        mapping_t( motor::msl::type_t::as_vec2(motor::msl::type_base::tuint), "uint2" ),
        mapping_t( motor::msl::type_t::as_vec3(motor::msl::type_base::tuint), "uint3" ),
        mapping_t( motor::msl::type_t::as_vec4(motor::msl::type_base::tuint), "uint4" ),
        mapping_t( motor::msl::type_t::as_float(), "float" ),
        mapping_t( motor::msl::type_t::as_vec2(), "float2" ),
        mapping_t( motor::msl::type_t::as_vec3(), "float3" ),
        mapping_t( motor::msl::type_t::as_vec4(), "float4" ),
        mapping_t( motor::msl::type_t::as_mat2(), "float2x2" ),
        mapping_t( motor::msl::type_t::as_mat3(), "float3x3" ),
        mapping_t( motor::msl::type_t::as_mat4(), "float4x4" ),
        mapping_t( motor::msl::type_t::as_tex1d(), "Texture1D" ),
        mapping_t( motor::msl::type_t::as_tex2d(), "Texture2D" ),
        mapping_t( motor::msl::type_t::as_tex2d_array(), "Texture2DArray" ),
        mapping_t( motor::msl::type_t::as_data_buffer(), "Buffer< float4 >" )

    } ;

    for( auto const& m : __mappings ) if( m.first == type ) return m.second ;

    return __mappings[ 0 ].second ;
}

motor::string_t generator::replace_types( motor::string_t code ) noexcept
{
    size_t p0 = 0 ;
    size_t p1 = code.find_first_of( ' ' ) ;
    while( p1 != std::string::npos )
    {
        auto const dist = p1 - p0 ;

        auto token = code.substr( p0, dist ) ;
        motor::msl::type_t const t = motor::msl::to_type( token ) ;
        if( t.base != motor::msl::type_base::unknown )
        {
            code.replace( p0, dist, this_t::map_variable_type( t ) ) ;
        }
        p0 = p1 + 1 ;
        p1 = code.find_first_of( ' ', p0 ) ;
    }

    return std::move( code ) ;
}

motor::string_t generator::map_variable_binding( motor::msl::shader_type const st,
    motor::msl::flow_qualifier const fq, motor::msl::binding binding ) noexcept
{
    typedef std::pair< motor::msl::binding, motor::string_t > mapping_t ;
    static mapping_t const __mappings[] =
    {
        mapping_t( motor::msl::binding::normal, "NORMAL" ),

        mapping_t( motor::msl::binding::color0, "COLOR0" ),
        mapping_t( motor::msl::binding::color1, "COLOR1" ),
        mapping_t( motor::msl::binding::color2, "COLOR2" ),
        mapping_t( motor::msl::binding::color3, "COLOR3" ),
        mapping_t( motor::msl::binding::color4, "COLOR4" ),
        mapping_t( motor::msl::binding::color5, "COLOR5" ),
        mapping_t( motor::msl::binding::color6, "COLOR6" ),
        mapping_t( motor::msl::binding::color7, "COLOR7" ),

        mapping_t( motor::msl::binding::texcoord0, "TEXCOORD0" ),
        mapping_t( motor::msl::binding::texcoord1, "TEXCOORD1" ),
        mapping_t( motor::msl::binding::texcoord2, "TEXCOORD2" ),
        mapping_t( motor::msl::binding::texcoord3, "TEXCOORD3" ),
        mapping_t( motor::msl::binding::texcoord4, "TEXCOORD4" ),
        mapping_t( motor::msl::binding::texcoord5, "TEXCOORD5" ),
        mapping_t( motor::msl::binding::texcoord6, "TEXCOORD6" ),
        mapping_t( motor::msl::binding::texcoord7, "TEXCOORD7" ),

        mapping_t( motor::msl::binding::vertex_id, "SV_VertexID" ),
        mapping_t( motor::msl::binding::primitive_id, "SV_PrimitiveID" ),
        mapping_t( motor::msl::binding::instance_id, "SV_InstanceID" )

    } ;

    static mapping_t const __mrt[] =
    {
        mapping_t( motor::msl::binding::color0, "SV_TARGET0" ),
        mapping_t( motor::msl::binding::color1, "SV_TARGET1" ),
        mapping_t( motor::msl::binding::color2, "SV_TARGET2" ),
        mapping_t( motor::msl::binding::color3, "SV_TARGET3" ),
        mapping_t( motor::msl::binding::color4, "SV_TARGET4" )
    } ;

    if( st == motor::msl::shader_type::vertex_shader && 
        fq == motor::msl::flow_qualifier::in && 
        binding == motor::msl::binding::position )
        return "POSITION" ;
    else if( binding == motor::msl::binding::position )
        return "SV_POSITION" ;

    // check render targets first
    {
        if( fq == motor::msl::flow_qualifier::out &&
            st == motor::msl::shader_type::pixel_shader &&
            motor::msl::is_color( binding ) )
        {
            for( auto const& m : __mrt ) if( m.first == binding ) return m.second ;
        }
    }

    // do other bindings
    {
        for( auto const& m : __mappings )
        {
            if( m.first == binding ) return m.second ;
        }
    }

    return "NO_BINDING_SPECIFIED" ;
}

motor::msl::generated_code_t::shaders_t generator::generate( motor::msl::generatable_cref_t genable_, 
    motor::msl::variable_mappings_cref_t var_map_ ) noexcept
{
    motor::msl::variable_mappings_t var_map = var_map_ ;
    motor::msl::generatable_t genable = genable_ ;

    // start renaming internal variables
    {
        for( auto& var : var_map ) 
        {
            if( var.fq == motor::msl::flow_qualifier::out &&
                var.st == motor::msl::shader_type::vertex_shader &&
                var.binding == motor::msl::binding::position )
            {
            }
        }
    }

    // exchange inout with in and out variables
    {
        auto iter = var_map.begin() ;
        while( iter != var_map.end() )
        {
            if( iter->fq == motor::msl::flow_qualifier::inout )
            {
                motor::msl::variable_mappings_t::value_type v = *iter ;
                {
                    v.fq = motor::msl::flow_qualifier::in ;
                    *iter = v ;
                }
                {
                    v.fq = motor::msl::flow_qualifier::out ;
                    iter = var_map.insert( iter, v ) + 1 ;
                }
            }
            ++iter ;
        }
    }

    // replace buildins
    {
        for( auto& s : genable.config.shaders )
        {
            for( auto& c : s.codes )
            {
                for( auto& l : c.lines )
                {
                    l = this_t::replace_buildin_symbols( std::move( l ) ) ;
                }
            }
        }

        for( auto& frg : genable.frags )
        {
            for( auto& f : frg.fragments )
            {
                //for( auto& l : c.lines )
                {
                    f = this_t::replace_buildin_symbols( std::move( f ) ) ;
                }
            }
        }
    }

    // inject composite buildins
    {
        auto test_configs = [&]( motor::msl::buildin_type const test_bi )
        {
            for( auto& s : genable.config.shaders )
            {
                for( auto& c : s.codes )
                {
                    for( auto& ubi : c.buildins)
                    {
                        if( test_bi == ubi.bi.t ) return true ;
                    }
                }
            }
            return false ;
        } ;

        auto test_fragments = [&]( motor::msl::buildin_type const test_bi )
        {
            for( auto& frg : genable.frags )
            {
                for( auto const & ubi : frg.buildins )
                {
                    if( test_bi == ubi.bi.t ) return true ;
                }
            }
            return false ;
        } ;

        for( size_t i=0; i<size_t(motor::msl::buildin_type::num_build_ins); ++i )
        {
            motor::msl::buildin_type test_bi = motor::msl::buildin_type(i) ;
            if( test_configs( test_bi ) || test_fragments( test_bi ) )
            {
                motor::msl::post_parse::library_t::fragment_t new_frg ;
                if( this_file::fragment_by_opcode( test_bi, new_frg ) ) 
                {
                    genable.frags.emplace_back( std::move( new_frg ) ) ;
                }
                continue ;
            }
        }
    }

    motor::msl::generated_code_t::shaders_t ret ;

    for( auto const& s : genable.config.shaders )
    {
        motor::msl::shader_type const s_type = s.type ;

        motor::msl::generated_code_t::shader_t shd ;

        for( auto& var : var_map )
        {
            if( var.st != s_type ) continue ;

            motor::msl::generated_code_t::variable_t v ;
            v.binding = var.binding ;
            v.fq = var.fq ;
            v.name = var.new_name ;
            shd.variables.emplace_back( std::move( v ) ) ;
        }

        // generate the code
        {
            if( s_type == motor::msl::shader_type::unknown )
            {
                motor::log::global_t::warning( "[hlsl generator] : unknown shader type" ) ;
                continue;
            }
            shd.type = s_type ;

            shd.codes.emplace_back( this_t::generate( genable, s, var_map, motor::msl::api_type::d3d11 ) ) ;
        }

        ret.emplace_back( std::move( shd ) ) ;
    }

    return std::move( ret ) ;

}

motor::msl::generated_code_t::code_t generator::generate( motor::msl::generatable_cref_t genable, motor::msl::post_parse::config_t::shader_cref_t s, 
    motor::msl::variable_mappings_cref_t var_mappings, motor::msl::api_type const type ) noexcept
{
    motor::msl::generated_code_t::code code ;

    motor::stringstream_t text ;

    motor::msl::shader_type_array_t shader_types ;

    // getting all involved shader types so interface 
    // blocks can be formed later on.
    {
        for( auto & i : shader_types ) i = motor::msl::shader_type::unknown ;

        size_t i=0; 
        for( auto const & shd : genable.config.shaders ) shader_types[i++] = shd.type ;
        motor::msl::sort_shader_type_array( shader_types ) ;
    }

    motor::msl::shader_type const sht_cur = s.type ;
    motor::msl::shader_type const sht_before = motor::msl::shader_type_before( s.type, shader_types ) ;
    motor::msl::shader_type const sht_after = motor::msl::shader_type_after( s.type, shader_types ) ;

    motor::string_t const input_struct_name = this_file::determine_input_structure_name( sht_cur, sht_before ) ;
    motor::string_t const output_struct_name = this_file::determine_output_structure_name( sht_cur, sht_after ) ;

    text << "// config name: " << genable.config.name << std::endl << std::endl ;

    // make prototypes declarations from function signatures
    // the prototype help with not having to sort funk definitions
    {
        text << "// Declarations // " << std::endl ;
        for( auto const& f : genable.frags )
        {
            text << this_t::map_variable_type( f.sig.return_type ) << " " ;
            text << f.sym_long.expand( "_" ) << " ( " ;
            for( auto const& a : f.sig.args )
            {
                text << this_t::map_variable_type( a.type )
                    << " " << a.name << ", " ;
            }
            text.seekp( -2, std::ios_base::end ) ;
            text << " ) ; " << std::endl ;
        }
        text << std::endl ;
    }

    // make all functions with replaced symbols
    {
        text << "// Definitions // " << std::endl ;
        for( auto const& f : genable.frags )
        {
            // make signature
            {
                text << this_t::map_variable_type( f.sig.return_type ) << " " ;
                text << f.sym_long.expand( "_" ) << " ( " ;
                for( auto const& a : f.sig.args )
                {
                    text << this_t::map_variable_type( a.type ) + " " + a.name + ", "  ;
                }
                text.seekp( -2, std::ios_base::end ) ;
                text << " )" << std::endl ;
            }

            // make body
            {
                text << "{" << std::endl ;
                for( auto const& l : f.fragments )
                {
                    text << this_t::replace_types( l ) << std::endl ;
                }
                text << "}" << std::endl ;
            }
        }
        text << std::endl ;
    }

    // textures
    {
        text << "// Textures //" << std::endl ;

        size_t tx_id = 0 ;

        for( auto const& v : s.variables )
        {
            if( v.fq != motor::msl::flow_qualifier::global ) continue ;
            if( !v.type.is_texture() ) continue ;

            motor::string_t const type_ = this_t::map_variable_type( v.type ) ;
            motor::string_t const register_tx_ =
                "register(t" + motor::to_string( tx_id ) + ") ;" ;
            motor::string_t const register_sp_ =
                "register(s" + motor::to_string( tx_id ) + ") ;" ;

            text << type_ << " " << v.name << " : " << register_tx_ << std::endl ;
            text << "SamplerState smp_" << v.name << " : " << register_sp_ << std::endl ;
            ++tx_id ;
        }
        text << std::endl ;
    }

    // cbuffers
    {
        text << "// Constant Buffers //" << std::endl ;

        text << "cbuffer ConstantBuffer : register( b0 ) " << std::endl ;
        text << "{" << std::endl ;
        for( auto const& v : s.variables )
        {
            if( v.fq != motor::msl::flow_qualifier::global ) continue ;
            if( v.type.is_texture() ) continue ;

            text << this_t::map_variable_type( v.type ) << " " << v.name << " ;" << std::endl ;
        }
        text << "}" << std::endl << std::endl ;
    }


    text << "// Inputs/Outputs //" << std::endl ;

    // inputs
    if( sht_cur == motor::msl::shader_type::vertex_shader )
    {
        text << "struct " << input_struct_name << std::endl ;
        text << "{" << std::endl ;
        for( auto const& v : var_mappings )
        {
            if( v.st != motor::msl::shader_type::vertex_shader ) continue ;
            if( v.fq != motor::msl::flow_qualifier::in ) continue ;
            if( v.fq == motor::msl::flow_qualifier::local ) continue ;
            
            motor::string_t name = v.new_name ;
            motor::string_t const type_ = this_t::map_variable_type( v.t ) ;
            motor::string_t const binding_ = this_t::map_variable_binding( s.type, v.fq, v.binding ) ;

            text << type_ << " " << name << " : " << binding_ << " ;" << std::endl ;
        }
        text << "} ;" << std::endl << std::endl ;
    }
    // inputs from previous outputs
    else 
    {
        text << "struct " << input_struct_name << std::endl ;
        text << "{" << std::endl ;
        for( auto const& v : var_mappings )
        {
            if( v.st != sht_before ) continue ;
            if( v.fq != motor::msl::flow_qualifier::out ) continue ;
            if( v.fq == motor::msl::flow_qualifier::local ) continue ;
            
            motor::string_t name = v.new_name ;
            motor::string_t const type_ = this_t::map_variable_type( v.t ) ;
            motor::string_t const binding_ = this_t::map_variable_binding( s.type, motor::msl::flow_qualifier::in, v.binding ) ;

            text << type_ << " " << name << " : " << binding_ << " ;" << std::endl ;
        }
        text << "} ;" << std::endl << std::endl ;
    }

    // outputs
    {
        text << "struct " << output_struct_name << std::endl ;
        text << "{" << std::endl ;
        for( auto const& v : s.variables )
        {
            if( v.fq != motor::msl::flow_qualifier::out ) continue ;

            motor::string_t name = v.name ;
            motor::string_t const type_ = this_t::map_variable_type( v.type ) ;
            motor::string_t const binding_ = this_t::map_variable_binding( s.type, v.fq, v.binding ) ;

            size_t const idx = motor::msl::find_by( var_mappings, v.name, v.binding, v.fq, s.type ) ;
            if( idx != size_t( -1 ) )
            {
                name = var_mappings[ idx ].new_name ;
            }

            text << type_ << " " << name << " : " << binding_ << " ;" << std::endl ;
        }
        text << "} ;" << std::endl << std::endl ;
    }


    text << "// The shader // " << std::endl ;
    {
        motor::string_t funk_name = "UNKNOWN" ;
        if( s.type == motor::msl::shader_type::vertex_shader ) funk_name = "VS" ;
        else if( s.type == motor::msl::shader_type::geometry_shader ) funk_name = "GS" ;
        else if( s.type == motor::msl::shader_type::pixel_shader ) funk_name = "PS" ;

        size_t num_in_verts = 0 ;
        size_t max_vertex_count = 0 ;
        motor::string_t input_prim_name ;
        motor::string_t output_stream_name ;

        for( auto const & pd : s.primitive_decls ) 
        {
            if( pd.fq == motor::msl::flow_qualifier::out )
            {
                max_vertex_count = pd.max_vertices ;

                switch( pd.pdt )
                {
                case motor::msl::primitive_decl_type::points: 
                    output_stream_name = "PointStream" ; break ;
                case motor::msl::primitive_decl_type::lines: 
                    output_stream_name = "LineStream" ; break ;
                case motor::msl::primitive_decl_type::triangles: 
                    output_stream_name = "TriangleStream" ; break ;
                default: break ;
                }
            }

            if( pd.fq == motor::msl::flow_qualifier::in )
            {
                switch( pd.pdt )
                {
                case motor::msl::primitive_decl_type::points: 
                    num_in_verts = 1 ; input_prim_name = "point " ; break ;
                case motor::msl::primitive_decl_type::lines: 
                    num_in_verts = 2 ; input_prim_name = "line " ; break ;
                case motor::msl::primitive_decl_type::triangles: 
                    num_in_verts = 3 ; input_prim_name = "triangle " ; break ;
                default: break ;
                }
            }
        }

        motor::string_t locals ;

        // locals
        {
            for( auto const& v : s.variables )
            {
                if( v.fq != motor::msl::flow_qualifier::local ) continue ;

                motor::string_t name = v.name ;
                motor::string_t const type_ = this_t::map_variable_type( v.type ) ;
                motor::string_t const binding_ = this_t::map_variable_binding( s.type, v.fq, v.binding ) ;

                // the new name will be replaced further down the road
                locals += ", " + type_ + " " + name + " : " + binding_ ;
            }
        }

        auto cpy_codes = s.codes ;
        for( auto& c : cpy_codes )
        {
            size_t curlies = 0 ;
            bool_t in_main = false ;
            for( auto iter = c.lines.begin(); iter != c.lines.end(); ++iter )
            {
                if( iter->find( "void main" ) != std::string::npos )
                {
                    if( s.type == motor::msl::shader_type::vertex_shader )
                    {
                        text << output_struct_name << " " << funk_name << " ( " << input_struct_name << " __input__" << locals << " )" << std::endl ;
                        text << "{" << std::endl ; ++iter ;
                        text << output_struct_name << " __output__ = (" << output_struct_name << ")0 ; " << std::endl ;
                    } 
                    else if( s.type == motor::msl::shader_type::geometry_shader )
                    {
                        text << "[ maxvertexcount( " << motor::to_string( max_vertex_count ) << " ) ]" << std::endl ;
                        text << "void " << funk_name << " ( " 
                            << input_prim_name << input_struct_name << " __input__["<< motor::to_string( num_in_verts ) <<"], " 
                            << "inout " << output_stream_name << "<" << output_struct_name << ">" << " prim_stream" << locals << " )" << std::endl ;
                        text << "{" << std::endl ; ++iter ;
                        text << output_struct_name << " __output__ = (" << output_struct_name << ")0 ; " << std::endl ;
                    }
                    else if( s.type == motor::msl::shader_type::pixel_shader )
                    {
                        text << output_struct_name << " PS( " << input_struct_name << " __input__ )" << std::endl ;
                        text << "{" << std::endl ; ++iter ;
                        text << output_struct_name << " __output__ = (" << output_struct_name << ")0 ; " << std::endl ;
                    }
                    curlies++ ;
                    in_main = true ;
                } 
                // early exit
                else if( in_main && iter->find( "return ;" ) != std::string::npos )
                {
                    if( sht_cur != motor::msl::shader_type::geometry_shader )
                        text << "return __output__ ;" << std::endl ;
                    else
                        text << "return ;" << std::endl ;
                }
                else
                {
                    if( in_main && *iter == "{" ) curlies++ ;
                    else if( in_main && *iter == "}" ) curlies--  ;

                    if( in_main && curlies == 0 && sht_cur != motor::msl::shader_type::geometry_shader )
                    {
                        text << "return __output__ ;" << std::endl ;
                        in_main = false ;
                    }
                    text << " " << *iter << std::endl ;
                }

                
            }
        }
    }

    auto shd = text.str() ;

    if( s.type == motor::msl::shader_type::geometry_shader )
    {
        size_t num_in_verts = 0 ;
        for( auto const & pd : s.primitive_decls ) 
        {
            if( pd.fq == motor::msl::flow_qualifier::in )
            {
                switch( pd.pdt )
                {
                case motor::msl::primitive_decl_type::points: 
                    num_in_verts = 1 ;  break ;
                case motor::msl::primitive_decl_type::lines: 
                    num_in_verts = 2 ; break ;
                case motor::msl::primitive_decl_type::triangles: 
                    num_in_verts = 3 ; break ;
                default: break ;
                }
            }
        }
        std::regex rex( "in.length *\\( *\\) *") ;
        shd = std::regex_replace( shd, rex, motor::to_string(num_in_verts) ) ; 
    }

    // variable dependencies
    {
        for( auto const& v : genable.vars )
        {
            auto const find_what = v.sym_long.expand() + " " ;
            size_t p0 = shd.find( find_what ) ; 
                    
            while( p0 != std::string::npos ) 
            {
                size_t const p1 = shd.find_first_of( " ", p0 ) ;
                shd = shd.substr( 0, p0 ) + this_t::replace_buildin_symbols( motor::string_t(v.value) ) + shd.substr( p1 ) ;
                p0 = shd.find( find_what, p1 ) ;
            }
        }
    }

    // fragment dependencies
    {
        for( auto const& f : genable.frags )
        {
            for( auto const& d : f.deps )
            {
                size_t p0 = shd.find( d.expand() ) ; 
                    
                while( p0 != std::string::npos ) 
                {
                    size_t const p1 = shd.find_first_of( " ", p0 ) ;
                    shd = shd.substr( 0, p0 ) + d.expand( "_" ) + shd.substr( p1 ) ;
                    p0 = shd.find( d.expand(), p1 ) ;
                }
            }
        }
    }

    // shader dependencies
    {
        for( auto const& d : s.deps )
        {
            size_t p0 = shd.find( d.expand() ) ; 
                    
            while( p0 != std::string::npos ) 
            {
                size_t const p1 = shd.find_first_of( " ", p0 ) ;
                shd = shd.substr( 0, p0 ) + d.expand( "_" ) + shd.substr( p1 ) ;
                p0 = shd.find( d.expand(), p1 ) ;
            }
        }
    }

    // replace in code in/out/globals
    {
        size_t const off = shd.find( "// The shader" ) ;

        // sub-optimal, but since the usage of regexp, the sub string
        // need to be processed so not the whole file is processed.
        motor::string_t sub = shd.substr( off ) ;

        for( auto const& v : var_mappings )
        {
            if( v.st != s.type ) continue ;
            if( v.fq == motor::msl::flow_qualifier::local ) continue ;

            motor::string_t flow ;
            motor::string_t struct_name ;

            if( v.fq == motor::msl::flow_qualifier::in )
            {
                flow = "in" ;
                struct_name = "__input__" ;
            } else if( v.fq == motor::msl::flow_qualifier::out )
            {
                flow = "out" ;
                struct_name = "__output__" ;
            } 

            std::regex rex( flow + " *(\\[ *[0-9]*[a-z]* *\\])? *\\." + v.old_name ) ;
            sub = std::regex_replace( sub, rex, " " + struct_name + "$1." + v.new_name  ) ; 
        }

        for( auto const& v : var_mappings )
        {
            if( v.st != s.type ) continue ;
            if( v.fq != motor::msl::flow_qualifier::local ) continue ;

            std::regex rex(  v.old_name + "(.*)" ) ;
            sub = std::regex_replace( sub, rex, " " + v.new_name + "$1" ) ; 
        }

        shd = shd.substr( 0, off ) + sub ;
    }

    {
        shd = std::regex_replace( shd,
            std::regex( " __float__ \\( ([0-9]+) \\, ([0-9]+) \\) " ),
            " $1.$2 " ) ;
        shd = std::regex_replace( shd,
            std::regex( " __uint__ \\( ([0-9]+) \\) " ),
            " $1u " ) ;
        shd = std::regex_replace( shd,
            std::regex( " __int__ \\( ([0-9]+) \\) " ),
            " $1 " ) ;
    }

    {
        shd = this_t::replace_types( std::move( shd ) ) ;
    }

    code.shader = shd ;
    code.api = type ;

    return std::move( code ) ;
}