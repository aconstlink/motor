#pragma once

#include <motor/graphics/types.h>
#include <motor/ogl/gl/gl.h>

namespace motor
{
    namespace platform
    {
        namespace gl3
        {
            static GLenum convert( motor::graphics::type const t ) noexcept
            {
                switch( t )
                {
                case motor::graphics::type::tbool:
                    return GL_BOOL ;
                case motor::graphics::type::tfloat:
                    return GL_FLOAT ;
                case motor::graphics::type::tdouble:
                    return GL_DOUBLE ;
                case motor::graphics::type::tint:
                    return GL_INT ;
                case motor::graphics::type::tuint:
                    return GL_UNSIGNED_INT ;
                case motor::graphics::type::tshort:
                    return GL_SHORT ;
                case motor::graphics::type::tushort:
                    return GL_UNSIGNED_SHORT ;
                case motor::graphics::type::tchar:
                    return GL_BYTE ;
                default: return GL_NONE ;
                }
                return GL_NONE ;
            }

            static GLenum convert( motor::graphics::primitive_type const pt ) noexcept
            {
                switch( pt )
                {
                case motor::graphics::primitive_type::lines: return GL_LINES ;
                case motor::graphics::primitive_type::triangles: return GL_TRIANGLES ;
                case motor::graphics::primitive_type::points: return GL_POINTS ;
                default:break ;
                }
                return 0 ;
            }

            static GLenum convert_to_transform_feedback_output( motor::graphics::primitive_type const pt ) noexcept
            {
                switch( pt )
                {
                case motor::graphics::primitive_type::lines: return GL_LINES ;
                case motor::graphics::primitive_type::triangles: return GL_TRIANGLES ;
                case motor::graphics::primitive_type::points: return GL_POINTS ;
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

            static std::pair< motor::graphics::type, motor::graphics::type_struct >  
                to_type_type_struct( GLenum const e ) noexcept
            {
                switch( e )
                {
                case GL_FLOAT: 
                    return std::make_pair( motor::graphics::type::tfloat, motor::graphics::type_struct::vec1 ) ;
                case GL_FLOAT_VEC2: 
                    return std::make_pair( motor::graphics::type::tfloat, motor::graphics::type_struct::vec2 ) ;
                case GL_FLOAT_VEC3: 
                    return std::make_pair( motor::graphics::type::tfloat, motor::graphics::type_struct::vec3 ) ;
                case GL_FLOAT_VEC4:
                    return std::make_pair( motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 ) ;
                case GL_INT:
                    return std::make_pair( motor::graphics::type::tint, motor::graphics::type_struct::vec1 ) ;
                case GL_INT_VEC2:
                    return std::make_pair( motor::graphics::type::tint, motor::graphics::type_struct::vec2 ) ;
                case GL_INT_VEC3:
                    return std::make_pair( motor::graphics::type::tint, motor::graphics::type_struct::vec3 ) ;
                case GL_INT_VEC4:
                    return std::make_pair( motor::graphics::type::tint, motor::graphics::type_struct::vec4 ) ;
                case GL_UNSIGNED_INT:
                    return std::make_pair( motor::graphics::type::tuint, motor::graphics::type_struct::vec1 ) ;
                case GL_UNSIGNED_INT_VEC2:
                    return std::make_pair( motor::graphics::type::tuint, motor::graphics::type_struct::vec2 ) ;
                case GL_UNSIGNED_INT_VEC3:
                    return std::make_pair( motor::graphics::type::tuint, motor::graphics::type_struct::vec3 ) ;
                case GL_UNSIGNED_INT_VEC4:
                    return std::make_pair( motor::graphics::type::tuint, motor::graphics::type_struct::vec4 ) ;
                case GL_BOOL:
                    return std::make_pair( motor::graphics::type::tbool, motor::graphics::type_struct::vec1 ) ;
                case GL_BOOL_VEC2:
                    return std::make_pair( motor::graphics::type::tbool, motor::graphics::type_struct::vec2 ) ;
                case GL_BOOL_VEC3:
                    return std::make_pair( motor::graphics::type::tbool, motor::graphics::type_struct::vec3 ) ;
                case GL_BOOL_VEC4:
                    return std::make_pair( motor::graphics::type::tbool, motor::graphics::type_struct::vec4 ) ;
                case GL_FLOAT_MAT2:
                    return std::make_pair( motor::graphics::type::tfloat, motor::graphics::type_struct::mat2 ) ;
                case GL_FLOAT_MAT3:
                    return std::make_pair( motor::graphics::type::tfloat, motor::graphics::type_struct::mat3 ) ;
                case GL_FLOAT_MAT4:
                    return std::make_pair( motor::graphics::type::tfloat, motor::graphics::type_struct::mat4 ) ;
                case GL_SAMPLER_1D:
                case GL_SAMPLER_2D:
                case GL_SAMPLER_3D:
                case GL_INT_SAMPLER_1D:
                case GL_INT_SAMPLER_2D:
                case GL_INT_SAMPLER_3D:
                case GL_UNSIGNED_INT_SAMPLER_1D:
                case GL_UNSIGNED_INT_SAMPLER_2D:
                case GL_UNSIGNED_INT_SAMPLER_3D:
                case GL_SAMPLER_CUBE:
                case GL_SAMPLER_1D_SHADOW:
                case GL_SAMPLER_2D_SHADOW:
                case GL_SAMPLER_BUFFER:
                case GL_INT_SAMPLER_BUFFER:
                case GL_UNSIGNED_INT_SAMPLER_BUFFER:
                case GL_SAMPLER_2D_ARRAY:
                case GL_INT_SAMPLER_2D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: 
                    return std::make_pair( motor::graphics::type::tint, motor::graphics::type_struct::vec1 ) ;
                default: break ;
                }
                return std::make_pair( motor::graphics::type::undefined, motor::graphics::type_struct::undefined ) ;
            }
        }

        namespace gl3
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

            static GLenum convert( motor::graphics::blend_factor const bf ) noexcept
            {
                return detail::convert_blend_factor_gl[ size_t( bf ) ] ;
            }

            static GLenum convert( motor::graphics::blend_function const bf ) noexcept
            {
                switch( bf )
                {
                case motor::graphics::blend_function::add: return GL_FUNC_ADD ;
                case motor::graphics::blend_function::sub: return GL_FUNC_SUBTRACT ;
                case motor::graphics::blend_function::reverse_sub: return GL_FUNC_REVERSE_SUBTRACT ;
                case motor::graphics::blend_function::min: return GL_MIN ;
                case motor::graphics::blend_function::max: return GL_MAX ;
                default: break ;
                }

                return GL_ZERO ;
            }
        }

