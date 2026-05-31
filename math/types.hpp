
#pragma once

//
// This file contains some run-type info
//
namespace motor
{
namespace math
{
enum class type
{
    invalid,
    tbool,
    tbyte,
    tubyte,
    tshort,
    tushort,
    tint,
    tuint,
    tfloat,
    tdouble,
    num_types
};

enum class type_struct
{
    invalid,
    vec1,
    vec2,
    vec3,
    vec4,
    mat2,
    mat3,
    mat4,
    num_type_structs
};

namespace detail
{
// sib
static size_t const __size_list_type[] = { 0, 1, 1, 1, 2, 2, 4, 4, 4, 8 };

// number of elements
static size_t const __size_list_type_struct[] = { 0, 1, 2, 3, 4, 4, 9, 16 };
} // namespace detail

static size_t num_elem( motor::math::type_struct const ts ) noexcept
{
    return detail::__size_list_type_struct[ size_t( ts ) >= size_t( type_struct::num_type_structs )
                                                ? 0
                                                : size_t( ts ) ];
}

static size_t size_of( motor::math::type const t ) noexcept
{
    return detail::__size_list_type[ size_t( t ) >= size_t( type::num_types ) ? 0 : size_t( t ) ];
}

static size_t size_of( motor::math::type const t, motor::math::type_struct const ts ) noexcept
{
    return motor::math::num_elem( ts ) * size_of( t ) ;
}


} // namespace math
} // namespace motor