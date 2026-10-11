// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

namespace motor
{
    namespace math
    {
        enum class continuity
        {
            c0, // piece-wise 
            c1, // 1st derivitive
            c2 // 2nd derivitive
        };
    }
}