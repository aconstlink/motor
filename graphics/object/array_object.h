#pragma once

#include "../object.hpp"

#include "../buffer/data_buffer.hpp"
#include <motor/std/vector>

namespace motor
{
    namespace graphics
    {
        class array_object : public object
        {
            motor_this_typedefs( array_object ) ;
           
        private:

            motor::string_t _name ;
            motor::graphics::data_buffer_t _db ;

        public:

            array_object( void_t ) {}

            array_object( motor::string_cref_t name ) : _name(name)
            {}

            array_object( motor::string_cref_t name, motor::graphics::data_buffer_cref_t db )
            {
                _name = name ;
                _db = db ;
            }

            array_object( motor::string_cref_t name, motor::graphics::data_buffer_rref_t db )
            {
                _name = name ;
                _db = std::move( db ) ;
            }

            array_object( this_cref_t rhv ) : object( rhv ) 
            {
                _db = rhv._db ;
                _name = rhv._name ;
            }

            array_object( this_rref_t rhv ) : object( std::move( rhv ) )
            {
                _db = std::move( rhv._db ) ;
                _name = std::move( rhv._name ) ;
            }

            ~array_object( void_t ) 
            {}

            this_ref_t operator = ( this_cref_t rhv )
            {
                object::operator=( rhv ) ;

                _db = rhv._db ;
                _name = rhv._name ;

                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv )
            {
                object::operator=( std::move( rhv ) ) ;

                _db = std::move( rhv._db ) ;
                _name = std::move( rhv._name ) ;

                return *this ;
            }

        public:

            motor::graphics::data_buffer_ref_t data_buffer( void_t ) noexcept
            {
                return _db ;
            }

            motor::graphics::data_buffer_cref_t data_buffer( void_t ) const noexcept
            {
                return _db ;
            }

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

        };
        motor_typedef( array_object ) ;
    }
}