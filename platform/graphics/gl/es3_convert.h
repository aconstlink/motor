#pragma once

#include "../../types.h"
#include <natus/ogl/es/es.h>

namespace natus
{
    namespace graphics
    {
        namespace es3
        {
            static GLenum convert( natus::graphics::primitive_type const pt ) noexcept
            {
                switch( pt )
                {
                case natus::graphics::primitive_type::lines: return GL_LINES ;
                case natus::graphics::primitive_type::triangles: return GL_TRIANGLES ;
                case natus::graphics::primitive_type::points: return GL_POINTS ;
                default:break ;
                }
                return 0 ;
            }

            static GLenum convert_to_transform_feedback_output( natus::graphics::primitive_type const pt ) noexcept
            {
                switch( pt )
                {
                case natus::graphics::primitive_type::lines: return GL_LINES ;
                case natus::graphics::primitive_type::triangles: return GL_TRIANGLES ;
                case natus::graphics::primitive_type::points: return GL_POINTS ;
                default:break ;
                }
                return 0 ;
            }

            static GLuint primitive_type_to_num_vertices( GLenum const pt ) noexcept
            {
                switch( pt )
                {
                case GL_LINES: return 2 ;
                case GL_TRIANGLES: return 3 ;
                case GL_POINTS: return 1 ;
                default:break ;
                }
                return 0 ;
            }

            static std::pair< natus::graphics::type, natus::graphics::type_struct >  
                to_type_type_struct( GLenum const e ) noexcept
            {
                switch( e )
                {
                case GL_FLOAT: 
                    return ::std::make_pair( natus::graphics::type::tfloat, natus::graphics::type_struct::vec1 ) ;
                case GL_FLOAT_VEC2: 
                    return ::std::make_pair( natus::graphics::type::tfloat, natus::graphics::type_struct::vec2 ) ;
                case GL_FLOAT_VEC3: 
                    return ::std::make_pair( natus::graphics::type::tfloat, natus::graphics::type_struct::vec3 ) ;
                case GL_FLOAT_VEC4:
                    return ::std::make_pair( natus::graphics::type::tfloat, natus::graphics::type_struct::vec4 ) ;
                case GL_INT:
                    return ::std::make_pair( natus::graphics::type::tint, natus::graphics::type_struct::vec1 ) ;
                case GL_INT_VEC2:
                    return ::std::make_pair( natus::graphics::type::tint, natus::graphics::type_struct::vec2 ) ;
                case GL_INT_VEC3:
                    return ::std::make_pair( natus::graphics::type::tint, natus::graphics::type_struct::vec3 ) ;
                case GL_INT_VEC4:
                    return ::std::make_pair( natus::graphics::type::tint, natus::graphics::type_struct::vec4 ) ;
                case GL_UNSIGNED_INT:
                    return ::std::make_pair( natus::graphics::type::tuint, natus::graphics::type_struct::vec1 ) ;
                case GL_UNSIGNED_INT_VEC2:
                    return ::std::make_pair( natus::graphics::type::tuint, natus::graphics::type_struct::vec2 ) ;
                case GL_UNSIGNED_INT_VEC3:
                    return ::std::make_pair( natus::graphics::type::tuint, natus::graphics::type_struct::vec3 ) ;
                case GL_UNSIGNED_INT_VEC4:
                    return ::std::make_pair( natus::graphics::type::tuint, natus::graphics::type_struct::vec4 ) ;
                case GL_BOOL:
                    return ::std::make_pair( natus::graphics::type::tbool, natus::graphics::type_struct::vec1 ) ;
                case GL_BOOL_VEC2:
                    return ::std::make_pair( natus::graphics::type::tbool, natus::graphics::type_struct::vec2 ) ;
                case GL_BOOL_VEC3:
                    return ::std::make_pair( natus::graphics::type::tbool, natus::graphics::type_struct::vec3 ) ;
                case GL_BOOL_VEC4:
                    return ::std::make_pair( natus::graphics::type::tbool, natus::graphics::type_struct::vec4 ) ;
                case GL_FLOAT_MAT2:
                    return ::std::make_pair( natus::graphics::type::tfloat, natus::graphics::type_struct::mat2 ) ;
                case GL_FLOAT_MAT3:
                    return ::std::make_pair( natus::graphics::type::tfloat, natus::graphics::type_struct::mat3 ) ;
                case GL_FLOAT_MAT4:
                    return ::std::make_pair( natus::graphics::type::tfloat, natus::graphics::type_struct::mat4 ) ;
                case GL_SAMPLER_2D:
                case GL_SAMPLER_3D:
                case GL_INT_SAMPLER_2D:
                case GL_INT_SAMPLER_3D:
                case GL_UNSIGNED_INT_SAMPLER_2D:
                case GL_UNSIGNED_INT_SAMPLER_3D:
                case GL_SAMPLER_CUBE:
                case GL_SAMPLER_2D_SHADOW:
                case GL_SAMPLER_BUFFER:
                case GL_INT_SAMPLER_BUFFER:
                case GL_UNSIGNED_INT_SAMPLER_BUFFER:
                case GL_SAMPLER_2D_ARRAY:
                case GL_INT_SAMPLER_2D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: 
                    return std::make_pair( natus::graphics::type::tint, natus::graphics::type_struct::vec1 ) ;
                default: break ;
                }
                return std::make_pair( natus::graphics::type::undefined, natus::graphics::type_struct::undefined ) ;
            }
        }
        namespace es3
        {
            namespace detail
            {
                static const GLenum convert_blend_factor_gl[] =
                {
                    GL_ZERO,
                    GL_ONE,
                    GL_SRC_COLOR,
                    GL_ONE_MINUS_SRC_COLOR,
                    GL_DST_COLOR,
                    GL_ONE_MINUS_DST_COLOR,
                    GL_SRC_ALPHA,
                    GL_ONE_MINUS_SRC_ALPHA,
                    GL_DST_ALPHA,
                    GL_ONE_MINUS_DST_ALPHA,
                    GL_CONSTANT_COLOR,
                    GL_ONE_MINUS_CONSTANT_COLOR,
                    GL_CONSTANT_ALPHA,
                    GL_ONE_MINUS_CONSTANT_ALPHA,
                    GL_SRC_ALPHA_SATURATE
                } ;

