#pragma once

#include "keyframe.hpp"
#include "time_remap_funk.h"
#include "evaluation_result.h"
#include "../spline/cubic_hermit_spline.hpp"
#include "../spline/linear_bezier_spline.hpp"

#include <motor/std/vector>
#include <functional>

namespace motor
{
    namespace math
    {
        template< typename T >
        class keyframe_sequence
        {
            motor_this_typedefs( keyframe_sequence<T> ) ;

        public:

            motor_typedefs( T, spline ) ;
            motor_typedefs( typename T::value_t, value ) ;
            motor_typedefs( keyframe<value_t>, keyframe ) ;
            motor_typedefs( motor::vector< keyframe_t >, keyframes ) ;
            motor_typedefs( motor::vector< size_t >, time_stamps ) ;

            motor_typedefs( typename keyframe_t::time_stamp_t, time_stamp ) ;

            motor_typedefs( motor::vector< float_t >, scalings ) ;

            typedef std::function< float_t( float_t ) > time_funk_t ;
            motor_typedefs( motor::vector<time_funk_t>, time_funks ) ;

        private:

            keyframes_t _keyframes ;
            spline_t _value_spline ;
        
            time_funks_t _time_funks ;

            motor::math::time_remap_funk_type _trf = 
                motor::math::time_remap_funk_type::none ;
            

        public:

            keyframe_sequence( void_t ) noexcept
            {
            }

            keyframe_sequence( motor::math::time_remap_funk_type const trf ) noexcept : 
                _trf( trf )
            {}

            keyframe_sequence( this_cref_t rhv ) noexcept
            {
                _keyframes = rhv._keyframes ;
                _value_spline = rhv._value_spline ;
                _time_funks = rhv._time_funks ;
                _trf = rhv._trf ;
            }

            keyframe_sequence( this_rref_t rhv ) noexcept
            {
                _keyframes = std::move( rhv._keyframes ) ;
                _value_spline = std::move( rhv._value_spline ) ;
                _time_funks = std::move( rhv._time_funks ) ;
                _trf = std::move( rhv._trf ) ;
            }

            ~keyframe_sequence( void_t ) noexcept
            {}

        public: // operators

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _keyframes = std::move( rhv._keyframes ) ;
                _value_spline = std::move( rhv._value_spline ) ;
                _time_funks = std::move( rhv._time_funks ) ;
                _trf = std::move( rhv._trf ) ;

                return *this ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _keyframes = rhv._keyframes ;
                _value_spline = rhv._value_spline ;
                _time_funks = rhv._time_funks ;
                _trf = rhv._trf ;

                return *this ;
            }

            keyframe_t operator[]( size_t const i ) const noexcept
            {
                assert( i < _keyframes.size() ) ;
                return _keyframes[i] ;
            }

        public:

            spline_cref_t get_spline( void_t ) noexcept
            {
                return _value_spline ;
            }


        public:

            bool_t insert( keyframe_cref_t kf ) noexcept
            {
                if( kf.get_time() == time_stamp_t(-1) )
                    return false ;
                
                // 1. insert keyframe
                auto const iter = _keyframes.insert( std::lower_bound( _keyframes.begin(), _keyframes.end(), kf ), kf ) ;
                
                // 2. fix spline
                _value_spline.insert( std::distance( _keyframes.begin(), iter ), kf.get_value() ) ;

                // 3. compute scalings
                {
                    // num_segments
                    size_t const ns = _keyframes.size()-1 ;
                    _time_funks.resize( ns ) ;

                    for( size_t i = 1; i < _keyframes.size(); ++i )
                    {
                        float_t const t1 = this_t::to_zero_one(_keyframes[i-0].get_time()) ;
                        float_t const t0 = this_t::to_zero_one(_keyframes[i-1].get_time()) ;

                        float_t const m = 1.0f / (float_t( ns ) * (t1-t0)) ;

                        float_t const t_off = ( float_t( i ) / float_t( ns ) ) - t1 * m ;

                        _time_funks[i-1] = [=]( float_t const t )
                        {
                            return t * m + t_off ;
                        } ;
                    }
                }

                return true ;
            }

