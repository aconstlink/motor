#
# Can be used to configure the rendering system 
# for the target platform.
#

set( MOTOR_GRAPHICS_CONFIGURED FALSE )

set( THIS_TARGET motor_graphics_options )
add_library( ${THIS_TARGET} INTERFACE )

set( MOTOR_GRAPHICS_NULL ON )
set( MOTOR_GRAPHICS_VULKAN OFF )
set( MOTOR_GRAPHICS_OPENGL OFF )
set( MOTOR_GRAPHICS_OPENGLES OFF )
set( MOTOR_GRAPHICS_DIRECT3D OFF )
set( MOTOR_GRAPHICS_EGL OFF )
set( MOTOR_GRAPHICS_GLX OFF )
set( MOTOR_GRAPHICS_WGL OFF )

#####################################################################
#
# Test OpenGL
#
set( OpenGL_GL_PREFERENCE GLVND )
find_package( OpenGL )

if( OPENGL_FOUND )

  set( MOTOR_GRAPHICS_OPENGL ON )
  target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_GRAPHICS_OPENGL )

  if( OpenGL_EGL_FOUND )
    set( MOTOR_GRAPHICS_EGL ON )
    find_library( EGL_LIB EGL "EGL 1.4 Library" )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_GRAPHICS_EGL )
    target_link_libraries( ${THIS_TARGET} INTERFACE ${EGL_LIB} )
    message( STATUS "[graphics] : EGL found" )
  endif()

  if( OpenGL_GLX_FOUND )
    set( MOTOR_GRAPHICS_GLX ON )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_GRAPHICS_GLX )
    if( TARGET OpenGL::OpenGL )
      target_link_libraries( ${THIS_TARGET} INTERFACE OpenGL::OpenGL )
    else()
      target_link_libraries( ${THIS_TARGET} INTERFACE ${OPENGL_LIBRARIES} )
    endif() 
    message( STATUS "[graphics] : GLX found" )
  endif()

  if( WIN32 )
    set( MOTOR_GRAPHICS_WGL ON )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_GRAPHICS_WGL )
    target_link_libraries( ${THIS_TARGET} INTERFACE OpenGL::GL )
    message( STATUS "[graphics] : WGL found" )
  endif()

endif()

#####################################################################
#
# Test OpenGLES 
#
find_library( OPENGLES3_LIBRARY GLESv2 "OpenGL ES v3.0 library")
if( OPENGLES3_LIBRARY )
  set( MOTOR_GRAPHICS_OPENGLES ON )
  target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_GRAPHICS_OPENGLES )
  target_link_libraries( ${THIS_TARGET} INTERFACE ${OPENGLES3_LIBRARY} )
  message( STATUS "[graphics] : OpenGLES v3.0 libs found" )
endif()
unset( OPENGLES3_LIBRARY CACHE )

#####################################################################
#
# Test Directx
#
if( MOTOR_TARGET_OS_WIN )
  find_package( Direct3d )
  if( TARGET d3d11 )
    set( MOTOR_GRAPHICS_DIRECT3D ON )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_GRAPHICS_DIRECT3D )
    target_link_libraries( ${THIS_TARGET} INTERFACE d3d11 )
    message( STATUS "[graphics] : Direc3d 11 found" )
  endif()
endif()
#####################################################################
#
# Test Vulkan
#

#####################################################################
##
set( MOTOR_TARGET_GRAPHICS_CONFIGURED TRUE )

#install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${THIS_TARGET} ${PROJECT_NAME}-targets )
