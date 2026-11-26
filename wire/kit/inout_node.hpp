// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../node/node.h"

#include <functional>
#include <utility>

namespace motor::wire
{
// One typed input and output; named slots remain available for graph wiring.
template < typename input_t, typename output_t >
class inout_node : public motor::wire::funk_node_named_t
{
    using base_t = motor::wire::funk_node_named_t;
    using this_t = inout_node< input_t, output_t >;

  public:

    motor_typedefs( motor::wire::input_slot< input_t >, input_slot );
    motor_typedefs( motor::wire::output_slot< output_t >, output_slot );
    using function_t = std::function< void_t( this_t * ) >;

  private:

    // Borrowed from the base class slot sheets, which own the slots.
    input_slot_mtr_t _input = nullptr;
    output_slot_mtr_t _output = nullptr;

  public:

    inout_node( motor::string_in_t name, motor::string_in_t input_name,
        motor::string_in_t output_name, function_t function ) noexcept
        : base_t( name, [ function = std::move( function ) ]( base_t * self )
    { function( static_cast< this_t * >( self ) ); } )
    {
        _input = motor::shared( input_slot_t( input_t{} ) );
        _output = motor::shared( output_slot_t( output_t{} ) );
        base_t::inputs().add( input_name, motor::share_unsafe( _input ) );
        base_t::outputs().add( output_name, motor::share_unsafe( _output ) );
    }

    inout_node( this_t const & ) = delete;
    inout_node( this_t && other ) noexcept
        : base_t( std::move( other ) ), _input( motor::move( other._input ) ),
          _output( motor::move( other._output ) )
    {
    }

    input_slot_mtr_safe_t get_input_slot( void_t ) noexcept
    {
        return motor::share( _input );
    }

    output_slot_mtr_t get_output_slot( void_t ) noexcept
    {
        return motor::share( _output );
    }

    input_slot_t & input( void_t ) noexcept
    {
        return *_input;
    }
    input_slot_t const & input( void_t ) const noexcept
    {
        return *_input;
    }
    output_slot_t & output( void_t ) noexcept
    {
        return *_output;
    }
    output_slot_t const & output( void_t ) const noexcept
    {
        return *_output;
    }
};

template < typename input_t, typename output_t, typename funk_t >
auto make_inout_node( motor::string_in_t name, motor::string_in_t input_name,
    motor::string_in_t output_name, funk_t && function )
{
    return motor::shared( inout_node< input_t, output_t >(
        name, input_name, output_name, std::forward< funk_t >( function ) ) );
}

// Example (include <motor/math/vector/vector3.hpp> and
// <motor/math/quaternion/quaternion4.hpp> before using this example):
// auto rotation = motor::wire::make_inout_node< float_t, motor::math::quat4f_t >(
//     "rotation", "time", "rotation", []( auto * self )
//     {
//         float_t const angle = self->input().get_value() * 0.5f;
//         self->output().set_value( motor::math::quat4f_t(
//             angle, motor::math::vec3f_t( 0.0f, 1.0f, 0.0f ) ) );
//     } );
// The scheduled node execution exchanges outputs after the callback.
// Connect slots AND execution edges, just as with any other wire node.
} // namespace motor::wire
