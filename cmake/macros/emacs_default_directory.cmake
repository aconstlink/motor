
macro( motor_emacs_default_directory motor_source_dir )

configure_file( 
  ${motor_source_dir}/cmake/configure/emacs.build.make.in.el 
  ${PROJECT_BINARY_DIR}/emacs.build.make.el )


endmacro()
