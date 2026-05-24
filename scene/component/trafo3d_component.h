
#pragma once

#include "icomponent.h"

#include <motor/wire/slot/sheet.hpp>
#include <motor/wire/slot/input_slot.h>
#include <motor/wire/slot/output_slot.h>
#include <motor/math/utility/3d/transformation.hpp>

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

  private: // slot data

    motor_typedefs( motor::wire::input_slot< motor::math::m3d::trafof_t >, in_slot );
    motor_typedefs( motor::wire::output_slot< motor::math::m3d::trafof_t >, out_slot );

    // borrowed slot pointer for local transformation.
    in_slot_ptr_t _trafo_is = nullptr;

    // borrowed slot pointer for computed transformation.
    out_slot_ptr_t _computed_os = nullptr;

  private: // domain data

    // relative or absolute @todo insert enum for that. Really?
    motor::math::m3d::trafof_t _trafo;

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
    void_t set_trafo( motor::math::m3d::trafof_in_t t ) noexcept
    {
        _trafo = t;
    }

#if 0
    in_slot_mtr_safe_t get_input_slot( void_t ) noexcept
    {
        return motor::share( _trafo );
    }
    out_slot_mtr_safe_t get_output_slot( void_t ) noexcept
    {
        return motor::share( _computed );
    }
#endif

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

    virtual void_t sync_inputs( void_t ) noexcept ;
    virtual void_t sync_outputs( void_t ) noexcept ;

  public:

    virtual bool_t inputs( motor::wire::inputs_out_t ) noexcept;
};
motor_typedef( trafo3d_component );
} // namespace scene
} // namespace motor