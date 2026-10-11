// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

#include "api.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API ivisitable
        {

        public:
            virtual ~ivisitable( void_t ) noexcept {}
        };
    }
}