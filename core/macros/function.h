#pragma once

#include "common.h"

#if defined( MOTOR_COMPILER_MSC )

    #define MOTOR_FUNC __func__ //__FUNCTION__
    #define MOTOR_LINE __LINE__

#elif defined( MOTOR_COMPILER_GNU )

    #define MOTOR_FUNC "Func"
    #define MOTOR_LINE __LINE__

#else

    #define MOTOR_FUNC "MOTOR_FUNC" 
    #define MOTOR_LINE 0

#endif




