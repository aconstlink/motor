#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
namespace wire
{
class MOTOR_WIRE_API islot
{
    motor_this_typedefs( islot );

  public:

    islot( void_t ) noexcept {}
    islot( this_cref_t ) = delete;

    virtual ~islot( void_t ) noexcept;

  public:

    virtual bool_t exchange( void_t ) noexcept = 0;
    virtual void_t disconnect( bool_t const propagate = true ) noexcept = 0;
};

class MOTOR_WIRE_API ioutput_slot : public islot
{
    motor_this_typedefs( ioutput_slot );

  public:

    virtual ~ioutput_slot( void_t ) noexcept;

  public:

    virtual bool_t exchange( void_t ) noexcept = 0;
    virtual void_t disconnect( bool_t const propagate = true ) noexcept = 0;
    virtual void_t disconnect( motor::wire::iinput_slot_ptr_t,
                               bool_t const propagate = true ) noexcept = 0;
    virtual bool_t connect( motor::wire::iinput_slot_mtr_safe_t,
                            bool_t const propagate = true ) noexcept = 0;
};
motor_typedef( ioutput_slot );

class MOTOR_WIRE_API iinput_slot : public islot
{
    motor_this_typedefs( iinput_slot );

  public:

    virtual ~iinput_slot( void_t ) noexcept;

  public:

    virtual bool_t exchange( void_t ) noexcept = 0;
    virtual void_t disconnect( bool_t const propagate = true ) noexcept = 0;
    virtual void_t disconnect( motor::wire::ioutput_slot_ptr_t,
                               bool_t const propagate = true ) noexcept = 0;
    virtual bool_t connect( motor::wire::ioutput_slot_mtr_safe_t,
                            bool_t const propagate = true ) noexcept = 0;
};
motor_typedef( iinput_slot );

static void_t release( motor::core::mtr_safe< islot > & s ) noexcept
{
    if( s != nullptr ) s->disconnect();
    motor::release( motor::move( s ) );
}

static void_t release( motor::core::mtr_safe< islot > && s ) noexcept
{
    if( s != nullptr ) s->disconnect();
    motor::release( motor::move( s ) );
}
} // namespace wire
} // namespace motor