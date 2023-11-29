#pragma once

#include <functional>

namespace natus
{
    namespace ogl
    {
        namespace detail
        {
            static const GLenum uniform_types_gl[] =
            {
                0,
                GL_FLOAT,
                GL_FLOAT_VEC2,
                GL_FLOAT_VEC3,
                GL_FLOAT_VEC4,
                GL_INT,
                GL_INT_VEC2,
                GL_INT_VEC3,
                GL_INT_VEC4,
                GL_UNSIGNED_INT,
                GL_UNSIGNED_INT_VEC2,
                GL_UNSIGNED_INT_VEC3,
                GL_UNSIGNED_INT_VEC4,
                GL_BOOL,
                GL_BOOL_VEC2,
                GL_BOOL_VEC3,
                GL_BOOL_VEC4,
                GL_FLOAT_MAT2,
                GL_FLOAT_MAT3,
                GL_FLOAT_MAT4,
                GL_SAMPLER_2D,
                GL_SAMPLER_3D,
                GL_INT_SAMPLER_2D,
                GL_INT_SAMPLER_3D,
                GL_UNSIGNED_INT_SAMPLER_2D,
                GL_UNSIGNED_INT_SAMPLER_3D,
                GL_SAMPLER_CUBE,
                GL_SAMPLER_2D_SHADOW,
                GL_SAMPLER_2D_ARRAY,
                GL_SAMPLER_BUFFER,
                GL_INT_SAMPLER_BUFFER,
                GL_UNSIGNED_INT_SAMPLER_BUFFER,
                GL_INT_SAMPLER_2D_ARRAY,
                GL_UNSIGNED_INT_SAMPLER_2D_ARRAY
            } ;

            static size_t const uniform_types_gl_size =
                sizeof ( uniform_types_gl ) / sizeof ( uniform_types_gl[ 0 ] ) ;
        }

        static GLuint uniform_size_of( GLenum const e ) noexcept
        {
            switch( e )
            {
            case GL_FLOAT: return sizeof( GLfloat ) ;
            case GL_FLOAT_VEC2: return sizeof( GLfloat ) * 2 ;
            case GL_FLOAT_VEC3: return sizeof( GLfloat ) * 3 ;
            case GL_FLOAT_VEC4: return sizeof( GLfloat ) * 4 ;
            case GL_INT: return sizeof( GLint );
            case GL_INT_VEC2: return sizeof( GLint ) * 2 ;
            case GL_INT_VEC3: return sizeof( GLint ) * 3 ;
            case GL_INT_VEC4: return sizeof( GLint ) * 4 ;
            case GL_UNSIGNED_INT: return sizeof( GLuint );
            case GL_UNSIGNED_INT_VEC2: return sizeof( GLuint ) * 2 ;
            case GL_UNSIGNED_INT_VEC3: return sizeof( GLuint ) * 3 ;
            case GL_UNSIGNED_INT_VEC4: return sizeof( GLuint ) * 4 ;
            case GL_BOOL: return sizeof( GLboolean );
            case GL_BOOL_VEC2: return sizeof( GLboolean ) * 2 ;
            case GL_BOOL_VEC3: return sizeof( GLboolean ) * 3 ;
            case GL_BOOL_VEC4: return sizeof( GLboolean ) * 4 ;
            case GL_FLOAT_MAT2: return sizeof( GLfloat ) * 4 ;
            case GL_FLOAT_MAT3: return sizeof( GLfloat ) * 9 ;
            case GL_FLOAT_MAT4: return sizeof( GLfloat ) * 16 ;
            case GL_SAMPLER_2D: return sizeof( GLint );
            case GL_SAMPLER_3D: return sizeof( GLint );
            case GL_INT_SAMPLER_2D: return sizeof( GLint );
            case GL_INT_SAMPLER_3D: return sizeof( GLint );
            case GL_UNSIGNED_INT_SAMPLER_2D: return sizeof( GLint );
            case GL_UNSIGNED_INT_SAMPLER_3D: return sizeof( GLint );
            case GL_SAMPLER_CUBE: return sizeof( GLint );
            case GL_SAMPLER_2D_SHADOW: return sizeof( GLint );
            case GL_SAMPLER_2D_ARRAY: return sizeof( GLint );
            case GL_SAMPLER_BUFFER: return sizeof( GLint );
            case GL_INT_SAMPLER_BUFFER: return sizeof( GLint );
            case GL_UNSIGNED_INT_SAMPLER_BUFFER: return sizeof( GLint );
            case GL_INT_SAMPLER_2D_ARRAY: return sizeof( GLint );
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return sizeof( GLint );
            default: return 0 ;
            }
            return 0 ;
        }

