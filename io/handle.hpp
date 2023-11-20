#pragma once

#include "api.h"
#include "typedefs.h"

#include "protos.h"

namespace motor
{
    namespace io
    {
        template< class I, class S >
        class handle
        {
            using __this_t = handle< I, S > ;
            motor_this_typedefs( __this_t ) ;
            motor_typedefs( I, item ) ;
            motor_typedefs( S, system ) ;

            friend system_t ;

        private:

            item_mtr_t _dptr = nullptr ;
            system_ptr_t _ios = nullptr ;

            handle( this_t::item_mtr_t ptr, this_t::system_ptr_t ios ) noexcept : 
                _dptr( motor::memory::copy_ptr(ptr) ), _ios(ios) {}

        public:

            handle( void_t ) noexcept {}
            handle( this_cref_t ) = delete ;
            handle( this_rref_t rhv ) noexcept : _dptr( motor::move( rhv._dptr ) ), _ios( rhv._ios ) {}
            ~handle( void_t ) noexcept
            {
                motor::memory::release_ptr( _dptr ) ;
            }

        public: // operator =

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _dptr = motor::move( rhv._dptr ) ;
                _ios = rhv._ios ;
                return *this ;
            }

        public:

            template< typename funk_t >
            motor::io::result wait_for_operation( funk_t funk ) noexcept 
            {
                if( _ios == nullptr ) return motor::io::result::invalid_handle ;
                return _ios->wait_for_operation( this_t( std::move( *this ) ), funk ) ;
            }
            
            
            bool_t can_wait( void_t ) const noexcept 
            {
                return _data_ptr != nullptr ;
            }
        };
    }
}