                static const size_t convert_blend_factors_gl_size =
                    sizeof( convert_blend_factor_gl ) / sizeof( convert_blend_factor_gl[ 0 ] ) ;
            }

            static GLenum convert( blend_factor const bf ) noexcept
            {
                return detail::convert_blend_factor_gl[ size_t( bf ) ] ;
            }
            static GLenum convert( natus::graphics::blend_function const bf ) noexcept
            {
                switch( bf )
                {
                case natus::graphics::blend_function::add: return GL_FUNC_ADD ;
                case natus::graphics::blend_function::sub: return GL_FUNC_SUBTRACT ;
                case natus::graphics::blend_function::reverse_sub: return GL_FUNC_REVERSE_SUBTRACT ;
                case natus::graphics::blend_function::min: return GL_MIN ;
                case natus::graphics::blend_function::max: return GL_MAX ;
                default: break ;
                }

                return GL_ZERO ;
            }
        }

        namespace es3
        {
            static GLenum convert_to_gl_format( natus::graphics::image_format const imf, 
                natus::graphics::image_element_type const t )
            {

                switch( imf )
                {
                case image_format::depth:
                    return GL_DEPTH_COMPONENT ;

                case image_format::rgb:
                    switch( t )
                    {
                    case natus::graphics::image_element_type::uint8: return GL_RGB ;
                    case natus::graphics::image_element_type::int16: return GL_RGB16I ;
                    case natus::graphics::image_element_type::int32: return GL_RGB32I ;
                    case natus::graphics::image_element_type::uint16: return GL_RGB16UI ;
                    case natus::graphics::image_element_type::uint32: return GL_RGB32UI ;
                    case natus::graphics::image_element_type::float16: return GL_RGB16F ;
                    case natus::graphics::image_element_type::float32: return GL_RGB32F ;
                    default: break ;
                    }
                    break  ;

                case image_format::rgba:
                    switch( t )
                    {
                    case natus::graphics::image_element_type::uint8: return GL_RGBA ;
                    case natus::graphics::image_element_type::int16: return GL_RGBA16I ;
                    case natus::graphics::image_element_type::int32: return GL_RGBA32I ;
                    case natus::graphics::image_element_type::uint16: return GL_RGBA16UI ;
                    case natus::graphics::image_element_type::uint32: return GL_RGBA32UI ;
                    case natus::graphics::image_element_type::float16: return GL_RGBA16F ;
                    case natus::graphics::image_element_type::float32: return GL_RGBA32F ;
                    default: break ;
                    }
                    break  ;

                case image_format::intensity:
                    switch( t )
                    {
                    case natus::graphics::image_element_type::uint8: return GL_RED ;
                    case natus::graphics::image_element_type::int16: return GL_R16I ;
                    case natus::graphics::image_element_type::int32: return GL_R32I ;
                    case natus::graphics::image_element_type::uint16: return GL_R16UI ;
                    case natus::graphics::image_element_type::uint32: return GL_R32UI ;
                    case natus::graphics::image_element_type::float16: return GL_R16F ;
                    case natus::graphics::image_element_type::float32: return GL_R32F ;
                    default: break ;
                    }
                    break  ;

                default: break ;

                }
                return 0 ;
            }

