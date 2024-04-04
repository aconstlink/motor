#pragma once

#include "api.h"
#include "typedefs.h"

#include <vector>
#include <mutex>

namespace motor { namespace memory {

    class observer
    {
        motor_this_typedefs( observer ) ;

    public:

        enum class alloc_type
        {
            unknown,
            managed,
            allocation,
            deallocation
        };

        struct message
        {
            alloc_type type ;
            size_t sib ;
        };

    private:

        std::mutex _mtx ;
        size_t _ridx = 0 ;
        std::vector< message > _messages[ 2 ] ;

        size_t _num_ref_inc = 0 ;
        size_t _num_ref_dec = 0 ;

    private:

        size_t ridx( void_t ) const noexcept
        {
            return _ridx ;
        }

        size_t widx( void_t ) const noexcept
        {
            return (_ridx + 1) % 2 ;
        }

        void_t swap_idx( void_t ) noexcept
        {
            _ridx = this_t::widx() ;
        }

    public:

        observer( void_t ) noexcept
        {
            _messages[ this_t::ridx() ].reserve( 5000 ) ;
            _messages[ this_t::widx() ].reserve( 10000 ) ;
        }

    public:

        size_t on_alloc( size_t const sib, bool_t const managed ) noexcept
        {
            auto const type = managed ? this_t::alloc_type::managed : this_t::alloc_type::allocation ;

            std::lock_guard< std::mutex > lk( _mtx ) ;
            _messages[ this_t::widx() ].emplace_back( this_t::message { type, sib } ) ; 

            return _messages[ this_t::widx() ].size() ;
        }

        void_t on_dealloc( size_t const sib ) noexcept
        {
            std::lock_guard< std::mutex > lk( _mtx ) ;
            _messages[ this_t::widx() ].emplace_back( 
                this_t::message { this_t::alloc_type::deallocation, sib } ) ;
        }

        void_t on_ref_inc( void_t ) noexcept
        {
            std::lock_guard< std::mutex > lk( _mtx ) ;
            ++_num_ref_inc ;
        }

        void_t on_ref_dec( void_t ) noexcept
        {
            std::lock_guard< std::mutex > lk( _mtx ) ;
            ++_num_ref_dec ;
        }

    public:

        struct observable_data
        {
            std::vector< message > messages ;
            size_t ref_inc ;
            size_t ref_dec ;
        };
        motor_typedef( observable_data ) ;

        // copy all alloc infos and clear the internal array
        observable_data_t swap_and_clear( void_t ) noexcept
        {
            observable_data_t ret ;

            {
                std::lock_guard< std::mutex > lk( _mtx ) ;

                ret.ref_inc = _num_ref_inc ;
                ret.ref_dec = _num_ref_dec ;

                _num_ref_dec = 0 ;
                _num_ref_inc = 0 ;

                this_t::swap_idx() ;
            }

            ret.messages = _messages[ this_t::ridx() ] ;
            _messages[ this_t::ridx() ].clear() ;

            return ret ;
        }
    };
    motor_typedef( observer ) ;
} }