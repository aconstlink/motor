#pragma once

#include "typedefs.h"
#include <motor/std/string>

namespace motor
{
    namespace graphics
    {
        enum class type
        {
            undefined,
            tchar,
            tuchar,
            tshort,
            tushort,
            tint,
            tuint,
            tfloat,
            tdouble,
            tbool,
            num_scalar_types
        } ;

        enum class type_struct
        {
            undefined,
            vec1, // scalar
            vec2,
            vec3,
            vec4,
            mat2,
            mat3,
            mat4,
            num_type_structures
        } ;

        struct ctype
        {
            type t = motor::graphics::type::undefined ;
            type_struct ts = motor::graphics::type_struct::undefined ;
        };

        namespace detail
        {
            static motor::string_t const type_string_array[] =
            {
                "t_not_implemented"
            } ;

            // this must correspond to the TYPE enum
            static size_t const type_size_array[] =
            {
                0, sizeof ( char ), sizeof ( unsigned char ), sizeof ( short ), sizeof ( unsigned short ),
                sizeof ( int ), sizeof ( unsigned int ), sizeof ( float ), sizeof ( double ), sizeof ( bool )
            } ;

            static motor::string_t const type_structure_string_array[] =
            {
                "ts_not_implemented"
            } ;

            // this must correspond to the TYPE_STRUCTURE enum
            static uint_t const _type_struct_size_array[] =
            {
                0, 1, 2, 3, 4, 4, 9, 16
            } ;

            // a vector every has 1 row, so vectors must stored in memory
            // like a row vector, of course.
            static uint_t const type_struct_rows[] =
            {
                0, 1, 1, 1, 1, 2, 3, 4
            } ;

            static uint_t const type_struct_columns[] =
            {
                0, 1, 2, 3, 4, 2, 3, 4
            } ;
        }

        static motor::string_cref_t to_string( type const /*t*/ ) noexcept
        {
            return detail::type_string_array[ 0 ] ;
        }

        static motor::string_cref_t to_string( type_struct const /*t*/ ) noexcept
        {
            return detail::type_structure_string_array[ 0 ] ;
        }

        static size_t size_of( type const t ) noexcept
        {
            return detail::type_size_array[ size_t( t ) ] ;
        }

        static size_t size_of( type_struct const ts ) noexcept
        {
            return detail::_type_struct_size_array[ size_t( ts ) ] ;
        }

        /// returns the number of rows a type structure has.
        /// Dont forget. Vectors have a row count of 1
        static size_t rows_of( type_struct const ts ) noexcept
        {
            return detail::type_struct_rows[ size_t( ts ) ] ;
        }

        /// returns the number of columns a type struct has.
        static size_t columns_of( type_struct const ts ) noexcept
        {
            return detail::type_struct_columns[ size_t( ts ) ] ;
        }

        static size_t num_components_of( type_struct const ts ) noexcept
        {
            return detail::type_struct_columns[ size_t( ts ) ] ;
        }
    }

    namespace graphics
    {
        enum class primitive_type
        {
            undefined,
            points,
            lines,
            triangles,
            num_primitive_types
        } ;

        namespace detail
        {
            static motor::string_t const primitive_type_string_array[] =
            {
                "undefined", "points", "lines", "triangles"
            } ;

            static size_t const vertex_count_for_primitive_type[] =
            {
                0,
                1,
                1,
                2,
                2,
                2,
                3,
                3,
                3,
                0
            } ;
        }

        static motor::string_cref_t to_string( primitive_type const pt )
        {
            return detail::primitive_type_string_array[ size_t( pt ) ] ;
        }

        static size_t num_vertices( primitive_type const pt )
        {
            return detail::vertex_count_for_primitive_type[ size_t( pt ) ] ;
        }
    }

    namespace graphics
    {
        enum class usage_type
        {
            unknown,
            buffer_static,
            buffer_dynamic
        };
    }

    namespace graphics
    {
        enum class image_format
        {
            unknown,
            rgb,
            rgba,
            intensity,
            depth
        };

        // per channel type and bit-ness
        enum class image_element_type
        {
            unknown,
            float8,
            float16,
            float32,
            int8,
            int16,
            int32,
            uint8,
            uint16,
            uint32
        };

        // returns the number of channels.
        static size_t size_of( motor::graphics::image_format const imf )
        {
            switch( imf )
            {
            case motor::graphics::image_format::rgb: return 3 ;
            case motor::graphics::image_format::rgba: return 4 ;
            default: break ;
            }
            return 0 ;
        }

        // returns the sib of a type
        static size_t size_of( motor::graphics::image_element_type const iet )
        {
            switch( iet )
            {
            case motor::graphics::image_element_type::float8: return 1 ;
            case motor::graphics::image_element_type::float16: return 2 ;
            case motor::graphics::image_element_type::float32: return 4 ;

            case motor::graphics::image_element_type::int8: return 1 ;
            case motor::graphics::image_element_type::int16: return 2 ;
            case motor::graphics::image_element_type::int32: return 4 ;

            case motor::graphics::image_element_type::uint8: return 1 ;
            case motor::graphics::image_element_type::uint16: return 2 ;
            case motor::graphics::image_element_type::uint32: return 4 ;

            default: break ;
            }
            return 0 ;
        }
    }

    namespace graphics
    {
        enum class texture_type
        {
            unknown,
            //texture_1d,
            texture_2d,
            //texture_3d,
            //texture_1d_array,
            texture_2d_array
        };

        enum class texture_wrap_mode
        {
            wrap_s,
            wrap_t,
            wrap_r,
            size
        } ;

        enum class texture_wrap_type
        {
            clamp,
            repeat,
            clamp_border,
            clamp_edge,
            mirror,
            size
        } ;

        enum class texture_filter_mode
        {
            mag_filter,
            min_filter,
            size
        } ;

        enum class texture_filter_type
        {
            nearest,
            linear,
            size
        } ;
    }

    namespace graphics
    {
        enum class color_target_type
        {
            unknown,
            rgba_uint_8,
            rgba_float_32,
        };

        enum class depth_stencil_target_type
        {
            unknown,
            depth32,
            depth24_stencil8
        };
    }

    namespace graphics
    {
        enum class streamout_mode
        {
            unknown,
            interleaved,
            separate
        };
    }
}