#pragma once

#include "protos.hpp"
#include "vector2b.hpp"

namespace motor 
{
    namespace math
    {
        class vector4b
        {
        public:

            typedef bool type_t ;
            typedef type_t & type_ref_t ;
            typedef type_t const & type_cref_t ;

            typedef vector2b vec2b_t ;
            typedef vec2b_t const & vec2b_cref_t ;

            typedef vector4< type_t > vec4_t ;

            typedef vector4b this_t ;
            typedef this_t & this_ref_t ;
            typedef this_t const & this_cref_t ;

        private:

            type_t _elem[4] ;

        public:

            //************************************************************************************
            vector4b( void )
            {
                _elem[0] = false ;
                _elem[1] = false ;
                _elem[2] = false ;
                _elem[3] = false ;
            }

            //************************************************************************************
            vector4b( type_t x, type_t y, type_t z, type_t w )
            {
                (*this)(x,y,z,w) ;
            }

            //************************************************************************************
            vector4b( vec2b_cref_t xy, vec2b_cref_t zw )
            {
                _elem[0] = xy.x() ;
                _elem[1] = xy.y() ;
                _elem[2] = zw.x() ;
                _elem[3] = zw.y() ;
            }

            //************************************************************************************
            vector4b( this_cref_t rhv ) 
            {
                (*this)(rhv.x(),rhv.y(),rhv.z(),rhv.w()) ;
            }

            //************************************************************************************
            vector4b( type_t b )
            {
                _elem[0] = b ;
                _elem[1] = b ;
                _elem[2] = b ;
                _elem[3] = b ;
            }

        public: // x,y,z,w accessor

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
            type_t w( void ) const {
                return _elem[3] ;
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
            type_ref_t w( void ) {
                return _elem[3] ;
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

            //************************************************************************************
            this_ref_t w( type_t val ){
                _elem[3] = val ;
                return *this ;
            }

            //************************************************************************************
            vec2b_t xy( void_t ) const noexcept
            {
                return vec2b_t( _elem[0], _elem[1] ) ;
            }

            //************************************************************************************
            vec2b_t zw( void_t ) const noexcept
            {
                return vec2b_t( _elem[2], _elem[3] ) ;
            }

        public: // operator ()

            //************************************************************************************
            this_ref_t operator()( type_t x, type_t y, type_t z, type_t w ){
                _elem[0]=x; _elem[1]=y; _elem[2]=z ; _elem[3]=w ;
                return *this ;
            }

        public: // operator []

            //************************************************************************************
            type_cref_t operator[]( size_t index ) const{
                return _elem[index] ;
            }

            //************************************************************************************
            type_ref_t operator[]( size_t index ) {
                return _elem[index] ;
            }

        public: // operator = 

            //************************************************************************************
            this_ref_t operator = ( type_t a )
            {
                _elem[0] = a ; _elem[1] = a ; _elem[2] = a ; _elem[3] = a ;
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t operator = ( this_cref_t rhv )
            {
                _elem[0] = rhv.x() ; _elem[1] = rhv.y() ; _elem[2] = rhv.z() ; _elem[3] = rhv.w() ;
                return (*this) ;
            }

        public:

            bool all( void ) const
            {
                return _elem[0] && _elem[1] && _elem[2] && _elem[3] ;
            }

            bool any( void ) const
            {
                return _elem[0] || _elem[1] || _elem[2] || _elem[3] ;
            }

        } ;
        typedef vector4b vec4b_t ;
    }
}