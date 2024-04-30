
#pragma once

#include <motor/memory/global.h>
#include <motor/memory/allocator.hpp>

#include <motor/core/macros/typedef.h>
#include <motor/core/types.hpp>

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

        enum class receive_result
        {
            ok,
            close
        };

        enum class transmit_result
        {
            proceed,
            break_now,
            have_more,
            break_after
        };


        static const size_t send_buffer_sib = 2048 ;

        using recv_funk_t = std::function< receive_result ( byte_cptr_t, size_t const ) > ;
        using send_funk_t = std::function< transmit_result ( byte_ptr_t, size_t &, size_t const ) > ;
        
    }
}


