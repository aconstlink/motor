
#pragma once

#include "../ilogger.h"

#include <motor/core/std/vector>

#include <functional>
#include <mutex>

namespace motor
{
    namespace log
    {
        class MOTOR_CORE_API store_logger : public  motor::log::ilogger
        {
            motor_this_typedefs( store_logger ) ;

        public:

            struct store_data
            {
                motor::log::log_level ll ;
                motor::core::string_t msg ;
            };
            motor_typedef( store_data );

        private:

            size_t const max_elems = 10000 ;
            motor_typedefs( motor::core::vector< store_data_t >, stores );

            mutable std::mutex _mtx ;
            stores_t _stores ;

            size_t _cp_start = 0 ;
            size_t _cp_end = 0 ;

        public:

            typedef std::function < void_t ( store_data_cref_t ) > funk_t ;

        public:

            store_logger( void_t ) noexcept ;
            store_logger( this_cref_t ) = delete ;
            store_logger( this_rref_t ) noexcept;
            virtual ~store_logger( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            virtual motor::log::result log( motor::log::log_level const ll, motor::core::string_cref_t msg ) noexcept ;

            size_t get_max_items( void_t ) const noexcept ;
            size_t get_num_items( void_t ) const noexcept ;
            void_t for_each( size_t const begin, size_t const end, funk_t ) const noexcept ;
            void_t copy( size_t const begin, size_t const end, stores_ref_t ) const noexcept ;

        };
        motor_typedef( store_logger ) ;
    }
}