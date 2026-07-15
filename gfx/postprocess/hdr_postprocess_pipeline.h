#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/object/state_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/object/framebuffer_object.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

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
    motor::graphics::framebuffer_object_mtr_t _post_fb = nullptr;

  private: // map to screen

    motor::graphics::state_object_mtr_t _mts_so = nullptr;
    motor::graphics::msl_object_mtr_t _msl = nullptr;

  private: // window size

    bool_t _size_changed = false;
    motor::math::vec2ui_t _dims;

  public:

    hdr_postprocess_pipeline( void_t ) noexcept;
    hdr_postprocess_pipeline( this_cref_t ) = delete;
    hdr_postprocess_pipeline( this_rref_t ) noexcept;
    ~hdr_postprocess_pipeline( void_t ) noexcept;

  public:

    motor::graphics::framebuffer_object_mtr_t borrow_hdr_fb( size_t const idx ) noexcept;
    motor::graphics::state_object_mtr_t borrow_hdr_states( void_t ) noexcept ;

  public:

    void_t on_resize( uint_t const w, uint_t const h ) noexcept;
    void_t init( void_t ) noexcept;
    void_t release( void_t ) noexcept;
    void_t init_render( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    void_t release_render( motor::graphics::gen4::frontend_ptr_t ) noexcept;

    void_t render( motor::graphics::gen4::frontend_ptr_t ) noexcept;
};
motor_typedef( hdr_postprocess_pipeline );
} // namespace gfx
} // namespace motor