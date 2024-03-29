
#pragma once

namespace motor
{
    namespace tool
    {
        enum class result
        {
            ok, 
            failed,
            invalid,
            invalid_argument
        };


        enum class async_result
        {
            ok,
            failed,
            invalid,
            in_transit,
            user_edit
        };
    }
}