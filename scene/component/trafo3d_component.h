// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "icomponent.h"

#include <motor/wire/slot/sheet.hpp>
#include <motor/wire/slot/input_slot.h>
#include <motor/wire/slot/output_slot.h>
#include <motor/wire/kit/trafo3_composer.hpp>
#include <motor/math/utility/3d/transformation.hpp>

#include <variant>

namespace motor
{
namespace scene
{
// @todo add relative and absolute transformation
// relative: the transformation is computed throughout the tree traversal
// absolute: should push transformation on the stack
class MOTOR_SCENE_API trafo3d_component : public icomponent
{
    motor_this_typedefs( trafo3d_component );

  public:

    static uint64_t const tid;
    virtual uint64_t type_id( void_t ) const noexcept override
    {
        return tid;
    }

  private: // slot data

    motor_typedefs( motor::wire::input_slot< motor::math::m3d::trafof_t >, in_slot );
    motor_typedefs( motor::wire::output_slot< motor::math::m3d::trafof_t >, out_slot );

    // borrowed slot pointer for local transformation.
    in_slot_ptr_t _trafo_is = nullptr;

    // borrowed slot pointer for computed transformation.
    out_slot_ptr_t _computed_os = nullptr;

    // we can have multiple composers per trafo.
    // all composers could be attached to a mixer
    using composer_variant_t = std::variant< std::nullptr_t, motor::wire::trafo3f_composer_mtr_t,
        motor::wire::trs3f_composer_mtr_t >;
    motor::vector< composer_variant_t > _composer;

    void_t disconnect( composer_variant_t & v ) const noexcept;
    void_t release( composer_variant_t & v ) const noexcept;

    // @todo mixer

  private: // domain data

    // computed local transformation, after the sync with any composer.
    motor::math::m3d::trafof_t _trafo;

    // this is a local coord frame. if kept untransformed,
    // any transformation is absolute. if transformed, any trans
    // formation is relative.
    motor::math::m3d::trafof_t _trafo_local;

    // the one the visitor computes.
    motor::math::m3d::trafof_t _computed;

  public:

    trafo3d_component( void_t ) noexcept;
    trafo3d_component( motor::math::m3d::trafof_cref_t ) noexcept;
    trafo3d_component( this_rref_t ) noexcept;
    trafo3d_component( this_cref_t ) = delete;
    virtual ~trafo3d_component( void_t ) noexcept;

  public:

    motor::math::m3d::trafof_cref_t get_computed( void_t ) const noexcept
    {
        return _computed;
    }
    motor::math::m3d::trafof_cref_t get_trafo( void_t ) const noexcept
    {
        return _trafo;
    }
    motor::math::m3d::trafof_cref_t get_local_trafo( void_t ) const noexcept
    {
        return _trafo_local;
    }

    void_t set_trafo( motor::math::m3d::trafof_in_t t ) noexcept
    {
        _trafo = t;
    }

    void_t set_trafo_local( motor::math::m3d::trafof_in_t t ) noexcept
    {
        _trafo_local = t;
    }

    void_t reset_trafo_local_position( void_t ) noexcept
    {
        _trafo_local.set_translation( motor::math::vec3f_t() );
    }

    void_t reset_trafo_local_scale( void_t ) noexcept
    {
        auto const t = _trafo_local.get_translation();
        auto const o = _trafo_local.get_orientation();
        _trafo_local = motor::math::m3d::trafof_t( motor::math::vec3f_t( 1.0f ), o, t );
    }

    void_t reset_trafo_local_rotation( void_t ) noexcept
    {
        auto const s = _trafo_local.get_scale();
        auto const t = _trafo_local.get_translation();

        _trafo_local = motor::math::m3d::trafof_t( s, motor::math::vec3f_t(), t );
    }

    in_slot_mtr_t borrow_trafo_is( void_t ) noexcept
    {
        return _trafo_is;
    }

  public:

    template < typename comp_type_t >
    motor::wire::trafo3f_composer_mtr_safe_t create_trafo_composer(
        motor::string_in_t name ) noexcept
    {
        auto ret = motor::shared( comp_type_t( name ) );
        _composer.emplace_back( composer_variant_t( motor::share( ret ) ) );
        return motor::move( ret );
    }

    template < typename comp_type_t >
    motor::wire::trafo3f_composer_mtr_t create_trafo_composer_and_borrow(
        motor::string_in_t name ) noexcept
    {
        auto ret = motor::shared( comp_type_t( name ) );
        _composer.emplace_back( ret ); // remember the shared
        return ret;                    // return borrowed
    }

    template < typename comp_type_t >
    bool_t attach_composer( motor::core::mtr_safe< comp_type_t > comp ) noexcept
    {
        if( comp.mtr() == nullptr ) return false;

        if( this_t::has_composer( comp.mtr() ) )
        {
            motor::release( motor::move( comp ) );
            return false;
        }

        _composer.emplace_back( this_t::composer_variant_t( motor::move( comp ) ) );
        return true;
    }

    template < typename comp_type_t >
    bool_t has_composer( comp_type_t * ptr ) const noexcept
    {
        composer_variant_t const needle( ptr );
        return std::find( _composer.begin(), _composer.end(), needle ) != _composer.end();
    }

  public:

    // allows a transformation visitor to update
    // the computed transformation.
    class visitor_interface
    {
        trafo3d_component * _owner;

      public:

        visitor_interface( trafo3d_component * owner ) noexcept : _owner( owner ) {}
        void_t set_computed( motor::math::m3d::trafof_cref_t trafo ) noexcept
        {
            _owner->set_computed( trafo );
        }
    };
    motor_typedef( visitor_interface );

  private:

    friend class visitor_interface;
    void_t set_computed( motor::math::m3d::trafof_cref_t trafo ) noexcept
    {
        _computed = trafo;
    }

    void_t init_slots( void_t ) noexcept;

    virtual void_t sync_inputs( void_t ) noexcept;
    virtual void_t sync_outputs( void_t ) noexcept;

  public:

    virtual bool_t inputs( motor::wire::inputs_out_t ) noexcept;
};
motor_typedef( trafo3d_component );
} // namespace scene
} // namespace motor