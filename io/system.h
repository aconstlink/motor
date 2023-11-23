#pragma once

#include "handle.hpp"
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

        private:

            struct load_item
            {
                motor_this_typedefs( load_item ) ;

                char_ptr_t data = nullptr ;
                size_t sib = 0 ;

                std::future<motor::io::result> ftr ;

                load_item( void_t ) noexcept {}
                load_item( this_rref_t rhv ) noexcept
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ftr = std::move( rhv.ftr ) ;
                }
                load_item( this_cref_t ) = delete ;
                ~load_item( void_t ) noexcept {}

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t rhv ) noexcept
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ftr = std::move( rhv.ftr ) ;
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
                
                std::future<motor::io::result> ftr ;

                store_item( void_t ) noexcept {}
                store_item( this_rref_t rhv ) noexcept
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ftr = std::move( rhv.ftr ) ;
                }
                store_item( this_cref_t ) = delete ;
                ~store_item( void_t ) noexcept {}

                this_ref_t operator = ( this_cref_t ) = delete ;
                this_ref_t operator = ( this_rref_t rhv ) noexcept
                {
                    motor_move_member_ptr( data, rhv ) ;
                    sib = rhv.sib ;
                    ftr = std::move( rhv.ftr ) ;
                    return *this ;
                }
            };
            motor_typedef( store_item ) ;

            using store_stack_t = motor::stack< store_item_ptr_t, 20 > ;
            using store_stack_cref_t = store_stack_t const & ;
            using store_stack_ref_t = store_stack_t & ;
            using store_stack_rref_t = store_stack_t && ;

            using store_items_t = motor::vector< load_item_ptr_t > ;

        public: // handles definitions

            using load_handle_t = motor::io::handle< load_item, this_t > ;
            using load_handle_rref_t = load_handle_t && ;
            using store_handle_t = motor::io::handle< store_item, this_t > ;
            using store_handle_rref_t = store_handle_t && ;


            friend load_handle_t ;
            friend store_handle_t ;

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

            this_t::load_handle_t load( motor::io::path_cref_t, std::launch const lt = std::launch::deferred,
                motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) noexcept ;

            this_t::load_handle_t load( motor::io::path_cref_t,
                size_t const offset = size_t( 0 ), size_t const sib = size_t( -1 ),
                std::launch const lt = std::launch::deferred, 
                motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) noexcept ;

            this_t::store_handle_t store( motor::io::path_cref_t, char_cptr_t, size_t const,
                std::launch const lt = std::launch::deferred, 
                motor::io::obfuscator_rref_t = motor::io::obfuscator_t()) noexcept ;

        private:

            motor::io::result wait_for_operation( this_t::load_handle_rref_t, motor::io::load_completion_funk_t ) noexcept ;
            motor::io::result wait_for_operation( this_t::store_handle_rref_t, motor::io::store_completion_funk_t ) noexcept ;

        private:

            load_item_ptr_t get_load_item( void_t ) noexcept ;
            store_item_ptr_t get_store_item( void_t ) noexcept ;

        };
        motor_typedef( system ) ;
    }
}