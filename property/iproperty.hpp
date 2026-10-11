// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "typedefs.h"

namespace motor
{
    namespace property
    {
        class iproperty
        {
        public: 
            virtual ~iproperty( void_t ) noexcept {}
        };
        motor_typedef( iproperty ) ;
    }
}