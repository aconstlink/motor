#pragma once

#include "../typedefs.h"
#include "continuity.h"
#include "../interpolation/interpolate.hpp"

#include <motor/vector.hpp>

namespace motor
{
    namespace math
    {
        // cubic bezier
        template< typename T >
        class cubic_bezier_spline
        {
            motor_this_typedefs( cubic_bezier_spline<T> ) ;
            motor_typedefs( T, type ) ;    
            motor_typedefs( T, value ) ;

            template< typename T2 >
            struct segment
            {
                T2 p0 ;
                T2 p1 ;
                T2 p2 ;
                T2 p3 ;
            };
            motor_typedefs( segment< float_t >, segmentf ) ;
            motor_typedefs( segment< value_t >, segmentv ) ;

        private:

            motor_typedefs( motor::vector< value_t >, points ) ;
            /// control point being evaluated
            points_t _cps ;

        public:

            enum class init_type
            {
                // take control points as passed.
                complete,
                // take given control points and construct a c1/c2 continous spline
                construct,
            };

        public:

            cubic_bezier_spline( void_t ) noexcept
            {}

            // init with first segment
            cubic_bezier_spline( value_cref_t p0, value_cref_t p1, value_cref_t p2, value_cref_t p3 ) noexcept
            {
                _cps.emplace_back( p0 ) ;
                _cps.emplace_back( p1 ) ;
                _cps.emplace_back( p2 ) ;
                _cps.emplace_back( p3 ) ;
            }

            // pass control points
            // if t == construct_c1 -> first three control points represent the first segment. Every other control point
            // is interpreted as interpolating points
            // @precondition li.size() >= 3
            cubic_bezier_spline( motor::vector< value_t > const & li, this_t::init_type t = this_t::init_type::complete ) noexcept
            {
                if( li.size() < 4 ) return ;

                if( t == this_t::init_type::complete )
                {
                    for( auto const & i : li )
                    {
                        _cps.emplace_back( i ) ;
                    }
                }
                else if( t == this_t::init_type::construct )
                {
                    _cps = this_t::construct_from_list( li ) ;
                }
            }

            cubic_bezier_spline( std::initializer_list< value_t > const & li, this_t::init_type t = this_t::init_type::complete ) noexcept :
                cubic_bezier_spline( motor::vector< value_t >( li), t )
            {
                
            }
            cubic_bezier_spline( this_rref_t rhv ) noexcept
            {
                (*this) = std::move(rhv) ;
            }

