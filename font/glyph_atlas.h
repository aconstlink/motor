#pragma once

#include "api.h"
#include "typedefs.h"
#include "result.h"

#include <motor/std/string>
#include <motor/std/vector>
#include <motor/math/vector/vector2.hpp>

namespace motor
{
    namespace font
    {
        class MOTOR_FONT_API glyph_atlas
        {
        motor_this_typedefs( glyph_atlas ) ;

        public: // glyph_info

            /// where to find the glyph in the atlas
            struct glyph_info
            {
                size_t point_size ;

                /// the glyph's code point
                utf32_t code_point ;

                /// user defined id
                size_t face_id ;

                /// in which image is the glyph stored
                size_t image_id ;

                /// x: x offset, y: y offset 
                /// in [0,1]
                motor::math::vec2f_t start ;

                /// x: width, y: height
                /// in [0,1]
                motor::math::vec2f_t dims ;

                /// x: horizontal advance
                /// y: vertical advance
                motor::math::vec2f_t adv ;

                float_t bearing ;
            };
            motor_typedef( glyph_info ) ;
            motor_typedefs( motor::vector< glyph_info_t >, glyph_info_map ) ;

        public: // image

            class image
            {
                motor_this_typedefs( image ) ;

            private:

                uint8_ptr_t _plane_ptr = nullptr ;
                size_t _width = 0 ;
                size_t _height = 0 ;

            public:

                image( void_t ) noexcept ;
                image( size_t width, size_t height ) noexcept ;
                image( this_cref_t ) = delete ;
                image( this_rref_t rhv ) noexcept ;
                ~image( void_t ) noexcept ;

                this_ref_t operator = ( this_rref_t rhv ) noexcept ;

            public:

                uint8_ref_t operator[]( size_t const i ) noexcept {
                    return _plane_ptr[ i ] ;
                }

            public:

                uint8_cptr_t get_plane( void_t ) const noexcept { return _plane_ptr ; }
                uint8_ptr_t get_plane( void_t ) noexcept { return _plane_ptr ; }

                size_t width( void_t ) const noexcept { return _width ; }
                size_t height( void_t ) const noexcept { return _height ; }
            };
            motor_typedef( image ) ;

        private:

            glyph_info_map_t _glyph_infos ;
            motor::vector<image_ptr_t> _atlas ;
            
            typedef std::pair< motor::string_t, size_t > name_id_t ;
            motor::vector< name_id_t > _face_name_to_id ;

        public:

            glyph_atlas( void_t ) ;
            glyph_atlas( this_cref_t ) = delete ;
            glyph_atlas( this_rref_t ) ;
            ~glyph_atlas( void_t ) ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

        public: // creator interface

            bool_t add_glyph( glyph_info_cref_t ) ;
            bool_t add_image( this_t::image_rref_t ) ;

        public: // user interface
            
            void_t map_face_id( motor::string_cref_t name, size_t const id ) noexcept ;
            bool_t face_id_for_name( motor::string_cref_t name, size_t & id ) const  noexcept ;

            size_t get_num_glyphs( void_t ) const ;
            bool_t get_glyph_info( size_t const i, glyph_info_out_t ) const ;

            size_t get_num_images( void_t ) const ;
            image_cptr_t get_image( size_t const id ) const ;

            bool_t find_glyph( size_t const, utf32_t const, size_t&, this_t::glyph_info_ref_t ) const ;

        public:

            size_t get_width( void_t ) const ;
            size_t get_height( void_t ) const ;


        };
        motor_typedef( glyph_atlas ) ;
    }
}