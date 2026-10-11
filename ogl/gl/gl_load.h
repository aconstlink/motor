// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
    namespace ogl
    {
        struct gl_load
        {
            static void_ptr_t load_function( char_cptr_t name ) noexcept ;
        } ;
    }
}