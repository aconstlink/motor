
#include "typedefs.h"
#include "parser_structs.hpp"
#include "generators/generator_structs.hpp"
#include "dependency_resolver.hpp"

#include <motor/log/global.h>

namespace motor
{
    namespace msl
    {
        class function_declaration_analyser
        {
            using signature_t = motor::msl::signature_t ;

            motor::msl::type_t return_type ;
            motor::string_t name ;
            motor::vector< signature_t::arg_t > _args ;

            motor::vector< motor::string_t > _tokens ;

        public:

            function_declaration_analyser( motor::vector< motor::string_t > const& tokens ) noexcept
            {
                _tokens = tokens ;
            }

            motor::msl::signature_t process( void_t ) noexcept
            {
                
                signature_t s ;

                auto iter_open = std::find( _tokens.begin(), _tokens.end(), "(" ) ;
                auto iter_close = std::find( _tokens.begin(), _tokens.end(), ")" ) ;

                if( iter_open == _tokens.end() ) return std::move( s ) ;
                if( iter_close == _tokens.end() ) return std::move( s ) ;

                // arg types
                {
                    auto beg = iter_open ;
                    auto iter = iter_open ;
                    auto end = ++iter_close ;
                    --iter_close ;

                    while( ++iter != end )
                    {
                        if( *iter == "," || iter == iter_close )
                        {
                            size_t const num_parts = std::distance( beg, iter ) - 1 ;

                            // empty arg list
                            if( num_parts == 0 )
                            {
                                signature_t::arg_t arg ;
                                arg.name = "" ;
                                arg.type = motor::msl::to_type( "void_t" ) ;
                                _args.emplace_back( std::move( arg ) ) ;
                            }
                            // or void_t maybe
                            else if( num_parts == 1 )
                            {
                                signature_t::arg_t arg ;
                                arg.name = "" ;
                                arg.type = motor::msl::to_type( *( iter - 1 ) ) ;
                                _args.emplace_back( std::move( arg ) ) ;
                            }
                            else if( num_parts >= 2 )
                            {
                                signature_t::arg_t arg ;
                                arg.name = *( iter - 1 ) ;
                                arg.type = motor::msl::to_type( *( iter - 2 ) ) ;
                                _args.emplace_back( std::move( arg ) ) ;
                            }
                            else 
                            {
                                // error. args must type and name only.
                                motor::log::global_t::error( "[parser] : arg must type and name only. Found: " +
                                    *iter ) ;

                                _args.emplace_back( signature_t::arg_t() ) ;
                            }

                            beg = iter ;
                        }
                    }
                }

                // name and return type
                {
                    if( std::distance( _tokens.begin(), iter_open ) >= 2 )
                    {
                        name = *--iter_open ;
                        return_type = motor::msl::to_type( *--iter_open ) ;
                    }
                    else return std::move( s ) ;
                }

                s.args = _args ;
                s.name = name ;
                s.return_type = return_type ;

                return std::move( s ) ;
            }
        };
    }
}