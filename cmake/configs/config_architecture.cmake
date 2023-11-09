

#
# determine/option the host and target architecture
#
# the values here can be initialized using a toolchain file

message( STATUS ">>> [motor config architecture] >>>" )
set( MOTOR_ARCH_CONFIGURED FALSE )

set( MOTOR_HOST_ARCH "Unknown" )
set( MOTOR_HOST_ARCH_X86 OFF )
set( MOTOR_HOST_ARCH_BIT_32 OFF )
set( MOTOR_HOST_ARCH_BIT_64 OFF )


set( MOTOR_TARGET_ARCH "Unknown" )
set( MOTOR_TARGET_ARCH_X86 ON ) # default
set( MOTOR_TARGET_ARCH_ARM OFF )
set( MOTOR_TARGET_ARCH_CBEA OFF )
set( MOTOR_TARGET_ARCH_PPC OFF )
set( MOTOR_TARGET_ARCH_MIPS OFF )
set( MOTOR_TARGET_ARCH_BIT_32 OFF )
set( MOTOR_TARGET_ARCH_BIT_64 ON ) #default

include( TestBigEndian )
TEST_BIG_ENDIAN( BIGENDIAN ) 
if( ${BIGENDIAN} )
  message( "[Endianness] : Big endian" )
else()
  message("[Endianness] : Little endian") 
endif()
set( MOTOR_TARGET_ARCH_BIG_ENDIAN ${BIGENDIAN} )
unset( BIGENDIAN )

set( MOTOR_ARCH_CONFIGURED TRUE )

message( STATUS "<<< [motor config architecture] <<<" )
