#pragma once

#include "../typedefs.h"

#include "protos.hpp"
#include "../quaternion/protos.hpp"
#include "../utility/fn.hpp"

#include "vector3b.hpp"

#include <limits>

namespace motor 
{
    namespace math
    {
        template< typename T >
        class vector3
        {
            motor_this_typedefs( vector3< T > ) ;

        public:

            motor_typedefs( T, type ) ;
            motor_typedefs( vector2<T>, vec2 ) ;
            motor_typedefs( vector3<T>, vec3 ) ;
            motor_typedefs( vector4<T>, vec4 ) ;
            motor_typedefs( vector3b, vec3b ) ;
            motor_typedefs( quaternion3< T >, quat3 ) ;
            motor_typedefs( quaternion4< T >, quat4 ) ;

        public:

            enum class component
            {
                x, y, z
            };

        public: // ctor

            //************************************************************************************
            vector3( void )
            {
                _elem[0] = type_t( 0 ) ;
                _elem[1] = type_t( 0 ) ;
                _elem[2] = type_t( 0 ) ;
            }

            //************************************************************************************
            vector3( type_t x, type_t y, type_t z )
            {
                _elem[0] = x ;
                _elem[1] = y ;
                _elem[2] = z ;
            }

            //************************************************************************************
            vector3( type_t s )
            {
                _elem[0] = s ;
                _elem[1] = s ;
                _elem[2] = s ;
            }

            //************************************************************************************
            vector3( this_cref_t rhv )
            {
                _elem[0] = rhv.x( ) ;
                _elem[1] = rhv.y( ) ;
                _elem[2] = rhv.z( ) ;
            }

            //************************************************************************************
            template< typename O >
            vector3( vector3< O > const & rhv )
            {
                _elem[0] = type_t( rhv.x( ) ) ;
                _elem[1] = type_t( rhv.y( ) ) ;
                _elem[2] = type_t( rhv.z( ) ) ;
            }

            //************************************************************************************
            vector3( vec2_in_t rhv, type_t z )
            {
                (*this)(rhv.x( ), rhv.y( ), z) ;
            }

            //************************************************************************************
            vector3( vec4_in_t rhv )
            {
                (*this)(rhv.x( ), rhv.y( ), rhv.z( )) ;
            }

            //************************************************************************************
            vector3( quat3_in_t rhv )
            {
                _elem[0] = rhv.I( ) ;
                _elem[1] = rhv.J( ) ;
                _elem[2] = rhv.K( ) ;
            }

            //************************************************************************************
            vector3( quat4_in_t rhv )
            {
                (*this) = rhv.vec( ) ;
            }

        public: // x,y,z accessor

            //************************************************************************************
            type_t x( void ) const
            {
                return _elem[0] ;
            }

            //************************************************************************************
            type_t y( void ) const
            {
                return _elem[1] ;
            }

