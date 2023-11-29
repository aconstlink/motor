#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/std/string>

namespace motor
{
    namespace msl
    {
        class node
        {
        };
        motor_typedef( node ) ;

        class group
        {
        };
        motor_typedef( group ) ;

        class leaf
        {
        };
        motor_typedef( leaf ) ;

        class statement : public group
        {
            motor::string_t _statement ;
        };
        motor_typedef( statement ) ;

        class variable : public leaf
        {
            // type
            // name
        };
        motor_typedef( variable ) ;
        
        class constant : public leaf
        {
            // type
            // name
            // value
        };
        motor_typedef( variable ) ;

        class function : public group
        {
            // buildin, operator or custom funk
            // signatue
            // arguments as children
        };
        motor_typedef( function ) ;

        
    }
}