
#pragma once

#include <motor/std/string>

#include <motor/memory/global.h>
#include <motor/memory/allocator.hpp>

#include <motor/base/macros/typedef.h>
#include <motor/base/types.hpp>

#include <array>

namespace motor
{
    namespace network
    {
        using namespace motor::core::types ;

        using socket_id_t = size_t ;
        using client_id_t = size_t ;

        enum class accept_result
        {
            ok,
            no_accept
        };

        enum class connect_result
        {
            initial,
            established,
            refused,
            closed,
            failed
        };

        static motor::string_t to_string( connect_result const res ) noexcept
        {
            static char const * const __connect_result_strings[] =
            { "initial", "established", "refused", "closed", "failed" } ;

            return __connect_result_strings[ size_t( res ) ] ;
        }

        enum class receive_result
        {
            ok,
            close
        };

        enum class transmit_result
        {
            ok,
            failed,
            connection_reset
        };

        enum class user_decision
        {
            keep_going,
            shutdown
        };

        enum class server_decision
        {
            keep_going,
            shutdown,
            shutdown_client
        };

        static const size_t send_buffer_sib = 2048 ;

        using recv_funk_t = std::function< receive_result ( byte_cptr_t, size_t const ) > ;
        using send_funk_t = std::function< transmit_result ( byte_ptr_t, size_t &, size_t const ) > ;
        
    }
}


