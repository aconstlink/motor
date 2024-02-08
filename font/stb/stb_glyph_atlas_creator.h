
#pragma once

#include "../structs.h"
#include "../glyph_atlas.h"

namespace motor
{
    namespace font
    {
        namespace stb
        {
            struct MOTOR_FONT_API glyph_atlas_creator
            {
                motor_this_typedefs( glyph_atlas_creator ) ;

            public:

                static glyph_atlas_t create_glyph_atlas( font_files_cref_t din, 
                    size_t const pt, size_t const dpi, motor::font::code_points_cref_t cps, 
                    size_t const max_img_width = 1024, size_t const max_img_height = 1024 ) ;

            };
            motor_typedef( glyph_atlas_creator ) ;
        }
    }
}