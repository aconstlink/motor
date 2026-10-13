
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "gl4.h"
#include "gl4_convert.h"

#include <motor/ogl/gl/gl.h>
#include <motor/ogl/gl/convert.hpp>
#include <motor/ogl/gl/error.hpp>
#include <motor/memory/global.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/std/vector>
#include <motor/std/stack>
#include <motor/std/string_split.hpp>
#include <motor/std/hash_map>

using namespace motor::platform::gen4;
using namespace motor::ogl;

#define gl4_log( text ) "[GL4] : " text
#define gl4_log_error( text ) motor::ogl::error::check_and_log( "[GL4] : " text )
#define gl4_log_error2( text, name ) motor::ogl::error::check_and_log( "[GL4] : " text, name )
#define gl4_log_error3( text, obj )                                                                \
    motor::ogl::error::check_and_log< decltype( obj ) >( "[GL4] : " text, obj )

struct gl4_independent
{
    motor_this_typedefs( gl4_independent );

    static bool_t construct_shader_data( motor::graphics::shader_object_ref_t obj ) noexcept
    {
    }

    //****************************************************************************************
    static bool_t compile_shader( GLuint const id, motor::string_cref_t code )
    {
        if( code.empty() ) return false;

        GLchar const * source_string = (GLchar const *)( code.c_str() );

        glShaderSource( id, 1, &source_string, 0 );
        if( gl4_log_error( "glShaderSource" ) ) return false;

        glCompileShader( id );
        if( gl4_log_error( "glCompileShader" ) ) return false;

        GLint ret;
        glGetShaderiv( id, GL_COMPILE_STATUS, &ret );

        GLint length;
        glGetShaderiv( id, GL_INFO_LOG_LENGTH, &length );

        if( ret == GL_TRUE && length <= 1 ) return true;

        if( length == 0 )
        {
            gl4_log_error( "shader compilation failed, but info log length is 0." );
            return false;
        }

        // print first line for info
        // user can place the shader name or any info there.
        {
            size_t pos = code.find_first_of( '\n' );
            motor::log::global::error( gl4_log( "First Line: " + code.substr( 0, pos ) ) );
        }

        // get the error message it is and print it
        {
            motor::memory::malloc_guard< char > info_log( length );

            glGetShaderInfoLog( id, length, 0, info_log );
            gl4_log_error( "glGetShaderInfoLog" );

            motor::vector< motor::string_t > tokens;
            motor::mstd::string_ops::split( motor::string_t( info_log ), '\n', tokens );

            for( auto const & msg : tokens )
            {
                motor::log::global::error( msg );
            }
        }
        return false;
    }
};