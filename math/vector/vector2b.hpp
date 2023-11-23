#pragma once

namespace motor 
{
    namespace math
    {
        class vector2b
        {
        public:

            typedef bool type_t ;
            typedef type_t & type_ref_t ;
            typedef type_t const & type_cref_t ;

            typedef vector2b vec2_t ;

            typedef vec2_t this_t ;
            typedef this_t & this_ref_t ;
            typedef this_t const & this_cref_t ;

        private:

            bool _elem[2] ;

        public:

            //************************************************************************************
            vector2b( void ){
                _elem[0] = false ;
                _elem[1] = false ;
            }

            //************************************************************************************
            vector2b( type_t x, type_t y ){
                (*this)(x,y) ;
            }

            //************************************************************************************
            vector2b( this_cref_t rhv ) {
                (*this)(rhv.x(),rhv.y()) ;
            }

        public: // x,y,z accessor

            //************************************************************************************
            type_t x( void ) const {
                return _elem[0] ;
            }

            //************************************************************************************
            type_t y( void ) const 
                {
                    return _elem[1] ;
            }

            //************************************************************************************
            type_ref_t x( void ) 
                {
                    return _elem[0] ;
            }

            //************************************************************************************
            type_ref_t y( void ) 
                {
                    return _elem[1] ;
            }

            //************************************************************************************
            this_ref_t x( type_t val )
                {
                    _elem[0] = val ;
                    return *this ;
            }

            //************************************************************************************
            this_ref_t y( type_t val )
                {
                    _elem[1] = val ;
                    return *this ;    
            }

        public: // operator ()

            //************************************************************************************
            this_cref_t operator()( void ){
                _elem[0] = type_t(0);_elem[1] = type_t(0);
                return (*this) ;
            }

            //************************************************************************************
            this_cref_t operator()( type_t x, type_t y ){
                _elem[0]=x; _elem[1]=y;
                return *this ;
            }

        public: // operator [] 

            //************************************************************************************
            type_t operator[]( size_t index ) const{
                return _elem[index] ;
            }

            //************************************************************************************
            type_ref_t operator[]( size_t index ) {
                return _elem[index] ;
            }

        public: // operator =

            //************************************************************************************
            this_ref_t operator = ( this_cref_t rhv ){
                _elem[0] = rhv.x() ; _elem[1] = rhv.y() ; 
                return (*this) ;
            }

        public:

            this_t operator && ( this_cref_t rhv ) const noexcept
            {
                return this_t( _elem[0] && rhv._elem[0], _elem[1] && rhv._elem[1] ) ;
            }

            this_t operator || ( this_cref_t rhv ) const noexcept
            {
                return this_t( _elem[0] || rhv._elem[0], _elem[1] || rhv._elem[1] ) ;
            }

        public: // common

            bool all( void ) const 
            {
                return _elem[0] && _elem[1] ;
            }

            bool any( void ) const
            {
                return _elem[0] || _elem[1] ;
            }

            template< typename T >
            motor::math::vector2< T > select( motor::math::vector2< T > const t, motor::math::vector2< T > const & f ) const noexcept
            {
                return motor::math::vector2< T >( _elem[0] ? t.x() : f.x(), _elem[1] ? t.y() : f.y() ) ;
            }
        };
        typedef vector2b vec2b_t ;
    }
}