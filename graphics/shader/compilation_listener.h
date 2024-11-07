

#pragma once

#include "shader_bindings.hpp"

#include <motor/concurrent/mrsw.hpp>

namespace motor
{
    namespace graphics
    {
        class compilation_listener
        {
        private:

            mutable motor::concurrent::mrsw_t _mtx ;

            size_t _has_changed = 0 ;

            bool_t _successful = false ;


            motor::graphics::shader_bindings_t _bindings ;

        public:

            void_t set( bool_t const compilation_sucessful, motor::graphics::shader_bindings_cref_t bindings ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx ) ;
                ++_has_changed ;
                _successful = compilation_sucessful ;

                if( compilation_sucessful ) 
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
                return _successful ;
            }

            // reset and return if compilation was successful
            bool_t get_if_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                if ( _successful ) sb = _bindings ;
                return _successful ;
            }

            // reset and return if compilation was successful
            bool_t reset_and_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx ) ;
                _has_changed = 0 ;
                auto const succ = _successful ;
                _successful = false ;
                if( succ ) sb = std::move( _bindings ) ;
                return succ ;
            }
        };
        motor_typedef( compilation_listener ) ;
    }
}