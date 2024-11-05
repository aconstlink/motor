
#pragma once

#include "variables.hpp"
#include "type_traits.hpp"

#include <motor/log/global.h>
#include <motor/memory/global.h>

namespace motor
{
    namespace graphics
    {
        class variable_set
        {
            motor_this_typedefs( variable_set ) ;
        
        private:

            struct data
            {
                motor::string_t name ;
                motor::graphics::type type ;
                motor::graphics::type_struct type_struct ;
                motor::graphics::ivariable_ptr_t var ;
            };
            motor_typedef( data ) ;
            motor::vector< data > _variables ;

            struct texture_data
            {
                motor::string_t name ;
                motor::graphics::ivariable_ptr_t var ;
            };
            motor_typedef( texture_data ) ;
            motor::vector< texture_data > _textures ;

            struct array_data
            {
                motor::string_t name ;
                motor::graphics::ivariable_ptr_t var ;
            };
            motor_typedef( array_data ) ;
            motor::vector< array_data > _arrays ;

            struct streamout_data
            {
                motor::string_t name ;
                motor::graphics::ivariable_ptr_t var ;
            };
            motor_typedef( streamout_data ) ;
            motor::vector< streamout_data > _streamouts ;

            std::mutex _mtx ;

        public:

            variable_set( void_t ) noexcept {}
            variable_set( this_cref_t ) = delete ;
            variable_set( this_rref_t rhv ) noexcept
            {
                *this = std::move( rhv )  ;
            }

            virtual ~variable_set( void_t ) noexcept
            {
                for( auto & d : _variables )
                {
                    motor::memory::global_t::dealloc( d.var ) ;
                }
                for( auto & d : _textures ) 
                {
                    motor::memory::global_t::dealloc( d.var ) ;
                }
                for( auto & d : _arrays ) 
                {
                    motor::memory::global_t::dealloc( d.var ) ;
                }
                for( auto & d : _streamouts ) 
                {
                    motor::memory::global_t::dealloc( d.var ) ;
                }
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _variables = std::move( rhv._variables ) ;
                _textures = std::move( rhv._textures ) ;
                _arrays = std::move( rhv._arrays ) ;
                _streamouts = std::move( rhv._streamouts ) ;
                return *this ;
            }

            this_ref_t operator = ( this_cref_t ) = delete ;

        public:

            template< class T >
            motor::graphics::data_variable< T > * data_variable( motor::string_cref_t name ) noexcept
            {
                auto const type = motor::graphics::type_traits< T >::gpu_type ;
                auto const type_struct = motor::graphics::type_traits< T >::gpu_type_struct ;

                return static_cast< motor::graphics::data_variable<T>*>( 
                    this_t::data_variable( name, type, type_struct ) ) ;
            }

            motor::graphics::ivariable_ptr_t data_variable( motor::string_cref_t name,
                motor::graphics::type const t, motor::graphics::type_struct const ts ) noexcept
            {
                motor::graphics::ivariable_ptr_t var = nullptr ;

                // before inserting, check if name and type match
                {
                    std::lock_guard< std::mutex > lk( _mtx ) ;

                    auto iter = std::find_if( _variables.begin(), _variables.end(),
                        [&] ( this_t::data const& d )
                    {
                        return d.name == name ;
                    } ) ;

                    if( iter != _variables.end() )
                    {
                        if( iter->type != t || iter->type_struct != ts )
                        {
                            motor::log::global_t::error( motor_log_fn( "type mismatch for " + name ) ) ;
                            return nullptr ;
                        }

                        return iter->var ;
                    }

                    {
                        switch( t )
                        {
                        case motor::graphics::type::tchar: var = this_t::from_type_struct<char_t>( name, ts ) ; break ;
                        case motor::graphics::type::tuchar: var = this_t::from_type_struct<uchar_t>( name, ts ) ; break ; 
                        case motor::graphics::type::tshort: var = this_t::from_type_struct<short_t>( name, ts ) ; break ;
                        case motor::graphics::type::tushort: var = this_t::from_type_struct<ushort_t>( name, ts ) ; break ;
                        case motor::graphics::type::tint: var = this_t::from_type_struct<int_t>( name, ts ) ; break ;
                        case motor::graphics::type::tuint: var = this_t::from_type_struct<uint_t>( name, ts ) ; break ;
                        case motor::graphics::type::tfloat: var = this_t::from_type_struct<float_t>( name, ts ) ; break ;
                        case motor::graphics::type::tdouble: var = this_t::from_type_struct<double_t>( name, ts ) ; break ;
                        case motor::graphics::type::tbool: var = this_t::from_type_struct<bool_t>( name, ts ) ; break ;
                        default: break ;
                        }

                        if( var == nullptr ) 
                        {
                            motor::log::global_t::error( motor_log_fn( "invalid type for variable " + name ) ) ;
                            return var ;
                        }

                        this_t::data_t d ;
                        d.name = name ;
                        d.type = t ;
                        d.type_struct = ts ;
                        d.var = var ;

                        _variables.emplace_back( d ) ;
                    }
                }

                return var ;
            }

