
#pragma once

#include <cassert>

namespace motor
{
    namespace core
    {
        // This class is just tagging the purpose
        // of a pointer passed. There is no ref counting.
        // 
        // A delayed pointer needs to be hold by the user
        // until some event happened and the user is 
        // allowed to release
        // The pointer receiver can or can not decide to 
        // copy the pointer until the receiver triggers the event.
        template< typename T >
        class mtr_delay
        {

        public:

            using this_t = mtr_delay<T> ;
            using this_cref_t = this_t const & ;
            using this_rref_t = this_t && ;
            using this_ref_t = this_t & ;

            using type_t = T ;
            using type_ref_t = type_t & ;
            using type_cref_t = type_t const & ;
            using type_mtr_t = type_t * ;
            using type_mtr_rref_t = type_t * && ;

            type_mtr_t _ptr = nullptr ;

        public:

            
            mtr_delay( this_cref_t rhv ) noexcept : _ptr( rhv._ptr ) {}
            mtr_delay( this_rref_t rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }
                        
            ~mtr_delay( void ) noexcept{}

            template< typename O >
            mtr_delay( mtr_delay< O > && rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }

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

            this_ref_t operator = ( type_mtr_t ptr ) noexcept 
            {
                _ptr = ptr ;
                return *this ;
            }

        private:
            mtr_delay( void ) noexcept {} ;
            mtr_delay( type_mtr_t ptr ) noexcept : _ptr ( ptr ){}
        };
    

        // This class is just tagging the purpose
        // of a pointer passed. There is no ref counting.
        //
        // This type is used to signal the user that a pointer
        // will be shared if passed further. The receiver
        // is responsible to do the ref counting.
        template< typename T >
        class mtr_shared
        {

        public:

            using this_t = mtr_shared<T> ;
            using this_cref_t = this_t const & ;
            using this_rref_t = this_t && ;
            using this_ref_t = this_t & ;

            using type_t = T ;
            using type_ref_t = type_t & ;
            using type_cref_t = type_t const & ;
            using type_mtr_t = type_t * ;
            using type_mtr_rref_t = type_t * && ;

            type_mtr_t _ptr = nullptr ;

        public:
            
            mtr_shared( this_cref_t rhv ) = delete ;

            mtr_shared( this_rref_t rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }

            mtr_shared( std::nullptr_t ) noexcept {}
            explicit mtr_shared( type_mtr_t ptr ) noexcept : _ptr( ptr ) {}
                        
            ~mtr_shared( void ) noexcept{}

            template< typename O >
            mtr_shared( mtr_shared< O > && rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }

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
                return _ptr ;
            }

            // get the managed pointer
            type_mtr_t mtr( void ) const noexcept
            {
                return _ptr ;
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
            mtr_shared( void ) noexcept {} ;
            //mtr_shared( type_mtr_t ptr ) noexcept : _ptr ( ptr ){}
        };

        // This class is just tagging the purpose
        // of a pointer passed. There is no ref counting.
        //
        // This type signals the user that the ownership of the
        // pointer is taken over. The receiver is resoponsible
        // for releasing the managed pointer.
        template< typename T >
        class mtr_unique
        {

        public:

            using this_t = mtr_unique<T> ;
            using this_cref_t = this_t const & ;
            using this_rref_t = this_t && ;
            using this_ref_t = this_t & ;

            using type_t = T ;
            using type_ref_t = type_t & ;
            using type_cref_t = type_t const & ;
            using type_mtr_t = type_t * ;
            using type_mtr_rref_t = type_t * && ;

            type_mtr_t _ptr = nullptr ;

        public:

            
            mtr_unique( this_cref_t ) = delete ;
            mtr_unique( this_rref_t rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }
                        
            ~mtr_unique( void ) noexcept{}

            template< typename O >
            mtr_unique( mtr_unique< O > && rhv ) noexcept : _ptr(rhv._ptr) 
            {
                rhv._ptr = nullptr ;
            }

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

            // get the managed pointerasync
            type_mtr_t mtr( void ) const noexcept
            {
                return _ptr ;
            }

            this_ref_t operator = ( type_mtr_t ptr ) noexcept 
            {
                _ptr = ptr ;
                return *this ;
            }

        private:
            mtr_unique( void ) noexcept {} ;
            mtr_unique( type_mtr_t ptr ) noexcept : _ptr ( ptr ){}
        };
    }

    template< typename T >
    static core::mtr_delay< T > delay( T * ptr ) noexcept
    {
        return core::mtr_delay<T>::make( ptr ) ;
    }

    template< typename T >
    static core::mtr_shared< T > share( T * ptr ) noexcept
    {
        return core::mtr_shared<T>::make( ptr ) ;
    }
    
    template< typename T >
    static core::mtr_unique< T > unique( T * & ptr ) noexcept
    {
        T * tmp = ptr ; ptr = nullptr ;
        return core::mtr_unique<T>::make( tmp ) ;
    }

    template< typename T >
    static core::mtr_unique< T > unique( T * && ptr ) noexcept
    {
        return core::mtr_unique<T>::make( ptr ) ;
    }

    template< typename T >
    static core::mtr_unique< T > move( T * & ptr ) noexcept
    {
        T * tmp = ptr ; ptr = nullptr ;
        return core::mtr_unique<T>::make( tmp ) ;
    }

    template< typename T >
    static core::mtr_unique< T > move( T * && ptr ) noexcept
    {        
        return core::mtr_unique<T>::make( ptr ) ;
    }
}