            static GLenum convert_to_gl_pixel_format( natus::graphics::image_format const imf )
            {
                switch( imf )
                {
                case natus::graphics::image_format::depth: return GL_DEPTH_COMPONENT ;
                case natus::graphics::image_format::rgb: return GL_RGB ;
                case natus::graphics::image_format::rgba: return GL_RGBA ;
                case natus::graphics::image_format::intensity: return GL_RED ;
                default: break ;
                }
                return 0 ;
            }

            static GLenum convert_to_gl_pixel_type( natus::graphics::image_element_type const pt )
            {
                switch( pt )
                {
                case natus::graphics::image_element_type::uint8: return GL_UNSIGNED_BYTE ;
                case natus::graphics::image_element_type::uint16: return GL_UNSIGNED_SHORT ;
                case natus::graphics::image_element_type::uint32: return GL_UNSIGNED_INT ;

                case natus::graphics::image_element_type::int8: return GL_BYTE ;
                case natus::graphics::image_element_type::int16: return GL_SHORT ;
                case natus::graphics::image_element_type::int32: return GL_INT ;

                case natus::graphics::image_element_type::float16: return GL_FLOAT ;
                case natus::graphics::image_element_type::float32: return GL_FLOAT ;
                default: break ;
                }
                return 0 ;
            }
        }

        namespace es3
        {
            namespace detail
            {
                static const GLenum texture_types_gl[] = {
                    GL_NONE,
                    //GL_TEXTURE_1D,
                    GL_TEXTURE_2D,
                    //GL_TEXTURE_3D,
                    //GL_TEXTURE_1D_ARRAY,
                    GL_TEXTURE_2D_ARRAY
                } ;

                static const GLenum filter_modes_gl[] = {
                    GL_TEXTURE_MAG_FILTER,
                    GL_TEXTURE_MIN_FILTER
                } ;

                static const GLenum filter_types_gl[] = {
                    GL_NEAREST,
                    GL_LINEAR
                } ;
            }

            static GLenum convert( texture_type const m ) noexcept
            {
                return natus::graphics::es3::detail::texture_types_gl[ ( size_t ) m ] ;
            }

            static GLenum convert( texture_filter_mode const m ) noexcept
            {
                return natus::graphics::es3::detail::filter_modes_gl[ ( size_t ) m ] ;
            }

            static GLenum convert( texture_filter_type const t ) noexcept
            {
                return natus::graphics::es3::detail::filter_types_gl[ ( size_t ) t ] ;
            }

            namespace detail
            {
                static const GLenum wrap_modes_gl[] = {
                    GL_TEXTURE_WRAP_S,
                    GL_TEXTURE_WRAP_T,
                    GL_TEXTURE_WRAP_R
                } ;

                static const GLenum wrap_types_gl[] = {
                    GL_CLAMP_TO_EDGE,
                    GL_REPEAT,
                    GL_CLAMP_TO_EDGE,
                    GL_CLAMP_TO_EDGE,
                    GL_MIRRORED_REPEAT
                } ;
            }

            static GLenum convert( texture_wrap_mode const m ) noexcept {
                return natus::graphics::es3::detail::wrap_modes_gl[ ( size_t ) m ] ;
            }

            static GLenum convert( texture_wrap_type const t ) noexcept {
                return natus::graphics::es3::detail::wrap_types_gl[ ( size_t ) t ] ;
            }

