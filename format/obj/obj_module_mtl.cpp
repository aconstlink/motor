
#include "obj_module.h"
#include "../module_registry.hpp"

#include <motor/msl/shader/forward_rendering_shader.h>
#include <motor/core/document.hpp>

using namespace motor::format ;

//*******************************************************************************************
motor::format::mtl_file wav_obj_module::load_mtl_file( motor::io::location_in_t loc, motor::string_rref_t the_file,
    motor::io::database_mtr_t db, motor::format::module_registry_mtr_safe_t mod_reg ) noexcept 
{
    motor::format::mtl_file ret ;
    core::document doc( std::move( the_file ) ) ;

    struct image_info
    {
        enum image_type
        {
            unknown,
            diffuse,
            specular,
            ambient
        };
        size_t idx ;
        image_type it ;
        motor::io::location_t loc ;
        motor::format::future_item_t item ;
    };
    motor::vector< image_info > tx_caches ;

    // load material info
    // also issues async image imports
    {
        motor::format::mtl_file::material cur_mat ;
        size_t mat_idx = size_t( -1 ) ;

        doc.for_each_line( [&] ( core::document::line_view const & line )
        {
            auto const & l = line.get_line() ;
            size_t const num_tokens = line.get_num_tokens() ;

            // newmtl
            if ( num_tokens == 2 && line.get_token( 0 ) == "newmtl" )
            {
                if ( mat_idx++ != size_t( -1 ) )
                {
                    ret.materials.emplace_back( std::move( cur_mat ) ) ;
                    cur_mat = motor::format::mtl_file::material() ;
                }

                cur_mat.name = motor::string_t( line.get_token( 1 ) ) ;
            }
            else if ( line.get_token( 0 ) == "Ks" && num_tokens == 4 )
            {
                float_t const x = std::atof( line.get_token( 1 ).data() ) ;
                float_t const y = std::atof( line.get_token( 2 ).data() ) ;
                float_t const z = std::atof( line.get_token( 3 ).data() ) ;

                cur_mat.specular_color = motor::math::vec3f_t( x, y, z ) ;

            }
            else if ( line.get_token( 0 ) == "Kd" && num_tokens == 4 )
            {
                float_t const x = std::atof( line.get_token( 1 ).data() ) ;
                float_t const y = std::atof( line.get_token( 2 ).data() ) ;
                float_t const z = std::atof( line.get_token( 3 ).data() ) ;

                cur_mat.diffuse_color = motor::math::vec3f_t( x, y, z ) ;
            }
            else if ( line.get_token( 0 ) == "Ka" && num_tokens == 4 )
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
            else if ( line.get_token( 0 ) == "map_Kd" && num_tokens >= 2 )
            {
                motor::string_t loc_name = motor::string_t( line.get_token( 1 ) ) ;
                cur_mat.map_diffuse = line.get_token( 1 ) ;

                for ( size_t i = 2; i < line.get_num_tokens(); ++i )
                {
                    loc_name += motor::string_t( " " ) ;
                    loc_name += line.get_token( i ) ;

                    cur_mat.map_diffuse += motor::string_t( "_" ) ;
                    cur_mat.map_diffuse += line.get_token( i ) ;
                }

                {
                    motor::io::location_t tx_loc = motor::io::location_t::from_path(
                        loc.as_path().parent_path().append( loc_name ) ) ;

                    tx_caches.emplace_back( image_info {
                        mat_idx + 1, image_info::image_type::diffuse,
                        tx_loc, mod_reg->import_from( tx_loc, db )
                        } )  ;
                }

                // use the locations' name as the resource locator
                {
                    motor::io::location_t tx_loc = motor::io::location_t::from_path(
                        loc.as_path().parent_path().append( cur_mat.map_diffuse ) ) ;
                    
                    auto const name = motor::io::location_t::from_path( 
                        tx_loc.as_path().replace_extension("") ).as_string() ;
                    
                    cur_mat.map_diffuse = name ;
                }
            }
            else if ( line.get_token( 0 ) == "map_Ks" && num_tokens == 2 )
            {

            }
            else if ( line.get_token( 0 ) == "map_Ka" && num_tokens == 2 )
            {

            }
            else if ( line.get_token( 0 ) == "illum" && num_tokens >= 2 )
            {
                // not used right now
                cur_mat.illum_model = 0 ;
            }
        } ) ;

        ret.materials.emplace_back( std::move( cur_mat ) ) ;
    }

    // loading textures
    {
        for ( auto & ca : tx_caches )
        {
            motor::format::item_mtr_t iitem = ca.item.get() ;

            if( auto * image_item = dynamic_cast<motor::format::image_item_ptr_t>( iitem ); image_item != nullptr  )
            {
                motor::format::mtl_file::material & cur_mat = ret.materials[ca.idx] ;

                if( ca.it == image_info::image_type::diffuse )
                {
                    cur_mat.image_diffuse = motor::move( image_item->img ) ;
                }
                else if ( ca.it == image_info::image_type::specular )
                {
                    cur_mat.image_specular = motor::move( image_item->img ) ;
                }
                
                motor::release( motor::move( image_item ) ) ;
            }
            else if( auto * status_item = dynamic_cast<motor::format::status_item_ptr_t>( iitem ); status_item != nullptr )
            {
                motor::log::global::error( "[obj material] : " + status_item->msg ) ;
                motor::release( motor::move( status_item ) ) ;
            }
            else
            {
                motor::log::global::error( "[obj material] : can not import image : " + ca.loc.as_string() ) ;
                motor::release( motor::move( iitem ) ) ;
            }
        }
    }

    motor::release( mod_reg ) ;

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