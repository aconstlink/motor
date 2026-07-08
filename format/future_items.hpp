#pragma once

#include "future_item.hpp"

#include <motor/scene/node/node.h>
#include <motor/wire/kit/time_node.hpp>

#include <motor/gfx/camera/generic_camera.h>
#include <motor/geometry/mesh/polygon_mesh.h>
#include <motor/audio/object/buffer_object.hpp>
#include <motor/graphics/texture/image.hpp>
#include <motor/font/glyph_atlas.h>

#include <future>

namespace motor
{
namespace format
{
struct image_item : public item
{
    image_item( motor::graphics::image_mtr_safe_t img_ ) : img( img_ ) {}
    virtual ~image_item( void_t ) {}

    motor::graphics::image_mtr_t img;

    virtual void_t release( void_t ) noexcept 
    {
        motor::release( motor::move( img ) );
    }
};
motor_typedef( image_item );

struct audio_item : public item
{
    audio_item( motor::audio::buffer_mtr_safe_t obj_ ) : obj( obj_ ) {}
    virtual ~audio_item( void_t ) {}

    motor::audio::buffer_mtr_t obj;

    virtual void_t release( void_t ) noexcept 
    {
        motor::release( motor::move( obj ) );
    }
};
motor_typedef( audio_item );

struct glyph_atlas_item : public item
{
    glyph_atlas_item( motor::font::glyph_atlas_mtr_safe_t obj_ ) : obj( obj_ ) {}
    virtual ~glyph_atlas_item( void_t ) {}

    motor::font::glyph_atlas_mtr_t obj;

    virtual void_t release( void_t ) noexcept 
    {
        motor::release( motor::move( obj ) );
    }
};
motor_typedef( glyph_atlas_item );

//***********************************************
// used when im/exporting meshes
struct mesh_item : public item
{
    mesh_item( void_t ) noexcept {}
    virtual ~mesh_item( void_t ) noexcept {}

    motor::string_t name;

    struct geometry
    {
        motor::string_t name;
        motor::geometry::polygon_mesh_t poly;

        // index into materials
        size_t material_idx;
    };

    motor::vector< geometry > geos;

    struct material
    {
        bool_t alpha_blending;
        motor::string_t material_name;
        motor::string_t original_name;
        // some material info
        motor::string_t shader;
    };

    motor::vector< material > materials;

    struct image
    {
        motor::string_t name;
        motor::graphics::image_mtr_t img_ptr;
    };

    motor::vector< image > images;

    virtual void_t release( void_t ) noexcept 
    {
        for( auto & img : images )
        {
            motor::release( motor::move( img.img_ptr ) );
        }
    }
};
motor_typedef( mesh_item );

//***********************************************
// used when im/exporting scenes with assets
struct scene_item : public item
{
    motor::scene::node_mtr_t root = nullptr;

    // can be used to start all animations
    motor::wire::time_node_mtr_t start_node = nullptr;

    // can be used as notification of animation end
    motor::wire::funk_node_mtr_t merger_node = nullptr;

    struct camera_sequence_item
    {
        motor::math::time_ms_t start;
        motor::math::time_ms_t end;
        motor::string_t node_name;
        motor::gfx::generic_camera_mtr_t cam;
    };

    // import a camera sequence.
    motor::vector< camera_sequence_item > camera_sequence;

    struct camera_item
    {
        motor::string_t camera_name;
        motor::scene::node_mtr_t node;
        motor::gfx::generic_camera_mtr_t cam;
    };
    motor::vector< camera_item > cameras;

    virtual void_t release( void_t ) noexcept
    {
        motor::release( motor::move( root ) );
        motor::release( motor::move( start_node ) );
        motor::release( motor::move( merger_node ) );

        for( auto & csi : camera_sequence )
            motor::release( motor::move( csi.cam ) );

        for( auto & ci : cameras )
        {
            motor::release( motor::move( ci.cam ) );
            motor::release( motor::move( ci.node ) );
        }
    }

    // @todo add asset maps here
};
motor_typedef( scene_item );
} // namespace format
} // namespace motor