        namespace gl3
        {
            static GLenum convert_to_gl_format( motor::graphics::image_format const imf, 
                motor::graphics::image_element_type const t )
            {
                switch( imf )
                {
                case motor::graphics::image_format::depth:
                    return GL_DEPTH_COMPONENT ;

                case motor::graphics::image_format::rgb:
                    switch( t )
                    {
                    case motor::graphics::image_element_type::uint8: return GL_RGB8 ;
                    case motor::graphics::image_element_type::int16: return GL_RGB16I ;
                    case motor::graphics::image_element_type::int32: return GL_RGB32I ;
                    case motor::graphics::image_element_type::uint16: return GL_RGB16UI ;
                    case motor::graphics::image_element_type::uint32: return GL_RGB32UI ;
                    case motor::graphics::image_element_type::float16: return GL_RGB16F ;
                    case motor::graphics::image_element_type::float32: return GL_RGB32F ;
                    default: break ;
                    }
                    break  ;

                case motor::graphics::image_format::rgba:
                    switch( t )
                    {
                    case motor::graphics::image_element_type::uint8: return GL_RGBA8 ;
                    case motor::graphics::image_element_type::int16: return GL_RGBA16I ;
                    case motor::graphics::image_element_type::int32: return GL_RGBA32I ;
                    case motor::graphics::image_element_type::uint16: return GL_RGBA16UI ;
                    case motor::graphics::image_element_type::uint32: return GL_RGBA32UI ;
                    case motor::graphics::image_element_type::float16: return GL_RGBA16F ;
                    case motor::graphics::image_element_type::float32: return GL_RGBA32F ;
                    default: break ;
                    }
                    break  ;

                case motor::graphics::image_format::intensity:
                    switch( t )
                    {
                    case motor::graphics::image_element_type::uint8: return GL_RED ;
                    case motor::graphics::image_element_type::int16: return GL_R16I ;
                    case motor::graphics::image_element_type::int32: return GL_R32I ;
                    case motor::graphics::image_element_type::uint16: return GL_R16UI ;
                    case motor::graphics::image_element_type::uint32: return GL_R32UI ;
                    case motor::graphics::image_element_type::float16: return GL_R16F ;
                    case motor::graphics::image_element_type::float32: return GL_R32F ;
                    default: break ;
                    }
                    break  ;

                default: break ;

                }
                return 0 ;
            }

