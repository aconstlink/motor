#pragma once

#include "../node/node.h"
#include <motor/math/typedefs.h>

namespace motor
{
namespace wire
{

class time_node : public motor::wire::funk_node< motor::wire::unnamed_slot_sheet_policy >
{
    using base_t = motor::wire::funk_node< motor::wire::unnamed_slot_sheet_policy >;
    motor_this_typedefs( time_node );
    motor_typedefs( motor::wire::input_slot< float_t >, float_is );
    motor_typedefs( motor::wire::output_slot< float_t >, float_os );
    motor_typedefs( motor::wire::output_slot< motor::math::time_ms_t >, ms_os );

  public:

    enum class input_time_mode
    {
        absolute,
        dt
    };

  private:

    input_time_mode _input_mode = input_time_mode::absolute;
    float_is_mtr_t _time_is = nullptr;
    float_os_mtr_t _sec_os = nullptr;
    ms_os_mtr_t _ms_os = nullptr;

  public:

    time_node( void_t ) noexcept : base_t( this_t::make_funk() )
    {
        this_t::init_slots();
    }
    time_node( this_t::input_time_mode const im ) noexcept
        : base_t( this_t::make_funk() ), _input_mode( im )
    {
        this_t::init_slots();
    }
    time_node( this_cref_t ) = delete;
    time_node( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _time_is( motor::move( rhv._time_is ) ),
          _sec_os( motor::move( rhv._sec_os ) ), _ms_os( motor::move( rhv._ms_os ) ),
          _input_mode( rhv._input_mode )
    {
        base_t::set_funk( this_t::make_funk() );
    }

    virtual ~time_node( void_t ) noexcept
    {
    // manged by the base slot policy
#if 0
        motor::wire::release( motor::move( _time_is ) );
        motor::wire::release( motor::move( _sec_os ) );
        motor::wire::release( motor::move( _ms_os ) );
#endif
    }

  public:

    float_is_mtr_t borrow_time_is( void_t ) noexcept
    {
        return _time_is;
    }

    float_is_mtr_safe_t get_time_is( void_t ) noexcept
    {
        return motor::share( _time_is );
    }

    float_os_mtr_t borrow_sec_os( void_t ) noexcept
    {
        return _sec_os;
    }

    float_os_mtr_safe_t get_sec_os( void_t ) noexcept
    {
        return motor::share( _sec_os );
    }

    ms_os_mtr_t borrow_ms_os( void_t ) noexcept
    {
        return _ms_os;
    }

    ms_os_mtr_safe_t get_ms_os( void_t ) noexcept
    {
        return motor::share( _ms_os );
    }

  private:

    base_t::funk_t make_funk( void_t ) noexcept
    {
        return [ = ]( base_t * /*self*/ )
        {
            float_t const t = _input_mode == input_time_mode::absolute
                                  ? this->_time_is->get_value()
                                  : this->_sec_os->get_value() +
                                        this->_time_is->get_value(); // otherwise, must be dt.

            this->_sec_os->set_and_exchange( t );
            this->_ms_os->set_and_exchange( motor::math::time_ms_t( t * 1000.0f ) );
        };
    }

    void_t init_slots( void_t ) noexcept
    {
        _time_is = motor::shared( float_is_t() );
        _sec_os = motor::shared( float_os_t() );
        _ms_os = motor::shared( ms_os_t() );

        base_t::inputs().add( motor::share_unsafe( _time_is ) );
        base_t::outputs().add( motor::share_unsafe( _sec_os ) );
        base_t::outputs().add( motor::share_unsafe( _ms_os ) );
    }
};
motor_typedef( time_node );

} // namespace wire
} // namespace motor
