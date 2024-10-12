

#pragma once

#include "input_slot.h"
#include "output_slot.h"

namespace motor
{
    namespace wire
    {
        template< typename T >
        class other_slot
        {
            using type_t = void_t ;
        };

        template<>
        struct other_slot< motor::wire::iinput_slot_t >
        {
            using type_t = motor::wire::ioutput_slot_t ; 
            using type_ptr_t = type_t * ;
            using type_safe_mtr_t = motor::core::mtr_safe< type_t > ;
        };

        template<>
        struct other_slot< motor::wire::ioutput_slot_t >
        {
            using type_t = motor::wire::iinput_slot_t ;
            using type_ptr_t = type_t * ;
            using type_safe_mtr_t = motor::core::mtr_safe< type_t > ;
        };
    }
}