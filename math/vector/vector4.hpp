#pragma once

#include "protos.hpp"

#include "../typedefs.h"

#include "vector4b.hpp"
#include "../utility/fn.hpp"

#include <limits>

namespace motor
{
    namespace math
    {
        template< typename T >
        class vector4
        {
            motor_this_typedefs( vector4<T> ) ;

        public:

            motor_typedefs( T, type ) ;
            motor_typedefs( vector2< type_t >, vec2 ) ;
            motor_typedefs( vector3< type_t >, vec3 ) ;
            motor_typedefs( vector4< type_t >, vec4 ) ;
            motor_typedefs( vector4b, vecb ) ;
            motor_typedefs( vector4b, vec4b ) ;

        public:

            //************************************************************************************
            vector4( void )
            {
                _elem[0] = type_t(0) ;
                _elem[1] = type_t(0) ;
                _elem[2] = type_t(0) ;
                _elem[3] = type_t(0) ;
            }

            //************************************************************************************
            vector4( type_t x, type_t y, type_t z, type_t w )
            {
                (*this)(x,y,z,w) ;
            }

            //************************************************************************************
            vector4( this_cref_t rhv ) 
            {
                (*this)(rhv.x(),rhv.y(),rhv.z(),rhv.w()) ;
            }

            //************************************************************************************
            vector4( vec3_t const & rhv ) 
            {
                (*this)(rhv.x(),rhv.y(),rhv.z(),type_t(0)) ;
            }

            //************************************************************************************
            vector4( vec3_t const & rhv, type_t w ) 
            {
                (*this)(rhv.x(),rhv.y(),rhv.z(),w) ;
            }

            //************************************************************************************
            vector4( vec2_t const & xy, vec2_t const & zw ) 
            {
                (*this)(xy.x(),xy.y(),zw.x(),zw.y()) ;
            }

            //************************************************************************************
            vector4( float_t const x, vec3_t const & yzw )
            {
                ( *this )( x, yzw.x(), yzw.y(), yzw.z() ) ;
            }

            //************************************************************************************
            vector4( float_t const x, float_t const y, vec2_t const & zw )
            {
                ( *this )( x, y, zw.x(), zw.y() ) ;
            }

            //************************************************************************************
            vector4( vec2_t const & xy, float_t const z, float_t const w  )
            {
                ( *this )( xy.x(), xy.y(), z, w ) ;
            }

            //************************************************************************************
            vector4( type_t s )
            {
                _elem[0] = s ;
                _elem[1] = s ;
                _elem[2] = s ;
                _elem[3] = s ;
            }

            //************************************************************************************
            template< typename O >
            vector4( vector4< O > const & rhv ) 
            {
                //(*this)(rhv.x(),rhv.y(),rhv.z()) ;
                _elem[0] = type_t(rhv.x()) ;
                _elem[1] = type_t(rhv.y()) ;
                _elem[2] = type_t(rhv.z()) ;
                _elem[3] = type_t(rhv.w()) ;
            }

        public: // x,y,z,w accessor

            //************************************************************************************
            type_cref_t x( void ) const {
                return _elem[0] ;
            }

            //************************************************************************************
            type_cref_t y( void ) const {
                return _elem[1] ;
            }

            //************************************************************************************
            type_cref_t z( void ) const {
                return _elem[2] ;
            }

