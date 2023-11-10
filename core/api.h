#pragma once

// this file is auto generated code.
// see cmake/configure/api.in.h

#if defined( MOTOR_BUILD_DLL )

#if defined( MOTOR_CORE_EXPORT )
#define MOTOR_CORE_API_C extern "C" __declspec(dllexport)
#define MOTOR_CORE_API __declspec(dllexport)
#else
#define MOTOR_CORE_API_C extern "C" __declspec(dllimport)
#define MOTOR_CORE_API __declspec(dllimport)
#endif    

#else

#define MOTOR_CORE_API_C 
#define MOTOR_CORE_API

#endif
