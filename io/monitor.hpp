#pragma once

#include "typedefs.h"
#include "location.hpp"

#include <motor/concurrent/typedefs.h>

#include <motor/std/vector>
#include <motor/std/string>
#include <mutex>

namespace motor
{
    namespace io
    {
        class monitor
        {
            motor_this_typedefs( monitor ) ;

        public:

            enum class notify
            {
                none,
                deletion,
                change,
                num_values
            };
            static motor::string_t to_string( this_t::notify const n ) noexcept
            {
                static char const * const __strings[] = { "none", "deletion", "change", "invalid" } ;
                return motor::string_t( __strings[ size_t(n) >= size_t(notify::num_values) ? size_t(notify::num_values) : size_t(n) ] ) ;
            }

        private:

            std::mutex _mtx ;

            struct data
            {
                notify n ;
                motor::io::location_t loc ;
            };
            motor_typedef( data ) ;

            motor::vector< data_t > _changed ;

        public:

            monitor( void_t ) noexcept {}

            monitor( this_cref_t ) = delete ;

            monitor( this_rref_t rhv ) noexcept 
            {
                _changed = std::move( rhv._changed ) ;
            }

            ~monitor( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) = delete ;

            this_ref_t operator = ( this_rref_t rhv ) noexcept 
            {
                _changed = std::move( rhv._changed ) ;
                return *this ;
            }

        public:

            void_t trigger_changed( motor::io::location_cref_t loc, notify const n ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                this_t::insert( { n, loc } ) ;
            }

        private:

            motor::vector< this_t::data_t > _tmp_data ;

        public:

            typedef std::function< void_t ( motor::io::location_cref_t, this_t::notify const ) > foreach_funk_t ;
            void_t for_each_and_swap( foreach_funk_t funk ) noexcept 
            {
                motor::vector< this_t::data_t > & tmp = _tmp_data ;
                {
                    motor::concurrent::lock_guard_t lk( _mtx ) ;
                    tmp = std::move( _changed ) ;
                }

                for( auto const & item : tmp ) 
                {
                    funk( item.loc, item.n ) ;
                }
            }

        private:

            bool_t has( motor::io::location_cref_t loc ) noexcept
            {
                for( auto const & item : _changed )
                {
                    if( item.loc == loc ) return true ;
                }
                return false ;
            }

            void_t insert( data_cref_t d ) noexcept
            {
                if( !this_t::has( d.loc ) )
                {
                    _changed.emplace_back( d ) ;
                }
            }
        };
        motor_typedef( monitor ) ;
    }
}