#pragma once

namespace motor
{
    namespace math
    {
        class vector3b
        {
        public:

            typedef bool type_t ;
            typedef type_t & type_ref_t ;

            typedef vector3b vec3_t ;

            typedef vec3_t this_t ;
            typedef this_t & this_ref_t ;
            typedef this_t const & this_cref_t ;

        private:

            type_t _elem[3] ;

        public: // ctor

            //************************************************************************************
            vector3b( void )
            {
                _elem[0] = false ;
                _elem[1] = false ;
                _elem[2] = false ;
            }

            //************************************************************************************
            vector3b( type_t x, type_t y, type_t z )
            {
                _elem[0] = x ;
                _elem[1] = y ;
                _elem[2] = z ;
            }

            //************************************************************************************
            vector3b( this_cref_t rhv ) 
            {
                _elem[0] = rhv.x() ;
                _elem[1] = rhv.y() ;
                _elem[2] = rhv.z() ;
            }

        public: // x,y,z accessor

            //************************************************************************************
            type_t x( void ) const {
                return _elem[0] ;
            }

            //************************************************************************************
            type_t y( void ) const {
                return _elem[1] ;
            }

            //************************************************************************************
            type_t z( void ) const {
                return _elem[2] ;
            }

            //************************************************************************************
            type_ref_t x( void ) {
                return _elem[0] ;
            }

            //************************************************************************************
            type_ref_t y( void ) {
                return _elem[1] ;
            }

            //************************************************************************************
            type_ref_t z( void ) {
                return _elem[2] ;
            }

            //************************************************************************************
            this_ref_t x( type_t val ){
                _elem[0] = val ;
                        return *this ;
            }

            //************************************************************************************
            this_ref_t y( type_t val ){
                _elem[1] = val ;
                        return *this ;
            }

            //************************************************************************************
            this_ref_t z( type_t val ){
                _elem[2] = val ;
                        return *this ;
            }

        public: // operator ()

            //************************************************************************************
            this_ref_t operator()( type_t b ){
                _elem[0]=b; _elem[1]=b; _elem[2]=b ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t operator()( type_t x, type_t y, type_t z ){
                _elem[0]=x; _elem[1]=y; _elem[2]=z ;
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

        public: // common

            bool all( void ) const
            {
                return _elem[0] && _elem[1] && _elem[2] ;
            }

            bool any( void ) const
            {
                return _elem[0] || _elem[1] || _elem[2] ;
            }
        } ;

        typedef vector3b vec3b_t ;
    }
}