
#pragma once

#include "iproperty.hpp"

namespace motor
{
    namespace property
    {
        template< typename T >
        class generic_property : public iproperty
        {
            motor_typedefs( T, value ) ;
            motor_this_typedefs( generic_property< T > ) ;

        private:

            value_t _data ;

        public:

            generic_property( void_t ) noexcept{}
            virtual ~generic_property( void_t ) noexcept {}

        public:

            void_t set( value_cref_t data ) noexcept
            {
                _data = data ;
            }

            value_cref_t get( void_t ) const noexcept
            {
                return _data ;
            }
        };
        
    }
}