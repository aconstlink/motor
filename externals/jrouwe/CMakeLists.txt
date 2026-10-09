message( STATUS ">>> [external] : JoltPhysics" )

# Keep the upstream names local to this integration and expose only JOLT_*.
# Existing cache values seed the new options on the first configure.
function( motor_jolt_cache_variable name type default_value description )
    string( REGEX REPLACE "^JPH_" "" public_name "${name}" )
    set( public_name "JOLT_${public_name}" )

    if( DEFINED CACHE{${name}} )
        get_property( default_value CACHE "${name}" PROPERTY VALUE )
    endif()
    set( ${public_name} "${default_value}" CACHE ${type} "${description}" )

    # Seed typed entries as well: upstream set(CACHE) must preserve our STRING
    # values even with CMP0126 OLD (Jolt currently requires CMake 3.20).
    set( ${name} "${${public_name}}" CACHE ${type} "${description}" FORCE )
    set( ${name} "${${public_name}}" PARENT_SCOPE )
    set( motor_jolt_cache_names ${motor_jolt_cache_names} "${name}" PARENT_SCOPE )
endfunction()

set( motor_jolt_cache_names )

motor_jolt_cache_variable( USE_ASSERTS BOOL OFF "Jolt: enable assertions" )
motor_jolt_cache_variable( DOUBLE_PRECISION BOOL OFF "Jolt: use double precision positions" )
motor_jolt_cache_variable( GENERATE_DEBUG_SYMBOLS BOOL ON "Jolt: generate debug symbols" )
motor_jolt_cache_variable( JPH_DEBUG_SYMBOL_FORMAT STRING "" "Jolt: debug symbol format" )
motor_jolt_cache_variable( OVERRIDE_CXX_FLAGS BOOL ON "Jolt: override release compiler flags" )
motor_jolt_cache_variable( CROSS_PLATFORM_DETERMINISTIC BOOL OFF "Jolt: cross-platform deterministic simulation" )
motor_jolt_cache_variable( CROSS_COMPILE_ARM BOOL OFF "Jolt: cross-compile for ARM" )
motor_jolt_cache_variable( CROSS_COMPILE_ARM_TARGET STRING "aarch64-linux-gnu" "Jolt: ARM cross-compilation target" )
motor_jolt_cache_variable( JPH_BUILD_SHARED_LIBS BOOL "${BUILD_SHARED_LIBS}" "Jolt: build a shared library" )
motor_jolt_cache_variable( INTERPROCEDURAL_OPTIMIZATION BOOL ON "Jolt: enable link-time optimization" )
motor_jolt_cache_variable( FLOATING_POINT_EXCEPTIONS_ENABLED BOOL ON "Jolt: enable floating-point exceptions" )
motor_jolt_cache_variable( CPP_EXCEPTIONS_ENABLED BOOL OFF "Jolt: enable C++ exceptions" )
motor_jolt_cache_variable( CPP_RTTI_ENABLED BOOL OFF "Jolt: enable C++ RTTI" )
motor_jolt_cache_variable( OBJECT_LAYER_BITS STRING "16" "Jolt: number of bits in ObjectLayer" )
set_property( CACHE JOLT_OBJECT_LAYER_BITS PROPERTY STRINGS 16 32 )

motor_jolt_cache_variable( USE_SSE4_1 BOOL ON "Jolt: enable SSE4.1" )
motor_jolt_cache_variable( USE_SSE4_2 BOOL ON "Jolt: enable SSE4.2" )
motor_jolt_cache_variable( USE_AVX BOOL ON "Jolt: enable AVX" )
motor_jolt_cache_variable( USE_AVX2 BOOL ON "Jolt: enable AVX2" )
motor_jolt_cache_variable( USE_AVX512 BOOL OFF "Jolt: enable AVX512" )
motor_jolt_cache_variable( USE_LZCNT BOOL ON "Jolt: enable LZCNT" )
motor_jolt_cache_variable( USE_TZCNT BOOL ON "Jolt: enable TZCNT" )
motor_jolt_cache_variable( USE_F16C BOOL ON "Jolt: enable F16C" )
motor_jolt_cache_variable( USE_FMADD BOOL ON "Jolt: enable FMADD" )
motor_jolt_cache_variable( USE_WASM_SIMD BOOL OFF "Jolt: enable WebAssembly SIMD" )
motor_jolt_cache_variable( JPH_USE_WASM64 BOOL OFF "Jolt: enable 64-bit WebAssembly" )

