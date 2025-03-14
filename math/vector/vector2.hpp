#pragma once

#include "protos.hpp"

#include "../typedefs.h"
#include "../utility/fn.hpp"
#include "../utility/constants.hpp"

#include "vector2b.hpp"

#include <algorithm>
#include <cmath>

namespace motor 
{
    namespace math
    {
        template< typename T >
        class vector2
        {
            motor_this_typedefs( vector2< T > ) ;


        private:

                T _elem[2] ;

        public:

            motor_typedefs( T, type ) ;

            motor_typedefs( vector2<type_t>, vec2 ) ;
            motor_typedefs( vector3<type_t>, vec3 ) ;
            motor_typedefs( vector4<type_t>, vec4 ) ;

            motor_typedefs( vector2b, vecb ) ;

        public:

            //***************************************************
            vector2( void ) noexcept : _elem{type_t(0),type_t(0) } {}

            //***************************************************
            vector2( type_t const s ) noexcept : _elem{ s, s } {}

            //***************************************************
            vector2( type_t x, type_t y ) noexcept : _elem{x, y}{}

            //***************************************************
            vector2( this_cref_t rhv )  noexcept : 
                _elem{ rhv._elem[0], rhv._elem[1] } {}

            //***************************************************
            template< typename other_t >
            vector2( vector2<other_t> const & rhv ) noexcept : 
                _elem{ type_t(rhv.x()), type_t(rhv.y()) } {}

            //***************************************************
            explicit vector2( vec3_t const & rhv )  noexcept : 
                _elem{ rhv.x(), rhv.y() }{}

            //***************************************************
            explicit vector2( vec4_t const & rhv )  noexcept :
                _elem{ rhv.x(), rhv.y() }{}

        public: // x,y,z accessor

            //***************************************************
            type_cref_t x( void ) const noexcept
            {
                return _elem[0] ;
            }

            //***************************************************
            type_cref_t y( void ) const noexcept
            {
                return _elem[1] ;
            }

            //***************************************************
            type_ref_t x( void ) noexcept
            {
                return _elem[0] ;
            }

            //***************************************************
            type_ref_t y( void )  noexcept
            {
                return _elem[1] ;
            }

            //***************************************************
            this_ref_t x( type_cref_t val ) noexcept
            {
                _elem[0] = val ;
                return *this ;
            }

            //***************************************************
            this_ref_t y( type_cref_t val ) noexcept
            {
                _elem[1] = val ;
                return *this ;
            }

            //***************************************************
            this_t xx( void_t ) const noexcept
            {
                return this_t( _elem[ 0 ], _elem[ 0 ] ) ;
            }

            //***************************************************
            this_t yy( void_t ) const noexcept 
            {
                return this_t( _elem[ 1 ], _elem[ 1 ] ) ;
            }

            //***************************************************
            this_t yx( void_t ) const noexcept 
            {
                return this_t( _elem[ 1 ], _elem[ 0 ] ) ;
            }

        public: // operator ()

            //***************************************************
            this_cref_t operator()( void ) noexcept
            {
                _elem[0] = type_t(0);_elem[1] = type_t(0);
                return (*this) ;
            }

            //***************************************************
            this_cref_t operator()( type_t x, type_t y ) noexcept
            {
                _elem[0]=x; _elem[1]=y;
                return *this ;
            }

        public: // operator [] // optimized

            //***************************************************
            type_cref_t operator[]( size_t index ) const
             noexcept{
                return _elem[index] ;
            }

            //***************************************************
            type_ref_t operator[]( size_t index )  noexcept
            {
                return _elem[index] ;
            }

        public: // operator +

            //***************************************************
            this_ref_t operator += ( this_cref_t rhv ) noexcept
            {
                _elem[0]+=rhv.x(); _elem[1]+=rhv.y(); 
                return (*this) ;
            }

            //***************************************************
            this_t operator + ( this_cref_t rhv ) const noexcept 
            {
                return this_t(_elem[0]+rhv.x(), _elem[1]+rhv.y()) ;
            }

            //***************************************************
            this_t operator + ( void_t ) const noexcept
            {
                return this_t( _elem[0], _elem[1] ) ;
            }

        public: // operator -

