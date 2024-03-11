#pragma once

#include "../typedefs.h"
#include "../enums.hpp"
#include "../symbol.hpp"
#include "../parser_structs.hpp"

namespace motor
{
    namespace msl
    {
        struct variable_mapping
        {
            motor::string_t new_name ;
            motor::string_t old_name ;
            motor::msl::binding binding ;
            motor::msl::flow_qualifier fq ;
            motor::msl::shader_type st ;
            motor::msl::type_t t ;
        };
        motor_typedef( variable_mapping ) ;
        motor_typedefs( motor::vector< variable_mapping_t >, variable_mappings ) ;

        static size_t find_by( variable_mappings_cref_t mappings, motor::string_cref_t name, motor::msl::binding binding, motor::msl::flow_qualifier const fq, motor::msl::shader_type const st ) noexcept
        {
            auto iter = std::find_if( mappings.begin(), mappings.end(), [&] ( variable_mapping_cref_t v ) 
            { 
                return v.old_name == name && v.binding == binding && v.fq == fq && st == v.st ;
            } ) ;
            return std::distance( mappings.begin(), iter ) ;
        }

        struct generatable
        {
            motor::msl::symbols_t missing ;
            motor::msl::post_parse::library_t::variables_t vars ;
            motor::msl::post_parse::library_t::fragments_t frags ;
            motor::msl::post_parse::config_t config ;
        };
        motor_typedef( generatable ) ;

        struct generated_code
        {
            // for possible later observation like bindings
            motor::msl::generatable_t rres ;

            struct variable
            {
                motor::string_t name ;
                motor::msl::binding binding ;
                motor::msl::flow_qualifier fq ;
            };
            motor_typedef( variable ) ;

            struct code
            {
                motor::msl::api_type api ;
                motor::string_t shader ;
            };
            motor_typedef( code ) ;

            struct shader
            {
                motor::msl::shader_type type ;
                motor::vector< variable > variables ;
                motor::vector< code > codes ;
            };
            motor_typedef( shader ) ;
            motor_typedefs( motor::vector< shader_t >, shaders ) ;
            shaders_t shaders ;

            // the input/output variables of the geometry stage
            // makes it easy to figure out the vertex in/output attributes.
            // geometry stage is every shader type between the vertex shader
            // and the geometry shader.
            motor::vector< variable > geometry_ins ;
            motor::vector< variable > geometry_outs ;

            motor::msl::streamout_type streamout = motor::msl::streamout_type::none ;

            typedef std::function< void_t ( motor::msl::shader_type, code_cref_t ) > for_each_code_t ;
            void_t sorted_by_api_type( motor::msl::api_type const t, for_each_code_t funk ) const noexcept
            {
                for( auto const& s : shaders )
                {
                    for( auto const& c : s.codes )
                    {
                        if( c.api == t ) funk( s.type, c ) ;
                    }
                }
            }

            void_t sorted_by_shader_type( for_each_code_t funk ) const noexcept
            {
                for( auto const& s : shaders )
                {
                    for( auto const& c : s.codes )
                    {
                        funk( s.type, c ) ;
                    }
                }
            }
        };
        motor_typedef( generated_code ) ;

        struct repl_sym
        {
            motor::string_t what ;
            std::function< motor::string_t ( motor::vector< motor::string_t > const& ) > repl ;
        } ;
        motor_typedef( repl_sym ) ;
        motor_typedefs( motor::vector< repl_sym >, repl_syms ) ;

        static motor::string_t perform_repl( motor::string_t s, repl_syms_cref_t repls ) noexcept
        {
            if( s.empty() ) return s ;

            for( auto const& repl : repls )
            {
                size_t p0 = s.find( repl.what ) ;
                if( p0 == 0 ) 
                {
                    s = " " + s ;
                    p0 = 1 ;
                }
                while( p0 != std::string::npos )
                {
                    if( (s[p0-1] != ' ') || (s[p0+repl.what.size()] != ' ') ) 
                    {
                        p0 = s.find( repl.what, p0 + repl.what.size() ) ;
                        continue ;
                    }

                    motor::vector< motor::string_t > args ;

                    size_t level = 0 ;
                    size_t beg = p0 + repl.what.size() + 3 ;
                    for( size_t i = beg; i < s.size(); ++i )
                    {
                        if( (level == 0 && s[ i ] == ',') ||
                            (level == 0 && s[ i ] == ')') )
                        {
                            if( i > beg ) args.emplace_back( s.substr( beg, ( i - 1 ) - beg ) ) ;
                            beg = i + 2 ;
                        }

                        if( s[ i ] == '(' ) ++level ;
                        else if( s[ i ] == ')' ) --level ;
                        if( level == size_t( -1 ) ) break ;
                    }
                    p0 = s.replace( p0, ( --beg ) - p0, repl.repl( args ) ).find( repl.what, p0 ) ;
                }
            }
            return std::move( s ) ;
        }
    }
    
    namespace msl
    {
        struct api_specific_buildin
        {
            motor_this_typedefs( api_specific_buildin ) ;

            motor::msl::signature_t sig ;
            motor::vector< motor::string_t > fragments ;

            bool_t operator == ( this_cref_t rhv ) const noexcept
            {
                return sig == rhv.sig ;
            }

            bool_t operator != ( this_cref_t rhv ) const noexcept
            {
                return sig != rhv.sig ;
            }
        };
        motor_typedef( api_specific_buildin ) ;
        using api_specific_buildins_t = motor::vector< api_specific_buildin_t > ;
    }
}
