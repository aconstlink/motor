
#include "forward_rendering_shader.h"

#include <motor/core/document.hpp>

#include <cstdio>

using namespace motor::msl ;

//************************************************************************
forward_rendering_shader::forward_rendering_shader( generator_info_rref_t gi ) noexcept 
{
    motor::core::document shader ;

    // if the diffuse and disslove textures have the same namee
    // read from the alpha channel of the diffuse texture.
    // otherwise, read from the dissolve texture.
    bool_t const read_alpha_from_diffuse = 
        gi.diffuse_tx_defv == gi.dissolve_tx_defv ;

    shader.println( motor::string_t( "config " ) + gi.name ) ;
    shader.println( "{" ) ;

    {
        motor::core::document::section_guard config_section( &shader ) ;

        shader.println( "vertex_shader" ) ;
        shader.println( "{" ) ;
        {
            motor::core::document::section_guard vs_section( &shader ) ;

            shader.println( "in vec3_t pos : position ;" ) ;
            shader.println_if( "in vec3_t nrm : normal ;", gi.has_normals() ) ;
            shader.println_if( "in vec2_t tx : texcoord0 ;", gi.has_texcoords() ) ;

            shader.println( "" ) ;
            shader.println( "out vec4_t pos : position ;" ) ;
            shader.println_if( "out vec3_t nrm : normal ;", gi.has_normals() ) ;
            shader.println_if( "out vec2_t tx : texcoord0 ;", gi.has_texcoords() ) ;
            shader.println_if( "out float_t ndl ;", !gi.per_pixel_lighing ) ;
            
            shader.println( "" ) ;
            shader.println( "mat4_t world : world ;" ) ;
            shader.println( "mat4_t view : view ;" ) ;
            shader.println( "mat4_t proj : proj ;" ) ;

            shader.println_if( "vec3_t light_dir( 1.0, 1.0, 1.0 ) ;", !gi.per_pixel_lighing ) ;
            shader.println( "" ) ;

            shader.println( "void main()" ) ;
            shader.println( "{" ) ;
            {
                motor::core::document::section_guard main_section( &shader ) ;
                if( !gi.per_pixel_lighing )
                {
                    shader.println_if( "out.ndl = dot( normalize( light_dir ), in.nrm ) ;", gi.has_normals() ) ;
                }
                else
                {
                    shader.println_if( "out.nrm = in.nrm ;", gi.has_normals() ) ;
                }
                
                shader.println( "out.pos = proj * view * world * vec4_t( in.pos, 1.0 ) ;" ) ;

                shader.println_if( "out.tx = in.tx ;", gi.has_texcoords()  ) ;

                if( !gi.per_pixel_lighing )
                {
                }
                else
                {
                }
            }
            shader.println( "}" ) ;
        }
        shader.println( "}" ) ;

        shader.println( "" ) ;

        shader.println( "pixel_shader" ) ;
        shader.println( "{" ) ;
        {
            motor::core::document::section_guard vs_section( &shader ) ;

            shader.println_if( "in vec3_t nrm : normal ;", gi.has_normals() ) ;
            shader.println_if( "in vec2_t tx : texcoord0 ;", gi.has_texcoords() ) ;
            shader.println_if( "in float_t ndl ;", !gi.per_pixel_lighing ) ;

            shader.println( "" ) ;

            if( !gi.ambient_tx_name.empty() )
            {
                char buffer[2048]  ;
                std::snprintf( buffer, 2048, "tex2d_t %s( \"%s\" ) ;", gi.ambient_tx_name.c_str(), gi.ambient_tx_defv.c_str() ) ;
                shader.println( buffer ) ;
            }

            if( !gi.diffuse_tx_name.empty() )
            {
                char buffer[ 2048 ]  ;
                std::snprintf( buffer, 2048, "tex2d_t %s( \"%s\" ) ;", gi.diffuse_tx_name.c_str(), gi.diffuse_tx_defv.c_str() ) ;
                shader.println( buffer ) ;
            }

            if ( !gi.dissolve_tx_name.empty() && !read_alpha_from_diffuse)
            {
                char buffer[ 2048 ]  ;
                std::snprintf( buffer, 2048, "tex2d_t %s( \"%s\" ) ;", gi.dissolve_tx_name.c_str(), gi.dissolve_tx_defv.c_str() ) ;
                shader.println( buffer ) ;
            }
            
            shader.println( "" ) ;

            if ( gi.per_pixel_lighing )
            {
                shader.println_if( "vec3_t light_dir ;", gi.has_normals() ) ;
            }

            shader.println( "" ) ;

            if( gi.use_ambient )
            {
                char buffer[2048]  ;
                std::snprintf( buffer, 2048, "vec3_t Ka( %.5f, %.5f, %.5f ) ;", gi.ambient.x(), gi.ambient.y(), gi.ambient.z() ) ;
                shader.println( buffer ) ;
            }
            else
            {
                shader.println( "vec3_t Ka( 0.0, 0.0, 0.0 ) ;" ) ;
            }

            if( gi.use_diffuse )
            {
                char buffer[ 2048 ]  ;
                std::snprintf( buffer, 2048, "vec3_t Kd( %.5f, %.5f, %.5f ) ;", gi.diffuse.x(), gi.diffuse.y(), gi.diffuse.z() ) ;
                shader.println( buffer ) ;
            }
            else
            {
                shader.println( "vec3_t Kd( 0.0, 0.0, 0.0 ) ;" ) ;
            }
            
            
            shader.println( "vec3_t Ks ;" ) ;
            shader.println( "" ) ;

            shader.println_if( "vec3_t light_dir ;", gi.has_normals() && gi.has_any_light() && gi.per_pixel_lighing) ;

            shader.println( "out vec4_t color : color ;" ) ;
            shader.println( "" ) ;

            shader.println( "void main()" ) ;
            shader.println( "{" ) ;
            {
                motor::core::document::section_guard main_section( &shader ) ;

                shader.println( "vec3_t ambient = Ka ; " ) ;
                shader.println( "vec3_t diffuse = Kd ; " ) ;
                shader.println( "float_t alpha = 1.0 ; " ) ;

                if ( !gi.ambient_tx_name.empty() && gi.has_texcoords() )
                {
                    char buffer[ 2048 ]  ;
                    std::snprintf( buffer, 2048, "ambient = ambient ' texture( %s, in.tx.xy ).xyz ;", gi.ambient_tx_name.c_str() ) ;
                    shader.println( buffer ) ;
                }

                if ( !gi.diffuse_tx_name.empty() && gi.has_texcoords() )
                {
                    char buffer[ 2048 ]  ;
                    std::snprintf( buffer, 2048, "diffuse = diffuse ' texture( %s, in.tx.xy ).xyz ;", gi.diffuse_tx_name.c_str() ) ;
                    shader.println( buffer ) ;
                }

                if ( !gi.dissolve_tx_name.empty() && gi.has_texcoords() )
                {
                    if( !read_alpha_from_diffuse )
                    {
                        char buffer[ 2048 ]  ;
                        std::snprintf( buffer, 2048, "alpha = texture( %s, in.tx.xy ).r ;", gi.dissolve_tx_name.c_str() ) ;
                        shader.println( buffer ) ;
                    }
                    else
                    {
                        char buffer[ 2048 ]  ;
                        std::snprintf( buffer, 2048, "alpha = texture( %s, in.tx.xy ).a ;", gi.diffuse_tx_name.c_str() ) ;
                        shader.println( buffer ) ;
                    }
                    
                }


                if ( gi.has_normals() && gi.has_any_light() && gi.per_pixel_lighing  )
                {
                    shader.println( "float_t ndl = dot( in.nrm, light_dir ) ;" ) ;
                    shader.println( "vec3_t color = Ka + Kd ' ndl ;" ) ;
                    shader.println( "out.color = vec4_t( color, alpha ) ;" ) ;
                }
                else if ( gi.has_normals() && gi.has_any_light() && !gi.per_pixel_lighing )
                {
                    shader.println( "float_t ndl = in.ndl ; " ) ;
                    shader.println( "diffuse = diffuse * ndl ;" ) ;
                    shader.println( "out.color = vec4_t( ambient + diffuse, alpha ) ;" ) ;
                    //shader.println( "out.color = vec4_t(in.tx*3, 1.0) ;" ) ;
                }

                

            }
            shader.println( "}" ) ;

        }
        shader.println( "}" ) ;

    }

    shader.println( "}" ) ;


    _doc = std::move( shader ) ;
}

//************************************************************************
forward_rendering_shader::forward_rendering_shader( this_rref_t rhv ) noexcept :
    _doc( std::move( rhv._doc ) )
{
}

//************************************************************************
forward_rendering_shader::~forward_rendering_shader( void_t ) noexcept
{
}

//************************************************************************
motor::string_t forward_rendering_shader::to_string( void_t ) const noexcept 
{
    return _doc.to_string() ;
}