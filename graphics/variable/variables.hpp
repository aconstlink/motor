
#pragma once

#include "../types.h"
#include "../typedefs.h"

#include <functional>

namespace motor
{
    namespace graphics
    {
        class ivariable
        {
        public:

            virtual ~ivariable( void_t ) noexcept {}

            virtual void_cptr_t data_ptr( void_t ) const noexcept = 0 ;
        };
        motor_typedef( ivariable ) ;

        template< class T >
        class data_variable : public ivariable
        {
            motor_this_typedefs( data_variable< T > ) ;
            motor_typedefs( T, value ) ;

        private:

            value_t _value ;

        public:

            data_variable( void_t ) noexcept
            {}

            data_variable( value_cref_t v ) noexcept : _value(v)
            {}

            data_variable( this_cref_t rhv ) noexcept 
            {
                _value = rhv._value ;
            }

            data_variable( this_rref_t rhv ) noexcept
            {
                _value = std::move( rhv._value ) ;
            }

            virtual ~data_variable( void_t ) noexcept {}

        public:

            void_t set( value_cref_t v ) noexcept { _value = v ; }
            void_t set( value_rref_t v ) noexcept { _value = v ; }
            value_cref_t get( void_t ) const noexcept { return _value ; }

            virtual void_cptr_t data_ptr( void_t ) const noexcept override
            {
                return reinterpret_cast< void_cptr_t >( &_value ) ;
            }
        };

        template<>
        class data_variable<motor::string_t> : public ivariable
        {
            motor_this_typedefs( data_variable< motor::string_t > ) ;
            motor_typedefs( motor::string_t, value ) ;

        private:

            motor::string_t _value ;
            size_t _hash = 0 ;

        public:

            data_variable( void_t ) noexcept
            {}

            data_variable( this_cref_t rhv ) noexcept
            {
                _value = rhv._value ;
                _hash = rhv._hash ;
            }

            data_variable( this_rref_t rhv ) noexcept
            {
                _value = std::move( rhv._value ) ;
                _hash = rhv._hash ;
            }

            virtual ~data_variable( void_t ) noexcept {}

        public:

            void_t set( value_cref_t v ) noexcept 
            { 
                _value = v ; 
                _hash = std::hash<motor::string_t>{}(v) ; 
            }

            void_t set( value_rref_t v ) noexcept 
            { 
                _value = std::move( v ) ; 
                _hash = std::hash<motor::string_t>{}(_value) ; 
            }

            value_cref_t get( void_t ) const noexcept { return _value ; }
            size_t hash( void_t ) const noexcept { return _hash ; }

            virtual void_cptr_t data_ptr( void_t ) const noexcept override
            {
                return nullptr ;
            }
        };
    }
}