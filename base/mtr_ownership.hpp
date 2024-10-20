
#pragma once

#include <cassert>
#include <utility>

namespace motor
{
    namespace core
    {
        template< typename T >
        struct mtr_borrow
        {
            using mtr_t = T * ;
        };

        // This class is just tagging the purpose
        // of a pointer passed. There is no ref counting.
        //
        // This type is used to signal the user that a pointer
        // will be shared if passed further. The receiver
        // is responsible to do the ref counting.
        template< typename T >
        class mtr_safe
        {

        public:

            using this_t = mtr_safe<T> ;
            using this_cref_t = this_t const & ;
            using this_rref_t = this_t && ;
            using this_ref_t = this_t & ;

            using type_t = T ;
            using type_ref_t = type_t & ;
            using type_cref_t = type_t const & ;
            using type_mtr_t = type_t * ;
            using mtr_t = type_t * ;
            using type_mtr_rref_t = type_t * && ;

            type_mtr_t _ptr = nullptr ;

        public:

            mtr_safe( void ) noexcept {} ;
            mtr_safe( this_cref_t rhv ) noexcept : _ptr( rhv._ptr ) {}
            mtr_safe( this_rref_t rhv ) noexcept : _ptr( rhv._ptr ) { rhv._ptr = nullptr ; }

            mtr_safe( std::nullptr_t ) noexcept {}
            explicit mtr_safe( type_mtr_t ptr ) noexcept : _ptr( ptr ) {}
                        
            ~mtr_safe( void ) noexcept{}

            template< typename O >
            mtr_safe( mtr_safe< O > && rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }

            operator type_mtr_t() noexcept
            {
                return _ptr ;
            }

            type_mtr_t operator -> ( void ) const noexcept
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

            bool operator != ( type_mtr_t ptr ) const noexcept
            {
                return _ptr != ptr ;
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
                return tmp ;
            }

            // get the managed pointer
            type_mtr_t mtr( void ) const noexcept
            {
                return _ptr ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept 
            {
                _ptr = rhv._ptr ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept 
            {
                _ptr = rhv._ptr ; rhv._ptr = nullptr ;
                return *this ;
            }

            this_ref_t operator = ( type_mtr_t ptr ) noexcept 
            {
                _ptr = ptr ;
                return *this ;
            }

            this_ref_t operator = ( std::nullptr_t ptr ) noexcept 
            {
                return *this ;
            }

        private:
            
            //mtr_safe( type_mtr_t ptr ) noexcept : _ptr ( ptr ){}
        };
    }
    
    // @deprecated @see move
    template< typename T >
    static core::mtr_safe< T > unique( T * & ptr ) noexcept
    {
        T * tmp = ptr ; ptr = nullptr ;
        return core::mtr_safe<T>::make( tmp ) ;
    }

    // @deprecated @see move
    template< typename T >
    static core::mtr_safe< T > unique( T * && ptr ) noexcept
    {
        return core::mtr_safe<T>::make( ptr ) ;
    }

    template< typename T >
    static core::mtr_safe< T > move( T * & ptr ) noexcept
    {
        T * tmp = ptr ; ptr = nullptr ;
        return core::mtr_safe<T>::make( tmp ) ;
    }

    template< typename T >
    static core::mtr_safe< T > move( T * && ptr ) noexcept
    {        
        return core::mtr_safe<T>::make( ptr ) ;
    }

    template< typename T >
    static core::mtr_safe< T > move( core::mtr_safe< T > & ptr ) noexcept
    {        
        return std::move( ptr ) ;
    }

    template< typename T >
    static core::mtr_safe< T > share_unsafe( T * ptr ) noexcept
    {
        return motor::core::mtr_safe<T>::make( ptr ) ;
    }
}

