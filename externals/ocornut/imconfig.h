
#pragma once

#if defined( MOTOR_BUILD_DLL )
#if defined( MOTOR_IMGUI_EXPORT )
#define IMGUI_API __declspec( dllexport )
#else
#define IMGUI_API __declspec( dllimport )
#endif
#else
#endif

// abusing this imgui config for implot. Implot does not have any config included, 
// so IMPLOT_API can not be set there.
#if defined( MOTOR_BUILD_DLL )
#if defined( MOTOR_IMPLOT_EXPORT )
#define IMPLOT_API __declspec( dllexport ) 
#else
#define IMPLOT_API __declspec( dllimport )
#endif
#else
#endif