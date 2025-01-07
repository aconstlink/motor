
#include "forward_rendering_shader.h"

#include <motor/core/document.hpp>

using namespace motor::msl ;

//************************************************************************
forward_rendering_shader::forward_rendering_shader( generator_info_rref_t gi) noexcept 
{
    motor::core::document shader ;

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
            shader.println_if( "in vec3_t tx : texcoord0 ;", gi.has_texcoords() ) ;

            shader.println( "" ) ;
            shader.println( "out vec4_t pos : position ;" ) ;
            shader.println_if( "out vec3_t nrm : normal ;", gi.has_normals() ) ;
            shader.println_if( "out vec3_t tx : texcoord0 ;", gi.has_texcoords() ) ;

            shader.println( "" ) ;
            shader.println( "mat4_t world : world ;" ) ;
            shader.println( "mat4_t view : view ;" ) ;
            shader.println( "mat4_t proj : proj ;" ) ;
            shader.println( "" ) ;

            shader.println( "void main()" ) ;
            shader.println( "{" ) ;
            {
                motor::core::document::section_guard main_section( &shader ) ;
                shader.println_if( "out.nrm = in.nrm ;", gi.has_normals()  ) ;
                shader.println( "out.pos = proj * view * world * vec4_t( in.pos, 1.0 ) ;" ) ;
            }
            shader.println( "}" ) ;
        }
        shader.println( "}" ) ;


        shader.println( "pixel_shader" ) ;
        shader.println( "{" ) ;
        {
            motor::core::document::section_guard vs_section( &shader ) ;

            shader.println_if( "in vec3_t nrm : normal ;", gi.has_normals() ) ;
            shader.println_if( "in vec3_t tx : texcoord0 ;", gi.has_texcoords() ) ;

            if( gi.use_ambient )
            {
                char buffer[2048]  ;
                sprintf_s( buffer, 2048, "vec3_t Ka( %.5f, %.5f, %.5f ) ;", gi.ambient.x(), gi.ambient.y(), gi.ambient.z() ) ;
                shader.println( buffer ) ;
            }
            else
            {
                shader.println( "vec3_t Ka ;" ) ;
            }

            if( gi.use_diffuse )
            {
                char buffer[ 2048 ]  ;
                sprintf_s( buffer, 2048, "vec3_t Kd( %.5f, %.5f, %.5f ) ;", gi.diffuse.x(), gi.diffuse.y(), gi.diffuse.z() ) ;
                shader.println( buffer ) ;
            }
            else
            {
                shader.println( "vec3_t Kd ;" ) ;
            }

            
            
            shader.println( "vec3_t Ks ;" ) ;

            shader.println_if( "vec3_t light_dir ;", gi.has_normals() && gi.has_any_light() ) ;

            shader.println( "out vec4_t color : color ;" ) ;

            shader.println( "void main()" ) ;
            shader.println( "{" ) ;
            {
                motor::core::document::section_guard main_section( &shader ) ;

                if ( gi.has_normals() && gi.has_any_light() )
                {
                    shader.println( "float_t ndl = dot( in.nrm, light_dir ) ;" ) ;
                    shader.println( "vec3_t color = Ka + Kd ' ndl ;" ) ;
                    shader.println( "out.color = vec4_t( color, 1.0 ) ;" ) ;
                }
                else
                {
                    shader.println( "out.color = vec4_t(Kd.x, Kd.y, Kd.z, 1.0 ) ;" ) ;
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