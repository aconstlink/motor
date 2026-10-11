# SPDX-FileCopyrightText: 2023-2026 Alex Link
# SPDX-License-Identifier: MIT


macro( motor_emacs_default_directory motor_source_dir )

configure_file( 
  ${motor_source_dir}/cmake/configure/emacs.build.make.in.el 
  ${PROJECT_BINARY_DIR}/emacs.build.make.el )


endmacro()