        static bool_t uniform_is_data( GLenum const e ) noexcept
        {
            switch( e )
            {
            case GL_FLOAT: 
            case GL_FLOAT_VEC2: 
            case GL_FLOAT_VEC3: 
            case GL_FLOAT_VEC4: 
            case GL_INT: 
            case GL_INT_VEC2: 
            case GL_INT_VEC3: 
            case GL_INT_VEC4: 
            case GL_UNSIGNED_INT: 
            case GL_UNSIGNED_INT_VEC2: 
            case GL_UNSIGNED_INT_VEC3: 
            case GL_UNSIGNED_INT_VEC4: 
            case GL_BOOL: 
            case GL_BOOL_VEC2: 
            case GL_BOOL_VEC3: 
            case GL_BOOL_VEC4: 
            case GL_FLOAT_MAT2: 
            case GL_FLOAT_MAT3: 
            case GL_FLOAT_MAT4: return true ;
            default: break ;
            }
            return false ;
        }

        static bool_t uniform_is_texture( GLenum const e ) noexcept
        {
            switch( e )
            {
            case GL_SAMPLER_2D: 
            case GL_SAMPLER_3D: 
            case GL_INT_SAMPLER_2D: 
            case GL_INT_SAMPLER_3D: 
            case GL_UNSIGNED_INT_SAMPLER_2D: 
            case GL_UNSIGNED_INT_SAMPLER_3D: 
            case GL_SAMPLER_CUBE: 
            case GL_SAMPLER_2D_SHADOW: 
            case GL_SAMPLER_2D_ARRAY: 
            case GL_INT_SAMPLER_2D_ARRAY: 
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return true;
            default: break ;
            }
            return false ;
        }

        static bool_t uniform_is_buffer( GLenum const e ) noexcept
        {
            switch( e )
            {
            case GL_SAMPLER_BUFFER: 
            case GL_INT_SAMPLER_BUFFER: 
            case GL_UNSIGNED_INT_SAMPLER_BUFFER: return true ;
            default: break ;
            }
            return false ;
        }

        static GLenum convert( natus::graphics::type const t ) noexcept
        {
            switch( t )
            {
            case natus::graphics::type::tbool:
                return GL_BOOL ;
            case natus::graphics::type::tfloat:
                return GL_FLOAT ;
            case natus::graphics::type::tdouble:
                return GL_FLOAT ;
            case natus::graphics::type::tint:
                return GL_INT ;
            case natus::graphics::type::tuint:
                return GL_UNSIGNED_INT ;
            case natus::graphics::type::tshort:
                return GL_SHORT ;
            case natus::graphics::type::tushort:
                return GL_UNSIGNED_SHORT ;
            case natus::graphics::type::tchar:
                return GL_BYTE ;
            default: return GL_NONE ;
            }
            return GL_NONE ;
        }

        static GLenum complex_to_simple_type( GLenum const e ) noexcept
        {
            switch( e )
            {
            case GL_FLOAT: return GL_FLOAT ;
            case GL_FLOAT_VEC2: return GL_FLOAT ;
            case GL_FLOAT_VEC3: return GL_FLOAT ;
            case GL_FLOAT_VEC4: return GL_FLOAT ;
            case GL_INT: return GL_INT ;
            case GL_INT_VEC2: return GL_INT ;
            case GL_INT_VEC3: return GL_INT ;
            case GL_INT_VEC4: return GL_INT ;
            case GL_UNSIGNED_INT: return GL_UNSIGNED_INT ;
            case GL_UNSIGNED_INT_VEC2: return GL_UNSIGNED_INT ;
            case GL_UNSIGNED_INT_VEC3: return GL_UNSIGNED_INT ;
            case GL_UNSIGNED_INT_VEC4: return GL_UNSIGNED_INT ;
            case GL_BOOL: return GL_BOOL ;
            case GL_BOOL_VEC2: return GL_BOOL ;
            case GL_BOOL_VEC3: return GL_BOOL ;
            case GL_BOOL_VEC4: return GL_BOOL ;
            case GL_FLOAT_MAT2: return GL_FLOAT ;
            case GL_FLOAT_MAT3: return GL_FLOAT ;
            case GL_FLOAT_MAT4: return GL_FLOAT ;
            case GL_SAMPLER_2D:
            case GL_SAMPLER_3D:
            case GL_INT_SAMPLER_2D:
            case GL_INT_SAMPLER_3D:
            case GL_UNSIGNED_INT_SAMPLER_2D:
            case GL_UNSIGNED_INT_SAMPLER_3D:
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_2D_SHADOW:
            case GL_SAMPLER_2D_ARRAY:
            case GL_SAMPLER_BUFFER:
            case GL_INT_SAMPLER_BUFFER:
            case GL_UNSIGNED_INT_SAMPLER_BUFFER:
            case GL_INT_SAMPLER_2D_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return GL_INT ;
            default: return 0 ;
            }
            return 0 ;
        }

