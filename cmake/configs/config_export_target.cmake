# SPDX-FileCopyrightText: 2023-2026 Alex Link
# SPDX-License-Identifier: MIT



macro( MOTOR_EXPORT trg )

  #message( ${trg} )
  export( TARGETS ${trg} NAMESPACE motor:: APPEND FILE ${MOTOR_BINARY_DIR}/motor-targets.cmake )

endmacro()

macro( MOTOR_EXPORT_clear )

file( REMOVE ${MOTOR_BINARY_DIR}/motor-targets.cmake )

endmacro()
