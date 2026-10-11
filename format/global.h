// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT



#pragma once

#include "api.h"
#include "typedefs.h"
#include "module_registry.hpp"

namespace motor
{
    namespace format
    {
        struct MOTOR_FORMAT_API global
        {
            static motor::format::module_registry_mtr_t register_default_modules( motor::format::module_registry_mtr_t ) ;
        };
        
    }
}