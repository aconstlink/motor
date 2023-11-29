

#pragma once

#include "api.h"
#include "typedefs.h"
#include "parser_structs.hpp"
#include <motor/std/vector>

namespace motor
{
    namespace msl
    {
        class MOTOR_MSL_API parser
        {
            motor_this_typedefs( parser ) ;

        private:

            motor::string_t _name ;

        public:

            motor_typedefs( motor::vector< motor::string_t >, statements ) ;

        public:

            parser( motor::string_cref_t name ) : _name( name ) {}
            ~parser( void_t ) {}

        public:

            motor::msl::post_parse::document_t process( motor::string_cref_t file ) noexcept ;

            // produces a statement list of all statements in the file
            motor::msl::post_parse::document_t process( motor::string_rref_t file ) noexcept ;

        private: // 

            motor::msl::parse::configs_t filter_config_statements( this_t::statements_rref_t ss ) const noexcept ;
            
            motor::msl::post_parse::configs_t analyse_configs( motor::msl::parse::configs_rref_t configs ) const noexcept ;

            motor::msl::parse::libraries_t filter_library_statements( this_t::statements_rref_t ss ) const noexcept ;

            statements_t replace_numbers( statements_rref_t ss ) const ;

            statements_t replace_operators( statements_rref_t ss ) const ;
            statements_t replace_buildins( statements_rref_t ss ) const ;

            motor::msl::post_parse::libraries_t analyse_libraries( motor::msl::parse::libraries_rref_t libs ) const noexcept ;

            statements_t filter_for_group( motor::string_cref_t what, statements_rref_t ss ) const noexcept ;

            // replacing the open/close tags in the statements list makes
            // later processing much easier/less code.
            statements_t replace_open_close( statements_rref_t ss ) const noexcept ;

            statements_t repackage( statements_rref_t ss ) const noexcept ;

        private: //

            bool_t some_first_checks( motor::string_cref_t file ) const noexcept  ;

            bool_t check_open_close( motor::string_cref_t open, motor::string_cref_t close, motor::string_cref_t file ) const noexcept ;

        private: // scan file

            statements_t scan( motor::string_rref_t file ) const noexcept ;

        private: // helper

            // ensures that certain characters have spaces in front or behind.
            // this makes later code analysis much easier and just less character checking.
            motor::string_t insert_spaces( motor::string_rref_t s ) const noexcept ;

            motor::string_t clear_line( motor::string_rref_t s ) const noexcept ;

            // simply removes all comments with
            // // and /**/
            motor::string_t remove_comment_lines( motor::string_rref_t s ) const noexcept ;

            motor::vector< motor::string_t > tokenize( motor::string_cref_t s ) const noexcept ;

            motor::msl::post_parse::used_buildins_t determine_used_buildins( motor::vector< motor::string_t > const & ) const noexcept ;
            
        };
        motor_typedef( parser ) ;

    }
}