#pragma once

#include "task/task.hpp"
#include "task/loose_thread_scheduler.hpp"

namespace motor
{
    namespace concurrent
    {
        class MOTOR_CONCURRENT_API scheduler
        {
            motor_this_typedefs( scheduler ) ;

        private:

            loose_thread_scheduler_t _loose ;

        public:

            struct update_accessor
            {
                static void_t udpate( this_ptr_t s ) noexcept 
                {
                    s->update() ;
                }
            };
            motor_typedef( update_accessor ) ;
            friend struct update_accessor ;

        private:

            void_t update( void_t ) noexcept ;

        public:

            scheduler( void_t ) noexcept ;
            scheduler( this_cref_t ) = delete ;
            scheduler( this_rref_t ) noexcept ;
            ~scheduler( void_t ) noexcept ;

        public:

            //void_t schedule_loose( motor::concurrent::task_mtr_t ) noexcept ;
        };
    }
}