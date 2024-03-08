#include "glsl4_generator.h"

#include <motor/log/global.h>

#include <motor/std/sstream>
#include <regex>

using namespace motor::msl::glsl ;

namespace this_file_glsl4
{
    motor::core::types::bool_t fragment_by_opcode( motor::msl::buildin_type bit, motor::msl::post_parse::library_t::fragment_out_t ret ) noexcept
    {
        if( bit == motor::msl::buildin_type::rand1 ) 
        {
            motor::msl::signature_t::arg_t a { motor::msl::type_t::as_float(), "x" } ;

            motor::msl::signature_t sig = motor::msl::signature_t
            { 
                motor::msl::type_t::as_float(), "__glsl4_rand_1__", { a } 
            } ;

            motor::vector< motor::string_t > lines 
            { 
                "return fract ( sin(n) * 43758.5453123 );"
            } ;

            motor::msl::post_parse::library_t::fragment_t frg ;
            frg.sym_long = motor::msl::symbol_t("__glsl4_rand_1__") ;
            frg.sig = std::move( sig ) ;
            frg.fragments = std::move( lines ) ;

            ret = std::move( frg ) ;

            return true ;
        }

        else if( bit == motor::msl::buildin_type::noise1 ) 
        {
            motor::msl::signature_t::arg_t a { motor::msl::type_t::as_float(), "x" } ;

            motor::msl::signature_t sig = motor::msl::signature_t
            { 
                motor::msl::type_t::as_float(), "__glsl4_noise_1__", { a } 
            } ;

            motor::vector< motor::string_t > lines 
            { 
                "float f1 = floor ( x ) ;",
                "float fc = fract ( x ) ;",
                "return lerp ( rand1 ( f1 ) , rand1 ( f1 + 1.0 ) , fc ) ;"
            } ;

            motor::msl::post_parse::library_t::fragment_t frg ;
            frg.sym_long = motor::msl::symbol_t("__glsl4_noise_1__") ;
            frg.sig = std::move( sig ) ;
            frg.fragments = std::move( lines ) ;
            
            motor::msl::post_parse::used_buildin_t ubi = 
            {
                0, 0, motor::msl::get_build_in( motor::msl::buildin_type::rand1 )
            } ;
            frg.buildins.emplace_back( ubi ) ;

            ret = std::move( frg ) ;

            return true ;
        }

        return false ;
    }
}