            static GLenum to_pixel_type( color_target_type const ct ) noexcept 
            {
                switch( ct )
                {
                case natus::graphics::color_target_type::rgba_float_32: return GL_FLOAT ;
                case natus::graphics::color_target_type::rgba_uint_8: return GL_UNSIGNED_BYTE ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_format( color_target_type const ct ) noexcept
            {
                switch( ct )
                {
                case natus::graphics::color_target_type::rgba_float_32: return GL_RGBA32F ;
                case natus::graphics::color_target_type::rgba_uint_8: return GL_RGBA ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_format( depth_stencil_target_type const t ) noexcept
            {
                switch( t )
                {
                case natus::graphics::depth_stencil_target_type::depth32: return GL_DEPTH_COMPONENT ;
                case natus::graphics::depth_stencil_target_type::depth24_stencil8: return GL_DEPTH_STENCIL ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_attachment( depth_stencil_target_type const t ) noexcept
            {
                switch( t )
                {
                case natus::graphics::depth_stencil_target_type::depth32: return GL_DEPTH_ATTACHMENT ;
                case natus::graphics::depth_stencil_target_type::depth24_stencil8: return GL_DEPTH_STENCIL_ATTACHMENT ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_type( depth_stencil_target_type const t ) noexcept
            {
                switch( t )
                {
                case natus::graphics::depth_stencil_target_type::depth32: return GL_FLOAT ;
                case natus::graphics::depth_stencil_target_type::depth24_stencil8: return GL_UNSIGNED_INT_24_8 ;
                default: break;
                }
                return GL_NONE ;
            }

            static size_t calc_sib( size_t const width, size_t const height, color_target_type const ct ) noexcept
            {
                switch( ct )
                {
                case natus::graphics::color_target_type::rgba_float_32: 
                    return width * height * 4 * 4 ;
                case natus::graphics::color_target_type::rgba_uint_8: 
                    return width * height * 4 * 1 ;
                default: break;
                }
                return GL_NONE ;
            }
        }

        namespace es3
        {
            static GLenum convert( natus::graphics::cull_mode const cm ) noexcept
            {
                switch( cm )
                {
                case natus::graphics::cull_mode::back: return GL_BACK ;
                case natus::graphics::cull_mode::front: return GL_FRONT ;
                case natus::graphics::cull_mode::none: return GL_NONE ;
                default: break;
                }
                return GL_NONE ;
            }


            static GLenum convert( natus::graphics::fill_mode const fm ) noexcept
            {
                switch( fm )
                {
                //case natus::graphics::fill_mode::fill: return GL_FILL ;
                //case natus::graphics::fill_mode::point: return GL_POINT ;
                //case natus::graphics::fill_mode::line: return GL_LINE ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum convert( natus::graphics::front_face const ff ) noexcept
            {
                switch( ff )
                {
                case natus::graphics::front_face::clock_wise: return GL_CW ;
                case natus::graphics::front_face::counter_clock_wise: return GL_CCW ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum convert_for_texture_buffer( natus::graphics::type const t, natus::graphics::type_struct const ts ) noexcept
            {
                switch( t )
                {
                case natus::graphics::type::tfloat: 
                    switch( ts ) 
                    {
                    case natus::graphics::type_struct::vec4:
                        return GL_RGBA32F ;
                    default: break ;
                    }
                    break ;

                case natus::graphics::type::tchar: 
                    switch( ts ) 
                    {
                    case natus::graphics::type_struct::vec4:
                        return GL_RGBA8 ;
                    default: break ;
                    }
                    break ;

                case natus::graphics::type::tuchar: 
                    switch( ts ) 
                    {
                    case natus::graphics::type_struct::vec4:
                        return GL_RGBA8UI ;
                    default: break ;
                    }
                    break ;

                case natus::graphics::type::tuint: 
                    switch( ts ) 
                    {
                    case natus::graphics::type_struct::vec4:
                        return GL_RGBA32UI ;
                    default: break ;
                    }
                    break ;

                case natus::graphics::type::tint: 
                    switch( ts ) 
                    {
                    case natus::graphics::type_struct::vec4:
                        return GL_RGBA32I ;
                    default: break ;
                    }
                    break ;

                default: break ;
                }

                return GL_NONE ;
            }
        }

        namespace es3
        {
            static GLenum convert( natus::graphics::streamout_mode const sm ) noexcept
            {
                switch( sm ) 
                {
                case natus::graphics::streamout_mode::interleaved: return GL_INTERLEAVED_ATTRIBS ;
                case natus::graphics::streamout_mode::separate: return GL_SEPARATE_ATTRIBS ;
                default: break ;
                }
                return GL_NONE ;
            }
        }
    }
}
