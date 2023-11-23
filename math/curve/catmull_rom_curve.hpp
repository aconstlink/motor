#pragma once

#include "../typedefs.h"
#include "../interpolation/interpolate.hpp" 

namespace motor
{
    namespace math
    {
        /// This is a catmull rom curve class that allows to interpolate
        /// two points (p1, p2) using two slopes (m1,m2).
        template< typename T >
        class catmull_rom_curve
        {
            motor_typedefs( T, value ) ;
            motor_typedefs( T, cpoint ) ;
            motor_this_typedefs( catmull_rom_curve<cpoint_t> ) ;        

        private:

            /// The catrom control points where the
            /// curve is interpolating through
            point_t _control_points[2] ;

            /// The slopes at the corresponding 
            /// control points.
            point_t _slopes[2] ;

        public:

            /**
                The default ctor allows to construct a temporary 
                curve object for further use. Though the operator() is not
                very meaningful with this setting.
            */
            catmull_rom_curve( void )
            {}

            /**
                Constructs the curve with the points to be interpolated (p1,p2) and 
                the slopes (m1,m2).
            */
            catmull_rom_curve( cpoint_cref_t p1, cpoint_cref_t m1, 
                cpoint_cref_t p2, cpoint_cref_t m2 )
            {
                _control_points[0] = p1 ;
                _control_points[1] = p2 ;
                _slopes[0] = m1 ;
                _slopes[1] = m2 ;
            }

            /** 
            */
            catmull_rom_curve( this_type_t const & rhv )
            {
                _control_points[0] = rhv._control_points[0] ;
                _control_points[1] = rhv._control_points[1] ;
                _slopes[0] = rhv._slopes[0] ;
                _slopes[1] = rhv._slopes[1] ;
            }
        } ;
    }
} 
