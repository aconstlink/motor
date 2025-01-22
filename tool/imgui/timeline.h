#pragma once

#include "imgui.h"
#include "../structs.h"

#include <motor/std/string>
#include <motor/std/vector>

#include <array>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API timeline
        {
            motor_this_typedefs( timeline ) ;


            // temporary buffer for producing the time string
            char _time_string_buffer[2048] ;

        private:

            // unit: milli_per_pixel
            size_t _zoom = 0 ;

            // in milli seconds
            size_t _hover = 0 ;

            // in milli seconds
            size_t _play = 0 ;

            // move timeline with player marker
            bool_t _lock_player = false ;

            size_t _max_milli = 0 ;

            motor::string_t _label ;

            bool_t _begin = false ;

            // used for moving the timeline
            ImVec2 _old_mouse_pos ;

        public:

            timeline( motor::string_in_t ) noexcept ;
            timeline( this_cref_t ) = delete ;
            timeline( this_rref_t ) noexcept ;
            ~timeline( void_t ) noexcept ;

            this_ref_t operator = ( this_cref_t ) noexcept ;
            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            bool_t begin( motor::tool::time_info_ref_t ti ) noexcept ;
            void_t end( void_t ) noexcept ;

        public:

            // add to the top of the timeline(down tri)
            bool_t set_time_marker( size_t const milli, bool_t const center = false ) noexcept ;
            
            // vertical line at milli
            bool_t set_hover_marker( size_t const milli ) noexcept ;

            // add to the bottom of the timeline(up tri)
            bool_t add_marker( size_t const milli, ImColor const & color ) ;

            struct time_region{
                size_t milli_begin ;
                size_t milli_end ;
            } ;
            motor_typedef( time_region ) ;

            // returns the region where the current view
            // begins and ends taking the zoom into account
            time_region_t get_time_region( void_t ) const noexcept ;

        public:

            inline size_t milli_to_pixel( size_t const ) const noexcept ;
            inline size_t pixel_to_milli( size_t const ) const noexcept ;

            size_t mtp( size_t const m ) const noexcept { return this_t::milli_to_pixel( m )  ; }
            size_t ptm( size_t const p ) const noexcept { return this_t::pixel_to_milli( p ) ; }

        private:

            char const * const make_time_string( size_t const milli ) noexcept ;
            char const * const make_time_string2( size_t const milli ) noexcept ;


        } ;
        motor_typedef( timeline ) ;
    }
}