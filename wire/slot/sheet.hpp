

#pragma once

#include "input_slot.h"
#include "output_slot.h"

#include <motor/std/hash_map>
#include <motor/std/string>

namespace motor
{
    namespace wire
    {
        // this class is used for storing 
        // signals/slots by name
        // usage: 
        // sheet< motor::wire::isignal >
        // sheet< motor::wire::islot >
        template< typename T >
        class sheet
        {
            using t_ptr_t = T * ;
            motor_this_typedefs( sheet< T > ) ;

        private:

            motor::hash_map< motor::string_t, t_ptr_t > _ts ;

        public:

            sheet( void_t ) noexcept {}
            sheet( this_rref_t rhv ) noexcept : _ts( std::move( rhv._ts ) ) {}
            sheet( this_cref_t ) = delete ;
            ~sheet( void_t ) noexcept 
            {
                for ( auto item : _ts )
                {
                    item.second->disconnect() ;
                    motor::memory::release_ptr( item.second ) ;
                }
            }

        public:

            void_t exchange( void_t ) noexcept
            {
                for( auto & item : _ts )
                {
                    item.second->exchange() ;
                }
            }

        public:

            // add signal
            bool_t add( motor::string_in_t name, motor::core::mtr_safe< T > && other ) noexcept 
            {
                auto iter = _ts.find( name ) ;
                if ( iter != _ts.end() )
                {
                    motor::release( other ) ;
                    return false ;
                }

                _ts[ name ] = motor::move( other ) ;

                return true ;
            }

            // remove signal
            bool_t remove( motor::string_in_t name ) noexcept 
            {
                auto iter = _ts.find( name ) ;
                if ( iter == _ts.end() ) return false ;

                iter->second->disconnect() ;

                motor::memory::release_ptr( iter->second ) ;
                _ts.erase( iter ) ;

                return true ;
            }

            // borrow signal i.e. no ref counting
            t_ptr_t borrow( motor::string_in_t name ) const noexcept 
            {
                auto iter = _ts.find( name ) ;
                if ( iter == _ts.end() ) return nullptr ;

                return iter->second ;
            }

            template< typename T2 >
            typename std::remove_pointer<T2>::type * borrow_by_cast( motor::string_in_t name ) const noexcept
            {
                auto iter = _ts.find( name ) ;
                if ( iter == _ts.end() ) return nullptr ;

                return dynamic_cast< typename std::remove_pointer<T2>::type * >( iter->second )  ;
            }

            // get a shared reference to the signal.
            motor::core::mtr_safe< T > get( motor::string_in_t name ) const noexcept 
            {
                auto iter = _ts.find( name ) ;
                if ( iter == _ts.end() ) return nullptr ;

                return motor::share( iter->second ) ;
            }
        };
        motor_typedefs( sheet< motor::wire::ioutput_slot >, outputs ) ;
        motor_typedefs( sheet< motor::wire::iinput_slot >, inputs ) ;
    }
}