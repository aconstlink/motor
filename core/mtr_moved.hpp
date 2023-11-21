
#pragma once

#include <cassert>

namespace motor
{
    namespace core
    {

        template< typename T >
        class mtr_moved
        {
            using this_t = mtr_moved ;
            using this_cref_t = this_t const & ;
            using this_rref_t = this_t && ;

            using type_t = T ;
            using type_ref_t = type_t & ;
            using type_cref_t = type_t const & ;
            using type_mtr_t = type_t * ;
            using type_mtr_rref_t = type_t * && ;

            type_mtr_t _ptr = nullptr ;

        public:

            mtr_moved( void ) = delete ;
            mtr_moved( this_cref_t ) = delete ;
            mtr_moved( this_rref_t rhv ) noexcept : _ptr(rhv._ptr) {}
            mtr_moved( type_mtr_t ptr ) noexcept : _ptr ( ptr ){}
            ~mtr_moved( void ) noexcept{}

            operator type_mtr_t() noexcept
            {
                return _ptr ;
            }

            type_mtr_t operator -> ( void ) noexcept
            {
                return _ptr ;
            }

            type_ref_t operator *( void ) noexcept
            {
                return *_ptr ;
            }

            type_cref_t operator *( void ) const noexcept
            {
                return *_ptr ;
            }
        };
    }

    template< typename T >
    static core::mtr_moved< T > move( T * & ptr ) noexcept
    {
        T * tmp = ptr ; ptr = nullptr ;
        return core::mtr_moved<T>( tmp ) ;
    }

    template< typename T >
    static core::mtr_moved< T > move( T * && ptr ) noexcept
    {        
        return core::mtr_moved<T>( ptr ) ;
    }
}

