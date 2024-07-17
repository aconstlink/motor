
#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "../slot/input_slot.h"
#include "../slot/output_slot.h"

#include <motor/concurrent/task/task.hpp>
#include <motor/concurrent/mrsw.hpp>

#include <motor/std/string>

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API inode
        {
            motor_this_typedefs( inode ) ;

        private:

            motor::concurrent::task_mtr_t _task ;

            using nodes_t = motor::vector< this_mtr_t > ;

            motor::concurrent::mrsw_t _mtx_in ;
            motor::concurrent::mrsw_t _mtx_out ;
            nodes_t _incoming ;
            nodes_t _outgoing ;
             
            motor::string_t _name ;

        public:

            inode( void_t ) noexcept ;
            inode( motor::string_in_t ) noexcept ;
            inode( this_rref_t ) noexcept ;
            inode( this_cref_t ) = delete ;
            virtual ~inode( void_t ) noexcept ;

            this_mtr_t then( this_mtr_safe_t ) noexcept ;

            motor::concurrent::task_mtr_safe_t get_task( void_t ) noexcept ;

            void_t disconnect( void_t ) noexcept ;

            motor::string_cref_t name( void_t ) const noexcept ;

        public:

            virtual void_t execute( void_t ) noexcept = 0 ;

        private:

            friend class inode ;

            // other connects before this
            // other -> this
            bool_t add_outgoing( this_ptr_t other ) noexcept ;

            void_t remove_outgoing( this_ptr_t other ) noexcept ;

            // other connects after this
            // this -> other
            void_t add_incoming( this_ptr_t other ) noexcept ;

            void_t remove_incoming( this_ptr_t other ) noexcept ;

            motor::concurrent::task_ptr_t task( void_t ) noexcept ;

            motor::concurrent::task_t::task_funk_t make_task_funk( void_t ) noexcept ;
        };
        motor_typedef( inode ) ;

        class MOTOR_WIRE_API node : public inode
        {
            motor_this_typedefs( node ) ;

        public: 

            using funk_t = std::function< void_t ( this_ptr_t ) > ;

        private:
            
            funk_t _funk ;

        public:

            node( funk_t f ) noexcept : _funk( f ) {}
            node( motor::string_in_t n, funk_t f ) noexcept : inode(n), _funk( f ) {}
            node( this_rref_t rhv ) noexcept : inode( std::move( rhv ) ), _funk( std::move( rhv._funk ) ) {}
            virtual ~node( void_t ) noexcept {}


        public:

            virtual void_t execute( void_t ) noexcept ;
        };
        motor_typedef( node ) ;
    }
}