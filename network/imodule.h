
#pragma once

#include "api.h"
#include "typedefs.h"

#include "ipv4_address.hpp"

namespace motor{ namespace network 
{
    class iserver_handler
    {
    public:

        virtual motor::network::accept_result on_accept( motor::network::client_id_t const,
            motor::network::ipv4::address_cref_t ) noexcept = 0 ;

        virtual void_t on_close( motor::network::client_id_t const ) noexcept = 0 ;

        virtual motor::network::receive_result on_receive( 
            motor::network::client_id_t const, byte_cptr_t, size_t const ) noexcept = 0 ;

        virtual motor::network::transmit_result on_send(
            motor::network::client_id_t const, byte_cptr_t & buffer, size_t & num_sib ) noexcept = 0 ;

        virtual motor::network::server_decision on_update( motor::network::client_id_t const ) noexcept = 0 ;
    };
    motor_typedef( iserver_handler ) ;

    struct create_tcp_server_info
    {
        motor::string_t name ;
        motor::network::ipv4::binding_point_t bp ;
        iserver_handler_mtr_safe_t handler ;
    };
    motor_typedef( create_tcp_server_info )  ;

    class MOTOR_NETWORK_API iclient_handler
    {

    public:

        virtual ~iclient_handler( void_t ) noexcept  {}

    public:

        virtual motor::network::user_decision on_connect( motor::network::connect_result const, size_t const tries ) noexcept = 0 ;
        virtual motor::network::user_decision on_sync( void_t ) noexcept = 0 ;
        virtual motor::network::user_decision on_update( void_t ) noexcept = 0 ;

        virtual void_t on_receive( byte_cptr_t, size_t const ) noexcept = 0 ;
        virtual void_t on_received( void_t ) noexcept = 0 ;

        virtual void_t on_send( byte_cptr_t & buffer, size_t & num_sib ) noexcept = 0 ;
        virtual void_t on_sent( motor::network::transmit_result const ) noexcept = 0 ;
    };
    motor_typedef( iclient_handler ) ;

    struct create_tcp_client_info
    {
        motor::string_t name ;
        motor::network::ipv4::binding_point_host_t bp ;
        iclient_handler_mtr_safe_t handler ;
    };
    motor_typedef( create_tcp_client_info )  ;

    class MOTOR_NETWORK_API imodule
    {
    public:

        virtual ~imodule( void_t ) noexcept {}

    public:

        virtual socket_id_t create_tcp_client( motor::network::create_tcp_client_info_rref_t ) noexcept = 0 ;
        virtual socket_id_t create_tcp_server( motor::network::create_tcp_server_info_rref_t ) noexcept = 0 ;

        virtual void_t update( void_t ) noexcept = 0 ;
    };
    motor_typedef( imodule ) ;
} }
