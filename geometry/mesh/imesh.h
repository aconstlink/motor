// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma  once

#include "../typedefs.h"
#include "../api.h"
#include "../result.h"
#include "../protos.h"

#include "../enums/component_formats.h"

namespace motor
{
    namespace geometry
    {
        class MOTOR_GEOMETRY_API imesh
        {
        public:

            virtual ~imesh( void_t ) {}
        };
        motor_typedef( imesh ) ;
    }
}
