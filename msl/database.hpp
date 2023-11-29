#pragma once

#include "typedefs.h"
#include "parser_structs.hpp"

#include <motor/concurrent/mrsw.hpp>

namespace motor
{
    namespace msl
    {
        class database
        {
            motor_this_typedefs( database ) ;

        public:

            motor_typedefs( motor::vector< motor::msl::symbol_t >, symbols ) ;

        private:

            mutable motor::concurrent::mrsw_t _ac ;

            motor::msl::post_parse::configs_t _configs ;

            motor::msl::post_parse::library_t::variables_t _vars ;
            motor::msl::post_parse::library_t::fragments_t _fragments ;

            #if 0
            template< typename T >
            struct data
            {
                motor::vector< motor::msl::symbol_t > deps ;
                T data ;
            };
            motor_typedefs( data< motor::msl::pregen::config_t >, config_data ) ;
            motor_typedefs( data< motor::msl::pregen::library_t >, lib_data ) ;

            motor::vector< config_data_t > _configs ;
            motor::vector< config_data_t > _libs ;
            #endif

        public:

            database( void_t ) noexcept{}

            database( this_cref_t ) = delete ;

            database( this_rref_t rhv ) noexcept :
                _vars (std::move( rhv._vars ) ) , 
                _fragments (std::move( rhv._fragments )),
                _configs (std::move( rhv._configs )) {}

            ~database( void_t ) noexcept{}

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _vars = std::move( rhv._vars ) ;
                _fragments = std::move( rhv._fragments ) ;
                _configs = std::move( rhv._configs ) ;
                return *this ;
            }

        public:
            
            void_t insert( motor::msl::post_parse::document_rref_t doc ) noexcept
            {
                this_t::symbols_t tmp ;
                this_t::insert( std::move( doc), tmp ) ;
            }

            void_t insert( motor::msl::post_parse::document_rref_t doc, symbols_out_t changed_configs ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;

                for( auto const & c : doc.configs )
                {
                    auto iter = std::find_if( _configs.begin(), _configs.end(), 
                        [&] ( motor::msl::post_parse::config_cref_t c1 ) { return c1.name == c.name ; } ) ;
                    if( iter != _configs.end() ) 
                    {
                        *iter = c ;
                    }
                    else
                    {
                        _configs.emplace_back( c ) ;
                    }
                    
                    changed_configs.emplace_back( motor::msl::symbol_t( c.name ) ) ;
                }

                symbols_t frags ;
                symbols_t vars ;

                for( auto const & l : doc.libraries )
                {
                    for( auto const & f : l.fragments )
                    {
                        auto iter = std::find( _fragments.begin(), _fragments.end(), f ) ;
                        if( iter != _fragments.end() ) 
                        {
                            *iter = f ;
                        }
                        else
                        {
                            _fragments.emplace_back( f ) ;
                        }

                        frags.emplace_back( f.sym_long ) ;
                    }

                    for( auto const & v : l.variables )
                    {
                        auto iter = std::find( _vars.begin(), _vars.end(), v ) ;
                        if( iter != _vars.end() ) 
                        {
                            *iter = v ;
                        }
                        else
                        {
                            _vars.emplace_back( v ) ;
                        }

                        vars.emplace_back( v.sym_long ) ;
                    }
                }

                // check fragment dependencies
                {
                    auto frags2 = std::move( frags ) ;
                    while( !frags2.empty() )
                    {
                        for( auto const& s : frags2 )
                        {
                            for( auto const& f : _fragments )
                            {
                                auto const iter = std::find( f.deps.begin(), f.deps.end(), s ) ;
                                if( iter == f.deps.end() ) continue ;

                                frags.emplace_back( f.sym_long ) ;
                            }

                            for( auto const& c : _configs )
                            {
                                for( auto const& shd : c.shaders )
                                {
                                    auto const iter = std::find( shd.deps.begin(), shd.deps.end(), s ) ;
                                    if( iter == shd.deps.end() ) continue ;

                                    changed_configs.emplace_back( motor::msl::symbol_t( c.name ) ) ;
                                }
                            }
                        }
                        frags2 = std::move( frags ) ;
                    }
                }

                // check variable dependencies
                {
                    for( auto const& s : vars )
                    {
                    }
                }
            }

        public:

            

        public:

            bool_t find_variable( motor::msl::symbol_cref_t sym, motor::msl::post_parse::library_t::variable_out_t res ) const 
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;
                for( auto const& v : _vars ) if( v.sym_long == sym ) { res = v ; return true ;  }
                return false ;
            }

            bool_t find_fragments( motor::msl::symbol_cref_t sym, motor::msl::post_parse::library_t::fragments_out_t frags ) const
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;

                for( auto const& f : _fragments ) if( f.sym_long == sym ) frags.emplace_back( f ) ;

                return frags.size() != 0 ;
            }

            bool_t find_config( motor::msl::symbol_cref_t sym, motor::msl::post_parse::config_out_t cfg ) const
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;
                for( auto const & c : _configs )
                {
                    if( sym == c.name ) { cfg = c; return true ; }
                }
                return false ;
            }

        };
        motor_typedef( database ) ;
    }
}