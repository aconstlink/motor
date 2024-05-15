#pragma once

#include "protos.hpp"

#include "../typedefs.h"
#include "../vector/protos.hpp"
#include "../vector/vector3.hpp"
#include "../utility/fn.hpp"
#include "matrix2x3.hpp"

namespace motor
{
    namespace math
    {
        template< typename type >
        class matrix2
        {
        public: // typedefs 

            motor_this_typedefs( matrix2< type > ) ;

            motor_typedefs( type, type ) ;
            motor_typedefs( matrix2< type_t >, mat2 ) ;
            motor_typedefs( matrix3< type_t >, mat3 ) ;
            motor_typedefs( matrix4< type_t >, mat4 ) ;
            motor_typedefs( vector2< type_t >, vec2 ) ;
            motor_typedefs( vector3< type_t >, vec3 ) ;
            motor_typedefs( vector4< type_t >, vec4 ) ;

            motor_typedefs( matrix2x3< type_t >, mat23 ) ;

        public: // ctor

            //************************************************************************************
            matrix2( void ) 
            {
                for( size_t i = 0; i<4; ++i )
                    _elem[i] = type_t(0) ;
            }

            //************************************************************************************
            matrix2( this_cref_t rhv ) 
            {
                for( size_t i=0; i<4; ++i ){
                    _elem[i] = rhv[i] ;
                }
            }

            matrix2( this_rref_t rhv ) noexcept
            {
                for( size_t i=0; i<4; ++i ){
                    _elem[i] = rhv[i] ;
                }
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                for( size_t i=0; i<4; ++i ){
                    _elem[i] = rhv[i] ;
                }
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                for( size_t i=0; i<4; ++i ){
                    _elem[i] = rhv[i] ;
                }
                return *this ;
            }

            //************************************************************************************
            matrix2( type_t s ) 
            {
                for( size_t i=0; i<4; ++i ){
                    _elem[i] = s ;
                }
            }

            //************************************************************************************
            matrix2( mat4_t const & rhv )
            {
                set_column( 0, rhv.column3(0) ) ;
                set_column( 1, rhv.column3(1) ) ;
            }

        public:

            //************************************************************************************
            static this_t make_rotation_matrix( type_t cos_a, type_t sin_a ) noexcept
            {
                this_t r ;
                r.set_row( 0, vec2_t( cos_a, -sin_a ) ) ;
                r.set_row( 1, vec2_t( sin_a, cos_a ) ) ;
                return r ;
            }

            //************************************************************************************
            static this_t make_rotation_matrix( vec2_cref_t cos_sin ) noexcept
            {
                this_t ret ;
                ret.set_row( 0, vec2_t( cos_sin.x(), -cos_sin.y() ) ) ;
                ret.set_row( 1, vec2_t( cos_sin.y(), cos_sin.x() ) ) ;
                return ret ;
            }

