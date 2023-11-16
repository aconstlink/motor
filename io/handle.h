#pragma once

#include "api.h"
#include "typedefs.h"
#include "protos.h"

namespace motor
{
    namespace io
    {
        class MOTOR_IO_API load_handle
        {
            motor_this_typedefs( load_handle ) ;

            friend class system ;

        private:

            motor::io::internal_item_ptr_t _data_ptr = nullptr ;
            motor::io::system_ptr_t _ios = nullptr ;

        private:

            load_handle( motor::io::internal_item_ptr_t, motor::io::system_ptr_t ) ;

        public:

            load_handle( void_t ) ;
            load_handle( this_rref_t ) ;
            ~load_handle( void_t ) ;

            load_handle( this_cref_t ) = delete ;

        public:

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t ) ;

        public:

            motor::io::result wait_for_operation( motor::io::load_completion_funk_t ) ;
            bool_t can_wait( void_t ) const noexcept ;
        };
        motor_typedef( load_handle ) ;

        class MOTOR_IO_API store_handle
        {
            motor_this_typedefs( store_handle ) ;

            friend class system ;

        private:

            motor::io::internal_item_ptr_t _data_ptr = nullptr ;
            motor::io::system_ptr_t _ios = nullptr ;

        private:

            store_handle( motor::io::internal_item_ptr_t, motor::io::system_ptr_t ) ;

        public:

            store_handle( void_t ) ;
            store_handle( this_rref_t ) ;
            ~store_handle( void_t ) ;

            store_handle( this_cref_t ) = delete ;

        public:

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t ) ;

        public:

            motor::io::result wait_for_operation( void_t ) ;
            motor::io::result wait_for_operation( motor::io::store_completion_funk_t ) ;
        };
        motor_typedef( store_handle ) ;
    }
}