            cubic_bezier_spline( this_cref_t rhv ) noexcept
            {
                (*this) = rhv ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _cps = std::move( rhv._cps ) ;
                return ( *this ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _cps = rhv._cps ;
                return ( *this ) ;
            }

        private:

            static void_t append_single_to_list( size_t const num_segments, value_in_t cp, points_ref_t inout ) noexcept
            {
                size_t const s = num_segments - 1 ;

                size_t const base = s * 3 ;

                auto const p0 = inout[ base + 0 ] ;
                auto const p1 = inout[ base + 1 ] ;
                auto const p2 = inout[ base + 2 ] ;
                auto const p3 = inout[ base + 3 ] ;

                auto const p = inout.back() ;
                inout.emplace_back( p + motor::math::interpolation<value_t>::cubic_dt( p0, p1, p2, p3, 1.0f ) * 0.34f ) ;
                inout.emplace_back( p + motor::math::interpolation<value_t>::cubic_dt2( p0, p1, p2, p3, 1.0f ) * 0.17f ) ;

                inout.emplace_back( cp ) ;
            }

            // first four control points need to represent a cubic bezier.
            static points_t construct_from_list( points_cref_t li ) noexcept
            {
                points_t ret ;

                ret.reserve( li.size() * 2 ) ;
                    
                ret.emplace_back( li[0] ) ;
                ret.emplace_back( li[1] ) ;
                ret.emplace_back( li[2] ) ;
                ret.emplace_back( li[3] ) ;

                size_t ns = 1 ;
                for( size_t i=4; i<li.size(); ++i )
                {
                    this_t::append_single_to_list( ns++, li[i], ret ) ;
                }

                return ret ;
            }


        public:

            void_t clear( void_t ) noexcept
            {
                _cps.clear() ;
            }

            // Add an interpolating control point.
            // computes the mid control point in c1 continuity.
            // Computes c1 if at least three points are present.
            void append( value_in_t cp ) noexcept
            {
                if( this_t::ncp() < 4 ) 
                {
                    _cps.emplace_back( cp ) ;
                    return ;
                }

                this_t::append_single_to_list( this_t::num_segments(), cp, _cps ) ;
            }

        public:

            /// return the number of segments.
            size_t num_segments( void_t ) const noexcept
            {
                // (ncp - 1) / p
                // where p is the power of the spline. cubic => 3
                return (_cps.size() - 1) / 3 ; 
            }

            /// return the number of segments.
            size_t get_num_segments( void_t ) const noexcept
            {
                return this_t::num_segments() ;
            }

            size_t ns( void_t ) const noexcept
            {
                return this_t::num_segments() ;
            }

            // quadratic spline has 3 points per segment (pps)
            size_t points_per_segment( void_t ) const noexcept
            {
                return 4 ;
            }

            size_t pps( void_t ) const noexcept
            {
                return this_t::points_per_segment() ;
            }

            // number of control points
            size_t ncp( void_t ) const noexcept
            {
                return _cps.size() ;
            }

            // evaluate at global t.
            // requires 3 control points at min.
            bool_t operator() ( float_t const t_g, value_out_t val_out ) const noexcept
            {
                if( this_t::ncp() < 4 ) return false ;
                auto const t = std::min( 1.0f, std::max( t_g, 0.0f ) ) ;

                float_t const local_t = this_t::global_to_local( t ) ;
                auto const seg = this_t::get_segment( t ) ;

                val_out = motor::math::interpolation<value_t>::cubic( seg.p0, seg.p1, seg.p2, seg.p3, local_t ) ;

                return true ;
            }

            // evaluates at global t.
            value_t operator() ( float_t const t ) const noexcept
            {
                value_t ret ;
                this_t::operator()( t, ret ) ;
                return ret ;
            }

        public:

            typedef std::function< void_t ( size_t const, value_cref_t ) > for_each_cp_funk_t ;
            void_t for_each_control_point( for_each_cp_funk_t funk ) const noexcept 
            {
                size_t idx = 0 ;
                for( auto const & p : _cps ) funk( idx++, p ) ;
                
            }

            points_cref_t control_points( void_t ) const noexcept
            {
                return _cps ;
            }

        private:

            // convert global t to segment index
            // @param tg global t E [0.0,1.0]
            // @return segment index E [0,ns-1]
            size_t segment_index( float_t const t ) const noexcept
            {
                return size_t( std::min( std::floor( t * float_t( this_t::ns() ) ), float_t( this_t::ns() - 1 ) ) ) ;
            }

            // returns the value segment for a global t.
            segmentv_t get_segment( float_t const t_g ) const noexcept
            {
                auto const si = this_t::segment_index( t_g ) ;
                return this_t::get_segment( si ) ;
            }

            segmentv_t get_segment( size_t const si ) const noexcept
            {
                size_t const i = si * (this_t::pps()-1) ;
                return this_t::segmentv_t( { _cps[i+0], _cps[i+1], _cps[i+2], _cps[i+3]} ) ;
            }

            // returns the segemnt of the t values for the involved cps
            // in the requested segment. This is used to compute the 
            // local t value.
            segmentf_t get_t_segment( size_t const si ) const noexcept
            {
                float_t const b = float_t( si * (this_t::pps() - 1) ) ;
                float_t const r = 1.0f / float_t(this_t::ncp() - 1) ;
                return segmentf_t{ (b + 0.0f)*r, (b + 1.0f)*r, (b + 2.0f)*r, (b + 3.0f)*r };
            }

            float_t global_to_local( float_t const t ) const noexcept
            {
                size_t const si = this_t::segment_index( t ) ;
                auto const st = this_t::get_t_segment( si ) ;
                return this_t::global_to_local( t, st ) ;
            }

            float_t global_to_local( float_t const t, segmentf_cref_t s ) const noexcept
            {
                return (t - s.p0) / (s.p3 - s.p0) ;
            }

            bool_t is_in_range( float_t const t ) const noexcept
            {
                return t >= 0.0f && t <= 1.0f ;
            }

        } ;
    }
}

