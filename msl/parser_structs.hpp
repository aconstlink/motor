
#pragma once

#include "api.h"
#include "typedefs.h"
#include "symbol.hpp"
#include "enums.hpp"
#include "default_variable.hpp" 

#include <motor/std/vector>
#include <motor/std/string>

#include <cstdlib>
#include <cstring>

namespace motor
{
    namespace msl
    {
        //*******************************************************************************************
        namespace parse
        {
            using variable_default_values_t = motor::vector< std::pair< motor::string_t, motor::string_t > > ;

            struct config
            {
                struct render_states
                {
                    motor::vector< motor::string_t > lines ;
                };
                motor::vector< render_states > rstates ;

                struct code
                {
                    motor::vector< motor::string_t > versions ;
                    motor::vector< motor::string_t > lines ;
                };

                // geometry shader in/out primitive types
                struct primitive_decl
                {
                    motor::string_t flow_qualifier ;
                    motor::string_t primitive_type ;
                    motor::string_t attributes ;
                };

                struct variable
                {
                    motor::string_t flow_qualifier ;
                    motor::string_t type ;
                    motor::string_t name ;
                    motor::string_t binding ;
                    motor::string_t line ;
                    variable_default_values_t default_value ;
                };

                struct shader
                {
                    motor::string_t type ;
                    motor::vector< primitive_decl > prim_decls ;
                    motor::vector< variable > variables ;
                    motor::vector< code > codes ;
                };
                motor::vector< shader > shaders ;

                motor::string_t name ;
            };
            motor_typedef( config ) ;
            motor_typedefs( motor::vector< config_t >, configs ) ;

            struct library
            {
                struct lib_function
                {
                    motor::string_t sig ;
                    motor::vector< motor::string_t > body ;
                };
                motor::vector< lib_function > functions ;
                
                struct variable
                {
                    motor::string_t type ;
                    motor::string_t name ;
                    motor::string_t value ;
                    motor::string_t line ;
                };

                motor::vector< variable > variables ;
                motor::vector< motor::string_t > names ;
            };
            motor_typedef( library ) ;
            motor_typedefs( motor::vector< library_t >, libraries ) ;
        }

        //*******************************************************************************************
        namespace post_parse
        {
            struct used_buildin
            {
                size_t line ;
                size_t token ;
                motor::msl::build_in_t bi ;
            };
            motor_typedef( used_buildin ) ;
            motor_typedefs( motor::vector< used_buildin_t >, used_buildins ) ;

            // ***********************************************
            struct config
            {
                struct shader
                {
                    struct primitive_decl
                    {
                        motor::msl::flow_qualifier fq ;
                        motor::msl::primitive_decl_type pdt ;
                        size_t max_vertices ; // output

                    };
                    motor_typedef( primitive_decl ) ;
                    motor::vector< primitive_decl > primitive_decls ;

                    struct variable
                    {
                        motor::msl::flow_qualifier fq ;
                        motor::msl::type_t type ;
                        motor::string_t name ;
                        motor::msl::binding binding ;
                        motor::string_t line ;
                        size_t def_val ;
                    };
                    motor_typedef( variable ) ;
                    motor::vector< variable > variables ;

                    struct code
                    {
                        motor::msl::language_class version ;
                        motor::vector< motor::string_t > lines ;
                        used_buildins_t buildins ;
                    };
                    motor_typedef( code ) ;
                    motor::vector< code > codes ;

                    symbols_t deps ;

                    shader_type type ;
                };
                motor_typedef( shader ) ;

                motor::vector< shader > shaders ;
                motor::vector< motor::msl::idefault_value_mtr_t > def_values ;

                motor::string_t name ;

                config( void_t ) noexcept{}
                
                config( config const & rhv ) noexcept : shaders( rhv.shaders ),
                    name( rhv.name )
                {
                    for ( auto * ptr : def_values ) motor::release( motor::move( ptr ) )  ;
                    
                    def_values.resize( rhv.def_values.size() ) ;

                    size_t i = size_t( -1 ) ;
                    for ( auto * ptr : rhv.def_values ) def_values[ ++i ] = motor::share( ptr )  ;
                }

