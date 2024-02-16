#pragma once

#include "../typedefs.h"
#include "../result.h"
#include "../api.h"

#include <motor/graphics/object/render_object.h>
#include <motor/graphics/object/shader_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/object/image_object.h>
#include <motor/graphics/object/state_object.h>
#include <motor/graphics/object/array_object.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/font/glyph_atlas.h>

#include <motor/io/location.hpp>
#include <motor/io/database.h>

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/matrix/matrix4.hpp>

#include <motor/std/vector>

#include <functional>

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API text_render_2d
        {
            motor_this_typedefs( text_render_2d ) ;

        private:
            
            motor::font::glyph_atlas_mtr_t _ga ;

            motor::string_t _name ;

        private: // default matrices

            motor::math::mat4f_t _proj ;
            motor::math::mat4f_t _view ;

        private:

            struct vertex 
            { 
                motor::math::vec2f_t pos ; 
            } ;

            motor::graphics::render_object_mtr_t _rc = nullptr ;
            motor::graphics::shader_object_mtr_t _sc = nullptr ;
            motor::graphics::geometry_object_mtr_t _gc = nullptr ;
            motor::graphics::image_object_mtr_t _ic = nullptr ;
            motor::graphics::state_object_mtr_t  _sto = nullptr ;

            motor::vector< motor::graphics::variable_set_mtr_t > _vars ;

            /// storing the glyph infos of the glyph atlas
            motor::graphics::array_object_mtr_t _glyph_infos = nullptr ;
            /// storing the text infos for rendering
            motor::graphics::array_object_mtr_t _text_infos = nullptr ;

            size_t num_quads = 1000 ;

        private:

            struct render_info
            {
                size_t start = 0 ;
                size_t num_elems = 0 ;
            };
            motor_typedef( render_info ) ;

            struct glyph_info
            {
                size_t offset ;
                motor::math::vec2f_t pos ;
                motor::math::vec3f_t color ;

                float_t point_size_scale ;
                float_t adv_x ;
            };
            motor_typedef( glyph_info ) ;

            struct group_info
            {
                motor_this_typedefs( group_info ) ;

                render_info_t ri ;

                motor::concurrent::mutex_t mtx ;
                motor::vector< glyph_info > glyph_infos ;
                
                motor::math::mat4f_t proj ;
                motor::math::mat4f_t view ;

                group_info( void_t ){}
                group_info( this_cref_t rhv ){
                    glyph_infos = rhv.glyph_infos ;
                    proj = rhv.proj ;
                    view = rhv.view ;
                }
                group_info( this_rref_t rhv ) {
                    glyph_infos = std::move( rhv.glyph_infos ) ;
                    proj = rhv.proj ;
                    view = rhv.view ;
                }
                ~group_info( void_t ) {}

                this_ref_t operator = ( this_cref_t rhv ) {
                    glyph_infos = rhv.glyph_infos ;
                    proj = rhv.proj ;
                    view = rhv.view ;
                    return *this ;
                }
                this_ref_t operator = ( this_rref_t rhv ) {
                    glyph_infos = std::move( rhv.glyph_infos ) ;
                    proj = rhv.proj ;
                    view = rhv.view ;
                    return *this ;
                }
            };
            motor_typedef( group_info ) ;
            motor_typedefs( motor::vector< group_info >, group_infos ) ;

            motor::concurrent::mutex_t _mtx_lis ;
            group_infos_t _gis ;

            motor_typedefs( motor::vector< size_t >, render_group_ids ) ;
            render_group_ids_t _render_groups ;

        public:

            text_render_2d( void_t ) noexcept ;
            text_render_2d( this_cref_t ) = delete ;
            text_render_2d( this_rref_t ) noexcept ;
            ~text_render_2d( void_t ) noexcept ;

        public:
            
            // can be used to set the atlas before init.
            // @note requires init afterwards
            void_t set_glyph_atlas( motor::font::glyph_atlas_mtr_safe_t ga ) noexcept { _ga = ga ; }

            void_t init( motor::string_cref_t, motor::font::glyph_atlas_mtr_safe_t, size_t const = 10 ) noexcept ;
            void_t init( motor::string_cref_t, size_t const = 10 ) noexcept ;

            void_t on_frame_init( motor::graphics::gen4::frontend_mtr_t ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) ;
            void_t set_view_proj( size_t const, motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) ;

            motor::gfx::result draw_text( size_t const group, size_t const font_id, size_t const point_size,
                motor::math::vec2f_cref_t pos, motor::math::vec4f_cref_t color, motor::string_cref_t ) ;

            //so_gfx::result set_canvas_info( canvas_info_cref_t ) ;
            motor::gfx::result prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t ) ;
            
            motor::gfx::result render( motor::graphics::gen4::frontend_mtr_t, size_t const ) ;

            motor::gfx::result release( void_t ) ;

        private:

            bool_t need_to_render( size_t const ) const noexcept ;

        public:

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

        };
        motor_typedef( text_render_2d ) ;
    }
}
