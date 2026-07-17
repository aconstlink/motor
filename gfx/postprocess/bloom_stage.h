#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/frontend/gen4/frontend.hpp>
#include <motor/graphics/variable/wire_variable_bridge.h>
#include <motor/property/property_sheet.hpp>

namespace motor
{
namespace gfx
{
class bloom_stage
{
    motor_this_typedefs( bloom_stage );

  private:

    motor::graphics::msl_object_mtr_t _msl_down = nullptr;
    motor::graphics::msl_object_mtr_t _msl_up = nullptr;
    motor::graphics::wire_variable_bridge_mtr_t _brg = nullptr;

    motor::property::property_sheet_mtr_t _prop_sheet = nullptr;

  public:

    enum class level_type
    {
        level_0, // full res 1:1
        level_1, // half res 1:2
        level_2, // quat res 1:4
        level_3, // 1:8 res
        level_4, // 1:16 res
        level_5, // 1:32 res
        num_level
    };

    static size_t to_idx( this_t::level_type const lt ) noexcept
    {
        return size_t( lt ) ;
    }

  private:

    struct per_level_data
    {
        motor::graphics::variable_set_mtr_t vs = nullptr ;
        motor::graphics::state_object_mtr_t so_down = nullptr;
        motor::graphics::state_object_mtr_t so_up = nullptr;
    };
    std::array< per_level_data, size_t( level_type::num_level ) > _per_level;

  public:

    bloom_stage( void_t ) noexcept;

    bloom_stage( this_cref_t ) = delete;
    bloom_stage( this_rref_t rhv ) noexcept;

  public:

    void_t change_resolution( uint_t const w, uint_t const h ) noexcept;

    void_t init( uint_t const w, uint_t const h ) noexcept;

    void_t release( void_t ) noexcept;

    void_t init_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept;

    void_t release_graphics( motor::graphics::gen4::frontend_ptr_t fe ) noexcept;

    void_t set_read_render_target_for_down( this_t::level_type const lt, motor::string_in_t name ) noexcept ;
    void_t set_read_render_target_for_up( this_t::level_type const lt, motor::string_in_t name ) noexcept ;

    void_t render_down( this_t::level_type const lt, motor::graphics::gen4::frontend_ptr_t fe ) noexcept;
    void_t render_up( this_t::level_type const lt, motor::graphics::gen4::frontend_ptr_t fe ) noexcept;

    motor::wire::inputs_mtr_t borrow_inputs( void_t ) noexcept;

    motor::property::property_sheet_mtr_t borrow_properties( void_t ) noexcept;
};
motor_typedef( bloom_stage );

} // namespace gfx
} // namespace motor