            //***************************************************
            this_cref_t operator -= ( this_cref_t rhv ) noexcept
            {
                _elem[0]-=rhv.x(); _elem[1]-=rhv.y(); 
                return (*this) ;
            }

            //***************************************************
            this_t operator - ( this_cref_t rhv ) const noexcept
            {
                return this_t(_elem[0]-rhv.x(), _elem[1]-rhv.y()) ;
            }

            //***************************************************
            this_cref_t operator -= ( type_cref_t rhv ) noexcept
            {
                _elem[0]-=rhv; _elem[1]-=rhv; 
                return (*this) ;
            }

            //***************************************************
            this_t operator - ( type_cref_t rhv ) const noexcept
            {
                return this_t(_elem[0]-rhv, _elem[1]-rhv) ;
            }

            //***************************************************
            this_t operator - ( void_t ) const noexcept{
                return this_t( -_elem[0], -_elem[1] ) ;
            }

        public: // mult

            inline void mult( type_t value ) noexcept
            {
                _elem[0]*=value; _elem[1]*=value; _elem[2]*=value;
            }

        public: // operator *

            //***************************************************
            this_cref_t operator *= ( this_cref_t rhv ) noexcept
            {
                _elem[0]*=rhv.x(); _elem[1]*=rhv.y();
                return (*this) ;
            }

            //***************************************************
            this_t operator * ( this_cref_t rhv ) const noexcept
            {
                return this_t(_elem[0]*rhv.x(), _elem[1]*rhv.y()) ;
            }

            //***************************************************
            this_cref_t operator *= ( type_cref_t rhv ) noexcept
            {
                _elem[0]*=rhv; _elem[1]*=rhv;
                return (*this) ;
            }

            //***************************************************
            this_t operator * ( type_cref_t rhv ) const noexcept
            {
                return this_t(_elem[0]*rhv, _elem[1]*rhv) ;
            }

            //***************************************************
            template< typename other_t >
            this_t operator * ( other_t rhv ) const noexcept
            {
                return this_t( type_t(_elem[0]*rhv), type_t(_elem[1]*rhv) ) ;
            }

            //***************************************************
            template< typename other_t >
            this_t operator * ( vector2<other_t> rhv ) const noexcept
            {
                return this_t( type_t(_elem[0]*rhv.x()), type_t(_elem[1]*rhv.y()) ) ;
            }

        public: // operator /

            //***************************************************
            this_ref_t operator /= ( this_cref_t rhv ) noexcept
            {
                _elem[0]/=rhv.x(); _elem[1]/=rhv.y(); 
                return (*this) ;
            }

            //***************************************************
            this_t operator / ( this_cref_t rhv ) const noexcept
            {
                return this_t(_elem[0]/rhv.x(), _elem[1]/rhv.y()) ;
            }

            //***************************************************
            this_ref_t operator /= ( type_cref_t rhv ) noexcept
            {
                _elem[0]/=rhv; _elem[1]/=rhv; 
                return (*this) ;
            }

            //***************************************************
            this_t operator / ( type_cref_t rhv ) const noexcept
            {
                return this_t(_elem[0]/rhv, _elem[1]/rhv) ;
            }

        public: // operator %

            //***************************************************
            this_t operator % ( this_cref_t rhv ) const noexcept
            {
                return this_t(_elem[0]%rhv.x(), _elem[1]%rhv.y()) ;
            }

        public: // operator =

            //***************************************************
            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _elem[0] = rhv.x() ; _elem[1] = rhv.y() ; 
                return (*this) ;
            }

            //***************************************************
            this_ref_t operator = ( vec4_t const & rhv ) noexcept
            {
                _elem[0] = rhv.x() ; _elem[1] = rhv.y() ; 
                return (*this) ;
            }

        public: // bitwise

            this_t operator &( this_cref_t rhv ) const noexcept
            {
                return this_t( _elem[0]&rhv.x(), _elem[1]&rhv.y() ) ;
            }

            this_t operator |( this_cref_t rhv ) const noexcept
            {
                return this_t( _elem[0]|rhv.x(), _elem[1]|rhv.y() ) ;
            }
        
            this_t operator <<( this_cref_t rhv ) const noexcept 
            {
                return this_t( _elem[0]<<rhv.x(), _elem[1]<<rhv.y() ) ;
            }

