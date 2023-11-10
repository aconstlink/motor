#pragma once

#include "macros/typedef.h"

#include <cstddef>

// including for typedefs of float_t/double_t
#include <cmath>
#include <cstdint>

namespace motor
{
    namespace core
    {
        namespace types
        {
            motor_typedefs(unsigned char, byte);
            motor_typedefs(char, char);
            motor_typedefs(unsigned char, uchar);

            motor_typedefs(short, short);
            motor_typedefs(unsigned short, ushort);
            motor_typedefs(int, int);
            motor_typedefs(unsigned int, uint);
            motor_typedefs(bool, bool);

            using ::float_t;
            using ::double_t;
            motor_typedef_extend(float);
            motor_typedef_extend(double);


            typedef unsigned long long ulonglong_t;

            typedef void void_t;
            typedef void_t* void_ptr_t;
            typedef void_t const* void_cptr_t;

            using ::int8_t;
            motor_typedef_extend(int8);

            using ::int16_t;
            motor_typedef_extend(int16);

            using ::int32_t;
            motor_typedef_extend(int32);

            using ::int64_t;
            motor_typedef_extend(int64);

            using ::uint8_t;
            motor_typedef_extend(uint8);

            using ::uint16_t;
            motor_typedef_extend(uint16);

            using ::uint32_t;
            motor_typedef_extend(uint32);

            using ::uint64_t;
            motor_typedef_extend(uint64);
        }
        
    }    
} 


