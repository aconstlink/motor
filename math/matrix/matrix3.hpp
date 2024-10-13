#pragma once

#include "protos.hpp"
#include "../vector/protos.hpp"
#include "../quaternion/protos.hpp"

#include "../typedefs.h"
#include "../utility/fn.hpp"

#include <cstring>

namespace motor
{
    namespace math
    {
        template< typename T >
        class matrix3
        {
        public: // typedefs 

            motor_this_typedefs( matrix3<T> ) ;

            motor_typedefs( T, type ) ;
            motor_typedefs( matrix4<T>, mat4 ) ;
            motor_typedefs( vector3<T>, vec3 ) ;
            motor_typedefs( vector4<T>, vec4 ) ;
            motor_typedefs( quaternion4<T>, quat4 ) ;

        public: // ctor

            //************************************************************************************
            matrix3( void ) 
            {
                _elem[0] = _elem[1] = _elem[2] = 
                _elem[3] = _elem[4] = _elem[5] = 
                _elem[6] = _elem[7] = _elem[8] = 
                type_t(0) ;
                
            }

            //************************************************************************************
            matrix3( this_cref_t rhv ) 
            {
                std::memcpy( (void_ptr_t)_elem, (void_cptr_t)rhv._elem, sizeof(type_t)*9 ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                std::memcpy( (void_ptr_t)_elem, (void_cptr_t)rhv._elem, sizeof(type_t)*9 ) ;
                return *this ;
            }

            //************************************************************************************
            matrix3( type_t s ) {
                _elem[0] = _elem[1] = _elem[2] = 
                _elem[3] = _elem[4] = _elem[5] = 
                _elem[6] = _elem[7] = _elem[8] = 
                s ;
            }

            //************************************************************************************
            matrix3( mat4_t const & rhv )
            {
                set_column( 0, rhv.column3(0) ) ;
                set_column( 1, rhv.column3(1) ) ;
                set_column( 2, rhv.column3(2) ) ;
            }

            //************************************************************************************
            matrix3( quat4_t const & q )
            {
                (*this)( q ) ;
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
            this_cref_t operator()( type_t rhv ){
                for( size_t i=0; i<9; ++i ) _elem[i] = rhv ;
                return (*this) ;
            }

            //************************************************************************************
            type_t operator()( size_t row, size_t column ) const {
                return _elem[row*3+column] ;
            }

            //************************************************************************************
            this_cref_t operator () ( quat4_t const & q )
            {
                const type_t q0 = q.real() ;
                const type_t q1 = q.i() ;
                const type_t q2 = q.j() ;
                const type_t q3 = q.k() ;

                //------------------------------------
                type_t q0_q0 = q0*q0 ;
                type_t q0_q1 = q0*q1 ;
                type_t q0_q2 = q0*q2 ;
                type_t q0_q3 = q0*q3 ;

                type_t q1_q1 = q1*q1 ;
                type_t q1_q2 = q1*q2 ;
                type_t q1_q3 = q1*q3 ;

                type_t q2_q2 = q2*q2 ;
                type_t q2_q3 = q2*q3 ;

                type_t q3_q3 = q3*q3 ;	
                //------------------------------------
                type_t c2q0_q1 = type_t(2) * q0_q1 ;
                type_t c2q0_q2 = type_t(2) * q0_q2 ;
                type_t c2q0_q3 = type_t(2) * q0_q3 ;

                type_t c2q1_q2 = type_t(2) * q1_q2 ;
                type_t c2q1_q3 = type_t(2) * q1_q3 ;

                type_t c2q2_q3 = type_t(2) * q2_q3 ;
                //------------------------------------

                _elem[0] = type_t(2) * (q0_q0+q1_q1) - type_t(1) ;
                _elem[1] = c2q1_q2 - c2q0_q3 ;
                _elem[2] = c2q1_q3 + c2q0_q2 ;

                _elem[3] = c2q1_q2 + c2q0_q3 ;
                _elem[4] = type_t(2) * (q0_q0+q2_q2) - type_t(1) ; 
                _elem[5] = c2q2_q3 - c2q0_q1 ;

                _elem[6] = c2q1_q3 - c2q0_q2 ;
                _elem[7] = c2q2_q3 + c2q0_q1 ;
                _elem[8] = type_t(2) * (q0_q0+q3_q3) - type_t(1) ;

                return (*this) ;
            }

            //************************************************************************************
            this_t operator () ( quat4_t const & q ) const {
                this_t mat( q ) ;
                return mat ;
            }

            //************************************************************************************
            this_t operator () ( type_t p, type_t y, type_t r ){

            }

        public: // common

            //************************************************************************************
            vec3_t row( size_t i ) const {
                const size_t start = i*3 ;
                return vec3_t( _elem[start],_elem[start+1], _elem[start+2] ) ;
            }

            //************************************************************************************
            /**
                The unsafe version. It doesn't check the index boundaries.
            */
            vec3_t row_u( size_t i )const 
            {
                const size_t start = i*3 ;
                return vec3_t( _elem[start],_elem[start+1], _elem[start+2] ) ;
            }

            //************************************************************************************
            this_ref_t set_row( size_t i, vec4_t const & v )
            {
                const size_t start = i*3 ;
                _elem[start] = v.x() ;
                _elem[start+1] = v.y() ;
                _elem[start+2] = v.z() ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t set_row( size_t i, vec3_t const & v )
            {
                const size_t start = i*3 ;
                _elem[start] = v.x() ;
                _elem[start+1] = v.y() ;
                _elem[start+2] = v.z() ;
                return *this ;
            }

            vec3_t column( size_t i ) const { return vec3_t(_elem[i], _elem[i+3], _elem[i+6]) ; }
            vec3_t get_column( size_t i ) const { return column(i) ; }

            //************************************************************************************
            this_ref_t set_column( size_t i, vec3_t const & v ) 
            {
                _elem[i] = v.x();
                _elem[i+3] = v.y();
                _elem[i+6] = v.z(); 
                return *this ;
            }

            //************************************************************************************
            // zero based indices
            type_t get_element( size_t r, size_t c ) const
            {
                return _elem[r * 3 + c] ;
            }

            //************************************************************************************
            this_ref_t zero( void_t )
            {
                for(size_t i=0;i<9; ++i) _elem[i] = type_t(0);
                return *this ;
            }

            //************************************************************************************
            this_ref_t transpose( void )
            {
                type_t tmp ;

                tmp = _elem[3] ;
                _elem[3] = _elem[1] ;
                _elem[1] = tmp ;

                tmp = _elem[6] ;
                _elem[6] = _elem[2] ;
                _elem[2] = tmp ;

                tmp = _elem[7] ;
                _elem[7] = _elem[5] ;
                _elem[5] = tmp ;

                return (*this) ;
            }

            //************************************************************************************
            this_t transposed( void ) const
            {
                this_t mat( *this ) ;
                return mat.transpose() ; ;
            }

            //************************************************************************************
            this_ref_t identity( void )
            {
                this->zero() ;
                _elem[0] = type_t(1);
                _elem[4] = type_t(1);
                _elem[8] = type_t(1);
                return (*this) ;
            }

            //************************************************************************************
            this_t identity( void ) const 
            {
                this_t mat( *this ) ;
                return mat.identity() ; ;
            }

            //************************************************************************************
            this_ref_t scale( type_t rhv ){
                _elem[0] *= rhv ;
                _elem[4] *= rhv ;
                _elem[8] *= rhv ;
                return (*this) ;	
            }

            //************************************************************************************
            this_t scaled( type_t rhv ) const{
                this_t mat( *this ) ;
                return mat.scale( rhv ) ; ;	
            }

            //************************************************************************************
            this_ref_t scale( type_t sx, type_t sy, type_t sz ){
                _elem[0] *= sx ;
                _elem[4] *= sy ;
                _elem[8] *= sz ;
                return (*this) ;	
            }

            //************************************************************************************
            this_t scaled( type_t sx, type_t sy, type_t sz ) const {
                this_t mat( *this ) ;
                return mat.scale( sx, sy, sz ) ;
            }

            //************************************************************************************
            this_ref_t scale( vec3_t const & rhv ){
                _elem[0] *= rhv.x() ;
                _elem[4] *= rhv.y() ;
                _elem[8] *= rhv.z() ;
                return (*this) ;	
            }

            //************************************************************************************
            this_t scaled( vec3_t const & rhv ) const {
                this_t mat(*this) ;
                return mat.scale( rhv ) ;
            }

            //************************************************************************************
            vec3_t main_diagonal( void_t ) const noexcept
            {
                return vec3_t( _elem[0], _elem[4], _elem[8] ) ;
            }

            //************************************************************************************
            type_t trace( void ) const 
            {
                return _elem[0]+_elem[4]+_elem[8] ;
            }

            //************************************************************************************
            type_t angle( void ) const {
                return motor::math::fn<type_t>::acos( (this_t::trace()-type_t(1))*type_t(0.5) ) ;
            }

            //************************************************************************************
            vec3_t rotation_axis( void ) const {
                return vec3_t(-_elem[5]+_elem[7],-_elem[6]+_elem[2],-_elem[1]+_elem[3]) ;
            }

            //************************************************************************************
            quat4_t orientation( void ) const {
                return quat4_t(angle(), rotation_axis()) ;
            }

        public: // operator +

            //************************************************************************************
            this_t operator + ( type_t rhv ) const {
                this_t mat((*this));
                for( size_t i=0; i<9; ++i ) mat[i] += rhv ;
                return mat ;
            }

            //************************************************************************************
            this_t operator + ( this_cref_t rhv ) const {
                this_t mat ;
                size_t i = 0 ;
                for( size_t i=0; i<9; ++i ) {
                    mat[i] = _elem[i] + rhv[i] ;
                }
                return mat ;
            }

            //************************************************************************************
            this_ref_t operator += ( type_t rhv ) {
                for( size_t i=0; i<9; ++i ) _elem[i] += rhv ;
                return (*this) ;
            }

        public: // operator -

            //************************************************************************************
            this_t operator - ( type_t rhv ) const {
                this_t mat((*this));
                for( size_t i=0; i<9; ++i ) mat[i] -= rhv ;
                return mat ;
            }


            //************************************************************************************
            this_ref_t operator -= ( type_t rhv ) {
                for( size_t i=0; i<9; ++i ) _elem[i] -= rhv ;
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t operator -= ( this_cref_t rhv ) {
                for( size_t i=0; i<9; ++i ) {
                    _elem[i] -= rhv[i] ;
                }
                return (*this) ;
            }

        public: // operator *

            //************************************************************************************
            this_t operator * ( type_t rhv ) const 
            {
                this_t mat((*this));
                for( size_t i=0; i<9; ++i ) mat[i] *= rhv ;
                return mat ;
            }

            //************************************************************************************
            this_cref_t operator *= ( type_t rhv ) 
            {
                for( size_t i=0; i<9; ++i ) _elem[i] *= rhv ;
                return (*this) ;
            }

            //************************************************************************************
            vec3_t operator * ( vec3_t const & rhv ) const 
            {
                vec3_t vec ;
                for( size_t i=0; i<3; ++i ) vec[i] = row(i).dot( rhv ) ; 
                return vec ;
            }

            //************************************************************************************
            this_t operator * ( this_cref_t rhv ) const 
            {
                this_t mat ;
            
                mat[0] = this_t::row(0).dot(rhv.column(0)) ;
                mat[1] = this_t::row(0).dot(rhv.column(1)) ;
                mat[2] = this_t::row(0).dot(rhv.column(2)) ;

                mat[3] = this_t::row(1).dot(rhv.column(0)) ;
                mat[4] = this_t::row(1).dot(rhv.column(1)) ;
                mat[5] = this_t::row(1).dot(rhv.column(2)) ;

                mat[6] = this_t::row(2).dot(rhv.column(0)) ;
                mat[7] = this_t::row(2).dot(rhv.column(1)) ;
                mat[8] = this_t::row(2).dot(rhv.column(2)) ;

                return mat ;
            } 

            //************************************************************************************
            this_ref_t operator *= ( this_cref_t rhv ) 
            {
                this_t mat( *this ) ;

                _elem[0] = mat.row(0).dot( rhv.column(0) ) ;
                _elem[1] = mat.row(0).dot( rhv.column(1) ) ;
                _elem[2] = mat.row(0).dot( rhv.column(2) ) ;

                _elem[3] = mat.row(1).dot( rhv.column(0) ) ;
                _elem[4] = mat.row(1).dot( rhv.column(1) ) ;
                _elem[5] = mat.row(1).dot( rhv.column(2) ) ;

                _elem[6] = mat.row(2).dot( rhv.column(0) ) ;
                _elem[7] = mat.row(2).dot( rhv.column(1) ) ;
                _elem[8] = mat.row(2).dot( rhv.column(2) ) ;

                return *this ;
            }

        public:

            //************************************************************************************
            static this_t make_identity( void_t ) noexcept
            {
                this_t r ;
                r._elem[0] = r._elem[4] = r._elem[8] = type_t(1) ;

                r._elem[1] = r._elem[2] = r._elem[3] = 
                r._elem[5] = r._elem[6] = r._elem[7] = type_t(0) ;

                return r ;
            }


        private:

            /// the matrix elements.
            type_t _elem[9] ;

        } ;

        motor_typedefs( matrix3<int_t>, mat3i ) ;
        motor_typedefs( matrix3<float_t>, mat3f ) ;
        motor_typedefs( matrix3<double_t>, mat3d ) ;
        motor_typedefs( matrix3<uint_t>, mat3ui ) ;
        motor_typedefs( matrix3<size_t>, mat3s ) ;
    }
}
