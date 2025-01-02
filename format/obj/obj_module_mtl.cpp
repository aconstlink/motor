
#include "obj_module.h"

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
static void_t _pl_( size_t const indent, motor::string_in_t ln, motor::string_ref_t shader, bool_t const b ) noexcept
{
    if ( !b ) return ;
    for ( size_t i = 0; i < indent << 2; ++i ) shader += " " ;
    shader += ln + "\n" ;
}

//*******************************************************************************************
static void_t _pl_( size_t const indent, motor::string_in_t ln, motor::string_ref_t shader ) noexcept
{
    return _pl_( indent, ln, shader, true ) ;
}

//*******************************************************************************************
motor::string_t wav_obj_module::generate_forward_shader( material_info_in_t mi ) noexcept
{
    size_t indent = 0 ;
    motor::string_t shader ;

    _pl_( indent, "config " + mi.name, shader ) ;
    _pl_( indent, "{", shader ) ;
    ++indent ;

    // vertex shader
    {
        _pl_( indent, "vertex_shader", shader ) ;
        _pl_( indent, "{", shader ) ;
        ++indent ;


        _pl_( indent, "in vec3_t pos : position ;", shader ) ;
        _pl_( indent, "in vec3_t nrm : normal ;", shader, mi.has_nrm ) ;
        _pl_( indent, "in vec3_t tx : texcoord0 ;", shader, mi.has_tx ) ;

        _pl_( indent, "", shader ) ;
        _pl_( indent, "out vec4_t pos : position ;", shader ) ;
        _pl_( indent, "out vec3_t nrm : normal ;", shader, mi.has_nrm ) ;
        _pl_( indent, "out vec3_t tx : texcoord0 ;", shader, mi.has_tx ) ;

        _pl_( indent, "", shader ) ;
        _pl_( indent, "mat4_t world : world ;", shader ) ;
        _pl_( indent, "mat4_t view : view ;", shader ) ;
        _pl_( indent, "mat4_t proj : proj ;", shader ) ;
        _pl_( indent, "", shader ) ;

        _pl_( indent, "void main()", shader ) ;
        _pl_( indent, "{", shader ) ;
        ++indent ;
        {
            _pl_( indent, "out.nrm = in.nrm ;", shader, mi.has_nrm ) ;
            _pl_( indent, "out.pos = proj * view * world * vec4_t( in.pos, 1.0 ) ;", shader ) ;
        }
        --indent ;
        _pl_( indent, "}", shader ) ;


        --indent ;
        _pl_( indent, "}", shader ) ;
    }

    {
        _pl_( indent, "pixel_shader", shader ) ;
        _pl_( indent, "{", shader ) ;
        ++indent ;

        _pl_( indent, "in vec3_t nrm : normal ;", shader, mi.has_nrm ) ;
        _pl_( indent, "in vec3_t tx : texcoord0 ;", shader, mi.has_tx ) ;

        _pl_( indent, "vec3_t Ka ;", shader ) ;
        _pl_( indent, "vec3_t Kd ;", shader ) ;
        _pl_( indent, "vec3_t Ks ;", shader ) ;

        _pl_( indent, "vec3_t light_dir ;", shader, mi.has_nrm ) ;

        _pl_( indent, "out vec4_t color : color ;", shader ) ;

        _pl_( indent, "void main()", shader ) ;
        _pl_( indent, "{", shader ) ;
        ++indent ;
        if ( mi.has_nrm )
        {
            _pl_( indent, "float_t ndl = dot( in.nrm, light_dir ) ;", shader ) ;
            _pl_( indent, "vec3_t color = Ka + Kd ' ndl ;", shader ) ;
            _pl_( indent, "out.color = vec4_t( color, 1.0 ) ;", shader ) ;
        }
        else
        {
            _pl_( indent, "out.color = vec4_t(Kd.x, Kd.y, Kd.z, 1.0 ) ;", shader ) ;
        }
        --indent ;
        _pl_( indent, "}", shader ) ;
        --indent ;
        _pl_( indent, "}", shader ) ;
    }

    --indent ;
    _pl_( indent, "}", shader ) ;

    return shader ;
}