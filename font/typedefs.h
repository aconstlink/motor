// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include <motor/std/vector>
#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>

#include <string>

namespace motor
{
    namespace font
    {
        using namespace motor::core ;
        using namespace motor::core::types ;


        motor_typedefs( uint32_t, utf32 ) ;
        motor_typedefs( motor::vector< utf32_t >, code_points ) ;
    }
}