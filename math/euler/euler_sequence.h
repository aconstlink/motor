// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

namespace motor
{
    namespace math
    {
        /// missing classic euler angle sequences
        enum class euler_sequence
        {
            invalid,
            xyz,
            yzx,
            zxy,
            xzy,
            zyx,
            yxz
        };
    }
}