            static GLenum convert_to_gl_pixel_format( motor::graphics::image_format const imf )
            {
                switch( imf )
                {
                case motor::graphics::image_format::depth: return GL_DEPTH_COMPONENT ;
                case motor::graphics::image_format::rgb: return GL_RGB ;
                case motor::graphics::image_format::rgba: return GL_RGBA ;
                case motor::graphics::image_format::intensity: return GL_RED ;
                default: break ;
                }
                return 0 ;
            }

            static GLenum convert_to_gl_pixel_type( motor::graphics::image_element_type const pt )
            {
                switch( pt )
                {
                case motor::graphics::image_element_type::uint8: return GL_UNSIGNED_BYTE ;
                case motor::graphics::image_element_type::uint16: return GL_UNSIGNED_SHORT ;
                case motor::graphics::image_element_type::uint32: return GL_UNSIGNED_INT ;

                case motor::graphics::image_element_type::int8: return GL_BYTE ;
                case motor::graphics::image_element_type::int16: return GL_SHORT ;
                case motor::graphics::image_element_type::int32: return GL_INT ;

                case motor::graphics::image_element_type::float16: return GL_FLOAT ;
                case motor::graphics::image_element_type::float32: return GL_FLOAT ;
                default : break ;
                }
                return 0 ;
            }
        }

        namespace gl3
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

            static GLenum convert( motor::graphics::texture_type const m ) noexcept
            {
                return motor::platform::gl3::detail::texture_types_gl[ ( size_t ) m ] ;
            }

            static GLenum convert( motor::graphics::texture_filter_mode const m ) noexcept
            {
                return motor::platform::gl3::detail::filter_modes_gl[ ( size_t ) m ] ;
            }

            static GLenum convert( motor::graphics::texture_filter_type const t ) noexcept
            {
                return motor::platform::gl3::detail::filter_types_gl[ ( size_t ) t ] ;
            }

            namespace detail
            {
                static const GLenum wrap_modes_gl[] = {
                    GL_TEXTURE_WRAP_S,
                    GL_TEXTURE_WRAP_T,
                    GL_TEXTURE_WRAP_R
                } ;

                static const GLenum wrap_types_gl[] = {
                    GL_CLAMP_TO_BORDER,
                    GL_REPEAT,
                    GL_CLAMP_TO_BORDER,
                    GL_CLAMP_TO_EDGE,
                    GL_MIRRORED_REPEAT
                } ;
            }

            static GLenum convert( motor::graphics::texture_wrap_mode const m ) noexcept {
                return motor::platform::gl3::detail::wrap_modes_gl[ ( size_t ) m ] ;
            }

            static GLenum convert( motor::graphics::texture_wrap_type const t ) noexcept {
                return motor::platform::gl3::detail::wrap_types_gl[ ( size_t ) t ] ;
            }

