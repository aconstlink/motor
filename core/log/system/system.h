
#pragma once

#include "../isystem.h"
#include "../logger/store_logger.h"

#include <motor/core/std/string>
#include <motor/core/std/vector>

namespace motor
{
    namespace log
    {
        // "system" being the default system
        class system : public isystem
        {
            motor_this_typedefs( system ) ;
            motor_typedefs( motor::core::vector< motor::log::ilogger_ptr_t >, loggers ) ;

            loggers_t _loggers ;
            motor::log::store_logger_t _slogger ;

        private:

            motor::log::ilogger_ptr_t _default_logger ;
            motor::log::ilogger_ptr_t _this_default_logger ;

        public:

            static this_ptr_t create( void_t ) noexcept ;
            static void_t destroy( this_ptr_t ) noexcept ;

        public:

            system( void_t ) noexcept ;
            system( this_rref_t ) noexcept ;
            virtual ~system( void_t ) noexcept ;

            virtual motor::log::result log( motor::log::log_level const ll, motor::core::string_cref_t msg ) noexcept ;
            virtual motor::log::result add_logger( motor::log::ilogger_ptr_t lptr ) noexcept ;
            virtual motor::log::ilogger_ptr_t set_default_logger( motor::log::ilogger_ptr_t lptr ) noexcept ;

            motor::log::store_logger_cptr_t get_store( void_t ) const noexcept ;
        };
        motor_typedef( system ) ;
    }
}