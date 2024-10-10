

#pragma once

#include "typedefs.h"

namespace motor
{
    namespace meta
    {
        using namespace motor::core::types ;

        template< typename T >
        class nameof
        {
        public:

            static char * const type_name( void_t ) noexcept ;
        } ;

        template<>
        class nameof< int_t >
        {
        public:

            static char const * const  type_name( void_t ) noexcept { return "int" ; }
        } ;

        template<>
        class nameof< float_t >
        {
        public:

            static char const * const  type_name( void_t ) noexcept { return "float" ; }
        } ;

        template<>
        class nameof< double_t >
        {
        public:

            static char * const  type_name( void_t ) noexcept { return "double" ; }
        } ;


        template<>
        class nameof< bool_t >
        {
        public:

            static char * const type_name( void_t ) noexcept { return "bool" ; }
        } ;

        
    }
}