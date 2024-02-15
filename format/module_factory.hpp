#pragma once

#include "api.h"
#include "typedefs.h"

#include "imodule.h"

namespace motor
{
    namespace format
    {
        class MOTOR_FORMAT_API imodule_factory
        {
        public:

            virtual imodule_mtr_t borrow_module( motor::string_cref_t ) noexcept = 0 ;
        };
        motor_typedef( imodule_factory ) ;

        template< typename T >
        class module_factory : public imodule_factory
        {

        private:

            T _t ;

        public:

            virtual imodule_mtr_t borrow_module( motor::string_cref_t ) noexcept 
            {
                return &_t ;
            }
        };
    }
}