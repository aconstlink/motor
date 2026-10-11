// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include <motor/base/macros/typedef.h>

namespace motor
{
    namespace concurrent
    {
        motor_class_proto_typedefs( job_scheduler ) ;

        motor_class_proto_typedefs( worker_thread ) ;
        motor_class_proto_typedefs( sync_object ) ;

        motor_class_proto_typedefs( ijob ) ;
    }
}