            this_t operator >>( this_cref_t rhv ) const noexcept
            {
                return this_t( _elem[0]>>rhv.x(), _elem[1]>>rhv.y() ) ;
            }

        public: // relational 

            /// this < rhv
            vecb_t less_than( this_cref_t rhv ) const noexcept
            {
                return vecb_t( _elem[0] < rhv.x(), _elem[1] < rhv.y() ) ;
            }

            /// this <= rhv
            vecb_t less_equal_than( this_cref_t rhv ) const noexcept
            {
                return vecb_t( _elem[0] <= rhv.x(), _elem[1] <= rhv.y() ) ;
            }

            /// this > rhv
            vecb_t greater_than( this_cref_t rhv ) const noexcept
            {
                return vecb_t( _elem[0] > rhv.x(), _elem[1] > rhv.y() ) ;
            }

            /// this >= rhv
            vecb_t greater_equal_than( this_cref_t rhv ) const noexcept
            {
                return vecb_t( _elem[0] >= rhv.x(), _elem[1] >= rhv.y() ) ;
            }

            /// this == rhv integer
            vecb_t equal( this_cref_t rhv ) const noexcept
            {
                return vecb_t( _elem[ 0 ] == rhv.x(), _elem[ 1 ] == rhv.y() ) ;
            }

        public: // min/max

            /// component-wise max operation
            /// returns max of this or rhv.
            this_ref_t max( this_cref_t rhv ) noexcept
            {
                _elem[0] = rhv._elem[0] > _elem[0] ? rhv._elem[0] : _elem[0] ;
                _elem[1] = rhv._elem[1] > _elem[1] ? rhv._elem[1] : _elem[1] ;
                return *this ;
            }

            this_t max_ed( this_cref_t rhv ) const noexcept 
            {
                return this_t(this_t(*this).max( rhv )) ;
            }

            /// component-wise min operation
            this_ref_t min( this_cref_t rhv ) noexcept 
            {
                _elem[0] = rhv._elem[0] < _elem[0] ? rhv._elem[0] : _elem[0] ;
                _elem[1] = rhv._elem[1] < _elem[1] ? rhv._elem[1] : _elem[1] ;
                return *this ;
            }

            this_t min_ed( this_cref_t rhv ) const noexcept 
            {
                return this_t(this_t(*this).min( rhv )) ;
            }

        public: // common

            //***************************************************
            type_t dot( this_cref_t rhv ) const noexcept
            {
                return _elem[0]*rhv.x()+_elem[1]*rhv.y();
            }

            //***************************************************
            type_t det( this_cref_t rhv ) const noexcept
            {
                return _elem[0] * rhv._elem[1] - _elem[1] * rhv._elem[0] ;
            }

            //***************************************************
            this_ref_t normalize( float_t & len ) noexcept {
                len = this_t::length() ;
                return ( std::abs(len) > std::numeric_limits<type_t>::epsilon() ) ? (*this) /= len : *this ;
            }

            //***************************************************
            this_t normalized( float_t & len ) const noexcept {
                return this_t( *this ).normalize( len ) ;
            } 

            //***************************************************
            this_ref_t normalize( void ) noexcept {
                type_t len ;
                return this_t::normalize( len ) ;
            }

            //***************************************************
            this_t normalized( void ) const noexcept {
                this_t vec( *this ) ;
                vec.normalize() ;
                return vec ;
            } 
        
            //***************************************************
            type_t length2( void ) const noexcept {
                return _elem[0]*_elem[0]+_elem[1]*_elem[1] ;
            }

            //***************************************************
            type_t length( void ) const  noexcept
            {
                return motor::math::fn<type_t>::sqrt( this_t::length2() ) ;
            }

            //***************************************************
            this_ref_t negate( void ) noexcept
            {
                _elem[0] = -_elem[0] ;_elem[1] = -_elem[1] ;
                return ( *this ) ;
            }

            //***************************************************
            this_t negated( void ) const noexcept
            {
                this_t v(*this) ;
                return v.negate() ;
            }

            //***************************************************
            this_t sign( void ) const noexcept
            {
                return this_t( motor::math::fn<type_t>::sign(_elem[0]), 
                    motor::math::fn<type_t>::sign(_elem[1]) ) ;
            }