//*******************************************************************************************************
motor::string_t glsl4_generator::replace_buildin_symbols( motor::string_rref_t code ) noexcept
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
                return args[ 0 ] + " * " + args[ 1 ] ;
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
                return args.size() == 1 ? "+ " + args[ 0 ] : args[ 0 ] + " + " + args[ 1 ] ;
            }
        },
        {
            motor::string_t( ":sub:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() == 0 || args.size() > 2 ) return "sub ( INVALID_ARGS ) " ;
                return args.size() == 1 ? "- " + args[ 0 ] : args[ 0 ] + " - " + args[ 1 ] ;
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
                return  "texture( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":rt_texcoords:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "rt_texcoords ( INVALID_ARGS ) " ;
                return  args[0] ;
            }
        },
        {
            motor::string_t( ":rt_texture:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "rt_texture ( INVALID_ARGS ) " ;
                return  "texture( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":rt_texture_offset:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "rt_texture_offset ( INVALID_ARGS ) " ;
                return  "textureOffset( " + args[ 0 ] + " , " + args[ 1 ] + " , " + args[2] + " ) " ;
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
                if( args.size() != 2 ) return "lessThan ( INVALID_ARGS ) " ;
                return "lessThan( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":gt_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "greaterThan ( INVALID_ARGS ) " ;
                return "greaterThan( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":lte_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "lessThanEqual ( INVALID_ARGS ) " ;
                return "lessThanEqual( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":gte_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "greaterThanEqual ( INVALID_ARGS ) " ;
                return "greaterThanEqual( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":eqeq_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "equal ( INVALID_ARGS ) " ;
                return "equal( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":neq_vec:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "notEqual ( INVALID_ARGS ) " ;
                return "notEqual( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
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
                return "return " + args[ 0 ] ;
            }
        },
        {
            motor::string_t( ":mix:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "mix ( INVALID_ARGS ) " ;
                return "mix ( " + args[ 0 ] + " , " + args[ 1 ] + " , " + args[ 2 ] + " ) " ;
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
                if( args.size() != 1 ) return "fract ( INVALID_ARGS ) " ;
                return "fract ( " + args[ 0 ] + " ) " ;
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
            motor::string_t( ":as_vec2:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "as_vec2 ( INVALID_ARGS ) " ;
                return "vec2 ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":as_vec3:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "as_vec3 ( INVALID_ARGS ) " ;
                return "vec3 ( " + args[ 0 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":as_vec4:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 1 ) return "as_vec4 ( INVALID_ARGS ) " ;
                return "vec4 ( " + args[ 0 ] + " ) " ;
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
                return args[0] + " " + args[1] + " [ " + args[2] + " ] = " + args[0] + " [ " + args[2] + " ] " + " ( " + tmp + " ) " ;
            }
        },
        {
            motor::string_t( ":texture_offset:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 3 ) return "texture_offset ( INVALID_ARGS ) " ;
                return  "textureOffset ( " + args[ 0 ] + " , " + args[ 1 ] + " , " + args[2] + " ) " ;
            }
        },
        {
            motor::string_t( ":texture_dims:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() == 1 ) return "textureSize( " + args[ 0 ] + " , 0 ) " ;
                if( args.size() == 2 ) return "textureSize( " + args[ 0 ] + " , " + args[1] + " ) " ;

                return "texture_dims ( INVALID_ARGS ) " ;
            }
        },
        {
            motor::string_t( ":emit_vertex:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 0 ) return "emit_vertex( INVALID_ARGS ) " ;
                

                return "EmitVertex ( ) " ;
            }
        },
        {
            motor::string_t( ":end_primitive:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 0 ) return "end_primitive( INVALID_ARGS ) " ;
                return "EndPrimitive ( ) " ;
            }
        },
        {
            motor::string_t( ":fetch_data:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() != 2 ) return "fetch_data ( INVALID_ARGS ) " ;
                return "texelFetch ( " + args[ 0 ] + ", " + args[ 1 ] + " ) " ;
            }
        },
        {
            motor::string_t( ":rand_1:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() == 1 ) return "__glsl4_rand_1__( " + args[ 0 ] + " ) " ;
                if( args.size() == 2 ) return "__glsl4_rand_1__( " + args[ 0 ] + " ) " ;

                return "rand_1 ( INVALID_ARGS ) " ;
            }
        },
        {
            motor::string_t( ":noise_1:" ),
            [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
            {
                if( args.size() == 1 ) return "__glsl4_noise_1__( " + args[ 0 ] + " ) " ;
                if( args.size() == 2 ) return "__glsl4_noise_1__( " + args[ 0 ] + " ) " ;

                return "texture_dims ( INVALID_ARGS ) " ;
            }
        }
    } ;

    return motor::msl::perform_repl( std::move( code ), repls ) ;
}

namespace this_file
{
    typedef std::pair< motor::msl::type_t, char const * const > mapping_t ;

    //******************************************************************************************************************************************
    static mapping_t map_variable_type( motor::msl::api_type const apit, motor::msl::type_cref_t type ) noexcept
    {
        if( apit == motor::msl::api_type::gl4 )
        {
            static mapping_t const __mappings[] =
            {
                mapping_t( motor::msl::type_t(), "unknown" ),
                mapping_t( motor::msl::type_t::as_void(), "void" ),
                mapping_t( motor::msl::type_t::as_bool(), "bool" ),
                mapping_t( motor::msl::type_t::as_int(), "int" ),
                mapping_t( motor::msl::type_t::as_vec2(motor::msl::type_base::tint), "ivec2" ),
                mapping_t( motor::msl::type_t::as_vec3(motor::msl::type_base::tint), "ivec3" ),
                mapping_t( motor::msl::type_t::as_vec4(motor::msl::type_base::tint), "ivec4" ),
                mapping_t( motor::msl::type_t::as_uint(), "uint" ),
                mapping_t( motor::msl::type_t::as_vec2(motor::msl::type_base::tint), "uvec2" ),
                mapping_t( motor::msl::type_t::as_vec3(motor::msl::type_base::tint), "uvec3" ),
                mapping_t( motor::msl::type_t::as_vec4(motor::msl::type_base::tint), "uvec4" ),
                mapping_t( motor::msl::type_t::as_float(), "float" ),
                mapping_t( motor::msl::type_t::as_vec2(), "vec2" ),
                mapping_t( motor::msl::type_t::as_vec3(), "vec3" ),
                mapping_t( motor::msl::type_t::as_vec4(), "vec4" ),
                mapping_t( motor::msl::type_t::as_mat2(), "mat2" ),
                mapping_t( motor::msl::type_t::as_mat3(), "mat3" ),
                mapping_t( motor::msl::type_t::as_mat4(), "mat4" ),
                mapping_t( motor::msl::type_t::as_tex1d(), "sampler1D" ),
                mapping_t( motor::msl::type_t::as_tex2d(), "sampler2D" ),
                mapping_t( motor::msl::type_t::as_tex2d_array(), "sampler2DArray" ),
                mapping_t( motor::msl::type_t::as_data_buffer(), "samplerBuffer" )
            } ;

            for( auto const& m : __mappings ) if( m.first == type ) return m ;

            return __mappings[ 0 ] ;
        }

        return mapping_t( motor::msl::type_t(), "unknown" ) ;
    }

    static motor::string_t map_variable_type_to_string( motor::msl::api_type const apit, motor::msl::type_cref_t type ) noexcept
    {
        return this_file::map_variable_type( apit, type ).second ;
    }


    static motor::string_t input_decl_type_to_string( motor::msl::primitive_decl_type const pd ) noexcept
    {
        switch( pd )
        {
        case motor::msl::primitive_decl_type::points: return "points" ; 
        case motor::msl::primitive_decl_type::lines: return "lines" ; 
        case motor::msl::primitive_decl_type::triangles: return "triangles" ; 
        default: break ;
        }
        return "UNKNOWN" ;
    }

    static motor::string_t output_decl_type_to_string( motor::msl::primitive_decl_type const pd ) noexcept
    {
        switch( pd )
        {
        case motor::msl::primitive_decl_type::points: return "points" ; 
        case motor::msl::primitive_decl_type::lines: return "line_strip" ; 
        case motor::msl::primitive_decl_type::triangles: return "triangle_strip" ; 
        default: break ;
        }
        return "UNKNOWN" ;
    }
}

//******************************************************************************************************************************************
motor::string_t glsl4_generator::to_texture_type( motor::msl::type_cref_t t ) noexcept
{
    typedef std::pair< motor::msl::type_ext, char const * const > __mapping_t ;

    static __mapping_t const __mappings[] = {
        __mapping_t( motor::msl::type_ext::unknown, "unknown" ),
        __mapping_t( motor::msl::type_ext::texture_1d, "sampler1D" ),
        __mapping_t( motor::msl::type_ext::texture_2d, "sampler2D" )
    } ;

    for( auto const& m : __mappings ) if( m.first == t.ext ) return m.second ;
    return __mappings[ 0 ].second ;
}

//******************************************************************************************************************************************
motor::string_t glsl4_generator::replace_types( motor::msl::api_type const apit, motor::string_t code ) noexcept
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
            code.replace( p0, dist, this_file::map_variable_type_to_string( apit, t ) ) ;
        }
        p0 = p1 + 1 ;
        p1 = code.find_first_of( ' ', p0 ) ;
    }

    return std::move( code ) ;
}