            bool_t has_data_variable( motor::string_in_t name ) const noexcept
            {
                for( auto const & d : _variables ) 
                {
                    if( d.name == name ) return true ;
                }
                return false ;
            }

            motor::graphics::data_variable< motor::string_t > * texture_variable( 
                motor::string_in_t name ) noexcept
            {
                motor::graphics::ivariable_ptr_t var = motor::memory::global_t::alloc(
                    motor::graphics::data_variable<motor::string_t>(), "texture variable" ) ;

                // before inserting, check if name and type match
                {
                    std::lock_guard< std::mutex > lk( _mtx ) ;

                    auto iter = std::find_if( _textures.begin(), _textures.end(),
                        [&] ( this_t::texture_data const& d )
                    {
                        return d.name == name ;
                    } ) ;

                    if( iter != _textures.end() )
                    {
                        motor::memory::global_t::dealloc( var ) ;

                        return static_cast< motor::graphics::data_variable< motor::string_t >* >( iter->var ) ;
                    }

                    this_t::texture_data_t d ;
                    d.name = name ;
                    d.var = var ;

                    _textures.emplace_back( d ) ;
                }

                return static_cast< motor::graphics::data_variable< motor::string_t >* >( var ) ;
            }

            bool_t has_texture_variable( motor::string_in_t name ) const noexcept
            {
                for ( auto const & d : _textures )
                {
                    if ( d.name == name ) return true ;
                }
                return false ;
            }

            motor::graphics::data_variable< motor::string_t > * array_variable( 
                motor::string_in_t name ) noexcept
            {
                motor::graphics::ivariable_ptr_t var = motor::memory::global_t::alloc(
                    motor::graphics::data_variable<motor::string_t>(), "array variable" ) ;

                // before inserting, check if name and type match
                {
                    std::lock_guard< std::mutex > lk( _mtx ) ;

                    auto iter = std::find_if( _arrays.begin(), _arrays.end(),
                        [&] ( this_t::array_data const& d )
                    {
                        return d.name == name ;
                    } ) ;

                    if( iter != _arrays.end() )
                    {
                        motor::memory::global_t::dealloc( var ) ;

                        return static_cast< motor::graphics::data_variable< motor::string_t >* >( iter->var ) ;
                    }

                    this_t::array_data_t d ;
                    d.name = name ;
                    d.var = var ;

                    _arrays.emplace_back( d ) ;
                }

                return static_cast< motor::graphics::data_variable< motor::string_t >* >( var ) ;
            }

            bool_t has_array_variable( motor::string_in_t name ) const noexcept
            {
                for ( auto const & d : _arrays )
                {
                    if ( d.name == name ) return true ;
                }
                return false ;
            }