motor_jolt_cache_variable( ENABLE_ALL_WARNINGS BOOL ON "Jolt: enable all warnings and treat them as errors" )
motor_jolt_cache_variable( TRACK_BROADPHASE_STATS BOOL OFF "Jolt: track broadphase statistics" )
motor_jolt_cache_variable( TRACK_NARROWPHASE_STATS BOOL OFF "Jolt: track narrowphase statistics" )
motor_jolt_cache_variable( JPH_TRACK_SIMULATION_STATS BOOL OFF "Jolt: track simulation statistics" )
motor_jolt_cache_variable( DEBUG_RENDERER_IN_DEBUG_AND_RELEASE BOOL ON "Jolt: debug renderer in Debug and Release" )
motor_jolt_cache_variable( DEBUG_RENDERER_IN_DISTRIBUTION BOOL OFF "Jolt: debug renderer in all configurations" )
motor_jolt_cache_variable( PROFILER_IN_DEBUG_AND_RELEASE BOOL ON "Jolt: profiler in Debug and Release" )
motor_jolt_cache_variable( PROFILER_IN_DISTRIBUTION BOOL OFF "Jolt: profiler in all configurations" )
motor_jolt_cache_variable( JPH_USE_EXTERNAL_PROFILE BOOL OFF "Jolt: use an external profiler" )
motor_jolt_cache_variable( DISABLE_CUSTOM_ALLOCATOR BOOL OFF "Jolt: disable custom memory allocators" )
motor_jolt_cache_variable( ENABLE_OBJECT_STREAM BOOL ON "Jolt: enable ObjectStream and RTTI attributes" )
motor_jolt_cache_variable( ENABLE_INSTALL BOOL ON "Jolt: generate installation rules" )
motor_jolt_cache_variable( USE_STATIC_MSVC_RUNTIME_LIBRARY BOOL ON "Jolt: static MSVC runtime (MSVC desktop only)" )

motor_jolt_cache_variable( JPH_USE_DX12 BOOL ON "Jolt: enable DirectX 12 compute" )
motor_jolt_cache_variable( JPH_USE_VK BOOL ON "Jolt: enable Vulkan compute" )
motor_jolt_cache_variable( JPH_USE_MTL BOOL ON "Jolt: enable Metal compute" )
motor_jolt_cache_variable( JPH_USE_CPU_COMPUTE BOOL ON "Jolt: enable CPU compute" )
motor_jolt_cache_variable( JPH_SHADER_DEBUG_SYMBOLS BOOL OFF "Jolt: generate shader debug symbols" )
motor_jolt_cache_variable( JPH_SHADER_OPTIMIZATION BOOL ON "Jolt: optimize shaders" )

# Jolt ignores these when embedded via add_subdirectory; retain that behavior.
motor_jolt_cache_variable( TARGET_UNIT_TESTS BOOL ON "Jolt: build unit tests (standalone only; inactive in motor)" )
motor_jolt_cache_variable( TARGET_HELLO_WORLD BOOL ON "Jolt: build HelloWorld (standalone only; inactive in motor)" )
motor_jolt_cache_variable( TARGET_PERFORMANCE_TEST BOOL ON "Jolt: build performance test (standalone only; inactive in motor)" )
motor_jolt_cache_variable( TARGET_SAMPLES BOOL ON "Jolt: build samples (standalone only; inactive in motor)" )
motor_jolt_cache_variable( TARGET_VIEWER BOOL ON "Jolt: build viewer (standalone only; inactive in motor)" )
mark_as_advanced( JOLT_TARGET_UNIT_TESTS JOLT_TARGET_HELLO_WORLD
    JOLT_TARGET_PERFORMANCE_TEST JOLT_TARGET_SAMPLES JOLT_TARGET_VIEWER )

add_subdirectory( "JoltPhysics/Build" )

# Remove only the explicitly wrapped entries, not compiler/SDK discovery data.
foreach( name IN LISTS motor_jolt_cache_names )
    unset( ${name} CACHE )
endforeach()
unset( motor_jolt_cache_names )

function( motor_jolt_target_folders directory )
    get_property( targets DIRECTORY "${directory}" PROPERTY BUILDSYSTEM_TARGETS )
    foreach( target IN LISTS targets )
        set_target_properties( ${target} PROPERTIES FOLDER "motor/externals/JoltPhysics" )
        message( STATUS ${target})
    endforeach()

    get_property( directories DIRECTORY "${directory}" PROPERTY SUBDIRECTORIES )
    foreach( child IN LISTS directories )
        motor_jolt_target_folders( "${child}" )
    endforeach()
endfunction()

motor_jolt_target_folders( "${CMAKE_CURRENT_SOURCE_DIR}/JoltPhysics/Build" )

message( STATUS "<<< [external] : JoltPhysics" )
