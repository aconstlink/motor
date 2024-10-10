
#pragma once

#include "api.h"
#include "typedefs.h"
#include "nameof.hpp"

#include <motor/std/string>
#include <motor/std/vector>

namespace motor
{
    namespace meta
    {
        // a complex variable can only have other 
        // variables but no value
        class ivariable
        {
            motor_this_typedefs( ivariable ) ;


        private:

            motor::string_t _name ;

        public:

        };


        template< typename T >
        class variable : public ivariable
        {
            motor_this_typedefs( variable< T > ) ;

        private:

            T _data ;

        public:

            T const & get( void_t ) const noexcept 
            {
                return _data ;
            }

        };
        
        class any
        {

            motor_this_typedefs( any ) ;

        private:

            char const * const _name ;

        public:
            
            any( char const * const name ) noexcept :
                _name( name ) {}

            char const * const name( void_t ) const noexcept
            {
                return _name ;
            }


            virtual char const * const type_name( void_t ) const noexcept = 0 ;

            struct for_each_info
            {
                size_t level = 0 ;
                motor::string_t full_name ;
            };
            motor_typedef( for_each_info ) ;

            using for_each_funk_t = std::function< void_t ( this_cref_t, for_each_info_in_t ) > ;

            void_t start( motor::meta::any::for_each_funk_t f ) const noexcept 
            {
                for_each_info_t info ;
                info.full_name = this_t::name() ;
                info.level = 0 ;

                f( *this, info ) ;

                ++info.level ;
                this->for_each_member( f, info ) ;
            }

        private: 

            virtual void_t for_each_member( motor::meta::any::for_each_funk_t, motor::meta::any::for_each_info_in_t ) const noexcept = 0 ;

            //using gen_name_funk_t = std::function< motor::string_t ( motor::string_rref_t ) > ;
            //virtual motor::string_t gen_name( gen_name_funk_t ) const noexcept = 0 ;
        };
        motor_typedef( any ) ;

        template< typename T >
        class data : public any
        {
            motor_this_typedefs( data< T > ) ;

            using base_t = any ;

        private:

            

        public:

            using type_t = T ;

            data( char const * const name ) noexcept : base_t( name ) {}
            data( motor::meta::any_in_t parent, motor::sstring_in_t name ) noexcept : base_t( parent, name ) {}

            virtual char const * const type_name( void_t ) const noexcept
            {
                return this_t::type_name_s() ;
            }

            static char const * const type_name_s( void_t ) noexcept 
            {
                return motor::meta::nameof< T >::type_name() ;
            }

            static bool_t compare( motor::meta::any_in_t rhv ) noexcept
            {
                return dynamic_cast< this_cptr_t > ( &rhv ) != nullptr ;
            }

            static std::pair< bool_t, this_t > do_cast( motor::meta::any_in_t rhv ) noexcept
            {
                bool_t const res = this_t::compare( rhv ) ;
                return std::pair< bool_t, this_t >( res, res ? dynamic_cast< this_cref_t > ( rhv ) : this_t( "" ) ) ;
            }

        private: // virtual 

            virtual void_t for_each_member( motor::meta::any::for_each_funk_t, motor::meta::any::for_each_info_in_t ) const noexcept 
            {}
        };

        using boolm_t = motor::meta::data< bool_t > ;
        using intm_t = motor::meta::data< int_t > ;
        using uintm_t = motor::meta::data< uint_t > ;
        using shortm_t = motor::meta::data< short_t > ;
        using floatm_t = motor::meta::data< float_t > ;
        using doublem_t = motor::meta::data< double_t > ;
    }
}