            static this_t make_rotation_matrix( float_t const angle ) noexcept
            {
                return make_rotation_matrix( motor::math::fn<type_t>::cos(angle), 
                    motor::math::fn<type_t>::sin(angle) ) ;
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
            this_cref_t operator()( type_t rhv ) noexcept
            {
                for( size_t i=0; i<4; ++i ) _elem[i] = rhv ;
                return (*this) ;
            }

            //************************************************************************************
            type_t operator()( size_t row, size_t column ) const noexcept 
            {
                return _elem[(row<<1)+column] ;
            }

        public: // common

            //************************************************************************************
            vec2_t row( size_t i ) const noexcept
            {
                size_t const idx = i<<1 ;
                return vec2_t( _elem[idx+0],_elem[idx+1] ) ;
            }

            //************************************************************************************
            this_ref_t set_row( size_t i, vec4_cref_t v ) noexcept
            {
                const size_t start = i<<1 ;
                _elem[start] = v.x() ;
                _elem[start+1] = v.y() ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t set_row( size_t i, vec3_t const & v ) noexcept
            {
                const size_t start = i<<1 ;
                _elem[start] = v.x() ;
                _elem[start+1] = v.y() ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t set_row( size_t i, vec2_t const & v ) noexcept
            {
                const size_t start = i<<1 ;
                _elem[start] = v.x() ;
                _elem[start+1] = v.y() ;
                return *this ;
            }

            //************************************************************************************
            vec2_t column( size_t i ) const noexcept 
            { 
                return vec2_t( _elem[i], _elem[i+2] ) ; 
            }

            //************************************************************************************
            vec2_t get_column( size_t i ) const noexcept 
            { 
                return column(i) ; 
            }

            //************************************************************************************
            this_ref_t set_column( size_t i, vec2_t const & v ) noexcept 
            {
                _elem[i] = v.x();
                _elem[i+2] = v.y();
                return *this ;
            }

            //************************************************************************************
            this_ref_t zero( void_t ) noexcept
            {
                for(size_t i=0;i<4; ++i) _elem[i] = type_t(0);
                return *this ;
            }

            //************************************************************************************
            this_ref_t transpose( void )
            {
                type_t tmp = _elem[2] ;
                _elem[2] = _elem[1] ;
                _elem[1] = tmp ;

                return *this ;
            }

            //************************************************************************************
            this_t transposed( void ) const
            {
                this_t mat( *this ) ;
                return mat.transpose() ;
            }

            //************************************************************************************
            this_ref_t identity( void )
            {
                this->zero() ;
                _elem[0] = type_t(1);
                _elem[3] = type_t(1);
                return (*this) ;
            }

            //************************************************************************************
            this_t identitied( void ) const 
            {
                this_t mat( *this ) ;
                return mat.identity() ;
            }

            //************************************************************************************
            this_ref_t scale( type_t rhv )
            {
                _elem[0] *= rhv ;
                _elem[3] *= rhv ;
                return (*this) ;	
            }

            //************************************************************************************
            this_t scaled( type_t rhv ) const
            {
                this_t mat( *this ) ;
                return mat.scale( rhv ) ;
            }

            //************************************************************************************
            this_ref_t scale( type_t sx, type_t sy )
            {
                _elem[0] *= sx ;
                _elem[3] *= sy ;
                return (*this) ;	
            }

            //************************************************************************************
            this_t scaled( type_t sx, type_t sy ) const 
            {
                this_t mat( *this ) ;
                return mat.scale( sx, sy ) ;
            }

            //************************************************************************************
            this_ref_t scale( vec3_t const & rhv )
            {
                return this_t::scale( rhv.x(), rhv.y() ) ;	
            }

            //************************************************************************************
            this_t scaled( vec3_t const & rhv ) const 
            {
                return this_t::scaled( rhv.x(), rhv.y() ) ;	
            }

            //************************************************************************************
            type_t trace( void ) const 
            {
                return _elem[0] + _elem[3] ;
            }

            //************************************************************************************
            type_t angle( void ) const 
            {
                return std::acos( (this_t::trace()-type_t(2))*(type_t(0.5)) ) ;
            }

        public: // operator +

            //************************************************************************************
            this_t operator + ( type_t rhv ) const 
            {
                this_t mat((*this));
                for( size_t i=0; i<4; ++i ) mat[i] += rhv ;
                return mat ;
            }

            //************************************************************************************
            this_t operator + ( this_cref_t rhv ) const 
            {
                this_t mat ;
                for( size_t i=0; i<4; ++i ) mat[i] = _elem[i] + rhv[i] ; 
                return mat ;
            }

            //************************************************************************************
            this_ref_t operator += ( type_t rhv ) 
            {
                for( size_t i=0; i<4; ++i ) _elem[i] += rhv ;
                return (*this) ;
            }

            //************************************************************************************
            this_ref_t operator += ( this_cref_t rhv )
            {
                for( size_t i=0; i<4; ++i ) _elem[i] += rhv[i] ; 
                return *this ;
            }

        public: // operator -

            //************************************************************************************
            this_t operator - ( type_t rhv ) const 
            {
                this_t mat( *this ) ;
                for( size_t i=0; i<4; ++i ) mat[i] -= rhv ;
                return mat ;
            }


            //************************************************************************************
            this_ref_t operator -= ( type_t rhv ) 
            {
                for( size_t i=0; i<4; ++i ) _elem[i] -= rhv ;
                return *this ;
            }

            //************************************************************************************
            this_ref_t operator -= ( this_cref_t rhv ) 
            {
                for( size_t i=0; i<4; ++i ) _elem[i] -= rhv[i] ; 
                return *this ;
            }

        public: // operator *

            //************************************************************************************
            this_t operator * ( type_t rhv ) const 
            {
                this_t mat( *this );
                for( size_t i=0; i<4; ++i ) mat[i] *= rhv ;
                return mat ;
            }

            //************************************************************************************
            this_cref_t operator *= ( type_t rhv ) 
            {
                for( size_t i=0; i<4; ++i ) _elem[i] *= rhv ;
                return (*this) ;
            }

            //************************************************************************************
            vec2_t operator * ( vec2_t const & rhv ) const 
            {
                vec2_t vec ;
                for( size_t i=0; i<2; ++i ){
                    vec[i] = row(i).dot( rhv ) ;
                }
                return vec ;
            }

            //************************************************************************************
            this_t operator * ( this_cref_t rhv ) const noexcept
            {
                this_t mat ;
            
                mat[0] = this_t::row(0).dot(rhv.column(0)) ;
                mat[1] = this_t::row(0).dot(rhv.column(1)) ;
            
                mat[2] = this_t::row(1).dot(rhv.column(0)) ;
                mat[3] = this_t::row(1).dot(rhv.column(1)) ;
            

                return mat ;
            } 

            //************************************************************************************
            this_ref_t operator *= ( this_cref_t rhv ) noexcept
            {
                this_t mat( *this ) ;

                _elem[0] = mat.row(0).dot(rhv.column(0)) ;
                _elem[1] = mat.row(0).dot(rhv.column(1)) ;

                _elem[2] = mat.row(1).dot(rhv.column(0)) ;
                _elem[3] = mat.row(1).dot(rhv.column(1)) ;

                return *this ;
            }

        public:

            //************************************************************************************
            static this_t rotation( float_t const angle ) noexcept
            {
                float_t const s = std::sin( angle ) ;
                float_t const c = std::cos( angle ) ;
                return this_t().set_row( 0, vec2_t( c, -s ) ).set_row( 1, vec2_t( s, c ) ) ;
            }

            //************************************************************************************
            static this_t from_columns( vec2_cref_t a, vec2_cref_t b ) noexcept
            {
                this_t ret ;
                ret.set_column( 0, a ) ;
                ret.set_column( 1, b ) ;
                return ret ;
            }

            //************************************************************************************
            static this_t from_rows( vec2_cref_t a, vec2_cref_t b ) noexcept
            {
                this_t ret ;
                ret.set_row( 0, a ) ;
                ret.set_row( 1, b ) ;
                return ret ;
            }

        private:

            /// the matrix elements.
            type_t _elem[4] ;

        } ;
    
        motor_typedefs( matrix2< int_t >, mat2i ) ;
        motor_typedefs( matrix2< float_t >, mat2f ) ;
        motor_typedefs( matrix2< double_t >, mat2d ) ;
        motor_typedefs( matrix2< uint_t >, mat2ui ) ;
        motor_typedefs( matrix2< size_t >, mat2s ) ;
    }
}
