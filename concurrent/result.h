#pragma once

namespace motor
{
    namespace concurrent
    {
        enum result
        {
            ok,
            failed,
            invalid_argument,
            mt_processing
        };
    }
}