            keyframe_t front( void_t ) const noexcept
            {
                return _keyframes.front() ;
            }

            keyframe_t back( void_t ) const noexcept
            {
                return _keyframes.back() ;
            }

            size_t get_num_keyframes( void_t ) const noexcept
            {
                return _keyframes.size() ;
            }

            void_t set_time_remapping( motor::math::time_remap_funk_type const trf ) noexcept
            {
                _trf = trf ;
            }

        public:

            motor::math::evaluation_result operator () ( time_stamp_t ts, value_out_t vo ) const noexcept
            {
                float_t t ;
            
                // remap time
                {
                    time_stamp_t const a = _keyframes.front().get_time() ;
                    time_stamp_t const b = _keyframes.back().get_time() ;
                    ts = motor::math::get_time_remap_funk( _trf )( ts, a, b ) ;
                }

                // before we can evaluate the anything, we need to check if
                // 1. the sequence has enough keyframes
                // 2. the time stamp is in time range of the sequence
                {
                    auto const res = this_t::is_in_range( ts, t ) ;

                    if( !motor::math::is_value_usable( res ) )
                        return res ;

                    if( motor::math::is_out_of_range( res ) )
                    {
                        this_t::get_extreme_value( ts, vo ) ;
                        return res ;
                    }
                }
            

                // @note the segment index si can not be calculated
                // by a simple multiplication like t*ns, because the
                // time stamps are NOT equally distributed!
                // So for now, we need to search it.
                {
                    size_t si = 0 ;

                    // do linear search
                    while( _keyframes[ si++ ].get_time() < ts ) ;

                    size_t const ns = _keyframes.size() - 1 ;

                    // need to do -2 because si++ adds one after 
                    // we found the keyframe index and since we need
                    // the segment index but searched for keyframe index.
                    si = std::min( std::max( si, size_t(2) ) - 2, ns - 1 ) ;
                    t = _time_funks[ si ]( t ) ;
                }

                {
                    auto const res = _value_spline( t, vo ) ;
                }

                return motor::math::evaluation_result::in_range ;
            }

            value_t operator () ( time_stamp_t const ts ) const noexcept
            {
                value_t v ;
                this->operator()( ts, v ) ;
                return v ;
            }

        public:

            /// checks if ts is in range and if so, returns where
            /// ts is in that range in [0,1]. The direct return value
            /// is true if in range, false otherwise.
            motor::math::evaluation_result is_in_range( time_stamp_t const ts, float_out_t at ) const noexcept
            {
                if( _keyframes.size() < 2 ) 
                    return motor::math::evaluation_result::invalid ;

                time_stamp_t const b = _keyframes.front().get_time() ;
                time_stamp_t const e = _keyframes.back().get_time() ;
            
                // if ts not E [b, e]
                {
                    if( ts < b )
                    {
                        at = 0.0f ;
                        return motor::math::evaluation_result::out_of_range ;
                    }
                    else if( ts > e )
                    {
                        at = 1.0f ;
                        return motor::math::evaluation_result::out_of_range ;
                    }
                }

                at = float_t( double_t( ts - b ) / double_t( e - b ) ) ;

                return motor::math::evaluation_result::in_range ;
            }

        private:

            //***************************************************************************
            float_t to_zero_one( time_stamp_t const ts ) const noexcept
            {
                time_stamp_t const b = _keyframes.front().get_time() ;
                time_stamp_t const e = _keyframes.back().get_time() ;

                return float_t( double_t( ts - b ) / double_t( e - b ) ) ;
            }

            //***************************************************************************
            bool_t get_extreme_value( time_stamp_t const ts, value_out_t vout ) const noexcept
            {
                time_stamp_t const b = _keyframes.front().get_time() ;
                time_stamp_t const e = _keyframes.back().get_time() ;

                if( ts < b )
                {
                    vout = _keyframes.front().get_value() ;
                    return true ;
                }
                else if( ts > e )
                {
                    vout = _keyframes.back().get_value() ;
                    return true ;
                }
                return false ;
            }

        } ;
    }
}