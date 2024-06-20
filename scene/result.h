#pragma once

namespace motor
{
    namespace scene
    {
        enum result
        {
            ok,
            failed,
            invalid_argument,

            repeat
        };

        static bool success( result r ) { return r == ok ; }
        static bool no_success( result r ) { return !success( r ) ; }
    }
}