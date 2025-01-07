
#include "obj_module.h"

#include <motor/msl/shader/forward_rendering_shader.h>
#include <motor/core/document.hpp>

using namespace motor::format ;

//*******************************************************************************************
motor::format::mtl_file wav_obj_module::load_mtl_file( motor::io::location_in_t loc, motor::string_rref_t the_file ) noexcept 
{
    motor::format::mtl_file ret ;
    motor::format::mtl_file::material cur_mat ;

    core::document doc( std::move( the_file ) ) ;
    
    size_t mat_idx = size_t( -1 ) ;

    doc.for_each_line( [&](core::document::line_view const & line)
    {  
        auto const & l = line.get_line() ;
        size_t const num_tokens = line.get_num_tokens() ;
        
        // newmtl
        if( num_tokens == 2 && line.get_token( 0 ) == "newmtl"  ) 
        {
            if( mat_idx++ != size_t( -1 ) )
            {
                ret.materials.emplace_back( std::move( cur_mat ) ) ;
                cur_mat = motor::format::mtl_file::material() ;
            }

            cur_mat.name = motor::string_t( line.get_token( 1 ) ) ;
        }
        else if ( line.get_token( 0 ) == "Ks" && num_tokens == 4 )
        {
            float_t const x = std::atof( line.get_token(1).data() ) ;
            float_t const y = std::atof( line.get_token(2).data() ) ;
            float_t const z = std::atof( line.get_token(3).data() ) ;

            cur_mat.specular_color = motor::math::vec3f_t( x, y, z ) ;

        }
        else if ( line.get_token( 0 ) == "Kd" && num_tokens == 4  )
        {
            float_t const x = std::atof( line.get_token( 1 ).data() ) ;
            float_t const y = std::atof( line.get_token( 2 ).data() ) ;
            float_t const z = std::atof( line.get_token( 3 ).data() ) ;

            cur_mat.diffuse_color = motor::math::vec3f_t( x, y, z ) ;
        }
        else if ( line.get_token( 0 ) == "Ka" && num_tokens == 4  )
        {
            float_t const x = std::atof( line.get_token( 1 ).data() ) ;
            float_t const y = std::atof( line.get_token( 2 ).data() ) ;
            float_t const z = std::atof( line.get_token( 3 ).data() ) ;

            cur_mat.ambient_color = motor::math::vec3f_t( x, y, z ) ;
        }
        else if ( line.get_token( 0 ) == "Ns" && num_tokens == 2 )
        {
            cur_mat.specular_exp = std::atof( line.get_token( 1 ).data() ) ;
        }
        else if ( line.get_token( 0 ) == "map_Kd" && num_tokens == 2 )
        {
            cur_mat.map_diffuse = line.get_token( 1 ) ;
        }
        else if ( line.get_token( 0 ) == "map_Ks" && num_tokens == 2 )
        {
            cur_mat.map_specular = line.get_token( 1 ) ;
        }
        else if ( line.get_token( 0 ) == "map_Ka" && num_tokens == 2 )
        {
            cur_mat.map_ambient = line.get_token( 1 ) ;
        }
        else if ( line.get_token( 0 ) == "illum" && num_tokens >= 2 )
        {
            // not used right now
            cur_mat.illum_model = 0 ;
        }        
    } ) ;

    ret.materials.emplace_back( std::move( cur_mat ) ) ;

    return ret ;
}

//*******************************************************************************************
motor::string_t wav_obj_module::generate_forward_shader( material_info_in_t mi ) noexcept
{
    motor::msl::forward_rendering_shader shader( 
        motor::msl::forward_rendering_shader::generator_info
        { 
            mi.name, 3, 3, 0, true, mi.mat.ambient_color, true, mi.mat.diffuse_color 
        } )  ;
    
    return shader.to_string() ;
}