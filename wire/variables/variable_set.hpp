
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../variable.hpp"

#include <motor/std/vector>

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API variable_set 
        {
            motor_this_typedefs( variable_set ) ;

        public:

            using pull_data_funk_t = std::function< void_t ( void_ptr_t, motor::wire::any_ptr_t ) > ;

        private:

            struct per_var_data
            {
                pull_data_funk_t funk ;
                motor::wire::any_mtr_t var ;
            };
            motor::vector< this_t::per_var_data > _variables ;

        public:

            variable_set( void_t ) noexcept {}

            variable_set( this_rref_t rhv ) noexcept : 
                _variables( std::move( rhv._variables ) ){}

            variable_set( this_cref_t ) = delete ;
            
            ~variable_set( void_t ) noexcept 
            {
                for( auto & d : _variables ) motor::release( motor::move( d.var ) ) ;
            }

        public:

            using for_each_variable_funk_t = std::function< void_t ( motor::wire::any_mtr_t ) > ;

            void_t for_each( for_each_variable_funk_t funk ) noexcept 
            {
                for( auto & d : _variables )
                {
                    funk( d.var ) ;
                }
            }
            
            // call the pull function for each variable
            template< typename T >
            void_t pull_from_each( T * owner ) noexcept
            {
                for( auto & d : _variables )
                {
                    d.funk( reinterpret_cast< void_ptr_t >( owner ), d.var ) ;
                }
            }

        public:

            // add a variable which should interface with some other variable
            // the pull function allows to "pull" the data from the any variable
            // and update the other variable which is not interfaceable.
            void_t add_variable( pull_data_funk_t funk, motor::wire::any_mtr_safe_t v ) noexcept 
            {
                for( auto & d : _variables ) 
                    if( d.var == v ) 
                    {
                        motor::release( v ) ;
                        return ;
                    }

                _variables.emplace_back( this_t::per_var_data { funk, motor::move( v ) } ) ;
            }
        };
    }
}