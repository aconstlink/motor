#pragma once

#include "../typedefs.h"

namespace motor
{
    namespace math
    {
        template< typename type_t >
        class fn
        {
            typedef fn< type_t > this_t;
            typedef type_t const typec_t;

        public:

            static type_t abs( type_t x ) {
                return x < type_t( 0 ) ? -x : x;
            }

            static type_t mod( typec_t x, typec_t m ) {
                const int_t n = int_t( x / m );
                return x < type_t( 0 ) ? x - m * type_t( n + 1 ) : x - m * type_t( n );
            }

        public:

            static int_t ceil( typec_t x ) {
                //return (int)x + (int)( (x > T(0)) && ( (int)x != x ) ) ;
                return x < type_t( 0 ) ? int_t( x ) : int_t( x + type_t( 1 ) );
            }

            static type_t floor( typec_t x ) {
                //return (int)x - (int)( (x < T(0)) && ( (int)x != x ) ) ;
                //return type_t( x > type_t(0) ? int_t(x) : int_t(x - type_t(1)) );
                return ::std::floor( x ) ;
            }
            static type_t fract( type_t v )
            {
                return v - this_t::floor( v ) ;
            }

            /// performs x^y
            static type_t pow( typec_t x, typec_t y ) {
                return ::std::pow( x, y ) ;
            }

            static type_t sqrt( typec_t x ) {
                return ::std::sqrt( x ) ;
            }

            static type_t sin( typec_t x ) {
                return ::std::sin( x ) ;
            }

            static type_t cos( typec_t x ) {
                return ::std::cos( x ) ;
            }

            static type_t acos( typec_t x ) {
                return ::std::acos( x ) ;
            }

        public:

            static type_t step( typec_t x, typec_t a ) {
                return x < a ? type_t( 0 ) : type_t( 1 );
            }

            static type_t pulse( typec_t x, typec_t a, typec_t b ) {
                return this_t::step( x, a ) - this_t::step( x, b );
            }

            static type_t clamp( typec_t x, typec_t a, typec_t b ) {
                return x < a ? a : x > b ? b : x;
            }

            static type_t saturate( typec_t x ) {
                return this_t::clamp( x, type_t( 0 ), type_t( 1 ) );
            }

            /// @precondition x in [a,b] && a < b
            static type_t box_step( typec_t x, typec_t a, typec_t b ) {
                return this_t::clamp( ( x - a ) / ( b - a ), type_t( 0 ), type_t( 1 ) );
            }

            /// @precondition x in [0,1]
            /// return 3x^2-2*x^3
            static type_t smooth_step( typec_t x ) {
                return ( x * x * ( type_t( 3 ) - ( x + x ) ) );
            }

            /// @precondition x in [a,b] && a < b
            /// if x is not in [a,b], use 
            /// smooth_step( clamp(x, a, b), a, b ) 
            static type_t smooth_step( typec_t x, typec_t a, typec_t b ) {
                return this_t::smooth_step( ( x - a ) / ( b - a ) );
            }

            /// @precondition x in [0,1]
            /// return 6x^5 - 15x^4 + 10t^3
            static type_t smooth_step_e5( typec_t x ) {
                return x * x * x * ( x * ( x * type_t( 6 ) - type_t( 15 ) ) + type_t( 10 ) );
            }

            /// @precondition x in [a,b] && a < b
            /// if x is not in [a,b], use 
            /// smooth_step2( clamp(x, a, b), a, b ) 
            static type_t smooth_step_e5( typec_t x, typec_t a, typec_t b ) {
                return this_t::smooth_step_e5( ( x - a ) / ( b - a ) );
            }

            static type_t smooth_pulse( typec_t x, typec_t a, typec_t b ) {
                return this_t::smooth_step( this_t::clamp( x, 0.0f, a ), 0.0f, a ) - 
                    this_t::smooth_step( this_t::clamp( x, b, 1.0f ), b, 1.0f );
            }

        public:

            /// @precondition x in [0,1]
            static type_t mix( typec_t x, typec_t a, typec_t b ) {
                return a + x * ( b - a );
            }

            /// @precondition x in [0,1]
            static type_t lerp( typec_t x, typec_t a, typec_t b ) {
                return this_t::mix( x, a, b );
            }

        public:

            static type_t sign( typec_t in_ ) {
                return type_t( int_t( type_t( 0 ) < in_ ) - int_t( type_t( 0 ) > in_ ) );
            }

        public:

            /// negative normalized value to positive normalized value
            /// takes a value in [-1,1] to [0,1]
            static type_t nnv_to_pnv( typec_t v )
            {
                return v * type_t( 0.5 ) + type_t( 0.5 ) ;
            }

            /// positive normalized value to negative normalized value
            /// takes a value in [0,1] to [-1,1]
            static type_t pnv_to_nnv( typec_t v )
            {
                return v * type_t( 2 ) - type_t( 1 ) ;
            }
        };
    }
}
