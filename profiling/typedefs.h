// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>

#include <chrono>

namespace motor
{
    namespace profiling
    {
        using namespace motor::core ;
        using namespace motor::core::types ;

        using clock_t = std::chrono::high_resolution_clock ;
    }
}