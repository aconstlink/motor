
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
            };

            motor::vector< material > materials ;
        };

        struct material_info
        {
            motor::string_t name ;

            bool_t has_nrm ;
            bool_t has_tx ;

            motor::format::mtl_file::material mat ;
        };
        motor_typedef( material_info ) ;
    }
}