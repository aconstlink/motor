#pragma once

#include "../typedefs.h"
#include <motor/graphics/frontend/gen4/frontend.hpp>

namespace motor
{
namespace gfx
{
struct postprocess_status
{
    enum class state_type { pending, ready, failed };

    state_type state = state_type::pending;
    // Static label of the first pending or failed stage; null when ready.
    char_cptr_t stage = nullptr;
    motor::graphics::result result = motor::graphics::result::invalid;

    static postprocess_status from_state(
        motor::graphics::object_t::data_manipulator::state_pair_t const status,
        char_cptr_t const stage ) noexcept
    {
        // A retry can still carry the result of the previous operation.
        if( status.first == motor::graphics::object_state::in_transit )
            return { state_type::pending, stage, status.second };
        if( status.second == motor::graphics::result::failed ||
            status.second == motor::graphics::result::invalid_argument )
            return { state_type::failed, stage, status.second };
        if( status.first == motor::graphics::object_state::ready &&
            status.second == motor::graphics::result::ok )
            return { state_type::ready, nullptr, status.second };
        return { state_type::pending, stage, status.second };
    }

    static postprocess_status combine( postprocess_status const a,
        postprocess_status const b ) noexcept
    {
        if( a.state == state_type::failed ) return a;
        if( b.state == state_type::failed ) return b;
        return a.state == state_type::pending ? a : b;
    }
};
motor_typedef( postprocess_status );

namespace detail
{
    inline postprocess_status_t check_postprocess_shader(
        motor::graphics::gen4::frontend_ptr_t fe,
        motor::graphics::object_mtr_t shader, char_cptr_t const stage ) noexcept
    {
        if( !fe || !shader ) return { postprocess_status_t::state_type::pending, stage };
        return postprocess_status_t::from_state( fe->decode( shader ), stage );
    }
}
} // namespace gfx
} // namespace motor
