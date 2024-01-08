#pragma once

// this file is auto generated code.
// see cmake/configure/api.in.h

#if defined( MOTOR_BUILD_DLL )

#if defined( MOTOR_CONCURRENT_EXPORT )
#define MOTOR_CONCURRENT_API_C extern "C" __declspec(dllexport)
#define MOTOR_CONCURRENT_API_CXX __declspec(dllexport)
#define MOTOR_CONCURRENT_API __declspec(dllexport)
#else
#define MOTOR_CONCURRENT_API_C extern "C" __declspec(dllimport)
#define MOTOR_CONCURRENT_API_CXX __declspec(dllimport)
#define MOTOR_CONCURRENT_API __declspec(dllimport)
#endif    

#else

#define MOTOR_CONCURRENT_API_C extern
#define MOTOR_CONCURRENT_API

#endif
