message( STATUS ">>> [external : OpenAL-Soft] >>>" )

option( ALSOFT_UTILS "" OFF )
option( ALSOFT_INSTALL_UTILS "" OFF )
option( ALSOFT_INSTALL_CONFIG "" OFF )
option( ALSOFT_INSTALL_EXAMPLES "" OFF )
option( ALSOFT_INSTALL_HRTF_DEFS "" OFF )
option( ALSOFT_INSTALL_AMBDEC_PRESETS "" OFF )
option( CMAKE_DISABLE_FIND_PACKAGE_WindowsSDK "" ON )

add_subdirectory( "openal-soft" )

#
# set target properties 
#

if( TARGET alrecord )
  set_target_properties( alrecord PROPERTIES FOLDER "motor/externals/openal-soft" )


  if( MOTOR_TARGET_OS_WIN )

    if( MOTOR_COMPILER_MSC )
      set( unwanted_compiler_options "/wd4204" "/wd4018" "/wd4060")
      target_compile_options( alrecord PRIVATE ${unwanted_compiler_options} )
    endif()

  endif()
endif()

if( TARGET alcommon )
set_target_properties( alcommon PROPERTIES FOLDER "motor/externals/openal-soft" )
endif()

if( TARGET al-excommon )
set_target_properties( al-excommon PROPERTIES FOLDER "motor/externals/openal-soft" )
endif()

if( TARGET altonegen )
set_target_properties( altonegen PROPERTIES FOLDER "motor/externals/openal-soft" )
endif()

if( TARGET build_version )
set_target_properties( build_version PROPERTIES FOLDER "motor/externals/openal-soft" )
endif()

if( TARGET common )
set_target_properties( common PROPERTIES FOLDER "motor/externals/openal-soft" )
if( MOTOR_COMPILER_GNU )
    target_compile_options( OpenAL PRIVATE -Wno-sign-compare )
  endif()
endif()

if( TARGET ex-common )
set_target_properties( ex-common PROPERTIES FOLDER "motor/externals/openal-soft" )
endif()

if( TARGET OpenAL )
  set_target_properties( OpenAL PROPERTIES FOLDER "motor/externals/openal-soft" )

  
  if( MOTOR_COMPILER_GNU )
    target_compile_options( OpenAL PRIVATE -Wno-conversion -Wno-pedantic -Wno-switch -Wno-attributes )
  endif()
  if( MOTOR_TARGET_OS_WIN )

    if( MOTOR_COMPILER_MSC )
      set( unwanted_compiler_options "/wd4834" "/wd4018" "/wd4060" )
      target_compile_options( OpenAL PRIVATE ${unwanted_compiler_options} )
    endif()

  endif()

endif()

if( TARGET openal-info )
  set_target_properties( openal-info PROPERTIES FOLDER "motor/externals/openal-soft" )
  if( MOTOR_COMPILER_GNU )
    target_compile_options( openal-info PRIVATE -Wno-pedantic )
  endif()
endif()



#
# unset variables
#
unset( AUDIOIO_INCLUDE_DIR CACHE )
unset( AUDIOUNIT_INCLUDE_DIR CACHE  )
unset( COREAUDIO_FRAMEWORK CACHE )
unset( FORCE_STATIC_VCRT CACHE )
unset( GIT_EXECUTABLE CACHE )
unset( Qt5Widgets_DIR CACHE )
unset( SDL2MAIN_LIBRARY CACHE )
unset( WINMM_LIBRARY CACHE )
unset( ALSA_INCLUDE_DIR CACHE )
unset( ALSA_LIBRARY CACHE )
unset( DSOUND_INCLUDE_DIR CACHE )
unset( DSOUND_LIBRARY CACHE )
unset( JACK_INCLUDE_DIR CACHE )
unset( JACK_LIBRARY CACHE )
unset( MYSOFA_INCLUDE_DIR CACHE )
unset( MYSOFA_LIBRARY CACHE )
unset( MYSOFA_M_LIBRARY CACHE )
unset( OPENSL_ANDROID_INCLUDE_DIR CACHE )
unset( OPENSL_INCLUDE_DIR CACHE )
unset( OPENSL_LIBRARY CACHE )
unset( OSS_INCLUDE_DIR CACHE )
unset( OSS_LIBRARY CACHE )
unset( PORTAUDIO_INCLUDE_DIR CACHE )
unset( PORTAUDIO_LIBRARY CACHE )
unset( PULSEAUDIO_INCLUDE_DIR CACHE )
unset( PULSEAUDIO_LIBRARY CACHE )
unset( SDL2_CORE_LIBRARY CACHE )
unset( SDL2_INCLUDE_DIR CACHE )
unset( SNDFILE_INCLUDE_DIR CACHE )
unset( SNDFILE_LIBRARY CACHE )
unset( SOUNDIO_INCLUDE_DIR CACHE )
unset( SOUNDIO_LIBRARY CACHE )
unset( ZLIB_INCLUDE_DIR CACHE )
unset( ZLIB_LIBRARY_DEBUG CACHE )
unset( ZLIB_LIBRARY_RELEASE CACHE )

message( STATUS "<<< [external : OpenAL-Soft] <<<" )
