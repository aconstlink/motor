#pragma once

#include "api.h"

#include "typedefs.h"
#include "function.h"

#include "log_level.h"

#include <cstdarg>
#include <mutex>

namespace motor
{
    namespace log
    {
        class MOTOR_LOG_API global
        {
            motor_this_typedefs( global ) ;

        private:

            /// should be of type "system", so the log system could be exchanged
            /// by any other system. But since the system stores modules, that is
            /// currently not a big problem.
            motor::log::system_ptr_t __default_log_system = nullptr ;

        private: // singleton

            static std::mutex _mtx ;
            static this_ptr_t _ptr ;

        public:

            global( void_t ) noexcept ;
            global( this_cref_t ) = delete ;
            global( this_rref_t ) noexcept ;
            ~global( void_t ) noexcept ;

        public: // singleton functions

            static this_ptr_t init( void_t ) noexcept ;
            static void_t deinit( void_t ) noexcept ;

            static this_ptr_t get( void_t ) noexcept ;

            static void_t add_logger( motor::log::ilogger_ptr_t ) noexcept ;
            static motor::log::store_logger_cptr_t get_store( void_t ) noexcept ;

        public:

            static void_t message( log_level const level, motor::string_cref_t msg ) noexcept ;
            static bool_t message( bool_t const condition, log_level const level, motor::string_cref_t msg ) noexcept ;
            static void_t status( motor::string_cref_t msg ) noexcept ;
            static bool_t status( bool_t const condition, motor::string_cref_t msg ) noexcept ;
            static void_t warning( motor::string_cref_t msg ) noexcept ;
            static bool_t warning( bool_t const condition, motor::string_cref_t msg ) noexcept ;
            static void_t error( motor::string_cref_t msg ) noexcept ;
            static void_t error_and_exit( motor::string_cref_t msg ) noexcept ;
            static void_t error_and_exit( bool_t const condition, motor::string_cref_t msg ) noexcept ;
            static bool_t error( bool_t const condition, motor::string_cref_t msg ) noexcept ;
            static void_t critical( motor::string_cref_t msg ) noexcept ;
            static bool_t critical( bool_t const condition, motor::string_cref_t msg ) noexcept ;

        public:

            static void_t message( log_level const level, char const * msg ) noexcept ;
            static bool_t message( bool_t const condition, log_level const level, char const * msg ) noexcept ;
            static void_t status( char const * msg ) noexcept ;
            static bool_t status( bool_t const condition, char const * msg ) noexcept ;
            static void_t warning( char const * msg ) noexcept ;
            static bool_t warning( bool_t const condition, char const * msg ) noexcept ;
            static void_t error( char const * msg ) noexcept ;
            static void_t error_and_exit( char const * msg ) noexcept ;
            static void_t error_and_exit( bool_t const condition, char const * msg ) noexcept ;
            static bool_t error( bool_t const condition, char const * msg ) noexcept ;
            static void_t critical( char const * msg ) noexcept ;
            static bool_t critical( bool_t const condition, char const * msg ) noexcept ;

        public:

            template< size_t sib >
            static void_t status( char const * fmt, ... ) noexcept
            {
                char buffer[sib] ;

                va_list args;
                va_start( args, fmt ) ;
                std::vsnprintf( buffer, sib, fmt, args ) ;
                va_end(args);

                this_t::status( buffer ) ;
            }

            template< size_t sib >
            static void_t warning( char const * fmt, ... ) noexcept
            {
                char buffer[sib] ;

                va_list args;
                va_start( args, fmt ) ;
                std::vsnprintf( buffer, sib, fmt, args ) ;
                va_end(args);

                this_t::warning( buffer ) ;
            }

            template< size_t sib >
            static bool_t warning_if( bool_t const b, char const * fmt, ... ) noexcept
            {
                if( b )
                {
                    char buffer[sib] ;

                    va_list args;
                    va_start( args, fmt ) ;
                    std::vsnprintf( buffer, sib, fmt, args ) ;
                    va_end(args);

                    this_t::warning( buffer ) ;
                }

                return b ;
            }

            template< size_t sib >
            static void_t error( char const * fmt, ... ) noexcept
            {
                char buffer[sib] ;

                va_list args;
                va_start( args, fmt ) ;
                std::vsnprintf( buffer, sib, fmt, args ) ;
                va_end(args);

                this_t::error( buffer ) ;
            }

            template< size_t sib >
            static void_t critical( char const * fmt, ... ) noexcept
            {
                char buffer[sib] ;

                va_list args;
                va_start( args, fmt ) ;
                std::vsnprintf( buffer, sib, fmt, args ) ;
                va_end(args);

                this_t::critical( buffer ) ;
            }

            template< size_t sib >
            static void_t message( log_level const ll, char const * fmt, ... ) noexcept
            {
                char buffer[sib] ;

                va_list args;
                va_start( args, fmt ) ;
                std::vsnprintf( buffer, sib, fmt, args ) ;
                va_end(args);

                this_t::message( ll, buffer ) ;
            }



        };
        motor_typedef( global ) ;
    }
}
