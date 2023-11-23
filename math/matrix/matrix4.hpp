#pragma once

#include "protos.hpp"

#include "../typedefs.h"
#include "../vector/protos.hpp"

namespace motor
{
    namespace math
    {
        template< typename T >
        class matrix4
        {
            motor_this_typedefs( matrix4<T> ) ;

            motor_typedefs( T, type ) ;
            motor_typedefs( motor::math::vector3<T>, vec3 ) ;
            motor_typedefs( motor::math::vector4<T>, vec4 ) ;
            motor_typedefs( motor::math::matrix3<T>, mat3 ) ;
            motor_typedefs( motor::math::matrix4<T>, mat4 ) ;
            
        public: // ctor

            //************************************************************************************
            matrix4( void ) 
            {
                for( size_t i = 0; i<16; ++i )
                    _elem[i] = type_t(0) ;
            }

            //************************************************************************************
            matrix4( this_cref_t rhv ) 
            {
                ::std::memcpy( (void_ptr_t)_elem, (void_cptr_t)rhv._elem, sizeof(type_t)*16 ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept 
            {
                ::std::memcpy( (void_ptr_t)_elem, (void_cptr_t)rhv._elem, sizeof(type_t)*16 ) ;
                return *this ;
            }

            //************************************************************************************
            matrix4( type_t s ) 
            {
                for( size_t i=0; i<16; ++i ){
                    _elem[i] = s ;
                }
            }

            //************************************************************************************
            matrix4( mat3_t const & rhv )
            {
                this_t::set_column( 0, rhv.get_column(0) ) ;
                this_t::set_column( 1, rhv.get_column(1) ) ;
                this_t::set_column( 2, rhv.get_column(2) ) ;

                this_t::set_column( 3, vec4_t(type_t(0)) ) ;
                this_t::set_row( 3, vec4_t(type_t(0)) ) ;
            }

            //************************************************************************************
            matrix4( mat3_t const & rhv, type_t e16 )
            {
                this_t::set_column( 0, rhv.get_column(0) ) ;
                this_t::set_column( 1, rhv.get_column(1) ) ;
                this_t::set_column( 2, rhv.get_column(2) ) ;

                this_t::set_column( 3, vec4_t(type_t(0)) ) ;
                this_t::set_row( 3, vec4_t(type_t(0)) ) ;


                _elem[15] = e16 ;
            }

        public: // operator []

            //************************************************************************************
            type_t operator[]( size_t index ) const {
                return _elem[index] ;
            }

            //************************************************************************************
            type_ref_t operator[]( size_t index ){
                return _elem[index] ;
            }

        public: // operator ()

            //************************************************************************************
            type_t operator()( size_t row, size_t column ) const {
                return _elem[column*4+row] ;
            }		

            //************************************************************************************
            this_ref_t operator()( type_t rhv ){
                for( size_t i=0; i<16; ++i ) _elem[i] = rhv ;
                return (*this) ;
            }

        public: // common

            //************************************************************************************
            vec4_t row( size_t i ) const {
                const size_t start = i << 2 ;
                return vec4_t( _elem[start],_elem[start+1], _elem[start+2], _elem[start+3] ) ;
            }

            /// returns row i as a vector 3
            vec3_t row3( size_t i ) const {
                const size_t start = i << 2 ;
                return vec3_t( _elem[start],_elem[start+1], _elem[start+2] ) ;
            }

            //************************************************************************************
            void_t get_row( size_t i, vec4_t & out ) const {
                const size_t start = i*4 ;
                out( _elem[start],_elem[start+1], _elem[start+2], _elem[start+3] ) ;
            }

            //************************************************************************************
            void_t get_row( size_t i, vec3_t & out ) const {
                const size_t start = i*4 ;
                out( _elem[start],_elem[start+1], _elem[start+2] ) ;
            }

            //************************************************************************************
            this_ref_t set_row( size_t i, vec4_t const & v ){
                const size_t start = i*4 ;
                _elem[start] = v.x() ;
                _elem[start+1] = v.y() ;
                _elem[start+2] = v.z() ;
                _elem[start+3] = v.w() ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t set_row( size_t i, vec3_t const & v ){
                const size_t start = i*4 ;
                _elem[start] = v.x() ;
                _elem[start+1] = v.y() ;
                _elem[start+2] = v.z() ;
                return *this ;
            }

            vec4_t column( size_t i ) const { return vec4_t(_elem[i], _elem[i+4], _elem[i+8], _elem[i+12]) ; }
            vec4_t get_column( size_t i ) const { return column(i) ; }
            vec3_t column3( size_t i ) const { return vec3_t(_elem[i], _elem[i+4], _elem[i+8]) ; }
            vec3_t get_column3( size_t i ) const { return column3(i) ; }
            void_t get_column( size_t i, vec4_t & out ) const { out(_elem[i], _elem[i+4], _elem[i+8], _elem[i+12]) ; }
            void_t get_column( size_t i, vec3_t & out ) const {out(_elem[i], _elem[i+4], _elem[i+8] ) ;}

            //************************************************************************************
            this_ref_t set_column( size_t i, vec4_t const & v ) {
                _elem[i] = v.x();
                _elem[i+4] = v.y();
                _elem[i+8] = v.z(); 
                _elem[i+12] = v.w() ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t set_column( size_t i, vec3_t const & v ) {
                _elem[i] = v.x();
                _elem[i+4] = v.y();
                _elem[i+8] = v.z(); 
                return *this ;
            }

            //************************************************************************************
            this_ref_t transpose( void ){
                type_t tmp ;

                tmp = _elem[4] ;
                _elem[4] = _elem[1] ;
                _elem[1] = tmp ;

                tmp = _elem[8] ;
                _elem[8] = _elem[2] ;
                _elem[2] = tmp ;

                tmp = _elem[12] ;
                _elem[12] = _elem[3] ;
                _elem[3] = tmp ;

                tmp = _elem[9] ;
                _elem[9] = _elem[6] ;
                _elem[6] = tmp ;

                tmp = _elem[13] ;
                _elem[13] = _elem[7] ;
                _elem[7] = tmp ;

                tmp = _elem[14] ;
                _elem[14] = _elem[11] ;
                _elem[11] = tmp ;

                return (*this) ;
            }

            //************************************************************************************
            this_ref_t identity( void ){
                (*this)(type_t(0)) ;
                _elem[0] = type_t(1);
                _elem[5] = type_t(1);
                _elem[10] = type_t(1);
                _elem[15] = type_t(1);
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t scale_by( type_t rhv )
            {
                _elem[0] *= rhv ;
                _elem[5] *= rhv ;
                _elem[10] *= rhv ;
                _elem[15] *= rhv ;
                return (*this) ;	
            }

            //************************************************************************************
            this_ref_t scale_by( type_t sx, type_t sy, type_t sz, type_t sw )
            {
                _elem[0] *= sx ;
                _elem[5] *= sy ;
                _elem[10] *= sz ;
                _elem[15] *= sw ;
                return (*this) ;	
            }

            //************************************************************************************
            this_ref_t scale_by( vec4_cref_t rhv )
            {
                _elem[0] *= rhv.x() ;
                _elem[5] *= rhv.y() ;
                _elem[10] *= rhv.z() ;
                _elem[15] *= rhv.w() ;
                return (*this) ;	
            }

            //************************************************************************************
            this_ref_t scale_by( vec3_cref_t rhv )
            {
                _elem[0] *= rhv.x() ;
                _elem[5] *= rhv.y() ;
                _elem[10] *= rhv.z() ;
                return (*this) ;	
            }

            this_ref_t set_upper( mat3_cref_t rhv )
            {
                _elem[0] = rhv._elem[0] ;
                _elem[1] = rhv._elem[1] ;
                _elem[2] = rhv._elem[2] ;

                _elem[4] = rhv._elem[3] ;
                _elem[5] = rhv._elem[4] ;
                _elem[6] = rhv._elem[5] ;

                _elem[8] = rhv._elem[6] ;
                _elem[9] = rhv._elem[7] ;
                _elem[10] = rhv._elem[8] ;

                return *this ;
            }

            /// keeping the 4x4 matrix, but only multiple a 3x3 matrix.
            /// usable for rotation accumulation.
            /// only touches the upper 3x3 matrix
            /// leaves the other values untouched.
            this_ref_t left_multiply( mat3_cref_t rhv )
            {
                const vec3_t row0 = rhv.row(0) ;
                const vec3_t row1 = rhv.row(1) ;
                const vec3_t row2 = rhv.row(2) ;

                const vec3_t column0 = this_t::column3(0) ;
                const vec3_t column1 = this_t::column3(1) ;
                const vec3_t column2 = this_t::column3(2) ;

                _elem[0] = row0.dot(column0) ;
                _elem[1] = row0.dot(column1) ;
                _elem[2] = row0.dot(column2) ;

                _elem[4] = row1.dot(column0) ;
                _elem[5] = row1.dot(column1) ;
                _elem[6] = row1.dot(column2) ;

                _elem[8] = row2.dot(column0) ;
                _elem[9] = row2.dot(column1) ;
                _elem[10] = row2.dot(column2) ;

                return *this ;
            }

            this_ref_t right_multiply( mat3_cref_t rhv )
            {
                const vec3_t row_0 = this_t::row3(0) ;
                const vec3_t row_1 = this_t::row3(1) ;
                const vec3_t row_2 = this_t::row3(2) ;

                _elem[0] = row_0.dot(rhv.column(0)) ;
                _elem[1] = row_0.dot(rhv.column(1)) ;
                _elem[2] = row_0.dot(rhv.column(2)) ;

                _elem[4] = row_1.dot(rhv.column(0)) ; 
                _elem[5] = row_1.dot(rhv.column(1)) ; 
                _elem[6] = row_1.dot(rhv.column(2)) ; 

                _elem[8] = row_2.dot(rhv.column(0)) ; 
                _elem[9] = row_2.dot(rhv.column(1)) ; 
                _elem[10] = row_2.dot(rhv.column(2)) ; 

                return *this ;
            }

            this_ref_t set_main_diagonal( vec3_cref_t diag )
            {
                return this_t::set_main_diagonal( 
                    vec4_t(diag, type_t(0) ) ) ;
            }

            this_ref_t set_main_diagonal( vec4_cref_t diag ) 
            {
                _elem[0] = diag.x() ;
                _elem[5] = diag.y() ;
                _elem[10] = diag.z() ;
                _elem[15] = diag.w() ;

                return *this ;
            }

            vec4_t get_vec4_diagonal( void_t ) const
            {
                return vec4_t(_elem[0], _elem[5], _elem[10], _elem[15]) ;
            }

            vec3_t get_vec3_diagonal( void_t ) const
            {
                return vec3_t( _elem[0], _elem[5], _elem[10] ) ;
            }

        public: // operator +

            //************************************************************************************
            this_t operator + ( type_t rhv ) const {
                this_t mat((*this));
                for( size_t i=0; i<16; ++i ) mat[i] += rhv ;
                return mat ;
            }

            //************************************************************************************
            this_t operator + ( this_cref_t rhv ) const {
                this_t mat ;
                for( size_t i=0; i<16; ++i ) {
                    mat[i] = _elem[i] + rhv[i] ;
                }
                return mat ;
            }

            //************************************************************************************
            this_ref_t operator += ( type_t rhv ) {
                for( size_t i=0; i<16; ++i ) _elem[i] += rhv ;
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t operator += ( this_cref_t rhv ){
                for( size_t i=0; i<16; ++i ) {
                    _elem[i] += rhv[i] ;
                }
                return (*this) ;
            }

        public: // operator -

            //************************************************************************************
            this_t operator - ( type_t rhv ) const {
                this_t mat((*this));
                for( size_t i=0; i<16; ++i ) mat[i] -= rhv ;
                return mat ;
            }

            //************************************************************************************
            this_t operator - ( this_cref_t rhv ) const {
                this_t mat ;
                size_t i = 0 ;
                for( size_t i=0; i<16; ++i ) {
                    mat[i] = _elem[i] - rhv[i] ;
                }
                return mat ;
            }

            //************************************************************************************
            this_ref_t operator -= ( type_t rhv ) {
                for( size_t i=0; i<16; ++i ) _elem[i] -= rhv ;
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t operator -= ( this_cref_t rhv ) {
                for( size_t i=0; i<16; ++i ) {
                    _elem[i] -= rhv[i] ;
                }
                return (*this) ;
            }

        public: // operator *

            //************************************************************************************
            this_t operator * ( type_t rhv ) const {
                this_t mat((*this));
                for( size_t i=0; i<16; ++i ) mat[i] *= rhv ;
                return mat ;
            }

            //************************************************************************************
            this_ref_t operator *= ( type_t rhv ) {
                for( size_t i=0; i<16; ++i ) _elem[i] *= rhv ;
                return (*this) ;
            }

            //************************************************************************************
            vec4_t operator * ( vec4_t const & rhv ) const {
                return vec4_t(
                    row(0).dot( rhv ),
                    row(1).dot( rhv ),
                    row(2).dot( rhv ),
                    row(3).dot( rhv )
                    ) ;
            }

            //************************************************************************************
            vec3_t operator * ( vec3_t const & rhv ) const {
                return vec3_t(
                    row3(0).dot( rhv ),
                    row3(1).dot( rhv ),
                    row3(2).dot( rhv )
                    ) ;
            }

            //************************************************************************************
            this_t operator * ( this_cref_t rhv ) const {
                this_t mat ;
            
                mat[0] = row(0).dot( rhv.column( 0 ) ) ;
                mat[1] = row(0).dot( rhv.column( 1 ) ) ;
                mat[2] = row(0).dot( rhv.column( 2 ) ) ;
                mat[3] = row(0).dot( rhv.column( 3 ) ) ;
            
                mat[4] = row(1).dot( rhv.column( 0 ) ) ;
                mat[5] = row(1).dot( rhv.column( 1 ) ) ;
                mat[6] = row(1).dot( rhv.column( 2 ) ) ;
                mat[7] = row(1).dot( rhv.column( 3 ) ) ;
            
                mat[8] = row(2).dot( rhv.column( 0 ) ) ;
                mat[9] = row(2).dot( rhv.column( 1 ) ) ;
                mat[10] = row(2).dot( rhv.column( 2 ) ) ;
                mat[11] = row(2).dot( rhv.column( 3 ) ) ;

                mat[12] = row(3).dot( rhv.column( 0 ) ) ;
                mat[13] = row(3).dot( rhv.column( 1 ) ) ;
                mat[14] = row(3).dot( rhv.column( 2 ) ) ;
                mat[15] = row(3).dot( rhv.column( 3 ) ) ;
            
                return mat ;
            }

            //************************************************************************************
            this_t operator * ( mat3_t const & rhv ) const 
            {
                this_t mat = this_t(*this) ;

                mat._elem[0] = this_t::row3(0).dot(rhv.column(0)) ;
                mat._elem[1] = this_t::row3(0).dot(rhv.column(1)) ;
                mat._elem[2] = this_t::row3(0).dot(rhv.column(2)) ;

                mat._elem[4] = this_t::row3(1).dot(rhv.column(0)) ;
                mat._elem[5] = this_t::row3(1).dot(rhv.column(1)) ;
                mat._elem[6] = this_t::row3(1).dot(rhv.column(2)) ;

                mat._elem[8] = this_t::row3(2).dot(rhv.column(0)) ;
                mat._elem[9] = this_t::row3(2).dot(rhv.column(1)) ;
                mat._elem[10] = this_t::row3(2).dot(rhv.column(2)) ;
            
                return mat ;
            }

            //************************************************************************************
            this_ref_t operator *= ( this_cref_t rhv ) {
                this_t mat ;
                size_t i = 0 ;
                for( size_t y=0; y<4; ++y ) {
                    for( size_t x=0; x<4; ++x, ++i ){
                        mat[i] = this_t::row(y).dot( rhv.column( x ) ) ;
                    }
                }
                (*this) = mat ;
                return (*this) ;
            }

        public:

            static this_t make_identity( void_t ) noexcept
            {
                this_t r ;
                for( size_t i = 0; i<16; ++i )
                    r._elem[i] = type_t(0) ;

                r._elem[0] = type_t(1) ;
                r._elem[5] = type_t(1) ;
                r._elem[10] = type_t(1) ;
                r._elem[15] = type_t(1) ;

                return r ;
            }

        private:

            /// the matrix elements.
            type_t _elem[16] ;
        } ;
    
        motor_typedefs( matrix4<int_t>, mat4i ) ;
        motor_typedefs( matrix4<float_t>, mat4f ) ;
        motor_typedefs( matrix4<double_t>, mat4d ) ;
        motor_typedefs( matrix4<uint_t>, mat4ui ) ;
        motor_typedefs( matrix4<size_t>, mat4s ) ;
    }
} 
