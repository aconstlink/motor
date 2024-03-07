#pragma once

#include "database.hpp"
#include "generators/generator.h"
#include "parser_structs.hpp"

namespace motor
{
    namespace msl
    {
        // given a symbol, this class resolves every dependency 
        // it finds. If one dependent symbol is not found, this
        // one is inserted into the missing list.
        class dependency_resolver
        {
            motor_this_typedefs( dependency_resolver ) ;

        public:

            motor::msl::generatable_t resolve( motor::msl::database_mtr_t db, motor::msl::symbol_cref_t sym ) noexcept
            {
                motor::msl::generatable_t res ;
                motor::msl::symbols_t syms( { sym } ) ;

                {
                    motor::msl::symbols_t tmp = std::move( syms ) ;

                    for( auto const& s : tmp )
                    {
                        motor::msl::post_parse::config_t c ;
                        if( db->find_config( s, c ) )
                        {
                            res.config = c ;
                            for( auto const & shd : c.shaders )
                            {
                                for( auto const& d : shd.deps )
                                {
                                    syms.emplace_back( d ) ;
                                }
                            }
                        }
                        else syms.emplace_back( sym ) ;
                    }
                }

                while( syms.size() != 0 )
                {
                    motor::msl::symbols_t tmp = std::move( syms ) ;

                    for( auto const& s : tmp )
                    {
                        // check variable first
                        {
                            motor::msl::post_parse::library_t::variable_t var ;
                            if( db->find_variable( s, var ) )
                            {
                                auto const iter = std::find( res.vars.begin(), res.vars.end(), var ) ;
                                if( iter == res.vars.end() ) res.vars.emplace_back( var ) ;
                                continue ;
                            }
                        }

                        {
                            motor::msl::post_parse::library_t::fragments_t frgs ;
                            if( !db->find_fragments( s, frgs ) )
                            {
                                res.missing.emplace_back( s ) ;
                                continue;
                            }

                            for( auto const& frg : frgs )
                            {
                                auto iter = std::find( res.frags.begin(), res.frags.end(), frg ) ;
                                if( iter != res.frags.end() ) continue ;

                                res.frags.emplace_back( frg ) ;

                                for( auto const& dep : frg.deps )
                                {
                                    syms.emplace_back( dep ) ;
                                }
                            }
                        }
                    }
                }

                return std::move( res ) ;
            }
        };
        motor_typedef( dependency_resolver ) ;
    }
}