//******************************************************************************************************************************************
motor::string_t glsl4_generator::determine_input_interface_block_name( motor::msl::shader_type const cur, motor::msl::shader_type const before ) noexcept 
{
    // input assembler to vertex shader -> not possible in glsl
    if( cur == motor::msl::shader_type::vertex_shader )
        return "ia_to_" + motor::msl::short_name( cur ) ;

    return motor::msl::short_name( before ) + "_to_" + motor::msl::short_name( cur ) ;
}

//******************************************************************************************************************************************
motor::string_t glsl4_generator::determine_output_interface_block_name( motor::msl::shader_type const cur, motor::msl::shader_type const after ) noexcept 
{
    // -> not possible in glsl
    if( cur == motor::msl::shader_type::pixel_shader )
        return "into_the_pixel_pot" ;
    
    if( cur != motor::msl::shader_type::pixel_shader && 
        after == motor::msl::shader_type::unknown )
        return "streamout" ; // must be streamout!!

    return motor::msl::short_name( cur ) + "_to_" + motor::msl::short_name( after ) ;
}

//******************************************************************************************************************************************
motor::msl::generated_code_t::shaders_t glsl4_generator::generate( motor::msl::generatable_cref_t genable_, 
    motor::msl::variable_mappings_cref_t var_map_ ) noexcept
{
    motor::msl::variable_mappings_t var_map = var_map_ ;
    motor::msl::generatable_t genable = genable_ ;

    // start renaming internal variables
    {
        for( auto& var : var_map )
        {
            if( var.binding == motor::msl::binding::vertex_id )
            {
                var.new_name = "gl_VertexID" ;
            }
            else if( var.binding == motor::msl::binding::primitive_id )
            {
                if( var.st == motor::msl::shader_type::geometry_shader ) var.new_name = "gl_PrimitiveIDIn" ;
                else var.new_name = "gl_PrimitiveID" ;
            }
            else if( var.binding == motor::msl::binding::instance_id )
            {
                var.new_name = "gl_InstanceID" ;
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
        {
            size_t accum = 0 ;

            // accumulate number of entries
            {
                for( auto& s : genable.config.shaders )
                {
                    for( auto& c : s.codes )
                    {
                        accum += c.buildins.size() ;
                    }
                }
                for( auto& frg : genable.frags )
                {
                    accum += frg.buildins.size() ;
                }
            }

            motor::msl::post_parse::used_buildins_t tmp ;
            tmp.reserve( accum + 10 ) ;

            // fill build-ins that need to be processed
            // in the 1st pass.
            {
                size_t offset = 0 ;
                for( auto& s : genable.config.shaders )
                {
                    for( auto& c : s.codes )
                    {
                        for( auto & ubi : c.buildins ) 
                        {
                            if( std::find_if( tmp.begin(), tmp.end(), [&]( decltype(ubi) const & d )
                            {
                                return d.bi.t == ubi.bi.t ;
                            } ) != tmp.end() ) continue ;

                            tmp.emplace_back( ubi ) ;
                        }
                    }
                }

                for( auto& frg : genable.frags )
                {
                    for( auto & ubi : frg.buildins ) 
                    {
                        if( std::find_if( tmp.begin(), tmp.end(), [&]( decltype(ubi) const & d )
                        {
                            return d.bi.t == ubi.bi.t ;
                        } ) != tmp.end() ) continue ;

                        tmp.emplace_back( ubi ) ;
                    }
                }
            }

            for( auto iter = tmp.begin(); iter != tmp.end(); ++iter )
            {
                auto & ubi = *iter ;

                motor::msl::buildin_type test_bi = ubi.bi.t ;
                {
                    motor::msl::post_parse::library_t::fragment_t new_frg ;
                    if( this_file_glsl4::fragment_by_opcode( test_bi, new_frg ) ) 
                    {
                        // need further testing for build-ins using build-ins!
                        for( auto & new_ubi : new_frg.buildins )
                        {
                            if( std::find_if( tmp.begin(), tmp.end(), [&]( decltype(new_ubi) const & d )
                            {
                                return d.bi.t == new_ubi.bi.t ;
                            } ) != tmp.end() ) continue ;

                            tmp.emplace_back( new_ubi ) ;
                        }

                        genable.frags.emplace_back( std::move( new_frg ) ) ;
                    }
                    continue ;
                }
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
                motor::log::global_t::warning( "[glsl generator] : unknown shader type" ) ;
                continue;
            }
            shd.type = s_type ;

            shd.codes.emplace_back( this_t::generate( genable, s, var_map, motor::msl::api_type::gl4 ) ) ;
        }

        ret.emplace_back( std::move( shd ) ) ;
    }

    return std::move( ret ) ;
}

//******************************************************************************************************************************************
motor::msl::generated_code_t::code_t glsl4_generator::generate( motor::msl::generatable_cref_t genable, motor::msl::post_parse::config_t::shader_cref_t shd_, 
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

    motor::msl::shader_type const sht_cur = shd_.type ;
    motor::msl::shader_type const sht_before = motor::msl::shader_type_before( shd_.type, shader_types ) ;
    motor::msl::shader_type const sht_after = motor::msl::shader_type_after( shd_.type, shader_types ) ;

    // mainly used if the input position binding should be omitted in the interface block and
    // being replaced by reading from the gl_Position and 
    // writing to the output position binding should be replaced by gl_Position
    bool_t const using_transform_feedback = sht_cur != motor::msl::shader_type::pixel_shader &&
        sht_after == motor::msl::shader_type::unknown ;

    // 1. glsl stuff at the front
    {
        switch( type )
        {
        case motor::msl::api_type::gl4:
            text << "#version 400 core" << " // " << genable.config.name << std::endl << std::endl ;
            break ;
        default:
            text << "#version " << "glsl_type case missing" << std::endl << std::endl ;
            break ;
        }
    }

    // add extensions for pixel shader
    if( shd_.type == motor::msl::shader_type::pixel_shader )
    {
        size_t num_color = 0 ;
        for( auto const& var : shd_.variables )
        {
            num_color += motor::msl::is_color( var.binding ) ? 1 : 0 ;
        }

        // mrt requires extensions for glsl 130
        if( num_color > 1 && type == motor::msl::api_type::gl4 )
        {
            text <<
                "#extension GL_ARB_separate_shader_objects : enable" << std::endl <<
                "#extension GL_ARB_explicit_attrib_location : enable" << std::endl << std::endl ;
        }
    }

    // geometry shader primitive decls
    if( sht_cur == motor::msl::shader_type::geometry_shader )
    {
        for( auto const & pd : shd_.primitive_decls )
        {
            if( pd.fq == motor::msl::flow_qualifier::in )
            {
                text << "layout ( " << this_file::input_decl_type_to_string(pd.pdt) << " ) in ;" << std::endl ;
            }
            else if( pd.fq == motor::msl::flow_qualifier::out )
            {
                text << "layout ( " << this_file::output_decl_type_to_string(pd.pdt) << ", max_vertices = " << 
                    motor::to_string(pd.max_vertices) << ")  out ;" << std::endl;
            }
        }

        text << std::endl ;
    }

    // 2. make prototypes declarations from function signatures
    // the prototype help with not having to sort funk definitions
    {
        text << "// Declarations // " << std::endl ;
        for( auto const& f : genable.frags )
        {
            text << this_file::map_variable_type_to_string( type, f.sig.return_type ) << " " ;
            text << f.sym_long.expand( "_" ) << " ( " ;
            for( auto const& a : f.sig.args )
            {
                text << this_file::map_variable_type_to_string( type, a.type ) + ", " ;
            }

            text.seekp( -2, std::ios_base::end ) ;
            text << " ) ; " << std::endl ;
        }
        text << std::endl ;
    }

    // 3. make all functions with replaced symbols
    {
        text << "// Definitions // " << std::endl ;
        for( auto const& f : genable.frags )
        {
            // make signature
            {
                text << this_file::map_variable_type_to_string( type, f.sig.return_type ) << " " ;
                text << f.sym_long.expand( "_" ) << " ( " ;
                for( auto const& a : f.sig.args )
                {
                    text << this_file::map_variable_type_to_string( type, a.type ) + " " + a.name + ", "  ;
                }
                text.seekp( -2, std::ios_base::end ) ;
                text << " )" << std::endl ;
            }

            // make body
            {
                text << "{" << std::endl ;
                for( auto l : f.fragments )
                {
                    //l = this_t::replace_buildin_symbols( std::move( l ) ) ;
                    text << this_t::replace_types( type, l ) << std::endl ;
                }
                text << "}" << std::endl ;
            }
        }
        text << std::endl ;
    }

    // 4. make all glsl uniforms from shader variables
    {
        size_t num_color = 0 ;
        for( auto const& var : shd_.variables )
        {
            num_color += motor::msl::is_color( var.binding ) ? 1 : 0 ;
        }

        size_t layloc_id = 0 ;
        text << "// Uniforms and in/out // " << std::endl ;

        // in/out interface block
        {
            // all ins
            {
                // no intput interface block in the vertex shader
                if( sht_cur != motor::msl::shader_type::vertex_shader )
                {
                    text << "in " << this_t::determine_input_interface_block_name( sht_cur, sht_before ) << std::endl ;
                    text << "{" << std::endl ;
                }

                for( auto & v : shd_.variables )
                {
                    if( v.fq != motor::msl::flow_qualifier::in ) continue ;

                    // omit system variables
                    //if( v.binding == motor::msl::binding::position && v.fq == motor::msl::flow_qualifier::out ) continue ;
                    if( v.binding == motor::msl::binding::vertex_id ) continue ;
                    if( v.binding == motor::msl::binding::instance_id ) continue ;
                    if( v.binding == motor::msl::binding::primitive_id ) continue ;

                    // don not generate input position except if coming into the vertex shader
                    if( sht_cur != motor::msl::shader_type::vertex_shader && 
                        v.binding == motor::msl::binding::position  ) continue ;

                    motor::string_t name = v.name ;
                    motor::string_t const type_ = this_file::map_variable_type_to_string( type, v.type ) ;

                    {
                        size_t const idx = motor::msl::find_by( var_mappings, v.name, v.binding, v.fq, shd_.type ) ;
                        if( idx < var_mappings.size() )
                        {
                            name = var_mappings[ idx ].new_name ;
                        }
                    }

                    // no intput interface block in the vertex shader
                    if( sht_cur == motor::msl::shader_type::vertex_shader ) text << "in" ;

                    text << " " << type_ << " " << name << " ; " << std::endl ;
                }

                // no intput interface block in the vertex shader
                if( sht_cur != motor::msl::shader_type::vertex_shader )
                {
                    text << "} stage_in" << (sht_cur == motor::msl::shader_type::geometry_shader ? "[] " : " ") << ";" << std::endl << std::endl ;
                }
                else text << std::endl ;
            }

            // all outs
            {
                // no output interface block in the fragment shader
                if( sht_cur != motor::msl::shader_type::pixel_shader )
                {
                    text << "out " << this_t::determine_output_interface_block_name( sht_cur, sht_after ) << std::endl ;
                    text << "{" << std::endl ;
                }

                for( auto & v : shd_.variables )
                {
                    if( v.fq != motor::msl::flow_qualifier::out ) continue ;

                    // omit system variables
                    //if( v.binding == motor::msl::binding::position && v.fq == motor::msl::flow_qualifier::out ) continue ;
                    if( v.binding == motor::msl::binding::vertex_id ) continue ;
                    if( v.binding == motor::msl::binding::instance_id ) continue ;
                    if( v.binding == motor::msl::binding::primitive_id ) continue ;

                    // do not place that position variable in the interface block.
                    if( !using_transform_feedback && v.binding == motor::msl::binding::position ) continue ;

                    motor::string_t name = v.name ;
                    motor::string_t const type_ = this_file::map_variable_type_to_string( type, v.type ) ;

                    {
                        size_t const idx = motor::msl::find_by( var_mappings, v.name, v.binding, v.fq, shd_.type ) ;
                        if( idx < var_mappings.size() )
                        {
                            name = var_mappings[ idx ].new_name ;
                        }
                    }

                    motor::string_t layloc ;

                    if( shd_.type == motor::msl::shader_type::pixel_shader && num_color > 1 )
                    {
                        layloc = "layout( location = " + motor::to_string( layloc_id++ ) + " ) " ;
                    }

                    text << layloc << "out " << type_ << " " << name << " ; " << std::endl ;
                }

                // no output interface block in the fragment shader
                if( sht_cur != motor::msl::shader_type::pixel_shader )
                {
                    text << "} stage_out ;" << std::endl << std::endl ;
                }
                else text << std::endl ;
            }
        }

        // uniform interface block but just rollin with ordinary uniform definitions for now
        {
            for( auto & v : shd_.variables )
            {
                // omit system variables
                //if( v.binding == motor::msl::binding::position && v.fq == motor::msl::flow_qualifier::out ) continue ;
                if( v.binding == motor::msl::binding::vertex_id ) continue ;
                if( v.binding == motor::msl::binding::instance_id ) continue ;
                if( v.binding == motor::msl::binding::primitive_id ) continue ;
                if( v.fq != motor::msl::flow_qualifier::global ) continue ;

                motor::string_t name = v.name ;
                motor::string_t const type_ = this_file::map_variable_type_to_string( type, v.type ) ;

                {
                    size_t const idx = motor::msl::find_by( var_mappings, v.name, v.binding, v.fq, shd_.type ) ;
                    if( idx < var_mappings.size() )
                    {
                        name = var_mappings[ idx ].new_name ;
                    }
                }

                text << "uniform " << type_ << " " << name << " ; " << std::endl ;
            }
            text << std::endl ;
        }
    }

    // 5. insert main/shader from config
    {
        text << "// The shader // " << std::endl ;
        for( auto const& c : shd_.codes )
        {
            for( auto l : c.lines )
            {
                l = this_t::replace_buildin_symbols( std::move( l ) ) ;
                text << this_t::replace_types( type, l ) << std::endl ;
            }
        }
    }

    // 6. post over the code and replace all dependencies and in/out
    {
        auto shd = text.str() ;

        // variable dependencies
        {
            for( auto const& v : genable.vars )
            {
                auto const find_what = v.sym_long.expand() + " " ;
                size_t p0 = shd.find( find_what ) ; 
                    
                while( p0 != std::string::npos ) 
                {
                    size_t const p1 = shd.find_first_of( " ", p0 ) ;
                    shd = shd.substr( 0, p0 ) + v.value + shd.substr( p1 ) ;
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
            for( auto const& d : shd_.deps )
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

        // replace all "in" positions by gl_Position
        if( sht_cur != motor::msl::shader_type::vertex_shader )
        {
            for( auto const& v : shd_.variables )
            {                
                if( v.binding != motor::msl::binding::position ) continue ;
                if( v.fq != motor::msl::flow_qualifier::in ) continue ;

                motor::string_t swizzle ;
                switch( v.type.struc )
                {
                case motor::msl::type_struct::vec1: swizzle = ".x"; break ;
                case motor::msl::type_struct::vec2: swizzle = ".xy"; break ;
                case motor::msl::type_struct::vec3: swizzle = ".xyz"; break ;
                default: break ;
                }

                std::regex rex( "in *(\\[ *[0-9]*[a-z]* *\\])? *\\." + v.name ) ;
                shd = std::regex_replace( shd, rex, "(gl_in$1.gl_Position"+swizzle+")" ) ; 

                break ;
            }
        }

        // replace all "out" positions by gl_Position
        if( !using_transform_feedback )
        {
            for( auto const& v : shd_.variables )
            {                
                if( v.binding != motor::msl::binding::position ) continue ;
                if( v.fq != motor::msl::flow_qualifier::out ) continue ;

                motor::string_t swizzle ;
                switch( v.type.struc )
                {
                case motor::msl::type_struct::vec1: swizzle = ".x"; break ;
                case motor::msl::type_struct::vec2: swizzle = ".xy"; break ;
                case motor::msl::type_struct::vec3: swizzle = ".xyz"; break ;
                default: break ;
                }

                std::regex rex( "out." + v.name ) ;
                shd = std::regex_replace( shd, rex, "(gl_Position"+swizzle+")" ) ; 

                break ;
            }
        }
        
        // replace in code in/out/globals
        {
            //size_t const off = shd.find( "// The shader" ) ;

            for( auto const& v : var_mappings )
            {
                if( v.st != shd_.type ) continue ;

                if( v.fq == motor::msl::flow_qualifier::in )
                {
                    motor::string_t const iblock_name = sht_cur!=motor::msl::shader_type::vertex_shader ? "stage_in$1." : "" ;

                    std::regex rex( "in *(\\[ *[0-9]*[a-z]* *\\])? *\\." + v.old_name ) ;
                    shd = std::regex_replace( shd, rex, iblock_name + v.new_name ) ; 
                }
                else if( v.fq == motor::msl::flow_qualifier::out )
                {
                    motor::string_t const iblock_name = sht_cur!=motor::msl::shader_type::pixel_shader ? "stage_out." : "" ;

                    std::regex rex( "out\\." + v.old_name ) ;
                    shd = std::regex_replace( shd, rex, iblock_name + v.new_name ) ; 
                }
                else
                {
                    std::regex rex( v.old_name ) ;
                    shd = std::regex_replace( shd, rex, v.new_name ) ; 
                }
            }
        }

        if( sht_cur == motor::msl::shader_type::geometry_shader )
        {
            std::regex rex( "in.length *\\( *\\) *") ;
            shd = std::regex_replace( shd, rex, "gl_in.length()" ) ; 
        }

        // replace numbers
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
            shd = this_t::replace_buildin_symbols( std::move( shd ) ) ;
        }

        code.shader = shd ;
    }

    code.api = type ;

    //motor::log::global_t::status(code.shader) ;
    //ret.emplace_back( std::move( code ) ) ;
    return std::move( code ) ;

}
