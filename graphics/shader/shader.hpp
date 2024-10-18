
#pragma once

#include <motor/std/string>
#include <motor/std/vector>

#include <functional>

namespace motor
{
    namespace graphics
    {
        class shader
        {
            motor_this_typedefs( shader ) ;

        private:

            motor::string_t _code ;

            //size_t _hash = 0 ;

        public:

            shader( void_t ) noexcept {}

            shader( motor::string_in_t code ) noexcept : _code( code ) 
            {
                //_hash = std::hash<motor::string_t>{}( code ) ;
            }

            shader( this_cref_t rhv ) noexcept 
            {
                *this = rhv ;
            }

            shader( this_rref_t rhv ) noexcept 
            {
                *this = std::move( rhv ) ;
            }

            virtual ~shader( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept 
            {
                _code = rhv._code ;
                //_hash = rhv._hash ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept 
            {
                _code = std::move( rhv._code ) ;
                //_hash = rhv._hash ;
                //rhv._hash = 0 ;                
                return *this ;
            }

            motor::string_t code( void_t ) const noexcept { return _code ; }
        };
        motor_typedef( shader ) ;
    }
} 