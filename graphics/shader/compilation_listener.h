

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
            size_t _compilation = 0 ;

            motor::graphics::shader_bindings_t _bindings ;

        public:

            void_t set( bool_t const compilation_sucessful, motor::graphics::shader_bindings_cref_t bindings ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx ) ;
                ++_has_changed ;
                _compilation = compilation_sucessful ? _compilation+1 : _compilation ;
                if( compilation_sucessful ) _bindings = bindings ;
            }

            bool_t has_changed( void_t ) const noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                return _has_changed != 0 ;
            }

            bool_t is_compilation_successful( void_t ) const noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                return _has_changed == _compilation ;
            }

            // reset and return if compilation was successful
            bool_t get_if_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                bool_t const ret = _has_changed == _compilation ;
                if ( ret ) sb = _bindings ;
                return ret ;
            }

            // reset and return if compilation was successful
            bool_t reset_and_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx ) ;
                bool_t const ret = _has_changed == _compilation ;
                _has_changed = 0 ;
                _compilation = 0 ;
                if( ret ) sb = std::move( _bindings ) ;
                return ret ;
            }
        };
        motor_typedef( compilation_listener ) ;
    }
}