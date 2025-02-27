
#pragma once

#include "../types.h"
#include "../typedefs.h"

#include <functional>
#include <chrono>

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

        //***************************************************
        class texture_variable_data
        {
            motor_this_typedefs( texture_variable_data ) ;

        private:

            size_t _hash = 0 ;
            motor::string_t _name ;

        public: // ctor

            texture_variable_data( void_t ) noexcept {}
            texture_variable_data( char_cptr_t name ) noexcept : _name( name ) {}
            texture_variable_data( motor::string_cref_t name ) noexcept : _name( name ), 
                _hash( std::chrono::high_resolution_clock::now().time_since_epoch().count() ) {}
            texture_variable_data( motor::string_rref_t name ) noexcept : _name( std::move( name ) ),
                _hash( std::chrono::high_resolution_clock::now().time_since_epoch().count() ) {
            }
            texture_variable_data( this_cref_t rhv ) noexcept : _name( rhv._name ),
                _hash( std::chrono::high_resolution_clock::now().time_since_epoch().count() ) {
            }
            texture_variable_data( this_rref_t rhv ) noexcept : _name( std::move( rhv._name ) ),
                _hash( std::chrono::high_resolution_clock::now().time_since_epoch().count() ) {
            }
            ~texture_variable_data( void_t ) noexcept {}

        public: // operator =

            this_ref_t operator = ( motor::string_cref_t name ) noexcept
            {
                _name = name ;
                _hash = std::chrono::high_resolution_clock::now().time_since_epoch().count() ;
                return *this ;
            }

            this_ref_t operator = ( motor::string_rref_t name ) noexcept
            {
                _name = std::move( name ) ;
                _hash = std::chrono::high_resolution_clock::now().time_since_epoch().count() ;
                return *this ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _name = rhv._name ;
                _hash = std::chrono::high_resolution_clock::now().time_since_epoch().count() ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;
                _hash = std::chrono::high_resolution_clock::now().time_since_epoch().count() ;
                return *this ;
            }

        public: // operator ==

            bool_t operator == ( motor::string_cref_t name ) const noexcept
            {
                return _name == name ;
            }

            bool_t operator == ( this_cref_t rhv ) const noexcept
            {
                return _name == rhv._name ;
            }

        public:

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

            size_t hash( void_t ) const noexcept
            {
                return _hash ;
            }

        };
        using texture_variable_t = data_variable< texture_variable_data > ;

        //***************************************************
        class array_variable_data
        {
            motor_this_typedefs( array_variable_data ) ;

        private:

            motor::string_t _name ;

        public: // ctor

            array_variable_data( void_t ) noexcept {}
            array_variable_data( char_cptr_t name ) noexcept : _name( name ) {}
            array_variable_data( motor::string_cref_t name ) noexcept : _name( name ) {}
            array_variable_data( motor::string_rref_t name ) noexcept : _name( std::move( name ) ) {}
            array_variable_data( this_cref_t rhv ) noexcept : _name( rhv._name ) {}
            array_variable_data( this_rref_t rhv ) noexcept : _name( std::move( rhv._name ) ) {}
            ~array_variable_data( void_t ) noexcept {}

        public: // operator =

            this_ref_t operator = ( motor::string_cref_t name ) noexcept
            {
                _name = name ;
                return *this ;
            }

            this_ref_t operator = ( motor::string_rref_t name ) noexcept
            {
                _name = std::move( name ) ;
                return *this ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _name = rhv._name ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;
                return *this ;
            }

        public: // operator ==

            bool_t operator == ( motor::string_cref_t name ) const noexcept
            {
                return _name == name ;
            }

            bool_t operator == ( this_cref_t rhv ) const noexcept
            {
                return _name == rhv._name ;
            }

        public:

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

        };
        using array_variable_t = data_variable< array_variable_data > ;

        //***************************************************
        class streamout_variable_data
        {
            motor_this_typedefs( streamout_variable_data ) ;

        private:

            motor::string_t _name ;

        public: // ctor

            streamout_variable_data( void_t ) noexcept {}
            streamout_variable_data( char_cptr_t name ) noexcept : _name( name ) {}
            streamout_variable_data( motor::string_cref_t name ) noexcept : _name( name ) {}
            streamout_variable_data( motor::string_rref_t name ) noexcept : _name( std::move( name ) ) {}
            streamout_variable_data( this_cref_t rhv ) noexcept : _name( rhv._name ) {}
            streamout_variable_data( this_rref_t rhv ) noexcept : _name( std::move( rhv._name ) ) {}
            ~streamout_variable_data( void_t ) noexcept {}

        public: // operator =

            this_ref_t operator = ( motor::string_cref_t name ) noexcept
            {
                _name = name ;
                return *this ;
            }

            this_ref_t operator = ( motor::string_rref_t name ) noexcept
            {
                _name = std::move( name ) ;
                return *this ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _name = rhv._name ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;
                return *this ;
            }

        public: // operator ==

            bool_t operator == ( motor::string_cref_t name ) const noexcept
            {
                return _name == name ;
            }

            bool_t operator == ( this_cref_t rhv ) const noexcept
            {
                return _name == rhv._name ;
            }

        public:

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }
        };
        using streamout_variable_t = data_variable< streamout_variable_data > ;

        template< typename T >
        std::pair< bool_t, motor::graphics::data_variable< T > * > cast_data_variable( motor::graphics::ivariable_ptr_t in_var ) noexcept
        {
            using ret_t = std::pair< bool_t, motor::graphics::data_variable< T > * > ;
            auto * ptr = dynamic_cast< motor::graphics::data_variable< T > * >( in_var ) ;
            return ptr != nullptr ? ret_t { true, ptr } : ret_t { false, nullptr }  ;
        }
        
    }
}