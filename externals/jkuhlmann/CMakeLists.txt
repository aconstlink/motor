message( STATUS ">>> [external : cgltf] >>>" )

# Used headers
set( INCLUDES 
  "cgltf/cgltf.h"
  "cgltf/cgltf_write.h"
  )

add_library( cgltf INTERFACE )
target_include_directories( cgltf INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/cgltf> )

if( MOTOR_COMPILER_MSC )
  target_compile_options( cgltf INTERFACE /wd4267 /wd4244 /wd4245 /wd4456 /wd4457 /wd4701 /wd4996  )
elseif( MOTOR_COMPILER_GNU )
  target_compile_options( cgltf INTERFACE -Wno-unused-local-typedefs -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable )
endif()

export( TARGETS cgltf 
  NAMESPACE ${PROJECT_NAME}::
  APPEND FILE ${MOTOR_BINARY_DIR}/${PROJECT_NAME}-external-targets.cmake )

install_library( cgltf ${PROJECT_NAME}-targets )

message( STATUS "[cgltf] : target available : cgltf" )
message( STATUS "<<< [external : cgltf] <<<" )
