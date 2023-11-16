#pragma once

// this file is auto generated code.
// see cmake/configure/api.in.h

#if defined( MOTOR_BUILD_DLL )

#if defined( MOTOR_IO_EXPORT )
#define MOTOR_IO_API_C extern "C" __declspec(dllexport)
#define MOTOR_IO_API __declspec(dllexport)
#else
#define MOTOR_IO_API_C extern "C" __declspec(dllimport)
#define MOTOR_IO_API __declspec(dllimport)
#endif    

#else

#define MOTOR_IO_API_C 
#define MOTOR_IO_API

#endif
