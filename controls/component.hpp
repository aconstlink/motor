
#pragma once

#include "api.h"
#include "typedefs.h"

namespace motor
{
    namespace controls
    {
        class icomponent
        {
        public: 
            virtual void_t update( void_t ) noexcept = 0 ;
        };
        motor_typedef( icomponent ) ;

        class input_component : public icomponent
        {
        public:

            virtual ~input_component( void_t ) {}
            virtual void_t update( void_t ) noexcept {}
        };
        motor_typedef( input_component ) ;

        class output_component : public icomponent
        {
        public:
            
            virtual ~output_component( void_t ) {}
            virtual void_t update( void_t ) noexcept {}
        };
        motor_typedef( output_component ) ;
    }
}