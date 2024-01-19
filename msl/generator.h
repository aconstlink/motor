
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

        private:

            motor::msl::variable_mappings_t generate_mappings( void_t ) const noexcept ;

        public: 

            //@obsolete
            motor::msl::generated_code_t generate_old( void_t ) noexcept ;

            motor::msl::variable_mappings_t generate( motor::msl::generated_code_inout_t ) noexcept ;

            // e.g G = motor::msl::glsl::generator_t
            template< typename G >
            motor::msl::generated_code_t generate( void_t ) noexcept
            {
                motor::msl::generated_code_t genc ;
                motor::msl::variable_mappings_t mappings = this_t::generate( genc ) ;

                motor::msl::generated_code_t::shaders_t shaders ;
                {
                    auto const shds = G().generate( _genable, mappings ) ;
                    for( auto shd_ : shds )
                    {
                        shaders.emplace_back( shd_ ) ;
                    }
                    genc.shaders = std::move( shaders ) ;
                }
                return genc ;
            }
        };
        motor_typedef( generator ) ;
    }
}