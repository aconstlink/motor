

#pragma once

#include "input_slot.h"
#include "output_slot.h"
#include "other_slot.hpp"

#include <motor/std/hash_map>
#include <motor/std/string>

#include <initializer_list>

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

            bool_t _disconnect_on_clear = true ;
            motor::hash_map< motor::string_t, t_ptr_t > _ts ;

        public:

            sheet( void_t ) noexcept {}
            sheet( this_rref_t rhv ) noexcept : _ts( std::move( rhv._ts ) ),
                _disconnect_on_clear( rhv._disconnect_on_clear ) {}
            sheet( this_cref_t ) noexcept = delete ;
            sheet( std::initializer_list< std::pair< motor::string_t, motor::core::mtr_safe<T> > > && lst, 
                bool_t const disconnect_on_clear = true ) noexcept : _disconnect_on_clear( disconnect_on_clear )
            {
                for( auto  elem : lst )
                {
                    this_t::add( elem.first, motor::move( elem.second ) ) ;
                }
            }

            ~sheet( void_t ) noexcept 
            {
                this_t::clear() ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _ts = std::move( rhv._ts ) ;
                _disconnect_on_clear = rhv._disconnect_on_clear ;
                return *this ;
            }

        public:

            void_t exchange( void_t ) noexcept
            {
                for( auto & item : _ts )
                {
                    item.second->exchange() ;
                }
            }

            void_t clear( void_t ) noexcept
            {
                for ( auto item : _ts )
                {
                    if( _disconnect_on_clear )
                        item.second->disconnect() ;

                    motor::memory::release_ptr( item.second ) ;
                }
                _ts.clear() ;
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

            motor::core::mtr_safe< T > get_or_add( motor::string_in_t name, motor::core::mtr_safe< T > && other ) noexcept
            {
                auto iter = _ts.find( name ) ;
                if ( iter != _ts.end() )
                {
                    motor::release( other ) ;
                    return motor::share( iter->second ) ;
                }

                _ts[ name ] = motor::share( other ) ;

                return motor::move( other ) ;
            }

            T * borrow_or_add( motor::string_in_t name, motor::core::mtr_safe< T > && other ) noexcept
            {
                auto iter = _ts.find( name ) ;
                if ( iter != _ts.end() )
                {
                    motor::release( other ) ;
                    return iter->second ;
                }

                auto * ret = other.mtr() ;
                _ts[ name ] = motor::move( other ) ;

                return ret ;
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

            bool_t connect( motor::string_in_t name, typename motor::wire::other_slot< T >::type_safe_mtr_t slot ) noexcept 
            {
                auto * this_slot = this_t::borrow( name ) ;
                if( this_slot == nullptr ) 
                {
                    motor::release( motor::move( slot ) ) ;
                    return false ;
                }
                return this_slot->connect( motor::move( slot ) ) ;
            }

            using for_each_slot_funk_t = std::function< void_t ( motor::string_in_t, t_ptr_t ) > ;
            void_t for_each_slot( for_each_slot_funk_t f ) noexcept
            {
                for( auto & i : _ts )
                {
                    f( i.first, i.second ) ;
                }
            }
        };
        motor_typedefs( sheet< motor::wire::ioutput_slot >, outputs ) ;
        motor_typedefs( sheet< motor::wire::iinput_slot >, inputs ) ;
    }
}