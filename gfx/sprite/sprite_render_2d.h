#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/object/state_object.h>
#include <motor/graphics/object/array_object.h>
#include <motor/graphics/object/render_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/variable/variable_set.hpp>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/concurrent/mrsw.hpp>

#include <motor/std/vector>
#include <motor/std/vector_pod>

#include <motor/math/matrix/matrix2.hpp>

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API sprite_render_2d
        {
            motor_this_typedefs( sprite_render_2d ) ;

        private: // data
            
            struct sprite
            {
                motor_this_typedefs( sprite ) ;

                motor::math::vec2f_t pos ;
                motor::math::mat2f_t frame ;
                motor::math::vec2f_t scale ;
                motor::math::vec4f_t uv_rect ;
                motor::math::vec2f_t pivot ;
                motor::math::vec4f_t color ;

                size_t slot ;
            };
            motor_typedef( sprite ) ;

            struct layer
            {
                motor::concurrent::mrsw_t mtx ;
                motor::vector_pod< sprite_t > sprites ;
            };
            motor_typedef( layer ) ;

            motor::concurrent::mutex_t _layers_mtx ;
            motor::vector< layer_ptr_t > _layers ;

            motor::concurrent::mutex_t _num_sprites_mtx ;
            size_t _num_sprites = 0 ;

        private: // graphics

            struct per_layer_render_data
            {
                size_t num_elems = 0 ;
                size_t num_quads = 0 ;
            };
            motor::vector< per_layer_render_data > _render_data ;

            struct render_layer_info
            {
                size_t start = 0 ;
                size_t end = 0 ;
            };
            motor::vector< render_layer_info > _render_layer_infos ;

            struct vertex
            {
                motor::math::vec2f_t pos ;
            };

            struct the_data
            {
                // world pos, scale : ( x, y, sx, sy )
                motor::math::vec4f_t pos_scale ;
                // coords frame (X, Y)
                motor::math::vec4f_t frame ;
                // uv rect, bottom left (x0,y0,x1,y1)
                motor::math::vec4f_t uv_rect ;

                // (pivot.xy, free, slot )
                motor::math::vec4f_t additional_info ;

                // animate the uv coords in x and y direction
                // (x ani, y ani, free, free)
                motor::math::vec4f_t uv_anim ;

                // color the sprite
                motor::math::vec4f_t color ;
            };

            motor::string_t _name ;
            motor::graphics::state_object_t _rs ;
            motor::graphics::array_object_t _ao ;
            motor::graphics::shader_object_t _so ;
            motor::graphics::render_object_t _ro ;
            motor::graphics::geometry_object_t _go ;
            
            size_t _max_quads = 2000 ;

            void_t add_variable_set( motor::graphics::render_object_ref_t ) noexcept ;

            bool_t _image_name_changed = true ;
            motor::string_t _image_name ;

            bool_t _reset_view_proj = false ;
            motor::math::mat4f_t _view ;
            motor::math::mat4f_t _proj ;

            struct prepare_update
            {
                bool_t vertex_realloc = false ;
                bool_t data_realloc = false ;
                bool_t reconfig_ro = false ;
            };

            prepare_update _pe ;

        public:

            #if 0
            struct sprite_sheet
            {
                struct sprite
                {
                    motor::math::vec4f_t uv_rect ;
                };

                // animaton times in milliseconds
                struct animation
                {
                    
                    size_t duration ;
                };

                // connects a sequence of sprites with animation data
                struct sprite_sequence
                {
                    motor::string_t name ;
                    motor::vector< sprite > sprites ;
                    motor::vector< size_t > durations ;
                    // animation type (single, repreat, reverse, ...)
                    
                };

                motor::vector< sprite_sequence > sequences ;
            };
            #endif

        public:

            sprite_render_2d( void_t ) noexcept;
            sprite_render_2d( this_cref_t ) = delete ;
            sprite_render_2d( this_rref_t ) noexcept;
            ~sprite_render_2d( void_t ) noexcept;

            void_t init( motor::string_cref_t, motor::string_cref_t ) noexcept ;
            void_t release( void_t ) noexcept ;

        public:

            void_t set_texture( motor::string_cref_t name ) noexcept ;

            // draw a sprite/uv rect from a texture of the screen
            void_t draw( size_t const l, motor::math::vec2f_cref_t pos, motor::math::mat2f_cref_t frame, 
                motor::math::vec2f_cref_t scale, motor::math::vec4f_cref_t uv_rect, size_t const slot, 
                motor::math::vec2f_cref_t pivot = motor::math::vec2f_t(0.0f), motor::math::vec4f_cref_t color = motor::math::vec4f_t(1.0f) ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept ;

        public:

            void_t configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;

            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;
            void_t prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;
            void_t render( motor::graphics::gen4::frontend_mtr_t fe, size_t const ) noexcept ;

        private:

            layer_ptr_t add_layer( size_t const i ) noexcept ;
        };
        motor_typedef( sprite_render_2d ) ;

        struct sprite_sheet
        {
            struct animation
            {
                struct sprite
                {
                    size_t idx ;
                    size_t begin ;
                    size_t end ;
                };
                size_t duration ;
                motor::string_t name ;
                motor::vector< sprite > sprites ;
            };

            struct object
            {
                motor::string_t name ;
                motor::vector< animation > animations ;
            };
            motor::vector< object > objects ;
            

            struct sprite
            {
                motor::math::vec4f_t rect ;
                motor::math::vec2f_t pivot ;
            };
            motor::vector< sprite > rects ;

            sprite determine_sprite( size_t const obj_id, size_t const ani_id, size_t const milli_time ) const noexcept
            {
                for( auto const & s : objects[obj_id].animations[ani_id].sprites )
                {
                    if( milli_time >= s.begin && milli_time < s.end )
                    {
                        return rects[s.idx] ;
                    }
                }

                return sprite() ;
            }
        };
        motor_typedef( sprite_sheet ) ;
    }
}