            //***************************************************
            this_ref_t abs( void ) noexcept
            {
                _elem[0] = type_t(std::abs(_elem[0])) ;
                _elem[1] = motor::math::fn<type_t>::abs(_elem[1]) ;
                return *this ;
            }

            //***************************************************
            this_t absed( void ) const noexcept
            {
                return this_t(*this).abs() ;
            }

            /// dont use
            /// sets the values of this to tyep_t(0) if the values are lower
            /// than the dead zone threshold.
            /// @return operates on this object. returns this.
            this_ref_t dead_zone( type_t threshold ) noexcept
            {
                _elem[0] = std::abs(_elem[0]) < threshold ? type_t(0) : _elem[0]  ;
                _elem[1] = std::abs(_elem[1]) < threshold ? type_t(0) : _elem[1]  ;
                return *this ;
            }

            /// dont use
            /// sets the values of this to tyep_t(0) if the values are lower
            /// than the dead zone threshold.
            /// @return returns a new object.
            this_t dead_zoned( type_t threshold ) const
            {
                return this_t(*this).dead_zone( threshold ) ;
            }

            //***************************************************
            this_ref_t clamp( this_cref_t min_val, this_cref_t max_val ) noexcept 
            {
                _elem[0] = std::min( std::max(_elem[0], min_val.x()), max_val.x()) ;
                _elem[1] = std::min( std::max(_elem[1], min_val.y()), max_val.y()) ;

                return *this ;
            }

            //***************************************************
            this_t clamped( this_cref_t min_val, this_cref_t max_val ) const noexcept
            {
                return this_t(*this).clamp( min_val, max_val ) ;
            }

            //***************************************************
            this_ref_t clamp( type_t min_val, type_t max_val )  noexcept
            {
                _elem[0] = std::min( std::max(_elem[0], min_val), max_val) ;
                _elem[1] = std::min( std::max(_elem[1], min_val), max_val) ;

                return *this ;
            }

            //***************************************************
            this_t clamped( type_t min_val, type_t max_val ) const noexcept 
            {
                return this_t(*this).clamp( min_val, max_val ) ;
            }

            //***************************************************
            type_t cross_as_scalar( this_cref_t other ) const noexcept 
            {
                return this_t::x() * other.y() - this_t::y() * other.x() ;
            }

            //***************************************************
            vec3_t crossed( this_cref_t other ) const  noexcept
            {
                return vec3_t( type_t(0), type_t(0), this_t::cross_as_scalar(other) ) ;
            }

            // return this_t( dot, cross.z() )
            this_t dot_cross( this_cref_t o ) const noexcept
            {
                return this_t( this_t::dot(o), this_t::x() * o.y() - this_t::y() * o.x() ) ;
            }

            //***************************************************
            vec2_ref_t fract( void_t ) noexcept
            {
                _elem[ 0 ] = motor::math::fn<type_t>::fract( _elem[ 0 ] ) ;
                _elem[ 1 ] = motor::math::fn<type_t>::fract( _elem[ 1 ] ) ;

                return *this ;
            }

            //***************************************************
            vec2_t fracted( void_t ) const noexcept
            {
                return this_t(
                    motor::math::fn<type_t>::fract( _elem[ 0 ] ),
                    motor::math::fn<type_t>::fract( _elem[ 1 ] ) );
            }

            //***************************************************
            // modulo self with rhv and return this
            this_ref_t mod( this_cref_t rhv ) noexcept
            {
                _elem[0] = motor::math::fn< type_t >::mod( _elem[0], rhv.x() ) ;
                _elem[1] = motor::math::fn< type_t >::mod( _elem[1], rhv.y() ) ;
            }

            //***************************************************
            // modulo with rhv and return result vector
            this_t modd( this_cref_t rhv ) noexcept
            {
                return this_t(
                    motor::math::fn< type_t >::mod( _elem[0], rhv.x() ),
                    motor::math::fn< type_t >::mod( _elem[1], rhv.y() )
                ) ;
            }

            //***************************************************
            vec2_ref_t floor( void_t ) noexcept 
            {
                _elem[ 0 ] = std::floor( _elem[ 0 ] ) ;
                _elem[ 1 ] = std::floor( _elem[ 1 ] ) ;
                return *this ;
            }

