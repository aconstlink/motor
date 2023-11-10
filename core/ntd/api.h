#pragma once

// this file is auto generated code.
// see cmake/configure/api.in.h

#if defined( NATUS_BUILD_DLL )

#if defined( NATUS_NTD_EXPORT )
#define NATUS_NTD_API_C extern "C" __declspec(dllexport)
#define NATUS_NTD_API __declspec(dllexport)
#else
#define NATUS_NTD_API_C extern "C" __declspec(dllimport)
#define NATUS_NTD_API __declspec(dllimport)
#endif    

#else

#define NATUS_NTD_API_C 
#define NATUS_NTD_API

#endif
