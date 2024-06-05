#pragma once

#include "imgui.h"

#include <motor/graphics/object/image_object.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/format/future_item.hpp>
#include <motor/format/module_registry.hpp>

#include <motor/io/database.h>
#include <motor/std/string>
#include <motor/std/vector>

#include <array>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API sprite_editor
        {
            motor_this_typedefs( sprite_editor ) ;

        private:

            enum class mode
            {
                bounds,
                pivot,
                hit,
                damage,
                animation
            };

        private:

            motor::io::location_t ss_loc ;
            motor::io::database_mtr_t _db ;
            motor::format::module_registry_mtr_t _mod_reg ;

            struct sprite_sheet
            {
                motor::string_t dname ;
                motor::string_t name ;
                motor::io::location_t img_loc ;

                // do not know if this is even required anymore.
                // only the image data is actually required.
                motor::graphics::image_object_mtr_t img ;
                bool_t img_configured ;

                // image dimensions
                motor::math::vec2ui_t dims ;

                struct sprite
                {
                    motor::string_t name ;
                    size_t bound_idx = size_t(-1) ;
                    size_t pivot_idx = size_t(-1) ;
                    size_t hit_idx = size_t(-1) ;
                    size_t damage_idx = size_t(-1) ;
                };
                motor_typedef( sprite ) ;
                motor::vector< sprite_t > sprites ;

                struct animation_frame
                {
                    size_t duration = size_t( -1 ) ;
                    size_t sidx = size_t( -1 ) ;
                };
                motor_typedef( animation_frame ) ;

                struct animation
                {
                    motor::string_t name ;
                    motor::vector< animation_frame > frames ;
                };
                motor_typedef( animation ) ;
                motor::vector< animation_t > animations ;

                // pixel bounds
                // format : x0, y0, x1, y1
                motor::vector< motor::math::vec4ui_t > bounds ;

                // animation pivots
                motor::vector< motor::math::vec2ui_t > anim_pivots ;

                // hit boxes
                // format : x0, y0, x1, y1
                motor::vector< motor::math::vec4ui_t > hits ;

                // damage boxes
                // format : x0, y0, x1, y1
                motor::vector< motor::math::vec4ui_t > damages ;


                // origin in world space
                // the world space is where the window rect and 
                // the image rect life together in a 1:1 relation
                motor::math::vec2f_t origin ;

                // zoom of the original 1:1 world rect
                // zoom == 1 means the window pixel size 
                // match image pixel size => 1:1
                float_t zoom = -0.5f ;

                struct sequence_item 
                {
                    size_t bound_idx ; // index into bounds
                    size_t pivot_idx ; // index into pivots
                    size_t milli ; // how long to show image
                };
                motor_typedef( sequence_item ) ;
                motor_typedefs( motor::vector< sequence_item_t >, sequence ) ;
                motor::map< motor::string_t, sequence_t > sequences ;
                
            };
            motor_typedef( sprite_sheet ) ;

            motor::vector< sprite_sheet_t > _sprite_sheets ;
            int_t _cur_item = 0 ;

            int_t _pixel_ratio ;
            motor::math::vec2i_t _cur_pixel ;
            
            motor::math::vec2f_t _screen_pos_image ;
            motor::math::vec2f_t _cur_mouse ;
            motor::math::vec2i_t _cr_mouse ;

            // content region dimensions
            motor::math::vec2f_t _crdims ;

            // offset from window orig to content region
            motor::math::vec2ui_t _croff ;

            this_t::mode _cur_mode = this_t::mode::bounds ;
            size_t _cur_hovered = size_t(-1) ;
            size_t _cur_sel_box = size_t(-1) ;
            size_t _cur_sel_ani = size_t(-1) ;
            size_t _cur_sel_frame = size_t( -1 ) ;
            size_t _cur_hovered_frame = size_t(-1) ;
            size_t _cur_hovered_frame_rel = size_t(-1) ;

            motor::vector< size_t > _pivot_ani_frame_sel ;

            bool_t _play_animation = true ;

        private:

            struct rect_drag_info
            {
                bool_t mouse_down_rect = false ;
                bool_t mouse_down_drag = false ;
                motor::math::vec4ui_t cur_rect ;
                motor::math::vec2ui_t drag_begin ;
                size_t drag_idx = size_t(-1) ;
            };
            motor_typedef( rect_drag_info ) ;
            rect_drag_info_t _bounds_drag_info ;
            rect_drag_info_t _hits_drag_info ;
            rect_drag_info_t _damages_drag_info ;

            // used for anim pivot dragging
            bool_t _mouse_down_drag_anim = false ;
            motor::math::vec2ui_t _drag_begin_anim ;
            size_t _drag_idx_anim = size_t(-1) ;

            struct load_item
            {
                motor::string_t disp_name ;
                motor::string_t name ;
                motor::io::location_t loc ;
                motor::format::future_item_t fitem ;
            } ;
            motor_typedef( load_item ) ;

            std::mutex _mtx_loads ;
            motor::vector< load_item_t > _loads ;

        private:

            struct sprite_render_pimpl ;
            motor_typedef( sprite_render_pimpl ) ;
            sprite_render_pimpl_ptr_t _srp = nullptr ;

        public:

            sprite_editor( void_t ) noexcept ;
            sprite_editor( motor::io::database_mtr_safe_t ) noexcept ;
            sprite_editor( this_cref_t ) = delete ;
            sprite_editor( this_rref_t ) noexcept ;
            ~sprite_editor( void_t ) noexcept ;

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t ) noexcept ;

            void_t add_sprite_sheet( motor::string_cref_t name, 
                motor::io::location_cref_t loc ) noexcept ;

            void_t on_update( void_t ) noexcept ;
            void_t on_tool( motor::graphics::gen4::frontend_ptr_t, motor::tool::imgui_ptr_t ) noexcept ;

            void_t store( motor::io::database_mtr_safe_t db ) noexcept ;

        private:

            void_t handle_mouse( int_t const selected ) ;
            void_t handle_mouse_drag_for_bounds( this_t::rect_drag_info_ref_t, motor::vector< motor::math::vec4ui_t > & ) ;
            void_t handle_mouse_drag_for_anim_pivot( int_t const ) ;

            // show the selected image in the content region
            void_t show_image( motor::tool::imgui_ptr_t imgui, int_t const selected ) ;

            // handle the rect the user draws with the mouse
            bool_t handle_rect( this_t::rect_drag_info_ref_t, motor::math::vec4ui_ref_t ) ;

            // rearrange rect so that 
            // smallest xy is bottom/left and 
            // largest xy is top/right
            // all image rects are stored like this
            // format: ( x0, y0, x1, y1 )
            motor::math::vec4ui_t rearrange_mouse_rect( motor::math::vec4ui_cref_t ) const ;

            // returns the current rect that is mapped to the content region in world space.
            // format: ( x0, y0, x1, y1 )
            motor::math::vec4f_t compute_cur_view_rect( int_t const selection ) const ;

            // image rect is bottom/left -> top/right with format x0,y0, x1,y1
            // window rect is top/left -> bottom/right with format x, y, w, h
            // @note the imgui window coords are 
            // x > 0 => left to right
            // y > 0 => top to bottom
            motor::math::vec4f_t image_rect_to_window_rect( int_t const selection, 
                motor::math::vec4f_cref_t image_rect ) const ;

            bool_t is_window_rect_inside_content_region( motor::math::vec4f_ref_t rect, bool_t const fix_coords = true ) const ;

            // rect in window coords with format x, y, w, h
            void_t draw_rect( motor::math::vec4f_cref_t, motor::math::vec4ui_cref_t = 
                motor::math::vec4ui_t(255) ) ;
            void_t draw_rect_for_scale( motor::math::vec4f_cref_t, motor::math::vec4ui_cref_t = 
                motor::math::vec4ui_t(255) ) ;

            void_t draw_scales( motor::math::vec4f_cref_t, motor::math::vec4ui_cref_t prect, 
                motor::math::vec4ui_t = motor::math::vec4ui_t(255) ) ;

            void_t draw_rect_info( motor::math::vec4f_cref_t, motor::math::vec4ui_cref_t ) noexcept ;
                
            size_t draw_rects( motor::vector< motor::math::vec4ui_t > const & rects,  size_t const selected = size_t(-1), size_t const hovered=size_t(-1),
                motor::math::vec4ui_cref_t color = motor::math::vec4ui_t(255),
                motor::math::vec4ui_cref_t over_color = motor::math::vec4ui_t(255) ) ;

            void_t draw_points( motor::vector< motor::math::vec2ui_t > const & points,
                motor::math::vec4ui_cref_t color = motor::math::vec4ui_t(255,0,0,255) ) ;

            bool_t is_ip_mouse_in_bound( motor::math::vec4ui_cref_t ) const ;

            // @precondition mouse must be in bound
            // @param rect image space rect with format x0,y0,x1,y1
            // @param hit
            // [0] : bottom left
            // [1] : left
            // [2] : top left
            // [3] : top
            // [4] : top right
            // [5] : right
            // [6] : bottom right
            // [7] : bottom
            bool_t intersect_bound_location( motor::math::vec2ui_cref_t cur_pixel, motor::math::vec4ui_cref_t rect, std::array< bool_t, 8 > & hit ) const ; 
            
        private:

            void_t display_animations_and_frames( void_t ) noexcept ;

        };
        motor_typedef( sprite_editor ) ;
    }
}