            //***************************************************
            vec2_t floored( void_t ) const noexcept
            {
                return this_t(
                    std::floor( _elem[ 0 ] ),
                    std::floor( _elem[ 1 ] ) ) ;
            }

            //***************************************************
            vec2_ref_t ceil( void_t ) noexcept
            {
                _elem[ 0 ] = std::ceil( _elem[ 0 ] ) ;
                _elem[ 1 ] = std::ceil( _elem[ 1 ] ) ;
                return *this ;
            }

            //***************************************************
            vec2_t ceiled( void_t ) const noexcept
            {
                return this_t( std::ceil( _elem[ 0 ] ), std::ceil( _elem[ 1 ] ) ) ;
            }

            //***************************************************
            vec2_t ortho( void_t ) const noexcept
            {
                return this_t( _elem[1], -_elem[0] ) ;
            }

            //***************************************************
            vec2_t ortho_right( void_t ) const noexcept
            {
                return this_t( _elem[1], -_elem[0] ) ;
            }

            //***************************************************
            vec2_t ortho_left( void_t ) const noexcept
            {
                return this_t( -_elem[1], _elem[0] ) ;
            }

            //***************************************************
            this_t cos_sin( type_t const angle ) noexcept
            {
                return this_t( std::cos( angle ), std::sin( angle ) ) ;
            }

            //***************************************************
            this_t sin_cos( type_t const angle ) noexcept
            {
                return this_t( std::sin( angle ), std::cos( angle ) ) ;
            }
        } ;

        motor_typedefs( vector2< char_t >, vec2c ) ;
        motor_typedefs( vector2< uchar_t >, vec2uc ) ;
        motor_typedefs( vector2< int_t >, vec2i ) ;
        motor_typedefs( vector2< int16_t >, vec2i16 ) ;
        motor_typedefs( vector2< float_t >, vec2f ) ;
        motor_typedefs( vector2< double_t >, vec2d ) ;
        motor_typedefs( vector2< uint_t >, vec2ui ) ;
        motor_typedefs( vector2< ushort_t >, vec2us ) ;

        template< typename T >
        static vector2< T > operator * ( float_t const f, vector2< T > const & o ) noexcept
        {
            return o * f ;
        }

        template< typename T >
        class vector2e{ };

        // @todo make this available for any floating point precision.
        template<>
        class vector2e<float_t>
        { 
        public:

            motor_typedefs( float_t, type ) ;
            motor_this_typedefs( vector2e< type_t > ) ;
            motor_typedefs( motor::math::vector2< type_t >, vec2 ) ;

            // transforms this vector from E {-1,1} to E {0,1}
            // e.g. -1 -> 0, 0 -> 0.5, 1 -> 1
            static vec2_t minus_one_to_one( this_t::vec2_cref_t a ) noexcept
            {
                return a * vec2_t( 0.5f ) + vec2_t( 0.5f ) ;
            }

            // does the opposite of minus_on_to_one
            static vec2_t one_to_minus_one( this_t::vec2_cref_t a ) noexcept
            {
                return a * vec2_t( 2.0f ) - vec2_t( 1.0f ) ;
            }

            // returns the full angle between the vectors a and b in radians in {0, 2pi}
            // @pre a and b must be normalized vectors. this function does no normalization.
            static type_t full_angle( this_t::vec2_cref_t a, this_t::vec2_cref_t b ) noexcept
            {
                auto const dc = a.dot_cross(b) ;
                float_t const sig = vec2_t( type_t(1), dc.sign().y() )[ size_t( std::abs( dc.sign().y() ) )] ;
                uint_t const idx = 1 - uint_t( motor::math::fn<float_t>::nnv_to_pnv( sig ) ) ;
                return std::acos( dc.x() ) * sig + motor::math::vec2f_t( 0, motor::math::constants<float_t>::pix2() )[ idx ] ;
            }

            // reflects the vector d on surface normal n.
            // @pre n must be normalized
            static vec2_t reflect( vec2_cref_t n, vec2_cref_t d ) noexcept
            {
                return d - 2.0f * n * n.dot( d ) ;
            }
        };

        motor_typedefs( vector2e< float_t >, vec2fe ) ;
        
    }
}
