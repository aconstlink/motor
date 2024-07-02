
#pragma once

#include "api.h"
#include "typedefs.h"

namespace motor
{
    namespace wire
    {
        class ivariable
        {
            motor_this_typedefs( ivariable ) ;

        public: 

            virtual ~ivariable( void_t ) noexcept {}

            virtual bool_t are_compatible( ivariable * ) const noexcept = 0 ;
            virtual bool_t assign_from( ivariable * ) noexcept = 0 ;
            virtual void_t assign_from_unsafe( ivariable * ) noexcept = 0 ;
        };
        motor_typedef( ivariable ) ;

        template< typename T >
        class variable : public ivariable
        {
            motor_this_typedefs( variable< T > ) ;

        public:

            class observer
            {
            };

        private:

            T _value ;

        public:

            variable( void_t ) noexcept : _value(0) {}
            variable( T const & v ) noexcept : _value( v ) {}
            virtual ~variable( void_t ) noexcept {}

        public:

            virtual bool_t are_compatible( ivariable * other ) const noexcept
            {
                return dynamic_cast<this_ptr_t>( other ) != nullptr ;
            }

            virtual bool_t assign_from( ivariable * other ) noexcept
            {
                if( auto * v = dynamic_cast< this_ptr_t >( other ); v != nullptr )
                {
                    this_t::assign_from( v ) ;
                    return true ;
                }
                return false ;
            }

            virtual void_t assign_from_unsafe( ivariable * other ) noexcept
            {
                this_t::assign_from( static_cast<this_ptr_t>( other ) ) ;
            }

        public:

            void_t assign_from( this_ptr_t other ) noexcept
            {
                _value = other->value() ;
            }

            T value( void_t ) const noexcept { return _value ; }
        };
    }
}