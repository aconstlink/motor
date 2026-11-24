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

    motor_typedefs( motor::wire::input_slot< motor::math::time_ms_t >, time_is );
    motor_typedefs( motor::wire::output_slot< motor::math::time_ms_t >, time_os );
    motor_typedefs( motor::wire::input_slot< float_t >, weight_is );
    motor_typedefs( motor::wire::output_slot< float_t >, weight_os );

  private:

    // The named slot sheets own the slots; these pointers are borrowed.
    time_is_mtr_t _time_is = nullptr;
    time_os_mtr_t _time_os = nullptr;
    weight_is_mtr_t _weight_is = nullptr;
    weight_os_mtr_t _weight_os = nullptr;

  public:

    explicit animation_clip_node( motor::string_in_t name ) noexcept
        : base_t( name, []( base_t * node )
    {
        auto * self = static_cast< this_t * >( node );
        self->_time_os->set_value( self->_time_is->pull_data() );
        self->_weight_os->set_value( self->_weight_is->pull_data() );
    } )
    {
        _time_is = motor::shared( time_is_t( 0 ) );
        _time_os = motor::shared( time_os_t( 0 ) );
        _weight_is = motor::shared( weight_is_t( 1.0f ) );
        _weight_os = motor::shared( weight_os_t( 1.0f ) );

        inputs().add( "time", motor::share_unsafe( _time_is ) );
        inputs().add( "weight", motor::share_unsafe( _weight_is ) );
        outputs().add( "time", motor::share_unsafe( _time_os ) );
        outputs().add( "weight", motor::share_unsafe( _weight_os ) );
    }

    animation_clip_node( this_cref_t ) = delete;
    animation_clip_node( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _time_is( motor::move( rhv._time_is ) ),
          _time_os( motor::move( rhv._time_os ) ), _weight_is( motor::move( rhv._weight_is ) ),
          _weight_os( motor::move( rhv._weight_os ) )
    {
    }

    // in milli seconds
    time_is_mtr_t borrow_time_is() noexcept
    {
        return _time_is;
    }

    // in milli seconds
    time_os_mtr_t borrow_time_os() noexcept
    {
        return _time_os;
    }

    weight_is_mtr_t borrow_weight_is() noexcept
    {
        return _weight_is;
    }

    weight_os_mtr_t borrow_weight_os() noexcept
    {
        return _weight_os;
    }

    time_is_mtr_safe_t get_time_is() noexcept
    {
        return motor::share( _time_is );
    }
    time_os_mtr_safe_t get_time_os() noexcept
    {
        return motor::share( _time_os );
    }
    weight_is_mtr_safe_t get_weight_is() noexcept
    {
        return motor::share( _weight_is );
    }
    weight_os_mtr_safe_t get_weight_os() noexcept
    {
        return motor::share( _weight_os );
    }
};
motor_typedef( animation_clip_node );
} // namespace motor::scene
