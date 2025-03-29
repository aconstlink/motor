
#pragma once

#include "../typedefs.h"
#include <motor/std/vector>

namespace motor
{
    namespace geometry
    {
        template< typename T >
        class convex_hull_2d
        {
            motor_this_typedefs( convex_hull_2d<T> ) ;

        public:

            motor_typedefs( motor::math::vec2f_t, point ) ;
            motor_typedefs( motor::vector< point_t >, points ) ;

        private:

            // this is the hull. Sorted so that two successive 
            // points form a line.
            points_t _points ;

        public:

            convex_hull_2d( void_t ) noexcept
            {
            }

            convex_hull_2d( points_cref_t points ) noexcept
            {
                this_t::construct( points ) ;
            }

            convex_hull_2d( points_rref_t points ) noexcept
            {
                this_t::construct( std::move( points ) ) ;
            }

            convex_hull_2d( this_cref_t rhv ) noexcept : 
                _points( rhv._points )
            {
            }

            convex_hull_2d( this_rref_t rhv ) noexcept : 
                _points( std::move( rhv._points ) )
            {
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _points = rhv._points ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _points = std::move( rhv._points ) ;
                return *this ;
            }


        public:

            this_cref_t construct( this_t::points_rref_t points ) noexcept
            {
                auto tmp = std::move( points ) ;

                auto const num_items = this_t::inplace_convex_hull( tmp ) ;

                _points.resize( num_items ) ;
                for( size_t i=0; i<num_items; ++i )
                {
                    _points[i] = tmp[i] ;
                }
                return *this ;
            }

            this_cref_t construct( this_t::points_cref_t points ) noexcept
            {
                points_t tmp = points ;
                return this_t::construct( std::move( tmp ) ) ;
            }

            static this_t make_convex_hull( this_t::points_cref_t points ) noexcept
            {
                this_t ch ;
                ch.construct( points ) ;
                return std::move( ch ) ;
            }


        public:

            bool_t is_point_inside( point_cref_t p ) const noexcept
            {
                return false ;
            }


            points_cref_t get_points( void_t ) const noexcept
            {
                return _points ;
            }

        private:

            // stores the convex hull in the front of the passed points container.
            // say the function returns c and the number of points is n, so the 
            // convex hull is stored like this
            // points[0...c] : contex hull
            // points[c+1 ... n-1] : points inside the convex hull
            static size_t inplace_convex_hull( this_t::points_ref_t points ) noexcept
            {
                if( points.size() == 0 ) return 0 ;

                size_t idx = 0 ;
            
                // starting with a point on the hull makes it 
                // much simpler, so find the most left point.
                {
                    for( size_t i=1; i<points.size(); ++i )
                    {
                        if( points[i].x() < points[idx].x() ) idx = i ;
                    }
                    std::swap( points[0], points[idx] ) ; idx = 0 ;
                }

                size_t num_restarts = 0 ;
                size_t num_segs = 0 ;
                for( size_t i = idx + 1; i< points.size(); ++i )
                {
                    auto const dir = (points[i] - points[idx]).normalize() ;
                    auto const ortho = dir.ortho() ;
                
                    bool_t is_restart = false ;
                    for( size_t j = i + 1; j<points.size(); ++j )
                    {
                        auto const dist = ortho.dot( points[j] - points[idx] ) ;

                        // point is on the line. Dont take it. It causes 
                        // trouble.
                        if( motor::math::fn<float_t>::abs(dist) < 0.001f ) continue ;
                    
                        // if we find a point that is more to the side 
                        // we test, we just change to that point and restart
                        // the hull finding. I think this is much like 
                        // the gift wrap convex hull algo.
                        if( dist < 0.0f )
                        {
                            std::swap( points[i], points[j] ) ;
                            --i ;
                            ++num_restarts ;
                            is_restart = true ;
                            break ;
                        }
                    }

                    // using num_restars to track infinite loop
                    // because --i.
                    if( num_restarts > points.size() ) break ; 

                    if( is_restart ) continue ;
                    num_restarts = 0 ;

                    { 
                        // if the distance with the first convex hull point at idx==0
                        // now is > 0.0f, we just
                        // made the circle complete has we have
                        // to quit the iteration. Comment out 
                        // the "else if" and see what happens.
                        auto const dist = ortho.dot( points[i] - points[0] ) ;

                        if( motor::math::fn<float_t>::abs(dist) < 0.001f)
                        {
                            // point is on the line
                        }
                        else if( dist > 0.0f ) break ;

                        ++num_segs ;
                        ++idx ;
                    }
                }
                return num_segs + 1;
            }
        };
    }
}