
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../future_item.hpp"

#include <motor/std/vector>
#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector4.hpp>

namespace motor
{
    namespace format
    {
        struct motor_document
        {
            struct sprite_sheet
            {
                struct image
                {
                    motor::string_t src ;
                };
                motor_typedef( image ) ;
                image_t image ;

                struct sprite
                {
                    struct hit
                    {
                        motor::string_t name ;
                        motor::math::vec4ui_t rect ;
                    };
                    motor_typedef( hit ) ;

                    struct animation
                    {
                        motor::math::vec4ui_t rect ;
                        motor::math::vec2i_t pivot ;
                    };
                    motor_typedef( animation ) ;

                    struct collision
                    {
                        motor::math::vec4ui_t rect ;
                    };
                    motor_typedef( collision ) ;
                    
                    motor::string_t name ;
                    
                    animation_t animation ;
                    collision_t collision ;
                    motor::vector< hit_t > hits ;
                };
                motor_typedef( sprite ) ;
                motor::vector< sprite_t > sprites ;

                struct animation
                {
                    struct frame
                    {
                        motor::string_t sprite ;
                        size_t duration ;
                    };
                    motor_typedef( frame ) ;

                    motor::string_t object ;
                    motor::string_t name ;
                    motor::vector< frame_t > frames ;
                };
                motor_typedef( animation ) ;
                motor::vector< animation_t > animations ;

                motor::string_t name ;
            };
            motor_typedef( sprite_sheet ) ;

            motor::vector< sprite_sheet_t > sprite_sheets ;
        };
        motor_typedef( motor_document ) ;
    }
}