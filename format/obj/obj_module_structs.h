
#pragma once

#include <motor/math/vector/vector3.hpp>

namespace motor
{
    namespace format
    {
        struct mtl_file
        {
            // starts with newmtl
            struct material
            {
                motor::string_t name ;

                motor::math::vec3f_t ambient_color ; // Ka
                motor::math::vec3f_t diffuse_color ; // Kd
                motor::math::vec3f_t specular_color ; // Ks

                float_t specular_exp ; // Ns

                float_t dissolve ; // d <=> Tr = 1 - d

                float_t refraction ; // Ni

                byte_t illum_model ; // illum

                motor::string_t map_diffuse ; // map_Kd
                motor::string_t map_ambient ; // map_Ka
                motor::string_t map_specular ; // map_Ks
                motor::string_t map_dissolve ; // map_d

                // set if dissolve or map_dissolve is set
                bool_t requires_alpha_blending = false ;

                static material make_default( motor::string_in_t name ) noexcept
                {
                    return material
                    {
                        name,
                        motor::math::vec3f_t(1.0f),
                        motor::math::vec3f_t(0.0f),
                        motor::math::vec3f_t(0.0f),
                        0.0,
                        1.0,
                        1.0,
                        1,
                        "", "", "", "", false
                    } ;
                }
            };

            struct image
            {
                enum image_type
                {
                    unknown,
                    diffuse,
                    specular,
                    ambient,
                    alpha
                };

                image_type it ;
                motor::string_t name ;
                motor::graphics::image_mtr_t the_image = nullptr ;
            };

            motor::vector< material > materials ;
            motor::vector< image > images ;
        };

        struct material_info
        {
            motor::string_t name ;

            bool_t has_nrm ;
            bool_t has_tx ;
            byte_t tx_comps ;

            motor::format::mtl_file::material mat ;
        };
        motor_typedef( material_info ) ;
    }
}