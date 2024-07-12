
#pragma once

#if defined( MOTOR_BUILD_DLL )
#if defined( MOTOR_IMNODES_EXPORT )
#define IMNODES_API __declspec( dllexport )
#else
#define IMNODES_API __declspec( dllimport )
#endif
#else
#endif