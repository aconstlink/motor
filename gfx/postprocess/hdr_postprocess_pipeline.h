#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "tone_map_stage.hpp"
#include "bright_pass_stage.hpp"

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/object/state_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/object/framebuffer_object.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/property/property_sheet.hpp>

#include <motor/std/hash_map>

namespace motor
{
namespace gfx
{
class MOTOR_GFX_API hdr_postprocess_pipeline
{
    motor_this_typedefs( hdr_postprocess_pipeline );

  private: // post off-screen

    motor::graphics::framebuffer_object_mtr_t _hdr_fbs[ 2 ];
    motor::graphics::state_object_mtr_t _hdr_so = nullptr;

    motor::graphics::state_object_mtr_t _post_so = nullptr;
    motor::graphics::geometry_object_mtr_t _post_quad = nullptr;

    enum class framebuffer_type
    {
        full_hdr_0,
        full_hdr_1,
        half_hdr,
        quater_hdr,
        full_ldr,
        num_types
    };
    std::array< motor::graphics::framebuffer_object_mtr_t, size_t( framebuffer_type::num_types ) >
        _post_fbs;

    // the resolution we use for the
    // post processing framebuffers
    motor::math::vec2ui_t _post_fb_dims = motor::math::vec2ui_t( 1920, 1080 );

  private: // map to screen

    motor::graphics::state_object_mtr_t _mts_so = nullptr;
    motor::graphics::msl_object_mtr_t _msl = nullptr;

  private: // window size

    bool_t _size_changed = false;
    motor::math::vec2ui_t _dims;

  private:

    motor::gfx::tone_map_stage_mtr_t _tone_map = nullptr;
    motor::gfx::bright_pass_stage_mtr_t _brightpass = nullptr;

  public:

    hdr_postprocess_pipeline( uint_t const w = 1920, uint_t const h = 1080 ) noexcept;
    hdr_postprocess_pipeline( this_cref_t ) = delete;
    hdr_postprocess_pipeline( this_rref_t ) noexcept;
    ~hdr_postprocess_pipeline( void_t ) noexcept;

  public:

    motor::graphics::framebuffer_object_mtr_t borrow_hdr_fb( size_t const idx ) noexcept;
    motor::graphics::state_object_mtr_t borrow_hdr_states( void_t ) noexcept;

  public:

    void_t on_resize( uint_t const w, uint_t const h ) noexcept;
    void_t init( void_t ) noexcept;
    void_t release( void_t ) noexcept;
    void_t init_render( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    void_t release_render( motor::graphics::gen4::frontend_ptr_t ) noexcept;

    void_t render( motor::graphics::gen4::frontend_ptr_t ) noexcept;

  public:

    using inputs_map_t = motor::hash_map< motor::string_t, motor::wire::inputs_mtr_t >;
    inputs_map_t inputs( void_t ) noexcept;

    using property_sheets_t =
        motor::hash_map< motor::string_t, motor::property::property_sheet_mtr_t >;
    property_sheets_t property_sheets( void_t ) noexcept;

  private:

    motor::graphics::framebuffer_object_mtr_t borrow_post_framebuffer(
        this_t::framebuffer_type const ) noexcept;
};
motor_typedef( hdr_postprocess_pipeline );
} // namespace gfx
} // namespace motor