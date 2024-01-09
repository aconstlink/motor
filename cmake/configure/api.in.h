#pragma once

// this file is auto generated code.
// see cmake/configure/api.in.h

#if defined( MOTOR_BUILD_DLL )

#if defined( MOTOR_@LIB_NAME_UPPER@_EXPORT )
#define MOTOR_@LIB_NAME_UPPER@_API_C extern "C" __declspec(dllexport)
#define MOTOR_@LIB_NAME_UPPER@_API __declspec(dllexport)
#else
#define MOTOR_@LIB_NAME_UPPER@_API_C extern "C" __declspec(dllimport)
#define MOTOR_@LIB_NAME_UPPER@_API __declspec(dllimport)
#endif    

#else

#define MOTOR_@LIB_NAME_UPPER@_API_C extern
#define MOTOR_@LIB_NAME_UPPER@_API

#endif
