#pragma once

#include "../typedefs.h"
#include "../interpolation/interpolate.hpp"

namespace motor
{
    namespace math
    {
        /// This is a cubic bezier curve class that allows to put
        /// four control points togetherand interpolate using a
        /// parameter t.        
        template< typename T >
        class cubic_bezier_curve
        {
            motor_typedefs( T, value ) ;
            motor_typedefs( T, cpoint ) ;
            motor_this_typedefs( cubic_bezier_curve<cpoint_t> ) ;

        private:

            /// The four control points that
            /// are used to evaluate the curve.
            cpoint_t _control_points[4] ;

        public:
        
            cubic_bezier_curve( void_t )
            {}
        
            cubic_bezier_curve( 
                value_cref_t p1, value_cref_t p2, 
                value_cref_t p3, value_cref_t p4 )
            {
                _control_points[0] = p1 ;
                _control_points[1] = p2 ;
                _control_points[2] = p3 ;
                _control_points[3] = p4 ;
            }

            cubic_bezier_curve( this_rref_t rhv )
            {
                *this = std::move(rhv) ;
            }

            cubic_bezier_curve( this_cref_t )
            {
                *this = rhv ;
            }

        public:

            ///
            this_ref_t operator = ( this_cref_t rhv )
            {
                for( size_t i = 0; i < 4; ++i )
                    _control_points[i] = rhv._control_points[i] ;

                return *this ;
            }

            ///
            this_ref_t operator = ( this_rref_t rhv )
            {
                for( size_t i = 0; i < 4; ++i )
                    _control_points[i] = rhv._control_points[i] ;

                return *this ;
            }

        public:

            ///
            value_t operator()( real_t t ) const
            {
                return motor::math::interpolation<cpoint_t>::cubic( 
                    this_t::at(0), this_t::at(1), this_t::at(2), this_t::at(3), t ) ;
            }

            ///
            inline cpoint_t const & control_point( size_t i ) const 
            {
                return _control_points[i<4?i:0] ;
            }

            /// 
            inline bool_t set_control_point( size_t index, 
                cpoint_in_t new_val, cpoint_out_t old_val )
            {
                if( index >= 4 ) return false ;

                old_val = _control_points[index] ;
                _control_points[index] = new_val ;
            
                return true ;
            }

        private:

            cpoint_cref_t at( size_t const i ) const 
            {
                return _control_points[i] ;
            }
        } ;
    }
} 