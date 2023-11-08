#pragma once

// this file is auto generated code.
// see cmake/configure/api.in.h

#if defined( NATUS_BUILD_DLL )

#if defined( NATUS_@LIB_NAME_UPPER@_EXPORT )
#define NATUS_@LIB_NAME_UPPER@_API_C extern "C" __declspec(dllexport)
#define NATUS_@LIB_NAME_UPPER@_API __declspec(dllexport)
#else
#define NATUS_@LIB_NAME_UPPER@_API_C extern "C" __declspec(dllimport)
#define NATUS_@LIB_NAME_UPPER@_API __declspec(dllimport)
#endif    

#else

#define NATUS_@LIB_NAME_UPPER@_API_C 
#define NATUS_@LIB_NAME_UPPER@_API

#endif
