#include "glsl4_generator.h"

#include <motor/log/global.h>

#include <motor/std/sstream>
#include <regex>

using namespace motor::msl::glsl ;

namespace this_file_glsl4
{
    //*******************************************************************************************************
    motor::string_t replace_buildin_symbols( motor::string_rref_t code ) noexcept
    {
        motor::msl::repl_syms_t const repls =
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
                motor::string_t( ":mod:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() != 2 ) return "mod ( INVALID_ARGS ) " ;
                    return "mod ( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;
                    //return "( " + args[0] + " - " + args[1] + " *  floor ( " + args[0] +" / " + args[1] + " ) )" ;
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
                    return "sign ( " + args[ 0 ] + " ) * floor ( " + args[ 0 ] + " ) " ;
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
                    if( args.size() == 1 ) return "__bi_rand_1__( " + args[ 0 ] + " ) " ;

                    return "rand_1 ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":rand_4:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_rand_4__( " + args[ 0 ] + " ) " ;

                    return "rand_1d ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":noise_1:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_noise_1__( " + args[ 0 ] + " ) " ;
                    if( args.size() == 2 ) return "__bi_noise_1__( " + args[ 0 ] + " ) " ;

                    return "noise_1d ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":perlin_1:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_perlin_1_2__( " + args[ 0 ] + " , 50 ) " ;
                    if( args.size() == 2 ) return "__bi_perlin_1_2__( " + args[ 0 ] + " , " + args[ 1 ] + " ) " ;

                    return "perlin_1d ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":perlin_2:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_perlin_2d__( " + args[ 0 ] + " ) " ;
                    return "perlin_2d ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":perlin_3:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_perlin_3d__( " + args[ 0 ] + " ) " ;
                    return "perlin_3d ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":snoise_2:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_snoise_2d__( " + args[ 0 ] + " ) " ;
                    return "snoise_2d ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":snoise_3:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_snoise_3d__( " + args[ 0 ] + " ) " ;
                    return "snoise_3d ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":iqnoise:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 3 ) return "__bi_iqnoise__ ( " + args[ 0 ] + " , "+ args[ 1 ] +" , "+ args[ 2 ] +" ) " ;
                    return "iqnoise ( INVALID_ARGS ) " ;
                }
            },
            {
                motor::string_t( ":fbm_1:" ),
                [=] ( motor::vector< motor::string_t > const& args ) -> motor::string_t
                {
                    if( args.size() == 1 ) return "__bi_fbm_1d__ ( " + args[ 0 ] + " , 5 ) " ;
                    else if( args.size() == 2 ) return "__bi_fbm_1d__ ( " + args[ 0 ] + " , "+ args[ 1 ] + " ) " ;
                    return "fbm_1d ( INVALID_ARGS ) " ;
                }
            }

        } ;

        return motor::msl::perform_repl( std::move( code ), repls ) ;
    }

    enum class api_build_in_types
    {
        rand_1d_1,
        rand_1d_2,
        rand_1d_3,
        rand_1d_4,
        rand_4d_4,
        noise_1d_1,
        noise_1d_2,
        noise_1d_3,
        int_perm_3d_3,
        int_perm_4d_4,
        int_pnoise_1d_2_1, // used for perlin_noise_1d
        perlin_1d_2_1,

        int_perlin_2d_fade_2d,
        perlin_2d,

        int_perlin3d_taylor_invsqrt_4d,
        int_perlin_3d_fade_3d,
        perlin_3d,

        snoise_2d,
        snoise_3d,

        int_hash3,
        iqnoise,

        fbm_1d_1,
        fbm_1d_2,
        fbm_1d_3
    } ;

    static size_t as_number( api_build_in_types const i ) noexcept
    {
        return size_t( i ) ;
    }

    //*******************************************************************************************************
    motor::core::types::bool_t buildin_by_opcode( motor::msl::buildin_type const bit, 
        motor::msl::api_specific_buildins_t & ret ) noexcept
    {
        motor::msl::api_specific_buildins_t const api_buildins = 
        {
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_rand_1__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_float(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return fract ( sin(x) * 43983.5453123 ) ;",
                }
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_rand_1__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return fract(sin(dot(x, vec2(12.8989, 4.1414))) * 43983.4549);",
                }
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_rand_1__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec3(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return fract( sin( dot( x, vec3( 12.8989, 4.1414, 23.94873 ) ) ) * 43983.4549 );",
                }
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_rand_1__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec4(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return fract ( sin ( dot ( x, vec4( 12.8989, 4.1414, 823.48367, 93.20831 ) ) ) * 43983.4549 );",
                }
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_vec4(),
                    "__bi_rand_4__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec4(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    R"( return vec4( 
                        fract ( sin ( dot ( x, vec4( 12.8989, 4.1414, 823.48367, 93.20831 ) ) ) * 4393.1549 ),
                        fract ( sin ( dot ( x, vec4( 4.1414, 823.48367, 93.20831, 12.8989 ) ) ) * 7683.4549 ),
                        fract ( sin ( dot ( x, vec4( 823.48367, 93.20831, 12.8989, 4.1414 ) ) ) * 3354.4523 ),
                        fract ( sin ( dot ( x, vec4( 93.20831, 12.8989, 4.1414, 823.48367 ) ) ) * 8383.4579 )
                        ) ;
                    )",
                }
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_noise_1__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_float(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "float f1 = floor ( x ) ;",
                    "float fc = fract ( x ) ;",
                    this_file_glsl4::replace_buildin_symbols("return mix ( :rand_1: ( f1 ) , :rand_1: ( f1 + 1.0 ) , fc ) ;" )                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_noise_1__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "vec2 ip = floor ( x ) ;",
                    "vec2 u = fract ( x ); ",
                    "u = u * u * ( 3.0 - 2.0 * u ) ;",
                    "float res = mix( ",
                    this_file_glsl4::replace_buildin_symbols( "mix ( :rand_1: ( ip ) , :rand_1: ( ip + vec2 ( 1.0 , 0.0 ) ) , u.x )," ),
                    this_file_glsl4::replace_buildin_symbols( "mix( :rand_1: ( ip+vec2( 0.0,1.0 ) ), :rand_1: ( ip+vec2 ( 1.0,1.0 ) ) ,u.x),u.y);"),
                    "return res*res;"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_noise_1__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec3(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "vec3 a = floor(x);"
                    "vec3 d = x - a;"
                    "d = d * d * (3.0 - 2.0 * d);"

                    "vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);"
                    "vec4 k1 = __internal_perm_4__(b.xyxy);"
                    "vec4 k2 = __internal_perm_4__(k1.xyxy + b.zzww);"

                    "vec4 c = k2 + a.zzzz;"
                    "vec4 k3 = __internal_perm_4__(c);"
                    "vec4 k4 = __internal_perm_4__(c + 1.0);"

                    "vec4 o1 = fract(k3 * (1.0 / 41.0));"
                    "vec4 o2 = fract(k4 * (1.0 / 41.0));"

                    "vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);"
                    "vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);"

                    "return o4.y * d.y + o4.x * (1.0 - d.y);"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_vec3(),
                    "__internal_perm_3__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec3(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return mod(x*x, 289.0);"
                    // original
                    //"return mod(((x*34.0)+1.0)*x, 289.0);"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_vec4(),
                    "__internal_perm_4__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec4(), "x" },
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return mod(x*x, 289.0);"
                    // original
                    //"return mod(((x*34.0)+1.0)*x, 289.0);"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__internal_perlin_1d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "p" },
                        { motor::msl::type_t::as_float(), "freq" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "float pi = 3.14159265358979323846 ;"
                    "vec2 ij = floor(p/freq);"
                    "vec2 xy = mod(p,freq)/freq;"
                    "xy = .5*(1.-cos(pi*xy));"
                    "float a = __bi_rand_1__ ( (ij+vec2(0.,0.)) ) ;"
                    "float b = __bi_rand_1__ ( (ij+vec2(1.,0.)) ) ;"
                    "float c = __bi_rand_1__ ( (ij+vec2(0.,1.)) ) ;"
                    "float d = __bi_rand_1__ ( (ij+vec2(1.,1.)) ) ;"
                    "float x1 = mix(a, b, xy.x);"
                    "float x2 = mix(c, d, xy.x);"
                    "return mix(x1, x2, xy.y);"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_perlin_1_2__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "p" },
                        { motor::msl::type_t::as_int(), "iter" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "float persistance = .5;"
                    "float n = 0.;"
                    "float normK = 0.;"
                    "float f = 4.;"
                    "float amp = 1.;"
                    //"int iCount = 0;"
                    "for (int i = 0; i<iter; i++){"
                    "n+=amp*__internal_perlin_1d__(p, f);"
                    "f*=2.;"
                    "normK+=amp;"
                    "amp*=persistance;"
                    //"if (iCount == res) break;"
                    //    "iCount++;"
                    "}"
                    "float nf = n/normK;"
                    "return nf*nf*nf*nf;"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_vec2(),
                    "__internal_perlin_2_fade_2d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "t" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return t*t*t*(t*(t*6.0-15.0)+10.0);"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_perlin_2d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "P" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    R"(vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
                    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
                    Pi = mod(Pi, 289.0);
                    vec4 ix = Pi.xzxz;
                    vec4 iy = Pi.yyww;
                    vec4 fx = Pf.xzxz;
                    vec4 fy = Pf.yyww;
                    vec4 i = __internal_perm_4__(iy)+__internal_perm_4__(ix) ;
                    vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0;
                    vec4 gy = abs(gx) - 0.5;
                    vec4 tx = floor(gx + 0.5);
                    gx = gx - tx;
                    vec2 g00 = vec2(gx.x,gy.x);
                    vec2 g10 = vec2(gx.y,gy.y);
                    vec2 g01 = vec2(gx.z,gy.z);
                    vec2 g11 = vec2(gx.w,gy.w);
                    vec4 norm = 1.79284291400159 - 0.85373472095314 * 
                    vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
                    g00 *= norm.x;
                    g01 *= norm.y;
                    g10 *= norm.z;
                    g11 *= norm.w;
                    float n00 = dot(g00, vec2(fx.x, fy.x));
                    float n10 = dot(g10, vec2(fx.y, fy.y));
                    float n01 = dot(g01, vec2(fx.z, fy.z));
                    float n11 = dot(g11, vec2(fx.w, fy.w));
                    vec2 fade_xy = __internal_perlin_2_fade_2d__(Pf.xy);
                    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
                    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
                    return 2.3 * n_xy;)"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_vec4(),
                    "__int_taylor_inv_sqrt_4d_4d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec4(), "x" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return 1.79284291400159 - 0.85373472095314 * x;"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_vec3(),
                    "__internal_perlin_3d_fade_3d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec3(), "t" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "return t*t*t*(t*(t*6.0-15.0)+10.0);"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_perlin_3d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec3(), "P" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    R"(vec3 Pi0 = floor(P);
                    vec3 Pi1 = Pi0 + vec3(1.0);
                    Pi0 = mod(Pi0, 289.0);
                    Pi1 = mod(Pi1, 289.0);
                    vec3 Pf0 = fract(P); 
                    vec3 Pf1 = Pf0 - vec3(1.0); 
                    vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
                    vec4 iy = vec4(Pi0.yy, Pi1.yy);
                    vec4 iz0 = Pi0.zzzz;
                    vec4 iz1 = Pi1.zzzz;

                    vec4 ixy = __internal_perm_4__( __internal_perm_4__(ix)+iy ) ;
                    vec4 ixy0 = __internal_perm_4__(ixy+iz0 ) ;
                    vec4 ixy1 = __internal_perm_4__(ixy+iz1 ) ;

                    vec4 gx0 = ixy0 / 7.0;
                    vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
                    gx0 = fract(gx0);
                    vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
                    vec4 sz0 = step(gz0, vec4(0.0));
                    gx0 -= sz0 * (step(0.0, gx0) - 0.5);
                    gy0 -= sz0 * (step(0.0, gy0) - 0.5);

                    vec4 gx1 = ixy1 / 7.0;
                    vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
                    gx1 = fract(gx1);
                    vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
                    vec4 sz1 = step(gz1, vec4(0.0));
                    gx1 -= sz1 * (step(0.0, gx1) - 0.5);
                    gy1 -= sz1 * (step(0.0, gy1) - 0.5);

                    vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
                    vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
                    vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
                    vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
                    vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
                    vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
                    vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
                    vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

                    vec4 norm0 = __int_taylor_inv_sqrt_4d_4d__(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
                    g000 *= norm0.x;
                    g010 *= norm0.y;
                    g100 *= norm0.z;
                    g110 *= norm0.w;
                    vec4 norm1 = __int_taylor_inv_sqrt_4d_4d__(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
                    g001 *= norm1.x;
                    g011 *= norm1.y;
                    g101 *= norm1.z;
                    g111 *= norm1.w;

                    float n000 = dot(g000, Pf0);
                    float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
                    float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
                    float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
                    float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
                    float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
                    float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
                    float n111 = dot(g111, Pf1);

                    vec3 fade_xyz = __internal_perlin_3d_fade_3d__(Pf0);
                    vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
                    vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
                    float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x);
                    return 2.2 * n_xyz;)"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_snoise_2d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "v" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "const vec4 C = vec4(0.211324865405187, 0.366025403784439,"
                            "-0.577350269189626, 0.024390243902439);"
                    "vec2 i  = floor(v + dot(v, C.yy) );"
                    "vec2 x0 = v -   i + dot(i, C.xx);"
                    "vec2 i1;"
                    "i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);"
                    "vec4 x12 = x0.xyxy + C.xxzz;"
                    "x12.xy -= i1;"
                    "i = mod(i, 289.0);"
                    "vec3 p = __internal_perm_3__( __internal_perm_3__( i.y + vec3(0.0, i1.y, 1.0 ))"
                    "+ i.x + vec3(0.0, i1.x, 1.0 ));"
                    "vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),"
                    "dot(x12.zw,x12.zw)), 0.0);"
                    "m = m*m ;"
                    "m = m*m ;"
                    "vec3 x = 2.0 * fract(p * C.www) - 1.0;"
                    "vec3 h = abs(x) - 0.5;"
                    "vec3 ox = floor(x + 0.5);"
                    "vec3 a0 = x - ox;"
                    "m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );"
                    "vec3 g;"
                    "g.x  = a0.x  * x0.x  + h.x  * x0.y;"
                    "g.yz = a0.yz * x12.xz + h.yz * x12.yw;"
                    "return 130.0 * dot(m, g);"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_snoise_3d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec3(), "v" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;"
                    "const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);"

                
                    "vec3 i  = floor(v + dot(v, C.yyy) );"
                    "vec3 x0 =   v - i + dot(i, C.xxx) ;"

                
                    "vec3 g = step(x0.yzx, x0.xyz);"
                    "vec3 l = 1.0 - g;"
                    "vec3 i1 = min( g.xyz, l.zxy );"
                    "vec3 i2 = max( g.xyz, l.zxy );"

                    
                    "vec3 x1 = x0 - i1 + 1.0 * C.xxx;"
                    "vec3 x2 = x0 - i2 + 2.0 * C.xxx;"
                    "vec3 x3 = x0 - 1. + 3.0 * C.xxx;"

                
                    "i = mod(i, 289.0 ); "
                    "vec4 p = __internal_perm_4__( __internal_perm_4__( __internal_perm_4__( "
                                "i.z + vec4(0.0, i1.z, i2.z, 1.0 ))"
                            "+ i.y + vec4(0.0, i1.y, i2.y, 1.0 )) "
                            "+ i.x + vec4(0.0, i1.x, i2.x, 1.0 ));"

                
                    "float n_ = 1.0/7.0; "
                    "vec3  ns = n_ * D.wyz - D.xzx;"

                    "vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  "

                    "vec4 x_ = floor(j * ns.z);"
                    "vec4 y_ = floor(j - 7.0 * x_ );   "

                    "vec4 x = x_ *ns.x + ns.yyyy;"
                    "vec4 y = y_ *ns.x + ns.yyyy;"
                    "vec4 h = 1.0 - abs(x) - abs(y);"

                    "vec4 b0 = vec4( x.xy, y.xy );"
                    "vec4 b1 = vec4( x.zw, y.zw );"

                    "vec4 s0 = floor(b0)*2.0 + 1.0;"
                    "vec4 s1 = floor(b1)*2.0 + 1.0;"
                    "vec4 sh = -step(h, vec4(0.0));"

                    "vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;"
                    "vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;"

                    "vec3 p0 = vec3(a0.xy,h.x);"
                    "vec3 p1 = vec3(a0.zw,h.y);"
                    "vec3 p2 = vec3(a1.xy,h.z);"
                    "vec3 p3 = vec3(a1.zw,h.w);"

                
                    "vec4 norm = __int_taylor_inv_sqrt_4d_4d__(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));"
                    "p0 *= norm.x;"
                    "p1 *= norm.y;"
                    "p2 *= norm.z;"
                    "p3 *= norm.w;"

                
                    "vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);"
                    "m = m * m;"
                    "return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), "
                                                "dot(p2,x2), dot(p3,x3) ) );"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_vec3(),
                    "__int_hash_3__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "p" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "vec3 q = vec3( dot(p,vec2(127.1,311.7)), "
                        "dot(p,vec2(269.5,183.3)), "
                        "dot(p,vec2(419.2,371.9)) );"
                    "return fract(sin(q)*43758.5453);"
                
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_iqnoise__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "x" },
                        { motor::msl::type_t::as_float(), "u" },
                        { motor::msl::type_t::as_float(), "v" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "vec2 p = floor(x);"
                    "vec2 f = fract(x);"
		
                    "float k = 1.0+63.0*pow(1.0-v,4.0);"
	
                    "float va = 0.0;"
                    "float wt = 0.0;"
                    "for( int j=-2; j<=2; j++ )"
                    "for( int i=-2; i<=2; i++ )"
                    "{"
                        "vec2 g = vec2( float(i),float(j) );"
                        "vec3 o = __int_hash_3__( p + g )*vec3(u,u,1.0);"
                        "vec2 r = g - f + o.xy;"
                        "float d = dot(r,r);"
                        "float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );"
                        "va += o.z*ww;"
                        "wt += ww;"
                    "}"
                    "return va/wt;"
                
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_fbm_1d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_float(), "x" },
                        { motor::msl::type_t::as_int(), "oct" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "float v = 0.0;"
                    "float a = 0.5;"
                    "float shift = float(100);"
                    "for (int i = 0; i < oct; ++i) {"
                        "v += a * __bi_noise_1__(x);"
                        "x = x * 2.0 + shift;"
                        "a *= 0.5;"
                    "}"
                    "return v;"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_fbm_1d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec2(), "x" },
                        { motor::msl::type_t::as_int(), "oct" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "float v = 0.0;"
                    "float a = 0.5;"
                    "vec2 shift = vec2(100);"
                    
                    //"mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));"
                    "for (int i = 0; i < oct; ++i) {"
                        "v += a * __bi_noise_1__(x);"
                        "x = /*rot */ x * 2.0 + shift;"
                        "a *= 0.5;"
                    "}"
                    "return v;"
                },
            },
            {
                // motor::msl::signature_t
                { 
                    motor::msl::type_t::as_float(),
                    "__bi_fbm_1d__",
                    // motor::msl::signature_t::args_t
                    {
                        { motor::msl::type_t::as_vec3(), "x" },
                        { motor::msl::type_t::as_int(), "oct" }
                    }
                },
                // fragmetns_t/strings_t
                {
                    "float v = 0.0;"
                    "float a = 0.5;"
                    "vec3 shift = vec3(100);"
                    "for (int i = 0; i < oct; ++i) {"
                        "v += a * __bi_noise_1__(x);"
                        "x = x * 2.0 + shift;"
                        "a *= 0.5;"
                    "}"
                    "return v;"
                },
            }

        } ;

        if( bit == motor::msl::buildin_type::rand_1d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_1)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_2)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_3)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_4)] ) ;
            
            return true ;
        }

        else if( bit == motor::msl::buildin_type::noise_1d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_1)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_2)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::noise_1d_1)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::noise_1d_2)] ) ;

            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perm_4d_4)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::noise_1d_3)] ) ;
            return true ;
        }

        else if( bit == motor::msl::buildin_type::perlin_1d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_1)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::rand_1d_2)] ) ;

            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_pnoise_1d_2_1)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::perlin_1d_2_1)] ) ;
            return true ;
        }

        else if( bit == motor::msl::buildin_type::perlin_2d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perm_4d_4)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perlin_2d_fade_2d)] ) ;

            ret.emplace_back( api_buildins[as_number(api_build_in_types::perlin_2d)] ) ;
            return true ;
        }

        else if( bit == motor::msl::buildin_type::perlin_3d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perm_4d_4)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perlin3d_taylor_invsqrt_4d)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perlin_3d_fade_3d)] ) ;

            ret.emplace_back( api_buildins[as_number(api_build_in_types::perlin_3d)] ) ;
            return true ;
        }

        else if( bit == motor::msl::buildin_type::snoise_2d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perm_3d_3)] ) ;

            ret.emplace_back( api_buildins[as_number(api_build_in_types::snoise_2d)] ) ;
            return true ;
        }

        else if( bit == motor::msl::buildin_type::snoise_3d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perm_4d_4)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_perlin3d_taylor_invsqrt_4d)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::snoise_3d)] ) ;
            return true ;
        }

        else if( bit == motor::msl::buildin_type::iqnoise ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::int_hash3)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::iqnoise)] ) ;
            return true ;
        }

        else if( bit == motor::msl::buildin_type::fbm_1d ) 
        {
            ret.emplace_back( api_buildins[as_number(api_build_in_types::noise_1d_1)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::noise_1d_2)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::noise_1d_3)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::fbm_1d_1)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::fbm_1d_2)] ) ;
            ret.emplace_back( api_buildins[as_number(api_build_in_types::fbm_1d_3)] ) ;
            return true ;
        }

        return false ;
    }
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
                    l = this_file_glsl4::replace_buildin_symbols( std::move( l ) ) ;
                }
            }
        }

        for( auto& frg : genable.frags )
        {
            for( auto& f : frg.fragments )
            {
                //for( auto& l : c.lines )
                {
                    f = this_file_glsl4::replace_buildin_symbols( std::move( f ) ) ;
                }
            }
        }
    }

    // inject composite buildins
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

        motor::msl::api_specific_buildins_t buildins_ ;
        buildins_.reserve( 10 ) ;

        for( auto iter = tmp.begin(); iter != tmp.end(); ++iter )
        {
            auto & ubi = *iter ;

            motor::msl::buildin_type test_bi = ubi.bi.t ;
                
            if( !this_file_glsl4::buildin_by_opcode( test_bi, buildins_ ) ) continue ;

            // need further testing for build-ins using build-ins!
            for( auto const & new_apibi : buildins_ )
            {
                if( std::find_if( genable.frags.begin(), genable.frags.end(), 
                    [&]( motor::msl::post_parse::library::fragment_in_t d )
                    { return d.sig == new_apibi.sig ; } ) == genable.frags.end() )
                {
                    genable.frags.emplace_back( motor::msl::post_parse::library_t::fragment_t(
                    {
                        // motor::msl::signature_t
                        new_apibi.sig,
                        new_apibi.fragments,
                        // motor::msl::post_parse::used_buildins_t (biuld_ins)
                        {},
                        // dependency symbols (deps)
                        {},
                        // sym_long
                        motor::msl::symbol_t( new_apibi.sig.name )
                    } ) ) ;
                }
            }
            buildins_.clear() ;
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
                l = this_file_glsl4::replace_buildin_symbols( std::move( l ) ) ;
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

        #if 0
        {
            shd = this_file_glsl4::replace_buildin_symbols( std::move( shd ) ) ;
        }
        #endif

        code.shader = shd ;
    }

    code.api = type ;

    //motor::log::global_t::status(code.shader) ;
    //ret.emplace_back( std::move( code ) ) ;
    return std::move( code ) ;

}
