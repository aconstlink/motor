// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include <motor/math/typedefs.h>
#include <motor/wire/node/node.h>

namespace motor::scene
{
class animation_clip_node : public motor::wire::funk_node_named_t
{
    using base_t = motor::wire::funk_node_named_t;
    motor_this_typedefs( animation_clip_node );

  public:

    motor_typedefs( motor::wire::input_slot< motor::math::float_t >, time_sec_is );
    motor_typedefs( motor::wire::output_slot< motor::math::float_t >, time_sec_os );
    motor_typedefs( motor::wire::input_slot< motor::math::time_ms_t >, time_ms_is );
    motor_typedefs( motor::wire::output_slot< motor::math::time_ms_t >, time_ms_os );
    motor_typedefs( motor::wire::input_slot< float_t >, weight_is );
    motor_typedefs( motor::wire::output_slot< float_t >, weight_os );

    using time_range_t = std::pair< motor::math::time_ms_t, motor::math::time_ms_t >;

  private:

    // The named slot sheets own the slots; these pointers are borrowed.
    time_sec_is_mtr_t _time_sec_is = nullptr;
    time_sec_os_mtr_t _time_sec_os = nullptr;
    time_ms_is_mtr_t _time_ms_is = nullptr;
    time_ms_os_mtr_t _time_ms_os = nullptr;
    weight_is_mtr_t _weight_is = nullptr;
    weight_os_mtr_t _weight_os = nullptr;

    // the min/max time range of all animation data
    // this clip groups together.
    time_range_t _time_range = { std::numeric_limits< motor::math::time_ms_t >::max(), 0 };

  public:

    explicit animation_clip_node( motor::string_in_t name ) noexcept
        : base_t( name, []( base_t * node )
    {
        auto * self = static_cast< this_t * >( node );

        if( self->_time_sec_is->has_changed() )
        {
            self->_time_sec_os->set_value( self->_time_sec_is->get_value_and_reset() );
            self->_time_ms_os->set_value( motor::math::time_ms_t(
                std::ceil( double_t( self->_time_sec_is->get_value_and_reset() ) * 1000.0 ) ) );
        }
        else if( self->_time_ms_is->has_changed() )
        {
            self->_time_sec_os->set_value( self->_time_ms_is->get_value_and_reset() / 1000.0f );
            self->_time_ms_os->set_value( self->_time_ms_is->get_value_and_reset() );
        }

        self->_weight_os->set_value( self->_weight_is->pull_data() );
    } )
    {
        _time_ms_is = motor::shared( time_ms_is_t( 0 ) );
        _time_ms_os = motor::shared( time_ms_os_t( 0 ) );
        _time_sec_is = motor::shared( time_sec_is_t( 0.0f ) );
        _time_sec_os = motor::shared( time_sec_os_t( 0.0f ) );
        _weight_is = motor::shared( weight_is_t( 1.0f ) );
        _weight_os = motor::shared( weight_os_t( 1.0f ) );

        inputs().add( "time_sec", motor::share_unsafe( _time_sec_is ) );
        inputs().add( "time_ms", motor::share_unsafe( _time_ms_is ) );
        inputs().add( "weight", motor::share_unsafe( _weight_is ) );
        outputs().add( "time_ms", motor::share_unsafe( _time_ms_os ) );
        outputs().add( "time_sec", motor::share_unsafe( _time_sec_os ) );
        outputs().add( "weight", motor::share_unsafe( _weight_os ) );
    }

    animation_clip_node( this_cref_t ) = delete;
    animation_clip_node( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _time_ms_is( motor::move( rhv._time_ms_is ) ),
          _time_ms_os( motor::move( rhv._time_ms_os ) ),
          _weight_is( motor::move( rhv._weight_is ) ), _weight_os( motor::move( rhv._weight_os ) ),
          _time_range( std::move( rhv._time_range ) ),
          _time_sec_is( motor::move( rhv._time_sec_is ) ),
          _time_sec_os( motor::move( rhv._time_sec_os ) )
    {
    }

    // connect only one time input, so there could be not
    // false connectivity in the timings
    void_t connect_time_is( this_t::time_ms_os_mtr_safe_t os ) noexcept
    {
        _time_sec_is->disconnect();
        _time_ms_is->disconnect();
        _time_sec_is->get_value_and_reset();
        _time_ms_is->get_value_and_reset();

        _time_ms_is->connect( motor::move( os ) );
    }

    // connect only one time input, so there could be not
    // false connectivity in the timings
    void_t connect_time_is( this_t::time_sec_os_mtr_safe_t os ) noexcept
    {
        _time_sec_is->disconnect();
        _time_ms_is->disconnect();

        _time_sec_is->connect( motor::move( os ) );
    }

    weight_is_mtr_t borrow_weight_is() noexcept
    {
        return _weight_is;
    }

    weight_os_mtr_t borrow_weight_os() noexcept
    {
        return _weight_os;
    }

    time_ms_os_mtr_safe_t get_time_ms_os() noexcept
    {
        return motor::share( _time_ms_os );
    }

    time_sec_os_mtr_safe_t get_time_sec_os() noexcept
    {
        return motor::share( _time_sec_os );
    }

    weight_is_mtr_safe_t get_weight_is() noexcept
    {
        return motor::share( _weight_is );
    }
    weight_os_mtr_safe_t get_weight_os() noexcept
    {
        return motor::share( _weight_os );
    }

    void_t update_time_range( this_t::time_range_t const & tr ) noexcept
    {
        _time_range.first = std::min( _time_range.first, tr.first );
        _time_range.second = std::max( _time_range.second, tr.second );
    }

    time_range_t get_time_range( void_t ) const noexcept
    {
        if( _time_range.first == std::numeric_limits< motor::math::time_ms_t >::max() )
            return { 0, 0 };
        return _time_range;
    }

    // in milli seconds
    time_ms_os_mtr_t borrow_time_ms_os() noexcept
    {
        return _time_ms_os;
    }

    // in seconds
    time_sec_os_mtr_t borrow_time_sec_os() noexcept
    {
        return _time_sec_os;
    }

  private:

    // in milli seconds
    time_ms_is_mtr_t borrow_time_ms_is() noexcept
    {
        return _time_ms_is;
    }

    // in seconds
    time_sec_is_mtr_t borrow_time_sec_is() noexcept
    {
        return _time_sec_is;
    }

    time_ms_is_mtr_safe_t get_time_ms_is() noexcept
    {
        return motor::share( _time_ms_is );
    }

    time_sec_is_mtr_safe_t get_time_sec_is() noexcept
    {
        return motor::share( _time_sec_is );
    }
};
motor_typedef( animation_clip_node );
} // namespace motor::scene