        typedef ::std::function< void_t ( GLuint, GLuint, void_ptr_t ) > uniform_funk_t ;

        static natus::ogl::uniform_funk_t uniform_funk( GLenum const e ) noexcept
        {
            natus::ogl::uniform_funk_t empty_funk =
                [=] ( GLuint, GLuint, void_ptr_t ) { } ;

            switch( e )
            {

                //************************************************************

            case GL_FLOAT: return [=] ( GLuint loc, GLuint /*count*/, void_ptr_t p )
            { glUniform1f( loc, *( const GLfloat* ) p ); } ;

            case GL_FLOAT_VEC2: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform2fv( loc, count, ( const GLfloat* ) p ); } ;

            case GL_FLOAT_VEC3: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform3fv( loc, count, ( const GLfloat* ) p ); } ;

            case GL_FLOAT_VEC4: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform4fv( loc, count, ( const GLfloat* ) p ); } ;

                //************************************************************

            case GL_INT: return [=] ( GLuint loc, GLuint /*count*/, void_ptr_t p )
            { glUniform1i( loc, *( const GLint* ) p ); } ;

            case GL_INT_VEC2: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform2iv( loc, count, ( const GLint* ) p ); } ;

            case GL_INT_VEC3: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform3iv( loc, count, ( const GLint* ) p ); } ;

            case GL_INT_VEC4: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform4iv( loc, count, ( const GLint* ) p ); } ;

                //************************************************************

            case GL_UNSIGNED_INT: return [=] ( GLuint loc, GLuint /*count*/, void_ptr_t p )
            { glUniform1ui( loc, *( const GLuint* ) p ); } ;

            case GL_UNSIGNED_INT_VEC2: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform2uiv( loc, count, ( const GLuint* ) p ); } ;

            case GL_UNSIGNED_INT_VEC3: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform3uiv( loc, count, ( const GLuint* ) p ); } ;

            case GL_UNSIGNED_INT_VEC4: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniform4uiv( loc, count, ( const GLuint* ) p ); } ;

                //************************************************************

            case GL_FLOAT_MAT2: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniformMatrix2fv( loc, count, GL_TRUE, ( const GLfloat* ) p ); } ;

            case GL_FLOAT_MAT3: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniformMatrix3fv( loc, count, GL_TRUE, ( const GLfloat* ) p ); } ;

            case GL_FLOAT_MAT4: return [=] ( GLuint loc, GLuint count, void_ptr_t p )
            { glUniformMatrix4fv( loc, count, GL_TRUE, ( const GLfloat* ) p ); } ;

                //************************************************************

            case GL_SAMPLER_2D: 
            case GL_SAMPLER_3D: 
            case GL_INT_SAMPLER_2D: 
            case GL_INT_SAMPLER_3D: 
            case GL_UNSIGNED_INT_SAMPLER_2D: 
            case GL_UNSIGNED_INT_SAMPLER_3D: 
            case GL_SAMPLER_CUBE: 
            case GL_SAMPLER_2D_SHADOW: 
            case GL_SAMPLER_2D_ARRAY: 
            case GL_SAMPLER_BUFFER:
            case GL_INT_SAMPLER_BUFFER: 
            case GL_UNSIGNED_INT_SAMPLER_BUFFER:
            case GL_INT_SAMPLER_2D_ARRAY: 
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: 
                return [=] ( GLuint loc, GLuint count, void_ptr_t p )
                { glUniform1iv( loc, count, ( const GLint* ) p ); } ;

            default: break ;
            }
            return empty_funk ;
        }

        typedef ::std::function< void_t ( void_ptr_t ) > uniform_default_value_funk_t ;

        static natus::ogl::uniform_default_value_funk_t uniform_default_value( GLenum const e ) noexcept
        {
            natus::ogl::uniform_default_value_funk_t empty_funk =
                [=] ( void_ptr_t ) { } ;

            switch( e )
            {

                //************************************************************

            case GL_FLOAT: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLfloat ) ) ; } ;

            case GL_FLOAT_VEC2: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLfloat ) * 2 ) ; } ;

            case GL_FLOAT_VEC3: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLfloat ) * 3 ) ; } ;

            case GL_FLOAT_VEC4: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLfloat ) * 4 ) ; } ;

                //************************************************************

            case GL_INT: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLint ) ) ; } ;

            case GL_INT_VEC2: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLint ) * 2 ) ; } ;

            case GL_INT_VEC3: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLint ) * 3 ) ; } ;

            case GL_INT_VEC4: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLint ) * 4 ) ; } ;

                //************************************************************

            case GL_UNSIGNED_INT: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLuint ) ) ; } ;

            case GL_UNSIGNED_INT_VEC2: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLuint ) * 2 ) ; } ;

            case GL_UNSIGNED_INT_VEC3: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLuint ) * 3 ) ; } ;

            case GL_UNSIGNED_INT_VEC4: return [=] ( void_ptr_t p )
            { ::std::memset( p, 0, sizeof( GLuint ) * 4 ) ; } ;

                //************************************************************

            case GL_FLOAT_MAT2: return [=] ( void_ptr_t p )
            { 
                ::std::memset( p, 0, sizeof( GLfloat ) * 4 ) ; 
                ( ( GLfloat* ) p )[ 0 ] = 1.0f ; ( ( GLfloat* ) p )[ 3 ] = 1.0f ;
            } ;

            case GL_FLOAT_MAT3: return [=] ( void_ptr_t p )
            { 
                ::std::memset( p, 0, sizeof( GLfloat ) * 9 ) ; 
                ( ( GLfloat* ) p )[ 0 ] = 1.0f ; ( ( GLfloat* ) p )[ 4 ] = 1.0f ; 
                ( ( GLfloat* ) p )[ 8 ] = 1.0f ; 
            } ;

            case GL_FLOAT_MAT4: return [=] ( void_ptr_t p )
            { 
                ::std::memset( p, 0, sizeof( GLfloat ) * 16 ) ; 
                ( ( GLfloat* ) p )[ 0 ] = 1.0f ; ( ( GLfloat* ) p )[ 5 ] = 1.0f ;
                ( ( GLfloat* ) p )[ 10 ] = 1.0f ; ( ( GLfloat* ) p )[ 15 ] = 1.0f ;
            } ;

                //************************************************************

            case GL_SAMPLER_2D: 
            case GL_SAMPLER_3D: 
            case GL_INT_SAMPLER_2D: 
            case GL_INT_SAMPLER_3D: 
            case GL_UNSIGNED_INT_SAMPLER_2D: 
            case GL_UNSIGNED_INT_SAMPLER_3D: 
            case GL_SAMPLER_CUBE: 
            case GL_SAMPLER_2D_SHADOW: 
            case GL_SAMPLER_2D_ARRAY: 
            case GL_SAMPLER_BUFFER:
            case GL_INT_SAMPLER_BUFFER: 
            case GL_UNSIGNED_INT_SAMPLER_BUFFER: 
            case GL_INT_SAMPLER_2D_ARRAY: 
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: 
                return [=] ( void_ptr_t p )
                { ::std::memset( p, 0, sizeof( GLint ) ) ; } ;

            default: break ;
            }
            return empty_funk ;
        }

        static natus::ntd::string_t to_string( GLenum const e ) noexcept
        {
            switch( e )
            {
            case GL_NO_ERROR: return natus::ntd::string_t( "GL_NO_ERROR" ) ;
            case GL_INVALID_ENUM: return natus::ntd::string_t( "GL_INVALID_ENUM" ) ;
            case GL_INVALID_VALUE: return natus::ntd::string_t( "GL_INVALID_VALUE" ) ;
            case GL_INVALID_OPERATION: return natus::ntd::string_t( "GL_INVALID_OPERATION" ) ;
            default: break ;
            }
            return ::std::to_string( e ) ;
        }
    }
}
