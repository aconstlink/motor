
#include "../api.h"
#include "../typedefs.h"

#include "../slot/sheet.hpp"

namespace motor
{
namespace wire
{
class named_slot_sheet_policy
{
    motor_this_typedefs( named_slot_sheet_policy );

  public:

    using inputs_t = motor::wire::inputs_t;
    using outputs_t = motor::wire::outputs_t;

    using inputs_ref_t = inputs_t &;
    using outputs_ref_t = outputs_t &;

  private:

    inputs_t _ins;
    outputs_t _outs;

  public:

    named_slot_sheet_policy( void_t ) noexcept {}
    named_slot_sheet_policy( this_cref_t ) = delete;
    named_slot_sheet_policy( this_rref_t rhv ) noexcept
        : _ins( std::move( rhv._ins ) ), _outs( std::move( rhv._outs ) )
    {
    }
    ~named_slot_sheet_policy( void_t ) noexcept {}

  public:

    this_t::inputs_ref_t inputs( void_t ) noexcept
    {
        return _ins;
    }

    this_t::outputs_ref_t outputs( void_t ) noexcept
    {
        return _outs;
    }
};

// slots will not be named. slots are dropped in the container
// as the user gives the slots.
// the slots container will not be sorted, so the index will stay.
class unnamed_slot_sheet_policy
{

    motor_this_typedefs( unnamed_slot_sheet_policy );

  public:

    using inputs_t = motor::wire::unnamed_inputs_t;
    using outputs_t = motor::wire::unnamed_outputs_t;

    using inputs_ref_t = inputs_t &;
    using outputs_ref_t = outputs_t &;

  private:

    inputs_t _ins;
    outputs_t _outs;

  public:

    unnamed_slot_sheet_policy( void_t ) noexcept {}
    unnamed_slot_sheet_policy( this_cref_t ) = delete;
    unnamed_slot_sheet_policy( this_rref_t rhv ) noexcept
        : _ins( std::move( rhv._ins ) ), _outs( std::move( rhv._outs ) )
    {
    }
    ~unnamed_slot_sheet_policy( void_t ) noexcept {}

  public:

    this_t::inputs_ref_t inputs( void_t ) noexcept
    {
        return _ins;
    }

    this_t::outputs_ref_t outputs( void_t ) noexcept
    {
        return _outs;
    }
};
} // namespace wire
} // namespace motor