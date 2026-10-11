// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "ivisitor.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API code_exe_visitor : public ivisitor
        {
            motor_this_typedefs( code_exe_visitor ) ;

        public:

            virtual ~code_exe_visitor( void_t ) noexcept {}

            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const  ) noexcept ;
            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept ;

        };
        motor_typedef( code_exe_visitor ) ;
    }
}