            //************************************************************************************
            type_t z( void ) const
            {
                return _elem[2] ;
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
            type_ref_t z( void )
            {
                return _elem[2] ;
            }

            //************************************************************************************
            this_ref_t x( type_cref_t val )
            {
                _elem[0] = val ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t y( type_cref_t val )
            {
                _elem[1] = val ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t z( type_cref_t val )
            {
                _elem[2] = val ;
                return *this ;
            }

            //************************************************************************************
            void xy( vec2_out_t vout ) const
            {
                vout.x( _elem[0] ) ;
                vout.y( _elem[1] ) ;
            }

            //************************************************************************************
            void yz( vec2_out_t vout ) const
            {
                vout.x( _elem[1] ) ;
                vout.y( _elem[2] ) ;
            }

            //************************************************************************************
            vec2_t xy( void ) const
            {
                return vec2_t( _elem[0], _elem[1] ) ;
            }

            //************************************************************************************
            vec2_t yz( void ) const
            {
                return vec2_t( _elem[1], _elem[2] ) ;
            }
    #if 0
            //************************************************************************************
            this_t swizzle( typename component x, typename component y, typename component z ) 
            {
                return this_t( _elem[size_t(x)], _elem[size_t(y)], _elem[size_t(z)] ) ;
            }
    #endif
        
        public: // operator ()

            //************************************************************************************
            void operator()(type_t all)
            {
                _elem[0] = all ;
                _elem[1] = all ;
                _elem[2] = all ;
                //return *this ;
            }

            //************************************************************************************
            inline void operator()(type_t x, type_t y, type_t z)
            {
                _elem[0] = x ;
                _elem[1] = y ;
                _elem[2] = z ;
                //return *this ;
            }

        public: // operator []

            //************************************************************************************

            type_cref_t operator[](size_t index) const
            {
                return _elem[index] ;
            }

            //************************************************************************************

            type_ref_t operator[](size_t index)
            {
                return _elem[index] ;
            }

        public: // add

            // without return value

            inline void add( this_cref_t rhv )
            {
                _elem[0] += rhv.x( ) ;
                _elem[1] += rhv.y( ) ;
                _elem[2] += rhv.z( ) ;
            }

        public: // operator +

            //************************************************************************************

            this_ref_t operator+=(this_cref_t rhv)
            {
                add( rhv ) ;
                return (*this) ;
            }

            //************************************************************************************

            this_t operator+(this_cref_t rhv) const
            {
                return this_t( _elem[0] + rhv.x( ), _elem[1] + rhv.y( ), _elem[2] + rhv.z( ) ) ;
            }

            //************************************************************************************

            this_ref_t operator+=(type_cref_t rhv)
            {
                _elem[0] += rhv ;
                _elem[1] += rhv ;
                _elem[2] += rhv ;
                return (*this) ;
            }

            //************************************************************************************

            this_t operator+(type_cref_t rhv) const
            {
                return this_t( _elem[0] + rhv, _elem[1] + rhv, _elem[2] + rhv ) ;
            }

        public: // operator -

            //************************************************************************************

            this_ref_t operator-=(this_cref_t rhv)
            {
                _elem[0] -= rhv.x( ) ;
                _elem[1] -= rhv.y( ) ;
                _elem[2] -= rhv.z( ) ;
                return (*this) ;
            }

            //************************************************************************************

            this_t operator-(this_cref_t rhv) const
            {
                return this_t( _elem[0] - rhv.x( ), _elem[1] - rhv.y( ), _elem[2] - rhv.z( ) ) ;
            }

            //************************************************************************************

            this_ref_t operator-=(type_cref_t rhv)
            {
                _elem[0] -= rhv ;
                _elem[1] -= rhv ;
                _elem[2] -= rhv ;
                return (*this) ;
            }

            //************************************************************************************

            this_t operator-(type_cref_t rhv) const
            {
                return this_t( _elem[0] - rhv, _elem[1] - rhv, _elem[2] - rhv ) ;
            }

            //***************************************************
            this_t operator - ( void_t ) const noexcept{
                return this_t( -_elem[0], -_elem[1], -_elem[2] ) ;
            }

        public: // mult

            inline void mult( type_t value )
            {
                _elem[0] *= value ;
                _elem[1] *= value ;
                _elem[2] *= value ;
            }

        public: // operator *

            //************************************************************************************

            this_ref_t operator*=(this_cref_t rhv)
            {
                _elem[0] *= rhv.x( ) ;
                _elem[1] *= rhv.y( ) ;
                _elem[2] *= rhv.z( ) ;
                return (*this) ;
            }

            //************************************************************************************

            this_t operator*(this_cref_t rhv) const
            {
                return this_t( _elem[0] * rhv.x( ), _elem[1] * rhv.y( ), _elem[2] * rhv.z( ) ) ;
            }

            //************************************************************************************

            inline this_ref_t operator*=(type_cref_t rhv)
            {
                mult( rhv ) ;
                return (*this) ;
            }

            //************************************************************************************

            inline this_t operator*(type_cref_t rhv) const
            {
                return this_t( _elem[0] * rhv, _elem[1] * rhv, _elem[2] * rhv ) ;
            }

            //************************************************************************************

            template< typename other_t >
            inline this_t operator*(other_t rhv) const
            {
                return this_t( type_t( _elem[0] * rhv ), type_t( _elem[1] * rhv ), type_t( _elem[2] * rhv ) ) ;
            }


        public: // operator /

            //************************************************************************************

            this_ref_t operator/=(this_cref_t rhv)
            {
                _elem[0] /= rhv.x( ) ;
                _elem[1] /= rhv.y( ) ;
                _elem[2] /= rhv.z( ) ;
                return (*this) ;
            }

            //************************************************************************************

            this_t operator/(this_cref_t rhv) const
            {
                return this_t( _elem[0] / rhv.x( ), _elem[1] / rhv.y( ), _elem[2] / rhv.z( ) ) ;
            }

            //************************************************************************************

            this_ref_t operator/=(type_cref_t rhv)
            {
                _elem[0] /= rhv ;
                _elem[1] /= rhv ;
                _elem[2] /= rhv ;
                return (*this) ;
            }

            //************************************************************************************

            this_t operator/(type_cref_t rhv) const
            {
                return this_t( _elem[0] / rhv, _elem[1] / rhv, _elem[2] / rhv ) ;
            }

        public: // operator =

            //************************************************************************************

            this_ref_t operator=(this_cref_t rhv)
            {
                _elem[0] = rhv.x( ) ;
                _elem[1] = rhv.y( ) ;
                _elem[2] = rhv.z( ) ;
                return (*this) ;
            }

            //************************************************************************************

            this_ref_t operator=( vec4_t const & rhv)
            {
                _elem[0] = rhv.x( ) ;
                _elem[1] = rhv.y( ) ;
                _elem[2] = rhv.z( ) ;
                return (*this) ;
            }

        public: // relational 

            /// this < rhv

            vec3b_t less_than( this_cref_t rhv ) const
            {
                return vec3b_t( _elem[0] < rhv.x( ), _elem[1] < rhv.y( ), _elem[2] < rhv.z( ) ) ;
            }

            /// this <= rhv

            vec3b_t less_equal_than( this_cref_t rhv ) const
            {
                return vec3b_t( _elem[0] <= rhv.x( ), _elem[1] <= rhv.y( ), _elem[2] <= rhv.z( ) ) ;
            }

            /// this > rhv

            vec3b_t greater_than( this_cref_t rhv ) const
            {
                return vec3b_t( _elem[0] > rhv.x( ), _elem[1] > rhv.y( ), _elem[2] > rhv.z( ) ) ;
            }

            /// this >= rhv

            vec3b_t greater_equal_than( this_cref_t rhv ) const
            {
                return vec3b_t( _elem[0] >= rhv.x( ), _elem[1] >= rhv.y( ), _elem[2] >= rhv.z( ) ) ;
            }

        public: // operator shift

            this_t operator>>(this_cref_t rhv) const
            {
                return this_t( _elem[0] >> rhv.x( ), _elem[1] >> rhv.y( ), _elem[2] >> rhv.z( ) ) ;
            }

            this_t operator<<(this_cref_t rhv) const
            {
                return this_t( _elem[0] << rhv.x( ), _elem[1] << rhv.y( ), _elem[2] << rhv.z( ) ) ;
            }

        public: // operator bit-wise

            this_t operator&(this_cref_t rhv) const
            {
                return this_t( _elem[0] & rhv.x( ), _elem[1] & rhv.y( ), _elem[2] & rhv.z( ) ) ;
            }

            this_t operator|(this_cref_t rhv) const
            {
                return this_t( _elem[0] | rhv.x( ), _elem[1] | rhv.y( ), _elem[2] | rhv.z( ) ) ;
            }

        public: // common

            //************************************************************************************

            inline type_t dot( this_cref_t rhv ) const
            {
                return _elem[0] * rhv.x( ) + _elem[1] * rhv.y( ) + _elem[2] * rhv.z( ) ;
            }

            //************************************************************************************

            type_t dot( vec4_t const & rhv ) const
            {
                return _elem[0] * rhv.x( ) + _elem[1] * rhv.y( ) + _elem[2] * rhv.z( ) ;
            }

            //************************************************************************************

            this_ref_t cross( this_cref_t rhv )
            {
                const type_t x = this_t::y( ) * rhv.z( ) - this_t::z( ) * rhv.y( ) ;
                const type_t y = this_t::z( ) * rhv.x( ) - this_t::x( ) * rhv.z( ) ;
                const type_t z = this_t::x( ) * rhv.y( ) - this_t::y( ) * rhv.x( ) ;

                _elem[0] = x ;
                _elem[1] = y ;
                _elem[2] = z ;

                return *this ;
            }

            //************************************************************************************

            this_t crossed( this_cref_t rhv ) const
            {
                return this_t( *this ).cross( rhv ) ;
            }

            //************************************************************************************

            this_ref_t normalize( void )
            {
                type_t len = this->length( ) ;
                if( std::abs( len ) > std::numeric_limits<type_t>::epsilon( ) ) ( *this ) /= len ;
                return (*this) ;
            }

            //************************************************************************************

            this_t normalized( void ) const
            {
                this_t vec( *this ) ;
                vec.normalize( ) ;
                return vec ;
            }

            //************************************************************************************

            type_t length( void ) const
            {
                return sqrt( length2( ) ) ;
            }

            //************************************************************************************

            type_t length2( void ) const
            {
                return (_elem[0] * _elem[0] + _elem[1] * _elem[1] + _elem[2] * _elem[2]) ;
            }

            //************************************************************************************

            this_ref_t negate( void )
            {
                _elem[0] = -_elem[0] ;
                _elem[1] = -_elem[1] ;
                _elem[2] = -_elem[2] ;
                return (*this) ;
            }

            //************************************************************************************

            this_t negated( void ) const
            {
                this_t v( *this ) ;
                return v.negate( ) ;
            }

            //************************************************************************************

            this_ref_t abs( void )
            {
                _elem[0] = motor::math::fn<type_t>::abs( _elem[0] ) ;
                _elem[1] = motor::math::fn<type_t>::abs( _elem[1] ) ;
                _elem[2] = motor::math::fn<type_t>::abs( _elem[2] ) ;
                return *this ;
            }

            //************************************************************************************

            this_t absed( void ) const
            {
                return this_t( *this ).abs( ) ;
            }

            //************************************************************************************
            this_t ortho_any( void_t ) const 
            {
                return this_t(
                    this_t::x(), -this_t::z(), this_t::y() ) ;
            }

            //************************************************************************************
            static this_t make_x_axis( void_t ) noexcept
            {
                return this_t(type_t(1),type_t(0), type_t(0)) ;
            }

            //************************************************************************************
            static this_t make_y_axis( void_t ) noexcept
            {
                return this_t(type_t(0),type_t(1), type_t(0)) ;
            }

            //************************************************************************************
            static this_t make_z_axis( void_t ) noexcept
            {
                return this_t(type_t(0),type_t(0), type_t(1)) ;
            }

        private:

            type_t _elem[3] ;

        } ;

        motor_typedefs( vector3<char_t>, vec3c ) ;
        motor_typedefs( vector3<uchar_t>, vec3uc ) ;
        motor_typedefs( vector3<int_t>, vec3i ) ;
        motor_typedefs( vector3<uint_t>, vec3ui ) ;
        motor_typedefs( vector3<ushort_t>, vec3us ) ;
        motor_typedefs( vector3<float_t>, vec3f ) ;
        motor_typedefs( vector3<double_t>, vec3d ) ;

        template< typename T >
        static vector3< T > operator * ( float_t const f, vector3< T > const & o ) noexcept
        {
            return o * f ;
        }
    }
}
