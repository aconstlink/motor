#pragma once

#include "api.h"
#include "typedefs.h"
#include "handle.hpp"
#include "monitor.hpp"
#include "obfuscator.hpp"
#include "location.hpp"

#include <motor/io/global.h>
#include <motor/concurrent/isleep.hpp>
#include <motor/concurrent/mrsw.hpp>
#include <motor/std/vector>
#include <motor/std/filesystem>

namespace motor
{
    namespace io
    {
        class MOTOR_IO_API database
        {
            motor_this_typedefs( database ) ;

        private: // monitoring stuff

            motor::concurrent::thread_t _update_thread ;
            motor::concurrent::interruptable_sleep_t _isleep ;

        public:

            typedef std::function< void_t ( char_cptr_t, size_t const, motor::io::result const ) > load_completion_funk_t ;

        private:
            
            // this is the cache of a file record.
            // it holds :
            // - the load/store handles of an operation
            // - the cached data
            // - the owner database
            // - the index into the databases' file record
            struct record_cache ;
            motor_typedef( record_cache ) ;
            friend struct record_cache ;

            struct file_record ;
            motor_typedef( file_record ) ;

        private:

            struct file_header ;
            motor_typedef( file_header ) ;

            struct db ;
            motor_typedef( db ) ;

            db_ptr_t _db = nullptr ;
            mutable motor::concurrent::mrsw_t _ac ;

            
            static motor::io::obfuscator_t obfuscator( void_t ) noexcept
            {
                return motor::io::obfuscator_t::variant_1() ;
            }

        public:

            enum class encryption
            {
                none
            };
            
            // user access for file record 
            class MOTOR_IO_API cache_access
            {
                motor_this_typedefs( cache_access ) ;

                friend class database ;

            private:

                record_cache_mtr_t _res ;
                cache_access( record_cache_mtr_t res ) noexcept ;

            public:

                cache_access( this_cref_t rhv ) noexcept ;
                cache_access( this_rref_t rhv ) noexcept ;
                ~cache_access( void_t ) noexcept ;

                this_ref_t operator = ( this_cref_t rhv ) noexcept ;

                this_ref_t operator = ( this_rref_t rhv ) noexcept ;

                // wait for io operation to be finished or taken from data cache
                // if cache exists, it will be preferred. Data cache should be updated
                // automatically on file change which will be notified by the monitor.
                bool_t wait_for_operation( motor::io::database::load_completion_funk_t funk ) ;

                // load the same location.
                this_t load( bool_t const reload = false ) noexcept ;

                // changes the location of this record cache access.
                this_t load( motor::io::location_cref_t loc, bool_t const reload = false ) noexcept ;
            };
            motor_typedef( cache_access ) ;

        public:

            database( void_t ) ;
            database( motor::io::path_cref_t base, motor::io::path_cref_t working_rel = "./",
                motor::io::path_cref_t name = "db" ) ;

            database( this_cref_t ) = delete ;
            database( this_rref_t rhv ) noexcept ;
            ~database( void_t ) ;

            this_ref_t operator = ( this_rref_t rhv ) noexcept ;

        public:

            // gives access to some file system structure
            // or to a motor database file.
            //
            // @param base the base path to the database file
            // @param working_rel where the data should be look for/stored at relative to base
            // @param name the db file/folder name
            //
            // Filesystem: /base/name
            // motor File: /base/name.motor
            bool_t init( motor::io::path_cref_t base, motor::io::path_cref_t working_rel = "./", 
                motor::io::path_cref_t name = "db" ) ;

            // pack the initialized resource into a motor file
            bool_t pack( this_t::encryption const = this_t::encryption::none ) ;

            // unpack the initialized resource to a file system
            bool_t unpack( void_t ) ;


            // store from memory
            motor::io::system_t::store_handle_t store( motor::io::location_cref_t location , char_cptr_t, size_t const ) noexcept ;
            
            // load to memory
            // @param loc the file location
            // @param reload force reload from database( filesystem/mdb file )
            this_t::cache_access_t load( motor::io::location_cref_t loc, bool_t const reload = false ) ;

            void_t dump_to_std( void_t ) const noexcept ;

        public: // monitor
            
            void_t attach( motor::io::location_cref_t, motor::io::monitor_mtr_safe_t ) noexcept ;
            void_t detach( motor::io::location_cref_t, motor::io::monitor_mtr_t ) noexcept ;
            
            void_t attach( motor::io::monitor_mtr_safe_t ) noexcept ;
            void_t detach( motor::io::monitor_mtr_t mon ) noexcept ;

        public:
            
            bool_t lookup_extension( motor::string_cref_t loc, motor::string_ref_t ) const noexcept ;

        private:

            file_record_t create_file_record( this_t::db_ref_t, motor::io::path_cref_t ) const noexcept;
            bool_t lookup( motor::io::location_cref_t loc, file_record_out_t ) const noexcept ;
            bool_t lookup( db const & db_, motor::io::location_cref_t loc, file_record_out_t ) const noexcept ;
            void_t file_change_stamp( this_t::file_record_cref_t ) noexcept ;
            void_t file_remove( this_t::file_record_cref_t ) noexcept ;
            
            void_t file_change_external( this_t::file_record_cref_t ) noexcept ;
            static void_t file_change_external( db & db_, this_t::file_record_cref_t ) noexcept ;

            motor::io::location_t location_for_index( size_t const ) const ;

        private: // monitor
            
            void_t spawn_update( void_t ) noexcept ;
            bool_t join_update( void_t ) noexcept ;

        private:

            void_t load_db_file( db & db_, motor::io::path_cref_t ) ;

        };
        motor_typedef( database ) ;
    }
}