            // allows to connect a streamout object with a data buffer in the shader
            motor::graphics::data_variable< motor::string_t > * array_variable_streamout( 
                motor::string_in_t name ) noexcept
            {
                motor::graphics::ivariable_ptr_t var = motor::memory::global_t::alloc(
                    motor::graphics::data_variable<motor::string_t>(), "array variable from streamout" ) ;

                // before inserting, check if name and type match
                {
                    std::lock_guard< std::mutex > lk( _mtx ) ;

                    auto iter = std::find_if( _streamouts.begin(), _streamouts.end(),
                        [&] ( this_t::streamout_data_cref_t d )
                    {
                        return d.name == name ;
                    } ) ;

                    if( iter != _streamouts.end() )
                    {
                        motor::memory::global_t::dealloc( var ) ;

                        return static_cast< motor::graphics::data_variable< motor::string_t >* >( iter->var ) ;
                    }

                    this_t::streamout_data_t d ;
                    d.name = name ;
                    d.var = var ;

                    _streamouts.emplace_back( d ) ;
                }

                return static_cast< motor::graphics::data_variable< motor::string_t >* >( var ) ;
            }

            using for_each_data_var_funk_t = std::function< void_t ( motor::string_in_t, motor::graphics::ivariable_ptr_t ) > ;
            void_t for_each_data_variable( for_each_data_var_funk_t f ) const noexcept
            {
                for( auto const & v : _variables )
                {
                    f( v.name, v.var ) ;
                }
            }

            using for_each_texture_var_funk_t = std::function< void_t ( motor::string_in_t, motor::graphics::data_variable< motor::string_t > * ) > ;

            void_t for_each_texture_variable( for_each_texture_var_funk_t f ) const noexcept
            {
                using ptr_t = motor::graphics::data_variable< motor::string_t > * ;
                for ( auto const & v : _textures )
                {
                    f( v.name, reinterpret_cast< ptr_t >( v.var ) ) ;
                }
            }

            void_t for_each_buffer_variable( for_each_texture_var_funk_t f ) const noexcept
            {
                using ptr_t = motor::graphics::data_variable< motor::string_t > * ;
                for ( auto const & v : _arrays )
                {
                    f( v.name, reinterpret_cast< ptr_t >( v.var ) ) ;
                }
            }

            bool_t has_any_variable( motor::string_in_t name ) const noexcept
            {
                if( this_t::has_data_variable( name ) ) return true ;
                if( this_t::has_texture_variable( name ) ) return true ;
                if( this_t::has_array_variable( name ) ) return true ;
                return false ;
            }
        private:

            template< typename T >
            motor::graphics::ivariable_ptr_t from_type_struct( 
                motor::string_cref_t /*name*/, motor::graphics::type_struct const ts ) noexcept
            {
                switch( ts )
                {
                case motor::graphics::type_struct::vec1: 
                    return motor::memory::global_t::alloc( 
                        motor::graphics::data_variable<T>(), "vec1 variable" ) ;
                case motor::graphics::type_struct::vec2: 
                    return motor::memory::global_t::alloc( 
                        motor::graphics::data_variable<motor::math::vector2<T>>(), "vec2 variable" ) ;
                case motor::graphics::type_struct::vec3:
                    return motor::memory::global_t::alloc( 
                        motor::graphics::data_variable<motor::math::vector3<T>>(), "vec3 variable" ) ;
                case motor::graphics::type_struct::vec4:
                    return motor::memory::global_t::alloc( 
                        motor::graphics::data_variable<motor::math::vector4<T>>(), "vec4 variable" ) ;
                case motor::graphics::type_struct::mat2:
                    return motor::memory::global_t::alloc( 
                        motor::graphics::data_variable<motor::math::matrix2<T>>(motor::math::matrix2<T>().identity()), "mat2 variable" ) ;
                case motor::graphics::type_struct::mat3:
                    return motor::memory::global_t::alloc( 
                        motor::graphics::data_variable<motor::math::matrix3<T>>(motor::math::matrix3<T>().identity()), "mat2 variable" ) ;
                case motor::graphics::type_struct::mat4:
                    return motor::memory::global_t::alloc( 
                        motor::graphics::data_variable<motor::math::matrix4<T>>(motor::math::matrix4<T>().identity()), "mat2 variable" ) ;
                default:
                    break ;
                }
                return nullptr ;
            }
        };
        motor_typedef( variable_set ) ;
    }
}
