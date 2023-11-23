#pragma once

#include "../typedefs.h"

namespace motor
{
    namespace math
    {
        template< typename T >
        class keyframe
        {
            motor_this_typedefs( keyframe<T> ) ;

        public:

            motor_typedefs( T, type ) ;
            motor_typedefs( T, value ) ;
            motor_typedefs( size_t, time_stamp ) ;
        
        private:

            value_t _value ;
            time_stamp_t _time ;

        public:

            keyframe( void_t ) noexcept : _value(0), _time(time_stamp_t(-1))
            {}

            keyframe( time_stamp_t ts ) noexcept : _value( 0 ), _time(ts)
            {}

            keyframe( time_stamp_t ts, value_cref_t v ) noexcept : _value(v), _time(ts)
            {}

            keyframe( this_cref_t rhv ) noexcept : _value(rhv._value), _time(rhv._time)
            {}

            keyframe( this_rref_t rhv ) noexcept : _value( rhv._value ), _time( rhv._time )
            {}

            ~keyframe( void_t ) noexcept
            {}

        public:

            bool_t operator < ( this_cref_t rhv ) const noexcept
            {
                return _time < rhv._time ;
            }

            bool_t operator > ( this_cref_t rhv ) const noexcept
            {
                return _time > rhv._time ;
            }

            bool_t operator <= ( this_cref_t rhv ) const noexcept
            {
                return _time <= rhv._time ;
            }

            bool_t operator >= ( this_cref_t rhv ) const noexcept
            {
                return _time >= rhv._time ;
            }

            bool_t operator == ( this_cref_t rhv ) const noexcept
            {
                return _time == rhv._time ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _time = rhv._time ;
                _value = rhv._value ;
                return *this ;
            }

        public:

            time_stamp_t get_time( void_t ) const noexcept
            {
                return _time ;
            }

            value_cref_t get_value( void_t ) const noexcept
            {
                return _value ;
            }
        };
    }
}