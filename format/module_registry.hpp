#pragma once

#include "api.h"
#include "typedefs.h"

#include "module_factory.hpp"

#include <motor/log/global.h>
#include <motor/concurrent/mrsw.hpp>
#include <motor/std/vector>

namespace motor
{
    namespace format
    {
        class MOTOR_FORMAT_API module_registry
        {
            motor_this_typedefs( module_registry ) ;

        private: 

            struct data
            {
                data( void_t ) {}
                data( data const & rhv ) : ext( rhv.ext ), fac(rhv.fac) {}
                data( data && rhv ) : ext( std::move(rhv.ext) ), fac(std::move(rhv.fac)) {}
                ~data( void_t ) {}
                data& operator = ( data const& rhv ) { ext = rhv.ext ; fac = rhv.fac ; return *this ; }
                data& operator = ( data && rhv ) { ext = std::move(rhv.ext) ; fac = std::move(rhv.fac) ; return *this ; }
                motor::string_t ext ;
                motor::format::imodule_factory_mtr_t fac ;
            };
            motor_typedef( data ) ;
            motor_typedefs( motor::vector< data_t >, factories ) ;
            factories_t _imports ;
            factories_t _exports ;

            motor::concurrent::mrsw_t _ac ;

        public:

            module_registry( void_t ) noexcept {}
            module_registry( this_cref_t ) = delete ;
            module_registry( this_rref_t rhv ) noexcept 
            {
                _imports = std::move( rhv._imports ) ;
                _exports = std::move( rhv._exports ) ;
            }
            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept 
            {
                _imports = std::move( rhv._imports ) ;
                _exports = std::move( rhv._exports ) ;
                return *this ;
            }
            ~module_registry( void_t ) noexcept 
            {
                for( auto & d : _imports ) motor::memory::release_ptr( d.fac ) ;
                for( auto & d : _exports ) motor::memory::release_ptr( d.fac ) ;
            }

        public:

            bool_t register_import_factory( motor::vector< motor::string_t > const & exts, motor::format::imodule_factory_mtr_t fac ) noexcept
            {
                for( auto const & ext : exts )
                {
                    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;

                    this_t::data_t d ;
                    d.ext = ext ;
                    d.fac = motor::share( fac );

                    // add to the front so newer modules will be preferred.
                    _imports.insert( _imports.begin(), std::move( d ) ) ;
                }

                return true ;
            }

            bool_t register_export_factory( motor::vector< motor::string_t > const& exts, motor::format::imodule_factory_mtr_t fac ) noexcept
            {
                for( auto const& ext : exts )
                {
                    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;
                    
                    this_t::data_t d ;
                    d.ext = ext ;
                    d.fac = fac ;

                    // add to the front so newer modules will be preferred.
                    _exports.insert( _exports.begin(), std::move( d ) ) ;
                }

                return true ;
            }

        public:

            motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_safe_t::mtr_t db ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;

                auto fac = this_t::borrow_import_factory( loc ) ;
                if( fac == nullptr )
                {
                    return std::async( std::launch::deferred, [=] ( void_t ) -> item_mtr_t
                    {
                        motor::log::global_t::warning( "Can not create factory for extension : " + loc.extension() ) ;
                        return motor::shared( motor::format::status_item_t( "Can not create factory for extension : " + loc.extension() ) ) ;
                    } ) ;
                }
                
                return fac->create_module( loc.extension() )->import_from( loc, std::move(db) ) ;
            }

            motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_safe_t::mtr_t db, 
                motor::property::property_sheet_safe_t::mtr_t ps ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;

                auto fac = this_t::borrow_import_factory( loc ) ;
                if( fac == nullptr )
                {
                    return std::async( std::launch::deferred, [&] ( void_t ) -> item_mtr_t
                    {
                        motor::log::global_t::warning( "Can not create factory for extension : " + loc.extension() ) ;
                        return motor::shared( motor::format::status_item_t( "Can not create factory for extension : " + loc.extension() ) ) ;
                    } ) ;
                }
                
                return fac->create_module( loc.extension() )->import_from( loc, std::move( db ), std::move( ps ) ) ;
            }

            motor::format::future_item_t export_to( motor::io::location_cref_t loc, motor::io::database_safe_t::mtr_t db, 
                motor::format::item_safe_t::mtr_t what ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;

                auto fac = this_t::borrow_export_factory( loc ) ;
                if( fac == nullptr )
                {
                    return std::async( std::launch::deferred, [&] ( void_t ) -> item_mtr_t
                    {
                        motor::log::global_t::warning( "Can not create factory for extension : " + loc.extension() ) ;
                        return motor::shared( motor::format::status_item_t( "Can not create factory for extension : " + loc.extension() ) ) ;
                    } ) ;
                }
                
                return fac->create_module( loc.extension() )->export_to( loc, std::move( db ), std::move(what) ) ;
            }

        private:

            motor::format::imodule_factory_mtr_t borrow_import_factory( motor::io::location_cref_t loc ) const noexcept
            {
                auto iter = std::find_if( _imports.begin(), _imports.end(), [&] ( this_t::data_cref_t d ) 
                { 
                    return d.ext == loc.extension(false) ;
                } ) ;

                if( iter == _imports.end() ) return nullptr ;

                return iter->fac ;
            }

            motor::format::imodule_factory_mtr_t borrow_export_factory( motor::io::location_cref_t loc ) const noexcept
            {
                auto iter = std::find_if( _exports.begin(), _exports.end(), [&] ( this_t::data_cref_t d ) 
                { 
                    return d.ext == loc.extension(false) ;
                } ) ;

                if( iter == _exports.end() ) return nullptr ;

                return iter->fac ;
            }
        };
        motor_typedef( module_registry ) ;
    }
}