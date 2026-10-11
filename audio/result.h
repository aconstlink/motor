// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

namespace motor
{
    namespace audio
    {
        enum class result
        {
            ok, 
            failed,
            invalid,
            invalid_argument,
            initial,
            in_progress,
            idle
        };
        motor_typedef( result ) ;


        enum class async_result
        {
            ok,
            failed,
            invalid,
            in_transit,
            user_edit
        };
    }
}