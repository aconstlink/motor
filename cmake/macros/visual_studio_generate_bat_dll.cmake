
#
# Pre Visual Studio 17 (2022) 
# It was required to add a path to the binary build directory for
# the windows runtime to find the compiled dlls
#
# Now it is not required anymore and the project can be opened via
# the .sln file where all compiled dlls are found when runing the applications.
#
macro( motor_vs_gen_bat_for_dlls  )
if( MSVC_IDE ) 
  
# for later usage: rename solution file and remove the old one. Need to be called after generation of solution file
  #file( RENAME ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.sln ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.DONT_START_THIS.sln )
  #file( REMOVE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.sln )
  file( WRITE "${CMAKE_BINARY_DIR}/START_HERE.bat" "set PATH=%PATH%;${motor_bindir_debug};${motor_bindir_debug}" "\n")
  file( APPEND "${CMAKE_BINARY_DIR}/START_HERE.bat" "call ${CMAKE_PROJECT_NAME}.sln" )
  
endif()
endmacro()
