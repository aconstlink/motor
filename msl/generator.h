
#pragma once

#include "api/glsl/generator.h"
#include "api/hlsl/generator.h"

#include <motor/std/vector>

namespace motor
{
    namespace msl
    {
        // generator using api specific generators
        // to generate the source code
        class MOTOR_MSL_API generator
        {
            motor_this_typedefs( generator ) ;

        private:

            motor::msl::generatable_t _genable ;

        public:

            generator( void_t ) noexcept {}
            generator( motor::msl::generatable_rref_t gen ) noexcept : _genable( std::move( gen ) ) {}
            generator( this_cref_t rhv ) noexcept : _genable( rhv._genable ) {}
            generator( this_rref_t rhv ) noexcept : _genable( std::move( rhv._genable ) ) {}
            ~generator( void_t ) noexcept {}

        public: 

            motor::msl::generated_code_t generate( void_t ) noexcept ;
        };
        motor_typedef( generator ) ;
    }
}