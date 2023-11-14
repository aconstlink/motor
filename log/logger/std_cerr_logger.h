
#pragma once

#include "../ilogger.h"

#include <mutex>

namespace motor
{
    namespace log
    {
        class std_cerr_logger : public motor::log::ilogger
        {
            std::mutex _mtx ;

        public:

            std_cerr_logger( void_t ) noexcept ;
            virtual ~std_cerr_logger( void_t ) noexcept ;

        public:

            virtual motor::log::result log( motor::log::log_level const ll, motor::string_cref_t msg ) noexcept ;
        };
    }
}