#pragma once

#include "api.h"
#include "protos.h"
#include "typedefs.h"
#include "result.h"

#include "id.hpp"

#include <motor/concurrent/typedefs.h>

namespace motor
{
    namespace graphics
    {
        /// 
        class MOTOR_GRAPHICS_API async_id
        {
            motor_this_typedefs( async_id ) ;

            friend class async ;

        private:

            motor::graphics::id_t _id ;            
            mutable motor::concurrent::mutex_t _mtx ;
            motor::graphics::async_result _res = motor::graphics::async_result::invalid ;
            motor::graphics::async_result _config = motor::graphics::async_result::invalid ;

        public:

            async_id( void_t ) noexcept {}
            async_id( this_cref_t ) = delete ;
            async_id( this_rref_t rhv ) noexcept
            {
                _id = std::move( rhv._id ) ;
                _res = std::move( rhv._res ) ;
            }
            ~async_id( void_t ) noexcept {}

            bool_t is_config( motor::graphics::async_result const res ) const 
            {
                motor::concurrent::lock_t lk( _mtx ) ;
                return _config == res ;
            }

            bool_t check_and_swap_user( motor::graphics::async_result const res )
            {
                motor::concurrent::lock_t lk( _mtx ) ;
                if( _config != res ) return false ;
                
                _config = motor::graphics::async_result::user_edit ;
                    
                return true ;

            }

        private: // async interface

            motor::graphics::id_t set( motor::graphics::id_rref_t id, motor::graphics::async_result res )
            {
                {
                    motor::concurrent::lock_guard_t lk( _mtx ) ;
                    _res = res ;
                }

                _id = std::move( id ) ;

                return motor::graphics::id_t() ;
            }

            motor::graphics::id_t id( void_t ) { return std::move( _id ) ; }

            motor::graphics::async_result swap( motor::graphics::async_result const res )
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                auto const old = _res ;
                _res = res ;
                return old ;
            }

            motor::graphics::async_result swap_config( motor::graphics::async_result const res )
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                auto const old = _config ;
                _config = res ;
                return old ;
            }
        };
        motor_typedef( async_id ) ;
    }
}