                config( config && rhv ) noexcept : shaders( std::move( rhv.shaders ) ),
                    name( std::move( rhv.name ) )
                {
                    for( auto * ptr : def_values ) motor::release( motor::move( ptr ) )  ;
                    def_values = std::move( rhv.def_values ) ;
                }

                config & operator = ( config const & rhv ) noexcept
                {
                    shaders = rhv.shaders ;
                    name = rhv.name ;

                    for ( auto * ptr : def_values ) motor::release( motor::move( ptr ) )  ;

                    def_values.resize( rhv.def_values.size() ) ;
                    
                    size_t i=size_t(-1) ;  
                    for ( auto * ptr : rhv.def_values ) def_values[++i] = motor::share( ptr )  ;
                    

                    return *this ;
                }
                config & operator = ( config && rhv ) noexcept
                {
                    shaders = std::move( rhv.shaders ) ;
                    name = std::move( rhv.name ) ;

                    for ( auto * ptr : def_values ) motor::release( motor::move( ptr ) )  ;
                    def_values = std::move( rhv.def_values ) ;
                    return *this ;
                }
                ~config( void_t ) noexcept
                {
                    for( auto * ptr : def_values ) motor::release( motor::move( ptr ) )  ;
                }
            };
            motor_typedef( config ) ;
            motor_typedefs( motor::vector< config >, configs ) ;

            // ***********************************************
            struct library
            {
                struct fragment
                {
                    motor::msl::signature_t sig ;

                    // more accurately, the code lines
                    motor::vector< motor::string_t > fragments ;
                    used_buildins_t buildins ;

                    symbols_t deps ;

                    motor::msl::symbol_t sym_long ;

                    bool_t operator == ( fragment const & other ) const 
                    {
                        if( sym_long != other.sym_long ) return false ;
                        return sig == other.sig ;
                    }
                };
                motor_typedef( fragment ) ;
                motor_typedefs( motor::vector< fragment >, fragments ) ;
                fragments_t fragments ;

                struct variable
                {
                    motor::msl::symbol_t sym_long ;

                    motor::msl::type_t type ;
                    motor::string_t name ;
                    motor::string_t value ;
                    motor::string_t line ;

                    bool_t operator == ( variable const & rhv ) const
                    {
                        return sym_long == rhv.sym_long ;
                    }

                    bool_t operator != ( variable const& rhv ) const
                    {
                        return sym_long != rhv.sym_long ;
                    }

                };
                motor_typedef( variable ) ;
                motor_typedefs( motor::vector< variable >, variables ) ;

                variables_t variables ;

                // @todo not required
                //motor::vector< motor::string_t > names ;
            };
            motor_typedef( library ) ;
            motor_typedefs( motor::vector< library_t >, libraries ) ;

            struct MOTOR_MSL_API document
            {
                configs_t configs ;
                libraries_t libraries ;

                libraries_t find_all( motor::msl::symbol_cref_t sym ) const noexcept
                {
                    libraries_t ret ;

                    for( auto const & l : libraries )
                    {
                        for( auto const & s : l.fragments )
                        {
                            if( sym == s.sym_long )
                            {
                                ret.emplace_back( l ) ;
                            }
                        }

                        // if shader found, it can not be a variable anymore
                        if( ret.size() > 0 ) continue ;

                        for( auto const& v : l.variables )
                        {
                            if( sym == v.sym_long )
                            {
                                ret.emplace_back( l ) ;
                            }
                        }
                    }

                    return std::move( ret ) ;
                }
            };
            motor_typedef( document ) ;
        }

