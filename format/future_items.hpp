#pragma once

#include "future_item.hpp"

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

            motor::graphics::image_mtr_t img ;
        };
        motor_typedef( image_item ) ;

        struct audio_item : public item
        {
            audio_item( motor::audio::buffer_mtr_safe_t obj_ ) : obj( obj_ ) {}
            virtual ~audio_item( void_t ) {}

            motor::audio::buffer_mtr_t obj ;
        };
        motor_typedef( audio_item ) ;

        struct glyph_atlas_item : public item
        {
            glyph_atlas_item( motor::font::glyph_atlas_mtr_safe_t obj_ ) : obj( obj_ ) {}
            virtual ~glyph_atlas_item( void_t ) {}
            
            motor::font::glyph_atlas_mtr_t obj ;
        };
        motor_typedef( glyph_atlas_item ) ;

        //***********************************************
        // used when im/exporting meshes
        struct mesh_item : public item
        {
            mesh_item( void_t ) noexcept {}
            virtual ~mesh_item( void_t ) noexcept {}

            motor::string_t name ;
            
            struct geometry
            {
                motor::string_t name ;
                motor::geometry::polygon_mesh_t poly ;
                
                // index into materials
                size_t material_idx ;
            };

            motor::vector< geometry > geos ;

            struct material
            {
                bool_t alpha_blending ;
                motor::string_t material_name ;
                motor::string_t original_name ;
                // some material info
                motor::string_t shader ;
            } ;

            motor::vector< material > materials ;

            struct image
            {
                motor::string_t name ;
                motor::graphics::image_mtr_t img_ptr ;
            };

            motor::vector< image > images ;
            
        };
        motor_typedef( mesh_item ) ;
    }
}