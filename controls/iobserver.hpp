// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

#include "typedefs.h"

#include <motor/log/global.h>

namespace motor
{
    namespace controls
    {
        class iobserver
        { 
        public:
            virtual ~iobserver( void_t ) noexcept{}
        };
        motor_typedef( iobserver ) ;

    }

}
