message( STATUS ">>> [external : stb] >>>" )

# Used headers
set( INCLUDES 
  "stb/stb_include.h"
  "stb/stb_image.h"
  "stb/stb_image_write.h"
  "stb/stb_truetype.h"
  "stb/stb_vorbis.c" )

add_library( stb INTERFACE )
#arget_sources( stb INTERFACE ${INCLUDES} )

target_include_directories( stb 
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/stb> 
  INTERFACE $<INSTALL_INTERFACE:include/stb> )

if( MOTOR_COMPILER_MSC )
  target_compile_options( stb INTERFACE /wd4267 /wd4244 /wd4245 /wd4456 /wd4457 /wd4701 /wd4996  )
elseif( MOTOR_COMPILER_GNU )
  target_compile_options( stb INTERFACE -Wno-unused-local-typedefs -Wno-unused-parameter -Wno-sign-compare -Wno-unused-but-set-variable )
endif()

message(${MOTOR_BINARY_DIR}/motor-targets.cmake)
export( TARGETS stb NAMESPACE motor:: APPEND FILE ${MOTOR_BINARY_DIR}/motor-targets.cmake )

#install_headers( "${SOURCES}" "include/lua" )
install( TARGETS stb EXPORT ${PROJECT_NAME}-targets )

message( STATUS "[stb] : target available : stb" )
message( STATUS "<<< [external : stb] <<<" )
