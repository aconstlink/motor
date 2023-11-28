
#pragma once

#include <cassert>

namespace motor
{
    namespace core
    {

        template< typename T >
        class mtr_moved
        {

        public:

            using this_t = mtr_moved<T> ;
            using this_cref_t = this_t const & ;
            using this_rref_t = this_t && ;

            using type_t = T ;
            using type_ref_t = type_t & ;
            using type_cref_t = type_t const & ;
            using type_mtr_t = type_t * ;
            using type_mtr_rref_t = type_t * && ;

            type_mtr_t _ptr = nullptr ;

        public:

            
            mtr_moved( this_cref_t ) = delete ;
            mtr_moved( this_rref_t rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }
                        
            ~mtr_moved( void ) noexcept{}

            template< typename O >
            mtr_moved( mtr_moved< O > && rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }

            explicit operator type_mtr_t() noexcept
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

            bool operator == ( type_mtr_t ptr ) const noexcept
            {
                return _ptr == ptr ;
            }

            static this_t make( type_mtr_t ptr ) noexcept
            {
                return this_t( ptr ) ;
            }

            static this_t make( void ) noexcept
            {
                return this_t( nullptr ) ;
            }

            // move out the mtr leaving this object
            // with a nullptr
            type_mtr_t move( void ) noexcept 
            {
                auto * tmp = _ptr ;
                _ptr = nullptr ;
                return _ptr ;
            }

            // get the managed pointer
            type_mtr_t mtr( void ) const noexcept
            {
                return _ptr ;
            }

        private:
            mtr_moved( void ) noexcept {} ;
            mtr_moved( type_mtr_t ptr ) noexcept : _ptr ( ptr ){}
        };
    }

    template< typename T >
    static core::mtr_moved< T > move( T * & ptr ) noexcept
    {
        T * tmp = ptr ; ptr = nullptr ;
        return core::mtr_moved<T>::make( tmp ) ;
    }

    template< typename T >
    static core::mtr_moved< T > move( T * && ptr ) noexcept
    {        
        return core::mtr_moved<T>::make( ptr ) ;
    }
}

