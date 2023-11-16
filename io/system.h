#pragma once

#include "handle.h"
#include "obfuscator.hpp"

#include <motor/concurrent/typedefs.h>
#include <mutex>
#include <motor/std/stack>

#include <motor/core/macros/move.h>

namespace motor
{
    namespace io
    {
        class MOTOR_IO_API system
        {
            motor_this_typedefs( system ) ;

            friend class load_handle ;
            friend class store_handle ;

        private:

            struct load_item
            {
                motor_this_typedefs( load_item ) ;

                char_ptr_t data = nullptr ;
                size_t sib = 0 ;

                std::condition_variable cv ;
                motor::concurrent::mutex_t mtx ;
                bool_t ready = false ;
                motor::io::result status = motor::io::result::invalid ;

                load_item( void_t ) {}
                load_item( this_rref_t rhv )
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ready = rhv.ready ;
                    status = rhv.status ;
                }
                load_item( this_cref_t ) = delete ;
                ~load_item( void_t ) {}

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t rhv )
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ready = rhv.ready ;
                    status = rhv.status ;
                    return *this ;
                }
            };
            motor_typedef( load_item ) ;

            using load_stack_t = motor::stack< load_item_ptr_t, 20 > ;
            using load_stack_cref_t = load_stack_t const & ;
            using load_stack_ref_t = load_stack_t & ;
            using load_stack_rref_t = load_stack_t && ;

            using load_items_t = motor::vector< load_item_ptr_t > ;

            struct store_item
            {
                motor_this_typedefs( store_item ) ;

                char_cptr_t data = nullptr ;
                size_t sib = 0 ;

                std::condition_variable cv ;
                motor::concurrent::mutex_t mtx ;
                bool_t ready = false ;
                motor::io::result status = motor::io::result::invalid ;

                store_item( void_t ) {}
                store_item( this_rref_t rhv )
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ready = rhv.ready ;
                    status = rhv.status ;
                }
                store_item( this_cref_t ) = delete ;
                ~store_item( void_t ) {}

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t rhv )
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ready = rhv.ready ;
                    status = rhv.status ;
                    return *this ;
                }
            };
            motor_typedef( store_item ) ;

            using store_stack_t = motor::stack< store_item_ptr_t, 20 > ;
            using store_stack_cref_t = store_stack_t const & ;
            using store_stack_ref_t = store_stack_t & ;
            using store_stack_rref_t = store_stack_t && ;

            using store_items_t = motor::vector< load_item_ptr_t > ;

        private:

            motor::concurrent::mutex_t _load_mtx ;
            load_stack_t _load_stack ;

            motor::concurrent::mutex_t _store_mtx ;
            store_stack_t _store_stack ;

        public:

            system( void_t ) noexcept ;
            ~system( void_t ) noexcept ;

            system( this_cref_t ) = delete ;
            system( this_rref_t ) noexcept ;


        public:

            motor::io::load_handle_t load( motor::io::path_cref_t,
                motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) noexcept ;

            motor::io::load_handle_t load( motor::io::path_cref_t,
                size_t const offset = size_t( 0 ), size_t const sib = size_t( -1 ),
                motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) noexcept ;

            motor::io::store_handle_t store( motor::io::path_cref_t, char_cptr_t, size_t const,
                motor::io::obfuscator_rref_t = motor::io::obfuscator_t()) noexcept ;

        private:

            motor::io::result wait_for_operation( motor::io::load_handle_rref_t, motor::io::load_completion_funk_t ) noexcept ;
            motor::io::result wait_for_operation( motor::io::store_handle_rref_t, motor::io::store_completion_funk_t ) noexcept ;

        private:

            load_item_ptr_t get_load_item( void_t ) noexcept ;
            store_item_ptr_t get_store_item( void_t ) noexcept ;

        };
        motor_typedef( system ) ;
    }
}