            static GLenum to_pixel_type( motor::graphics::color_target_type const ct ) noexcept 
            {
                switch( ct )
                {
                case motor::graphics::color_target_type::rgba_float_32: return GL_FLOAT ;
                case motor::graphics::color_target_type::rgba_uint_8: return GL_UNSIGNED_BYTE ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_format( motor::graphics::color_target_type const ct ) noexcept
            {
                switch( ct )
                {
                case motor::graphics::color_target_type::rgba_float_32: return GL_RGBA32F ;
                case motor::graphics::color_target_type::rgba_uint_8: return GL_RGBA ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_format( motor::graphics::depth_stencil_target_type const t ) noexcept
            {
                switch( t )
                {
                case motor::graphics::depth_stencil_target_type::depth32: return GL_DEPTH_COMPONENT ;
                case motor::graphics::depth_stencil_target_type::depth24_stencil8: return GL_DEPTH_STENCIL ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_attachment( motor::graphics::depth_stencil_target_type const t ) noexcept
            {
                switch( t )
                {
                case motor::graphics::depth_stencil_target_type::depth32: return GL_DEPTH_ATTACHMENT ;
                case motor::graphics::depth_stencil_target_type::depth24_stencil8: return GL_DEPTH_STENCIL_ATTACHMENT ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum to_gl_type( motor::graphics::depth_stencil_target_type const t ) noexcept
            {
                switch( t )
                {
                case motor::graphics::depth_stencil_target_type::depth32 : return GL_FLOAT ;
                case motor::graphics::depth_stencil_target_type::depth24_stencil8: return GL_UNSIGNED_INT_24_8 ;
                default: break;
                }
                return GL_NONE ;
            }

            static size_t calc_sib( size_t const width, size_t const height, motor::graphics::color_target_type const ct ) noexcept
            {
                switch( ct )
                {
                case motor::graphics::color_target_type::rgba_float_32: 
                    return width * height * 4 * 4 ;
                case motor::graphics::color_target_type::rgba_uint_8: 
                    return width * height * 4 * 1 ;
                default: break;
                }
                return GL_NONE ;
            }
        }

        namespace gl3
        {
            static GLenum convert( motor::graphics::cull_mode const cm ) noexcept
            {
                switch( cm )
                {
                case motor::graphics::cull_mode::back: return GL_BACK ;
                case motor::graphics::cull_mode::front: return GL_FRONT ;
                case motor::graphics::cull_mode::none: return GL_NONE ;
                default: break;
                }
                return GL_NONE ;
            }


            static GLenum convert( motor::graphics::fill_mode const fm ) noexcept
            {
                switch( fm )
                {
                case motor::graphics::fill_mode::fill: return GL_FILL ;
                case motor::graphics::fill_mode::point: return GL_POINT ;
                case motor::graphics::fill_mode::line: return GL_LINE ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum convert( motor::graphics::front_face const ff ) noexcept
            {
                switch( ff )
                {
                case motor::graphics::front_face::clock_wise: return GL_CW ;
                case motor::graphics::front_face::counter_clock_wise: return GL_CCW ;
                default: break;
                }
                return GL_NONE ;
            }

            static GLenum convert_for_texture_buffer( motor::graphics::type const t, motor::graphics::type_struct const ts ) noexcept
            {
                switch( t )
                {
                case motor::graphics::type::tfloat: 
                    switch( ts ) 
                    {
                    case motor::graphics::type_struct::vec4:
                        return GL_RGBA32F ;
                    default: break ;
                    }
                    break ;

                case motor::graphics::type::tchar: 
                    switch( ts ) 
                    {
                    case motor::graphics::type_struct::vec4:
                        return GL_RGBA8 ;
                    default: break ;
                    }
                    break ;

                case motor::graphics::type::tuchar: 
                    switch( ts ) 
                    {
                    case motor::graphics::type_struct::vec4:
                        return GL_RGBA8UI ;
                    default: break ;
                    }
                    break ;

                case motor::graphics::type::tuint: 
                    switch( ts ) 
                    {
                    case motor::graphics::type_struct::vec4:
                        return GL_RGBA32UI ;
                    default: break ;
                    }
                    break ;

                case motor::graphics::type::tint: 
                    switch( ts ) 
                    {
                    case motor::graphics::type_struct::vec4:
                        return GL_RGBA32I ;
                    default: break ;
                    }
                    break ;

                default: break ;
                }

                return GL_NONE ;
            }
        }

        namespace gl3
        {
            static GLenum convert( motor::graphics::streamout_mode const sm ) noexcept
            {
                switch( sm ) 
                {
                case motor::graphics::streamout_mode::interleaved: return GL_INTERLEAVED_ATTRIBS ;
                case motor::graphics::streamout_mode::separate: return GL_SEPARATE_ATTRIBS ;
                default: break ;
                }
                return GL_NONE ;
            }
        }
    }
}