        static motor::msl::idefault_value_mtr_t create_default_value( motor::msl::type const t,
            motor::msl::parse::variable_default_values_t const & values ) noexcept
        {
            char buffer[4][2048] ;

            if( values.size() == 0 ) return nullptr ;

            motor::msl::idefault_value_mtr_t ret = nullptr ;

            if ( t == motor::msl::type::as_float() && values.size() >= 1 )
            {
                std::memcpy( buffer[0], values[0].second.c_str(), values[0].second.size() );
                ret = motor::shared( motor::msl::generic_default_value< float_t >( float_t( std::atof( buffer[0] ) ) ) ) ;
            }
            else if ( t == motor::msl::type::as_int() && values.size() >= 1 )
            {
                std::memcpy( buffer[0], values[0].second.c_str(), values[0].second.size() );
                ret = motor::shared( motor::msl::generic_default_value< int_t >( std::atoi( buffer[0] ) ) ) ;
            }
            else if ( t == motor::msl::type::as_uint() && values.size() >= 1 )
            {
                std::memcpy( buffer[0], values[0].second.c_str(), values[0].second.size() );
                ret = motor::shared( motor::msl::generic_default_value< uint_t >( std::atol( buffer[0] ) ) ) ;
            }

            else if ( t == motor::msl::type::as_vec2( motor::msl::type_base::tfloat ) && values.size() >= 2 )
            {
                std::memcpy( buffer[0], values[0].second.c_str(), values[0].second.size() );
                std::memcpy( buffer[1], values[1].second.c_str(), values[1].second.size() );

                ret = motor::shared( motor::msl::generic_default_value< motor::math::vec2f_t >(
                    motor::math::vec2f_t(
                        float_t( std::atof( buffer[0] ) ),
                        float_t( std::atof( buffer[1] ) ) ) ) ) ;
            }
            else if ( t == motor::msl::type::as_vec3( motor::msl::type_base::tfloat ) )
            {
                std::memcpy( buffer[ 0 ], values[ 0 ].second.c_str(), values[ 0 ].second.size() );
                std::memcpy( buffer[ 1 ], values[ 1 ].second.c_str(), values[ 1 ].second.size() );
                std::memcpy( buffer[ 2 ], values[ 2 ].second.c_str(), values[ 2 ].second.size() );

                ret = motor::shared( motor::msl::generic_default_value< motor::math::vec3f_t >(
                    motor::math::vec3f_t(
                        float_t( std::atof( buffer[0] ) ),
                        float_t( std::atof( buffer[1] ) ),
                        float_t( std::atof( buffer[2] ) ) ) ) ) ;
            }
            else if ( t == motor::msl::type::as_vec4( motor::msl::type_base::tfloat ) )
            {
                std::memcpy( buffer[ 0 ], values[ 0 ].second.c_str(), values[ 0 ].second.size() );
                std::memcpy( buffer[ 1 ], values[ 1 ].second.c_str(), values[ 1 ].second.size() );
                std::memcpy( buffer[ 2 ], values[ 2 ].second.c_str(), values[ 2 ].second.size() );
                std::memcpy( buffer[ 3 ], values[ 3 ].second.c_str(), values[ 3 ].second.size() );

                ret = motor::shared( motor::msl::generic_default_value< motor::math::vec4f_t >(
                    motor::math::vec4f_t(
                        float_t( std::atof( buffer[0] ) ),
                        float_t( std::atof( buffer[1] ) ),
                        float_t( std::atof( buffer[2] ) ),
                        float_t( std::atof( buffer[3] ) ) ) ) ) ;
            }

            else if ( t == motor::msl::type::as_vec2( motor::msl::type_base::tint ) )
            {
            }
            else if ( t == motor::msl::type::as_vec3( motor::msl::type_base::tint ) )
            {
            }
            else if ( t == motor::msl::type::as_vec4( motor::msl::type_base::tint ) )
            {
            }

            else if ( t == motor::msl::type::as_vec2( motor::msl::type_base::tuint ) )
            {
            }
            else if ( t == motor::msl::type::as_vec3( motor::msl::type_base::tuint ) )
            {
            }
            else if ( t == motor::msl::type::as_vec4( motor::msl::type_base::tuint ) )
            {
            }

            return ret ;
        }
    }
}