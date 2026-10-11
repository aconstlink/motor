// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT



#pragma once


#include "icomponent.h"

#include "../global.h"

namespace motor
{
    namespace scene
    {
        class variables_component : public motor::scene::icomponent
        {
            typedef icomponent base_t ;
            motor_this_typedefs( variables_component ) ;

        public:


        };
        motor_typedef( variables_component ) ;

    }
}