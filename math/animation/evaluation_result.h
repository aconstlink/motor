// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../typedefs.h"

namespace motor
{
    namespace math
    {
        enum class evaluation_result
        {
            invalid,
            out_of_range,
            in_range
        };

        static bool_t is_value_usable( evaluation_result const ev )
        {
            return ev != evaluation_result::invalid ;
        }

        static bool_t is_in_range( evaluation_result const ev )
        {
            return ev == evaluation_result::in_range ;
        }

        static bool_t is_out_of_range( evaluation_result const ev )
        {
            return ev == evaluation_result::out_of_range ;
        }
    }
}
