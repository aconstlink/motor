#pragma once

#include "../api.h"
#include "../result.h"
#include "../typedefs.h"
#include "../app.h"

#include <motor/concurrent/typedefs.h>

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API platform_application
        {
            motor_this_typedefs( platform_application ) ;

        private:

            motor::application::app_res_t _app ;
            
            struct shared_data
            {
                bool_t update_running = false ;
            };
            shared_data* _sd ;

            motor::concurrent::mutex_t _mtx ;
            motor::concurrent::thread_t _thr ;

        public:

            platform_application( void_t ) ;
            platform_application( this_cref_t ) = delete ;
            platform_application( this_rref_t ) ;
            platform_application( motor::application::app_res_t ) ;
            virtual ~platform_application( void_t ) ;

        public:
            
            int_t exec( void_t ) ;

        private:

            /// at the moment, the app object must be added by the applications' ctor
            /// this is due to multi-threading issues when creating a window in the app.
            motor::application::result set( motor::application::app_res_t ) ;

            motor::application::result start_update_thread( void_t ) ;
            void_t stop_update_thread( void_t ) ;

        public: // interface 

            virtual motor::application::result on_exec( void_t ) = 0 ;
        };
        motor_res_typedef( platform_application ) ;
    }
}