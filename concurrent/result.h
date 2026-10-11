// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

namespace motor
{
    namespace concurrent
    {
        enum result
        {
            ok,
            failed,
            invalid_argument,
            mt_processing
        };
    }
}