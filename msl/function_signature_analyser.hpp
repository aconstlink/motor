

#pragma once

#include "typedefs.h"
#include "enums.hpp"


#include <motor/ntd/vector.hpp>

namespace motor
{
    namespace msl
    {
        // @precondition the line passed in must 
        // be a complete statement
        class function_signature_analyser
        {
            motor_this_typedefs( function_signature_analyser ) ;

        private:

            motor::string_t _line ;

            struct funk
            {
                motor::msl::type_t return_type ;
                motor::string_t name ;
                motor::vector< motor::msl::type_t > args ;
            };
            motor_typedef( funk ) ;

            motor::vector< funk_t > _funks ;

        public:

            function_signature_analyser( motor::string_cref_t line ) noexcept : _line( line ) {}


        };
        motor_typedef( function_signature_analyser ) ;
    }
}