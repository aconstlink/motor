#pragma once

#include "api.h"
#include "typedefs.h"
#include <motor/std/string>



namespace motor
{
    namespace platform
    {
        using namespace motor::core::types ;

        class MOTOR_PLATFORM_API cpu_id
        {
            motor_this_typedefs( cpu_id ) ;
            uint32_t regs[4];

        public:

            explicit cpu_id( unsigned const i ) noexcept ;

            uint32_t const & eax() const noexcept { return regs[0] ; }
            uint32_t const & ebx() const noexcept { return regs[1] ; }
            uint32_t const & ecx() const noexcept { return regs[2] ; }
            uint32_t const & edx() const noexcept { return regs[3] ; }

            static motor::string_t vendor_string( void_t ) noexcept ;
            static motor::string_t brand_string( void_t ) noexcept ;
            static motor::string_t instruction_sets_string( void_t ) noexcept ;
        };
        motor_typedef( cpu_id ) ;
    }
}

