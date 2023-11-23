#pragma once

#include "protos.hpp"
#include "../matrix/protos.hpp"
#include "../vector/protos.hpp"
#include "../typedefs.h"

namespace motor
{
    namespace math
    {
        template< typename T >
        class quaternion4
        {
            motor_this_typedefs( quaternion4<T> ) ;

            motor_typedefs( T, type ) ;
            motor_typedefs( motor::math::vector3<T>, vec3 ) ;
            motor_typedefs( motor::math::vector4<T>, vec4 ) ;
            motor_typedefs( motor::math::matrix3<T>, mat3 ) ;
            motor_typedefs( motor::math::matrix4<T>, mat4 ) ;
            motor_typedefs( motor::math::quaternion3<T>, quat3 ) ;

        public: // ctors

            //************************************************************************************
            quaternion4( void )
            {
                this->zero() ;
            }

            //************************************************************************************
            /*quaternion4( type_t angle, vec3_t const & rot )
            {
                (*this)( angle, rot ) ;
            }*/

            //************************************************************************************
            quaternion4( type_t angle, vec3_t const & rot )
            {
                (*this)( angle, rot ) ;
            }

            //************************************************************************************
            quaternion4( type_t angle, vec3_cref_t axis, motor::math::axis_normalized )
            {
                this_t::rotatate_norm_axis( axis, angle ) ;
            }

            //************************************************************************************
            quaternion4( this_cref_t rhv )
            {
                _elem[0] = rhv.real() ;
                _elem[1] = rhv.i() ;
                _elem[2] = rhv.j() ;
                _elem[3] = rhv.k() ;
            }

            //************************************************************************************
            quaternion4( type_t real, type_t i, type_t j, type_t k ) 
            {
                _elem[0] = real ;
                _elem[1] = i ;
                _elem[2] = j ;
                _elem[3] = k ;
            }

            //************************************************************************************
            /**
                in this vector, the elements must mean:
                vec[w] = real part ;
                vec[x,y,z] = vector part ;
            */
            quaternion4( vec4_t const & vec ) 
            {
                _elem[0] = vec.w() ;
                _elem[1] = vec.x() ;
                _elem[2] = vec.y() ;
                _elem[3] = vec.z() ;

            }

            //************************************************************************************
            /*quaternion4( mat3x3_t const & rot )
            {
                quaternion( rot.angle(), rot.rotation_axis() ) ;
            }*/

        public: // getter/ setter

            //************************************************************************************
            type_t real( void ) const {
                return _elem[0] ;
            }

            //************************************************************************************
            type_ref_t real( void ) {
                return _elem[0] ;
            }

            //************************************************************************************
            type_t i( void ) const{
                return _elem[1] ;
            }

            //************************************************************************************
            type_ref_t i( void ) {
                return _elem[1] ;
            }

            //************************************************************************************
            type_t j( void ) const{
                return _elem[2] ;
            }

            //************************************************************************************
            type_ref_t j( void ){
                return _elem[2] ;
            }

            //************************************************************************************
            type_t k( void ) const{
                return _elem[3] ;
            }

            //************************************************************************************
            type_ref_t k( void ){
                return _elem[3] ;
            }

            //************************************************************************************
            vec3_t vec( void ) const{
                return vec3_t(_elem[1],_elem[2],_elem[3]) ;
            }

            //************************************************************************************
            void vec( vec3_t const & rhv ){
                _elem[1]=rhv.x() ;
                _elem[2]=rhv.y() ;
                _elem[3]=rhv.z() ;
            }

        public: // operator ()
        
            //************************************************************************************
            this_cref_t operator () ( type_t angle, vec3_t const & rot ){
                angle = angle * type_t(0.5) ;
                _elem[0] = motor::math::fn<type_t>::cos( angle ) ;
                this->vec( rot.normalized() * motor::math::fn<type_t>::sin( angle ) ) ;
                return *this ;
            }

            //************************************************************************************
            this_t operator () ( type_t angle, vec3_t const & rot ) const{
                this_t q( *this ) ;
                q( angle, rot ) ;
                return q ;
            }

        public: // operator +
    
            //************************************************************************************
            this_t operator + ( this_cref_t rhv ) const {
                return this_t(_elem[0]+rhv.real(), _elem[1]+rhv.i(), _elem[2]+rhv.j(), _elem[3]+rhv.k()) ;
            }

            //************************************************************************************
            this_ref_t operator += ( this_cref_t rhv ) {
                _elem[0]+=rhv.real() ;
                _elem[1]+=rhv.i() ;
                _elem[2]+=rhv.j() ; 
                _elem[3]+=rhv.k() ;
                return (*this) ;
            }

