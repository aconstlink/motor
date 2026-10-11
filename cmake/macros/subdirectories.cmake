# SPDX-FileCopyrightText: 2023-2026 Alex Link
# SPDX-License-Identifier: MIT

macro( subdirectories result curdir )
  file( GLOB children RELATIVE ${curdir} ${curdir}/* )
  set( directories "")
  foreach( child ${children} )
    if( IS_DIRECTORY ${curdir}/${child} )
      list( APPEND directories ${child} )
    endif()
  endforeach()
  set( ${result} ${directories} )
endmacro()
