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

            virtual imodule_safe_t::mtr_t create_module( motor::string_cref_t ) noexcept = 0 ;
        };
        motor_typedef( imodule_factory ) ;

        template< typename T >
        class module_factory : public imodule_factory
        {
        public:

            virtual imodule_safe_t::mtr_t create_module( motor::string_cref_t ) noexcept 
            {
                return motor::shared< T >( T() ) ;
            }
        };
    }
}