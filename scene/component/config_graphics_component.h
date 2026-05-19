
#pragma once

#include "icomponent.h"

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/object/image_object.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/std/hash_map>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API config_graphics_component : public icomponent
{
    motor_this_typedefs( config_graphics_component );

  public:

    enum class status
    {
        uninitilized,
        in_transit,
        ready,
        released
    };

  private:

    motor::graphics::msl_object_mtr_t _msl = nullptr;
    motor::graphics::geometry_object_mtr_t _geo = nullptr;
    motor::vector< motor::graphics::image_object_mtr_t > _imgs;

    motor::hash_map< size_t, status > _stats ;

  public:

    config_graphics_component( void_t ) noexcept;
    config_graphics_component( this_rref_t ) noexcept;
    config_graphics_component( this_cref_t ) = delete;
    virtual ~config_graphics_component( void_t ) noexcept;

  public:

    void_t set_msl( motor::graphics::msl_object_safe_t obj ) noexcept;
    void_t set_geo( motor::graphics::geometry_object_safe_t obj ) noexcept;
    void_t add_img( motor::graphics::image_object_safe_t ) noexcept;

    // interface for the render visitor or a visitor with the
    // proper frontend attached.
    // after this function, this object can be released/destroyed. all
    // internal items are freed/released.
    // @return true if items are properly initialized and cleaned up.
    bool_t init_and_cleanup( size_t const wid, motor::graphics::gen4::frontend_ptr_t ) noexcept;

  private:

    // sends graphics items to graphics for initialization via the frontend.
    status do_init( size_t const wid, motor::graphics::gen4::frontend_ptr_t ) noexcept;
};
motor_typedef( config_graphics_component );

} // namespace scene
} // namespace motor