        public: // operator -

            //************************************************************************************
            this_t operator - ( this_cref_t rhv ) const {
                return this_t(_elem[0]-rhv.real(), _elem[1]-rhv.i(), _elem[2]-rhv.j(), _elem[3]-rhv.k()) ;
            }

            //************************************************************************************
            this_ref_t operator -= ( this_cref_t rhv ) {
                _elem[0]-=rhv.real() ;
                _elem[1]-=rhv.i() ;
                _elem[2]-=rhv.j() ; 
                _elem[3]-=rhv.k() ;
                return (*this) ;
            }

        public: // operator *

            //************************************************************************************
            this_t operator * ( this_cref_t rhv ) const {
                this_t q ;
                q.real() = _elem[0]*rhv.real()-this->vec().dot(rhv.vec()) ;
                q.vec( 
                    rhv.vec()*_elem[0] + 
                    this->vec()*rhv.real() + 
                    this->vec().crossed(rhv.vec()) 
                    ) ;
                return q ;
            }

            //************************************************************************************
            this_ref_t operator *= ( this_cref_t rhv ){
                this_t q ;
                q.real() = _elem[0]*rhv.real()-this->vec().dot(rhv.vec()) ;
                q.vec( 
                    rhv.vec()*_elem[0] + 
                    this->vec()*rhv.real() + 
                    this->vec().crossed(rhv.vec()) 
                    ) ;
                (*this) = q ;
                return (*this ); 
            }

            //************************************************************************************
            vec3_t operator * ( vec3_t const & v ) const {
                vec3_t v_q = (*this).vec() ;
                return	v*((((type_t)2)*_elem[0]*_elem[0])-((type_t)1) ) +
                    (
                    v_q * v_q.dot( v ) + 
                    v_q.crossed( v ) * _elem[0] 
                    )*((type_t)2);
            }

        
        public: // operator =
        
            //************************************************************************************
            this_ref_t operator = ( this_cref_t rhv ){
                _elem[0] = rhv.real() ;
                _elem[1] = rhv.i() ;
                _elem[2] = rhv.j() ;
                _elem[3] = rhv.k() ;
                return (*this ) ;
            }

            //************************************************************************************
            this_ref_t operator = ( quat3_t const & rhv ){
                _elem[0] = type_t(0) ;
                _elem[1] = rhv.i() ;
                _elem[2] = rhv.j() ;
                _elem[3] = rhv.k() ;

                return (*this) ;
            }

        public: // common

            //************************************************************************************
            mat3_t to_matrix( void ) const 
            {
                return mat3_t(*this) ;
            }

            //************************************************************************************
            quat3_t transform( quat3_t const & v ) const {
                return quat3_t( transform( vec3_t( v ) ) ) ;
            }

            //************************************************************************************
            vec3_t transform( vec3_t const & v ) const {
                vec3_t v_q = (*this).vec() ;
                return	v*( (((type_t)2)*_elem[0]*_elem[0])-1 ) 
                        + v_q * (((type_t)2)*v_q.dot( v ))
                        + v_q.crossed( v ) * ((type_t)2) * _elem[0] ;
            }

            //************************************************************************************
            this_ref_t conjugate( void ){
                _elem[1] = -_elem[1] ;
                _elem[2] = -_elem[2] ;
                _elem[3] = -_elem[3] ;
                return (*this) ;
            }

            //************************************************************************************
            this_t conjugated( void ) const{
                this_t quat(*this) ;
                return quat.conjugate() ;
            }

            //************************************************************************************
            type_t length2( void ) const {
                return (_elem[0]*_elem[0]+_elem[1]*_elem[1]+_elem[2]*_elem[2]+_elem[3]*_elem[3]) ;
            }

            //************************************************************************************
            type_t length( void ) const {
                return sqrt(length2()) ;
            }

            //************************************************************************************
            this_ref_t zero( void_t )
            {
                _elem[0] = type_t(0);
                _elem[1] = type_t(0);
                _elem[2] = type_t(0);
                _elem[3] = type_t(0);
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t rotatate_norm_axis( vec3_cref_t axis, type_t angle )
            {
                angle = angle * type_t(0.5) ;
                _elem[0] = motor::math::fn<type_t>::cos( angle ) ;
                this->vec( axis * motor::math::fn<type_t>::sin( angle ) ) ;
                return *this ;
            }

        private: // variables

            // [0] = real
            // [1,2,3] = i,j,k
            type_t _elem[4] ;

        } ;
        motor_typedefs( quaternion4<float_t>, quat4f ) ;
        motor_typedefs( quaternion4<double_t>, quat4d ) ;
    }
}