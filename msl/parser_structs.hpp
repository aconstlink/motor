
#pragma once

#include "api.h"
#include "typedefs.h"
#include "symbol.hpp"
#include "enums.hpp"

#include <motor/std/vector>
#include <motor/std/string>

namespace motor
{
    namespace msl
    {
        //*******************************************************************************************
        namespace parse
        {
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

                motor::string_t name ;
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
    }
}