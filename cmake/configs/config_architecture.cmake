

#
# determine/option the host and target architecture
#
# the values here can be initialized using a toolchain file

message( STATUS ">>> [natus config architecture] >>>" )
set( NATUS_ARCH_CONFIGURED FALSE )

set( NATUS_HOST_ARCH "Unknown" )
set( NATUS_HOST_ARCH_X86 OFF )
set( NATUS_HOST_ARCH_BIT_32 OFF )
set( NATUS_HOST_ARCH_BIT_64 OFF )


set( NATUS_TARGET_ARCH "Unknown" )
set( NATUS_TARGET_ARCH_X86 ON ) # default
set( NATUS_TARGET_ARCH_ARM OFF )
set( NATUS_TARGET_ARCH_CBEA OFF )
set( NATUS_TARGET_ARCH_PPC OFF )
set( NATUS_TARGET_ARCH_MIPS OFF )
set( NATUS_TARGET_ARCH_BIT_32 OFF )
set( NATUS_TARGET_ARCH_BIT_64 ON ) #default

include( TestBigEndian )
TEST_BIG_ENDIAN( BIGENDIAN ) 
if( ${BIGENDIAN} )
  message( "[Endianness] : Big endian" )
else()
  message("[Endianness] : Little endian") 
endif()
set( NATUS_TARGET_ARCH_BIG_ENDIAN ${BIGENDIAN} )
unset( BIGENDIAN )

set( NATUS_ARCH_CONFIGURED TRUE )

message( STATUS "<<< [natus config architecture] <<<" )
