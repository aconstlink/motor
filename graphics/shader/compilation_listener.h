

#pragma once

#include "shader_bindings.hpp"

#include <motor/concurrent/mrsw.hpp>

namespace motor
{
    namespace graphics
    {
        class compilation_listener
        {
            motor_this_typedefs( compilation_listener ) ;

        public:

            enum class state
            {
                initial,
                in_process,
                failed,
                successful
            };

        private:

            mutable motor::concurrent::mrsw_t _mtx ;

            size_t _has_changed = 0 ;

            state _state ;


            motor::graphics::shader_bindings_t _bindings ;

        public:

            void_t set( this_t::state const s, motor::graphics::shader_bindings_cref_t bindings ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx ) ;
                ++_has_changed ;
                _state = s ;

                if( s == this_t::state::successful ) 
                {
                    _bindings = bindings ;
                }
            }

            bool_t has_changed( void_t ) const noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                return _has_changed != 0 ;
            }

            // @note only valid until reset.
            bool_t is_compilation_successful( void_t ) const noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                return _state == this_t::state::successful ;
            }

            // reset and return if compilation was successful
            bool_t get_if_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                bool_t const ret = _state == this_t::state::successful ;
                if ( ret ) sb = _bindings ;
                return ret ;
            }

            // reset and return if compilation was successful
            bool_t reset_and_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx ) ;
                _has_changed = 0 ;
                auto const succ = _state == this_t::state::successful ;
                _state = this_t::state::initial ;
                if( succ ) sb = std::move( _bindings ) ;
                return succ ;
            }
        };
        motor_typedef( compilation_listener ) ;
    }
}