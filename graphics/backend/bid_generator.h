#pragma once

#include "../typedefs.h"
#include <mutex>

namespace motor
{
    namespace graphics
    {
        class bid_generator
        {
            motor_this_typedefs( bid_generator ) ;

        private:

            std::mutex _mtx ;
            size_t _bid = 0 ;

        public:

            bid_generator( void_t ) noexcept {}
            bid_generator( this_cref_t ) = delete ;
            bid_generator( this_rref_t ) = delete ;

            ~bid_generator( void_t ) noexcept {}

        public:

            size_t generate( void_t ) noexcept
            {
                std::lock_guard< std::mutex > lk( _mtx ) ;
                return _bid++ ;
            }
        };

        static bid_generator bid_gen ;
    }
}