            //************************************************************************************
            type_cref_t w( void ) const {
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
            vec2_t xx( void ) const 
            {
                return vec2_t( _elem[0], _elem[0] ) ;
            }

            //************************************************************************************
            vec2_t xy( void ) const 
            {
                return vec2_t( _elem[0], _elem[1] ) ;
            }

            //************************************************************************************
            vec2_t xz( void ) const 
            {
                return vec2_t( _elem[0], _elem[2] ) ;
            }

            //************************************************************************************
            vec2_t xw( void ) const 
            {
                return vec2_t( _elem[0], _elem[3] ) ;
            }
            //************************************************************************************
            vec2_t yz( void ) const 
            {
                return vec2_t( _elem[1], _elem[2] ) ;
            }
            //************************************************************************************
            vec2_t zx( void ) const 
            {
                return vec2_t( _elem[2], _elem[0] ) ;
            }
            //************************************************************************************
            vec2_t zy( void ) const 
            {
                return vec2_t( _elem[2], _elem[1] ) ;
            }
            //************************************************************************************
            vec2_t zz( void ) const 
            {
                return vec2_t( _elem[2], _elem[2] ) ;
            }
            //************************************************************************************
            vec2_t zw( void ) const 
            {
                return vec2_t( _elem[2], _elem[3] ) ;
            }


            //************************************************************************************
            this_ref_t xy( vec2_cref_t xy_ ) 
            {
                _elem[0] = xy_.x() ; 
                _elem[1] = xy_.y() ;

                return *this ;
            }

            

            //************************************************************************************
            this_ref_t zw( vec2_cref_t zw_ ) 
            {
                _elem[2] = zw_.x() ; 
                _elem[3] = zw_.y() ;

                return *this ;
            }

            //************************************************************************************
            vec3_t xyz( void ) const
            {
                return vec3_t(_elem[0], _elem[1], _elem[2] ) ;
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

        public: // operator +

            //************************************************************************************
            this_ref_t operator += ( this_cref_t rhv ){
                _elem[0]+=rhv.x(); _elem[1]+=rhv.y(); _elem[2]+=rhv.z(); _elem[3]+=rhv.w();
                return (*this) ;
            }

            //************************************************************************************
            this_t operator + ( this_cref_t rhv ) const{
                return this_t(_elem[0]+rhv.x(), _elem[1]+rhv.y(), _elem[2]+rhv.z(), _elem[3]+rhv.w()) ;
            }

            //************************************************************************************
            this_ref_t operator += ( type_cref_t rhv ){
                _elem[0]+=rhv; _elem[1]+=rhv; _elem[2]+=rhv; _elem[3]+=rhv;
                return (*this) ;
            }

            //************************************************************************************
            this_t operator + ( type_cref_t rhv ) const{
                return this_t(_elem[0]+rhv, _elem[1]+rhv, _elem[2]+rhv, _elem[3]+rhv) ;
            }

        public: // operator -

            //************************************************************************************
            this_ref_t operator -= ( this_cref_t rhv ){
                _elem[0]-=rhv.x(); _elem[1]-=rhv.y(); _elem[2]-=rhv.z(); _elem[3]-=rhv.w();
                return (*this) ;
            }

            //************************************************************************************
            this_t operator - ( this_cref_t rhv ) const{
                return this_t(_elem[0]-rhv.x(), _elem[1]-rhv.y(), _elem[2]-rhv.z(), _elem[3]-rhv.w()) ;
            }

            //************************************************************************************
            this_ref_t operator -= ( type_cref_t rhv ){
                _elem[0]-=rhv; _elem[1]-=rhv; _elem[2]-=rhv; _elem[3]-=rhv;
                return (*this) ;
            }

            //************************************************************************************
            this_t operator - ( type_cref_t rhv ) const{
                return this_t(_elem[0]-rhv, _elem[1]-rhv, _elem[2]-rhv, _elem[3]-rhv) ;
            }

            //***************************************************
            this_t operator - ( void_t ) const noexcept{
                return this_t( -_elem[0], -_elem[1], -_elem[2], -_elem[3] ) ;
            }

        public: // operator *

            //************************************************************************************
            this_ref_t operator *= ( this_cref_t rhv ){
                _elem[0]*=rhv.x(); _elem[1]*=rhv.y(); _elem[2]*=rhv.z(); _elem[3]*=rhv.w();
                return (*this) ;
            }

            //************************************************************************************
            this_t operator * ( this_cref_t rhv ) const{
                return this_t(_elem[0]*rhv.x(), _elem[1]*rhv.y(), _elem[2]*rhv.z(), _elem[3]*rhv.w()) ;
            }

            //************************************************************************************
            this_ref_t operator *= ( type_cref_t rhv ){
                _elem[0]*=rhv; _elem[1]*=rhv; _elem[2]*=rhv; _elem[3]*=rhv;
                return (*this) ;
            }

            //************************************************************************************
            this_t operator * ( type_cref_t rhv ) const{
                return this_t(_elem[0]*rhv, _elem[1]*rhv, _elem[2]*rhv, _elem[3]*rhv) ;
            }

            //************************************************************************************
            template< typename other_t >
            this_t operator * ( other_t rhv ) const{
                return this_t( type_t(_elem[0]*rhv), type_t(_elem[1]*rhv), type_t(_elem[2]*rhv), type_t(_elem[3]*rhv)) ;
            }


        public: // operator /

            //************************************************************************************
            this_ref_t operator /= ( this_cref_t rhv ){
                _elem[0]/=rhv.x(); _elem[1]/=rhv.y(); _elem[2]/=rhv.z(); _elem[3]/=rhv.w();
                return (*this) ;
            }

            //************************************************************************************
            this_t operator / ( this_cref_t rhv ) const{
                return this_t(_elem[0]/rhv.x(), _elem[1]/rhv.y(), _elem[2]/rhv.z(), _elem[3]/rhv.w() ) ;
            }

            //************************************************************************************
            this_ref_t operator /= ( type_cref_t rhv ){
                _elem[0]/=rhv; _elem[1]/=rhv; _elem[2]/=rhv; _elem[3]/=rhv;
                return (*this) ;
            }

            //************************************************************************************
            this_t operator / ( type_cref_t rhv ) const{
                return this_t(_elem[0]/rhv, _elem[1]/rhv, _elem[2]/rhv, _elem[3]/rhv ) ;
            }

        public: // operator =

            //************************************************************************************
            this_ref_t operator = ( type_t all ){
                _elem[0] = all ; _elem[1] = all ; _elem[2] = all ; _elem[3] = all ;
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t operator = ( this_cref_t rhv ){
                _elem[0] = rhv.x() ; _elem[1] = rhv.y() ; _elem[2] = rhv.z() ; _elem[3] = rhv.w() ;
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t operator = ( vec3_t const & rhv ){
                _elem[0] = rhv.x() ; _elem[1] = rhv.y() ; _elem[2] = rhv.z() ; // _elem[3] = type_t(0) ;
                return (*this) ;
            }

        public: // relational 

            /// this < rhv
            vecb_t less_than( this_cref_t rhv ) const
            {
                return vecb_t( _elem[0] < rhv.x(), _elem[1] < rhv.y(), _elem[2] < rhv.z(), _elem[3] < rhv.w() ) ;
            }

            /// this <= rhv
            vecb_t less_equal_than( this_cref_t rhv ) const
            {
                return vecb_t( _elem[0] <= rhv.x(), _elem[1] <= rhv.y(), _elem[2] <= rhv.z(), _elem[3] <= rhv.w() ) ;
            }

            /// this > rhv
            vecb_t greater_than( this_cref_t rhv ) const
            {
                return vecb_t( _elem[0] > rhv.x(), _elem[1] > rhv.y(), _elem[2] > rhv.z(), _elem[3] > rhv.w() ) ;
            }

            /// this >= rhv
            vecb_t greater_equal_than( this_cref_t rhv ) const
            {
                return vecb_t( _elem[0] >= rhv.x(), _elem[1] >= rhv.y(), _elem[2] >= rhv.z(), _elem[3] >= rhv.w() ) ;
            }

            /// this == rhv
            vecb_t equal( this_cref_t rhv ) const 
            {
                return vecb_t( _elem[0] == rhv.x(), _elem[1] == rhv.y(), _elem[2] == rhv.z(), _elem[3] == rhv.w() ) ;
            }

        public: // operator shift

            this_t operator >>( this_cref_t rhv ) const
            {
                return this_t( _elem[0]>>rhv.x(), _elem[1]>>rhv.y(), _elem[2]>>rhv.z(), _elem[3]>>rhv.w() ) ;
            }

            this_t operator <<( this_cref_t rhv ) const
            {
                return this_t( _elem[0]<<rhv.x(), _elem[1]<<rhv.y(), _elem[2]<<rhv.z(), _elem[3]<<rhv.w() ) ;
            }

        public: // operator bit-wise

            this_t operator &( this_cref_t rhv ) const
            {
                return this_t( _elem[0]&rhv.x(), _elem[1]&rhv.y(), _elem[2]&rhv.z(), _elem[3]&rhv.w() ) ;
            }

            this_t operator |( this_cref_t rhv ) const
            {
                return this_t( _elem[0]|rhv.x(), _elem[1]|rhv.y(), _elem[2]|rhv.z(), _elem[3]|rhv.w() ) ;
            }

        public: // selection

            /// true in s: select component from on_true.
            /// false in s: select component from on_false.
            this_t selected( this_cref_t on_true, this_cref_t on_false, vecb_cref_t s ) const 
            {
                return vec4_t( 
                    s.x() ? on_true.x() : on_false.x(), 
                    s.y() ? on_true.y() : on_false.y(), 
                    s.z() ? on_true.z() : on_false.z(), 
                    s.w() ? on_true.w() : on_false.w() 
                    ) ;
            }

            /// true in s: select component from this.
            /// false in s: select component from on_false.
            this_t select( this_cref_t on_false, vecb_cref_t s ) 
            {
                _elem[0] = s.x() ? _elem[0] : on_false.x() ;
                _elem[1] = s.y() ? _elem[1] : on_false.y() ;
                _elem[2] = s.z() ? _elem[2] : on_false.z() ;
                _elem[3] = s.w() ? _elem[3] : on_false.w() ;
            
                return *this ;
            }

        public: // common


            //************************************************************************************
            type_t dot( this_cref_t rhv ) const
            {
                return _elem[0]*rhv.x()+_elem[1]*rhv.y()+_elem[2]*rhv.z()+_elem[3]*rhv.w();
            }

            //************************************************************************************
            this_ref_t normalize( void )
            {
                type_t len = this->length() ;
                if( std::abs(len) > std::numeric_limits<type_t>::epsilon() ) (*this) /= len ;
                return (*this) ;
            }

            //************************************************************************************
            this_t normalized( void ) const
            {
                this_t v( *this );
                v.normalize() ;
                return v ;
            }
        
            //************************************************************************************
            type_t length( void ) const 
            {
                return sqrt(length2()) ; ;
            }

            //************************************************************************************
            /// Retruns the length squared: length^2. For short, it is not using the sqrt function.
            type_t length2( void ) const 
            {
                return (_elem[0]*_elem[0]+_elem[1]*_elem[1]+_elem[2]*_elem[2]+_elem[3]*_elem[3]) ;
            }

            //************************************************************************************
            this_ref_t negate( void )
            {
                _elem[0] = -_elem[0] ;_elem[1] = -_elem[1] ;_elem[2] = -_elem[2] ; _elem[3] = -_elem[3] ;
                return (*this) ;
            }

            //************************************************************************************
            this_t negated( void ) const
            {
                return this_t( *this ).negate() ;
            }

            //************************************************************************************
            this_ref_t xyz_normalize( void )
            {
                type_t w = _elem[3] ;
                this_t::normalize() ;
                _elem[3] = w ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t abs( void )
            {
                _elem[0] = motor::math::fn<type_t>::abs(_elem[0]) ;
                _elem[1] = motor::math::fn<type_t>::abs(_elem[1]) ;
                _elem[2] = motor::math::fn<type_t>::abs(_elem[2]) ;
                _elem[3] = motor::math::fn<type_t>::abs(_elem[3]) ;
                return *this ;
            }

            //************************************************************************************
            this_t absed( void ) const
            {
                return this_t(*this).abs() ;
            }

        private:

            type_t _elem[4] ;

        } ;

        motor_typedefs( vector4< char >, vec4c ) ;
        motor_typedefs( vector4< unsigned char >, vec4uc ) ;
        motor_typedefs( vector4< int >, vec4i ) ;
        motor_typedefs( vector4< float >, vec4f ) ;
        motor_typedefs( vector4< double >, vec4d ) ;
        motor_typedefs( vector4< uint_t >, vec4ui ) ;
        motor_typedefs( vector4< ushort_t>, vec4us ) ;
    }
}
