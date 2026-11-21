// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include <motor/math/utility/3d/trs.hpp>
#include <motor/std/vector>
#include <motor/wire/node/node.h>
#include <cmath>

namespace motor::scene
{
// Mixes complete local TRS poses. Weights are relative, not additive.
class weighted_mixer_node : public motor::wire::funk_node_named_t
{
    using base_t = motor::wire::funk_node_named_t;
    motor_this_typedefs( weighted_mixer_node );

  public:

    using pose_t = motor::math::trs3f_t;
    motor_typedefs( motor::wire::output_slot< pose_t >, pose_os );

    using trafo_t = motor::math::m3d::trafof_t;
    motor_typedefs( motor::wire::output_slot< trafo_t >, trafo_os );
    motor_typedefs( motor::wire::output_slot< float_t >, weight_os );

  private:

    using vec3_t = motor::math::vec3f_t;
    motor_typedefs( motor::wire::input_slot< pose_t >, pose_is );
    motor_typedefs( motor::wire::input_slot< float_t >, weight_is );

    struct input_pair
    {
        pose_is_mtr_t pose;
        weight_is_mtr_t weight;
    };

    trafo_t _rest;
    motor::vector< input_pair > _pairs; // pointers borrowed from the slot sheets
    trafo_os_mtr_t _out;

  public:

    explicit weighted_mixer_node(
        motor::string_in_t name, trafo_t const & rest = trafo_t{} ) noexcept
        : base_t( name, []( base_t * n ) { static_cast< this_t * >( n )->mix(); } ), _rest( rest ),
          _out( motor::shared( trafo_os_t( rest ) ) )
    {
        outputs().add( "trafo", motor::share_unsafe( _out ) );
    }

    weighted_mixer_node( this_cref_t ) = delete;
    weighted_mixer_node( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _rest( rhv._rest ), _pairs( std::move( rhv._pairs ) ),
          _out( motor::move( rhv._out ) )
    {
    }

    // Wire the data edges as a pair. Execution edges are owned by the caller.
    bool_t add_input( pose_os_mtr_safe_t pose, weight_os_mtr_safe_t weight ) noexcept
    {
        if( pose == nullptr || weight == nullptr )
        {
            motor::release( motor::move( pose ) );
            motor::release( motor::move( weight ) );
            return false;
        }

        auto const id = motor::to_string( _pairs.size() );
        input_pair p{
            motor::shared( pose_is_t( pose_t{} ) ), motor::shared( weight_is_t( 0.0f ) ) };

        inputs().add( "trafo." + id, motor::share_unsafe( p.pose ) );
        inputs().add( "weight." + id, motor::share_unsafe( p.weight ) );

        p.pose->connect( motor::move( pose ) );
        p.weight->connect( motor::move( weight ) );

        _pairs.emplace_back( p );

        return true;
    }

    trafo_os_mtr_t borrow_trafo_os() noexcept
    {
        return _out;
    }
    trafo_os_mtr_safe_t get_trafo_os() noexcept
    {
        return motor::share( _out );
    }

  private:

    void_t mix() noexcept
    {
        input_pair const * strongest = nullptr;
        double_t total = 0.0;
        size_t count = 0;

        // determine strongest pose
        for( auto const & p : _pairs )
        {
            float_t const w = p.weight->pull_data();
            if( !( w > 0.0f ) || !std::isfinite( w ) ) continue;
            p.pose->pull_data();
            if( !strongest || w > strongest->weight->get_value() ) strongest = &p;
            total += w;
            ++count;
        }

        if( count < 2 )
        {
            _out->set_value( strongest ? strongest->pose->get_value().to_transformation() : _rest );
            return;
        }

        pose_t result;
        result.scale = vec3_t( 0.0f );

        motor::math::quat4f_t sum;

        // q and -q describe the same rotation. Align signs before averaging.
        auto const & reference = strongest->pose->get_value().rotation;
        for( auto const & p : _pairs )
        {
            float_t const w = p.weight->get_value();
            if( !( w > 0.0f ) || !std::isfinite( w ) ) continue;

            auto const & pose = p.pose->get_value();
            float_t const t = float_t( w / total );
            result.translation += pose.translation * t;
            result.scale += pose.scale * t;
            auto const & q = pose.rotation;
            sum += q * ( reference.dot( q ) < 0.0f ? -t : t );
        }
        result.rotation = sum.normalized();
        _out->set_value( result.to_transformation() );
    }
};
motor_typedef( weighted_mixer_node );
} // namespace motor::scene
