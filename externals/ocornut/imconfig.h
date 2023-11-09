
#pragma once

#if defined( MOTOR_BUILD_DLL )
#if defined( MOTOR_IMGUI_EXPORT )
#define IMGUI_API __declspec( dllexport )
#else
#define IMGUI_API __declspec( dllimport )
#endif
#else
#endif