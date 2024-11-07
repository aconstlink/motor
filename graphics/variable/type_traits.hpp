
#include "../types.h"


#include <limits>
#include <cstdlib>

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

#include <motor/math/matrix/matrix2.hpp>
#include <motor/math/matrix/matrix3.hpp>
#include <motor/math/matrix/matrix4.hpp>


namespace motor
{
    namespace graphics
    {
        template< typename type_t >
        class type_traits
        {
            static const motor::graphics::type gpu_type ;
            static const motor::graphics::type_struct gpu_type_struct ;
        };

        template<>
        class type_traits< ushort_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tshort ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec1 ;
        };

        template<>
        class type_traits< motor::math::vec2us_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tshort ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec2 ;
        };

        template<>
        class type_traits< motor::math::vec3us_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tshort ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec3 ;
        };

        template<>
        class type_traits< motor::math::vec4us_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tshort ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec4 ;
        };

        template<>
        class type_traits< uint_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec1 ;
        };

        template<>
        class type_traits< motor::math::vec2ui_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec2 ;
        };

        template<>
        class type_traits< motor::math::vec3ui_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec3 ;
        };

        template<>
        class type_traits< motor::math::vec4ui_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec4 ;
        };

        template<>
        class type_traits< int_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec1 ;
        };

        template<>
        class type_traits< motor::math::vec2i_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec2 ;
        };

        template<>
        class type_traits< motor::math::vec3i_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec3 ;
        };

        template<>
        class type_traits< motor::math::vec4i_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tint ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec4 ;
        };

        template<>
        class type_traits< float_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tfloat ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec1 ;
        };

        template<>
        class type_traits< motor::math::vec2f_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tfloat ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec2 ;
        };

        template<>
        class type_traits< motor::math::vec3f_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tfloat ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec3 ;
        };

        template<>
        class type_traits< motor::math::vec4f_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tfloat ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec4 ;
        };

        template<>
        class type_traits< bool_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tbool ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec1 ;
        };

        template<>
        class type_traits< motor::math::vec2b_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tbool ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec2 ;
        };

        template<>
        class type_traits< motor::math::vec3b_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tbool ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec3 ;
        };

        template<>
        class type_traits< motor::math::vec4b_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tbool ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec4 ;
        };

        template<>
        class type_traits< char_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec1 ;
        };

        template<>
        class type_traits< motor::math::vec2c_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec2 ;
        };

        template<>
        class type_traits< motor::math::vec3c_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec3 ;
        };

        template<>
        class type_traits< motor::math::vec4c_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec4 ;
        };

        template<>
        class type_traits< uchar_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec1 ;
        };

        template<>
        class type_traits< motor::math::vec2uc_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec2 ;
        };

        template<>
        class type_traits< motor::math::vec3uc_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec3 ;
        };

        template<>
        class type_traits< motor::math::vec4uc_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tuchar ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::vec4 ;
        };

        template<>
        class type_traits< motor::math::mat2f_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tfloat ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::mat2 ;
        };

        template<>
        class type_traits< motor::math::mat3f_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tfloat ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::mat3 ;
        };

        template<>
        class type_traits< motor::math::mat4f_t >
        {
        public:

            static const motor::graphics::type gpu_type = motor::graphics::type::tfloat ;
            static const motor::graphics::type_struct gpu_type_struct = motor::graphics::type_struct::mat4 ;
        };
    }
}