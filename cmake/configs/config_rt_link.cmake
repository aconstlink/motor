
##
## Run-Time Linkage
##
message( STATUS ">>> Checking Run-Time Linkage"  )

if( NOT MOTOR_LIBRARY_BUILD_SHARED )
set( MOTOR_RUNTIME_LINKAGE "Static" CACHE STRING "How to link the C++ Runtime?" FORCE )
else()
set( MOTOR_RUNTIME_LINKAGE "Shared" CACHE STRING "How to link the C++ Runtime?" FORCE )
endif()

set_property(CACHE MOTOR_RUNTIME_LINKAGE PROPERTY STRINGS "Static" "Shared")

set( MOTOR_RUNTIME_STATIC FALSE )
set( MOTOR_RUNTIME_DYNAMIC FALSE )

if( ${MOTOR_RUNTIME_LINKAGE} STREQUAL "Static" )
    set( MOTOR_RUNTIME_STATIC TRUE )
elseif( ${MOTOR_RUNTIME_LINKAGE} STREQUAL "Shared" )
    set( MOTOR_RUNTIME_DYNAMIC TRUE )
endif()

message( STATUS "<<< Run-Time Linkage: " ${MOTOR_RUNTIME_LINKAGE} )


