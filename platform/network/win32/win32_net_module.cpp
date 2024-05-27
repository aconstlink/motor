
#include "win32_net_module.h"
#include <motor/network/convert.hpp>

#include <motor/log/global.h>

#include <array>

using namespace motor::platform ;
using namespace motor::platform::win32 ;

//*****************************************************************
win32_net_module::win32_net_module( void_t ) noexcept
{
    WSAData data ;
    auto const res = WSAStartup( MAKEWORD( 2, 2 ), &data ) ;
    if ( res != 0 )
    {
        motor::log::global::error( "[win32_net_module::initialize] : WSAStartup" ) ;
    }
    else _is_init = true ;
}

//*****************************************************************
win32_net_module::win32_net_module( this_rref_t rhv ) noexcept
{
    _tcp_clients = std::move( rhv._tcp_clients ) ;
    _is_init = rhv._is_init ;
    rhv._is_init = false ;
}

//*****************************************************************
win32_net_module::~win32_net_module( void_t ) noexcept
{
    this_t::release_all_tcp() ;
    if ( _is_init ) WSACleanup() ;
}

//*****************************************************************
motor::network::socket_id_t win32_net_module::create_tcp_server(
    motor::network::create_tcp_server_info_rref_t info_in ) noexcept
{
    if ( info_in.handler == nullptr )
    {
        motor::log::global_t::error("[win32_net_module] : server handler required") ;
        return motor::network::socket_id_t( -1 ) ;
    }

    motor::mtr_release_guard<motor::network::iserver_handler_t> handler( std::move( info_in.handler ) ) ;

    addrinfo * result = NULL;
    addrinfo hints;

    ZeroMemory( &hints, sizeof( hints ) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    {
        // Resolve the server address and port
        auto const res = getaddrinfo( NULL, std::to_string( info_in.bp.port ).c_str(), &hints, &result );
        if ( res != 0 )
        {
            motor::log::global::error( "[win32_net_module::create_tcp_server] : getaddrinfo" ) ;
            motor::log::global::error( "[win32_net_module::create_tcp_server] : WSAGetLastError " +
                motor::to_string( WSAGetLastError() ) ) ;
            return motor::network::socket_id_t( -1 ) ;
        }
    }

    SOCKET s = INVALID_SOCKET ;

    // Create server socket
    {
        s = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
        if ( s == INVALID_SOCKET )
        {
            motor::log::global::error( "[win32_net_module::create_tcp_server] : socket" ) ;
            motor::log::global::error( "[win32_net_module::create_tcp_server] : WSAGetLastError " +
                motor::to_string( WSAGetLastError() ) ) ;
            return motor::network::socket_id_t( -1 ) ;
        }
    }

    // bind to endpoint
    {
        auto const res = bind( s, result->ai_addr, (int) result->ai_addrlen );
        if ( res == SOCKET_ERROR )
        {
            motor::log::global::error( "[win32_net_module::create_tcp_server] : socket" ) ;
            motor::log::global::error( "[win32_net_module::create_tcp_server] : WSAGetLastError " +
                motor::to_string( WSAGetLastError() ) ) ;
            return motor::network::socket_id_t( -1 ) ;
        }
    }

    // start listening for incoming connections
    {
        auto const res = listen( s, SOMAXCONN );
        if ( res == SOCKET_ERROR )
        {
            motor::log::global::error( "[win32_net_module::create_tcp_server] : listen" ) ;
            motor::log::global::error( "[win32_net_module::create_tcp_server] : WSAGetLastError " +
                motor::to_string( WSAGetLastError() ) ) ;
            return motor::network::socket_id_t( -1 ) ;
        }
    }

    // do non-blocking mode
    {
        u_long mode = 1;  // 1 to enable non-blocking socket
        auto const res = ioctlsocket( s, FIONBIO, &mode );
        if ( res != 0 )
        {
            motor::log::global::error( "[win32_net_module::create_tcp_server] : ioctlsocket" ) ;
            motor::log::global::error( "[win32_net_module::create_tcp_server] : WSAGetLastError " +
                motor::to_string( WSAGetLastError() ) ) ;
        }
    }

    freeaddrinfo( result );

    motor::network::socket_id_t const sid = this_t::create_tcp_server_id( s ) ;

    // thread for accepting, handshaking and recv/send
    {
        this_t::tcp_server_data_ptr_t tcpd = _tcp_servers[ sid ] ;
        tcpd->port = info_in.bp.port ;
        tcpd->handler = handler.move() ;

        tcpd->t = std::thread( [=] ( void_t )
        {
            motor::string_t const log_start = "[TCP server " + info_in.name + "] : " ;
            motor::log::global::status( log_start + "online and accepting incoming connections at " + motor::to_string(info_in.bp.port) ) ;

            tcpd->running = true ;

            while ( tcpd->running )
            {
                sockaddr_in addr ;
                int_t addr_sib = sizeof( sockaddr_in ) ;

                SOCKET s = accept( tcpd->s, (sockaddr *) &addr, &addr_sib );
                if ( s != INVALID_SOCKET )
                {
                    motor::network::ipv4::binding_point const bp = {
                        uint_t( ntohs( addr.sin_port ) ),
                        motor::network::ipv4::address {
                            addr.sin_addr.S_un.S_un_b.s_b1,
                            addr.sin_addr.S_un.S_un_b.s_b2,
                            addr.sin_addr.S_un.S_un_b.s_b3,
                            addr.sin_addr.S_un.S_un_b.s_b4 }
                    } ;
                    motor::log::global::status( log_start + "client connected via " + bp.to_string() ) ;
                    
                    auto const cid = tcpd->get_client_id( this_t::tcp_server_data_t::client_info { s, bp } ) ;
                    auto const ares = tcpd->handler->on_accept( cid, bp.address ) ;
                    if ( ares == motor::network::accept_result::no_accept )
                    {
                        motor::log::global::status( log_start + "client rejected via " + bp.to_string() ) ;
                        tcpd->invalidate_client( cid ) ;
                    }
                }
                else if ( s == INVALID_SOCKET )
                {
                    auto const res = WSAGetLastError() ;
                    if ( res == WSAEWOULDBLOCK )
                    {}
                    else
                    {
                        motor::log::global::error( "[win32_net_module::server_thread] : accept" ) ;
                        motor::log::global::error( "[win32_net_module::server_thread] : WSAGetLastError " +
                            motor::to_string(res) ) ;
                    }
                }

                {
                    // handle clients
                    size_t idx = 0 ;
                    for ( auto & ci : tcpd->clients )
                    {
                        // send
                        {
                            byte_cptr_t buffer = nullptr ;
                            size_t sib = 0 ;

                            // send as long as there is data to be sent.
                            while ( true )
                            {
                                auto const res = tcpd->handler->on_send( idx, buffer, sib ) ;

                                if ( sib == 0 ) break ;
                                
                                // do the actual send operation
                                {
                                    auto const has_sent = send( ci.s, (const char *) buffer, int( sib ), 0 ) ;
                                    if ( has_sent == SOCKET_ERROR )
                                    {
                                        motor::log::global::error( "[win32_net_module::server_thread] : send" ) ;
                                        motor::log::global::error( "[win32_net_module::server_thread] : WSAGetLastError " +
                                            motor::to_string( has_sent ) ) ;
                                    }
                                    if ( has_sent != sib )
                                    {
                                        // need to fix here. need to call on_send multiple times
                                        motor::log::global_t::error( "[win32 tcp_server] : has_sent != sib" ) ;
                                    }
                                }

                                if ( res == motor::network::transmit_result::ok ) break ;
                            }
                        }
                            
                        // receive
                        {
                            size_t const buflen = 2048 ;
                            char_t buffer[ buflen ] ;

                            sockaddr in_addr ;
                            int len = 0 ;

                            auto const received = recvfrom( ci.s, buffer, buflen, 0, &in_addr, &len );

                            // closed
                            if ( received == 0 )
                            {
                                tcpd->handler->on_close( idx ) ;
                                break ;
                            }

                            // timeout and others
                            if ( received == -1 )
                            {
                                //std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) ) ;
                                continue ;
                            }

                            auto const res = tcpd->handler->on_receive( idx, (byte_cptr_t) buffer, received ) ;
                            if ( res == motor::network::receive_result::close ) break  ;
                        }

                        ++idx ;
                    }
                }


                std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) ) ;
            }
        } ) ;
    }

    return sid ;
}

//*****************************************************************
SOCKET win32_net_module::connect_client( motor::network::ipv4::binding_point_host const & bp ) const noexcept
{
    SOCKET s = INVALID_SOCKET ;
    addrinfo * result = NULL ;
    addrinfo * ptr = NULL ;
    addrinfo hints ;

    {
        ZeroMemory( &hints, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }

    // 1. Resolve the server address and port 
    // 2. try to connect
    {
        auto const res = getaddrinfo( bp.host.c_str(),
            bp.service.c_str(), &hints, &result );

        if ( res != 0 )
        {
            motor::log::global::error( "[win32_net_module::connect_client] : getaddrinfo" ) ;
            motor::log::global::error( "[win32_net_module::connect_client] : WSAGetLastError " +
                motor::to_string( WSAGetLastError() ) ) ;
            return motor::network::socket_id_t( -1 ) ;
        }

        for ( ptr = result; ptr != NULL; ptr = ptr->ai_next )
        {
            s = socket( ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol ) ;
            if ( s == INVALID_SOCKET )
            {
                motor::log::global::error( "[win32_net_module::connect_client] : socket" ) ;
                motor::log::global::error( "[win32_net_module::connect_client] : WSAGetLastError " +
                    motor::to_string( WSAGetLastError() ) ) ;
                return motor::network::socket_id_t( -1 ) ;
            }

            // Connect to server.
            auto const con_res = connect( s, ptr->ai_addr, (int) ptr->ai_addrlen );
            if ( con_res == SOCKET_ERROR )
            {
                closesocket( s );
                s = INVALID_SOCKET;
                continue;
            }
            break;
        }
    }

    freeaddrinfo( result );

    if ( s == INVALID_SOCKET )
    {
        motor::log::global::error( "[win32_net_module::connect_client] : connect" ) ;
        motor::log::global::error( "[win32_net_module::connect_client] : WSAGetLastError " +
            motor::to_string( WSAGetLastError() ) ) ;

        return motor::network::socket_id_t( -1 ) ;
    }

    // may not be need. all sockets are non-blocking
    {
        DWORD timeout_ms = 1000 ;
        auto const res = setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, (char_cptr_t) &timeout_ms, sizeof DWORD );
        motor::log::global_t::error( res != 0, "[win32_net_module::connect_client] : setsockopt SO_RCVTIMEO" ) ;
    }

    {
        uint_t max_packet_size ;
        int len = sizeof( uint_t ) ;
        auto res = getsockopt( s, SOL_SOCKET, SO_MAX_MSG_SIZE, (char *) ( &max_packet_size ), &len )  ;
        motor::log::global_t::error( res != 0, "[win32_net_module::connect_client] : getsockopt SO_MAX_MSG_SIZE" ) ;
    }

    // do non-blocking mode
    {
        u_long mode = 1;  // 1 to enable non-blocking socket
        auto const res = ioctlsocket( s, FIONBIO, &mode );
        if ( res != 0 )
        {
            motor::log::global::error( "[win32_net_module::connect_client] : ioctlsocket" ) ;
            motor::log::global::error( "[win32_net_module::connect_client] : WSAGetLastError " +
                motor::to_string( WSAGetLastError() ) ) ;
        }
    }

    return s ;
}

//*****************************************************************
motor::network::socket_id_t win32_net_module::create_tcp_client(
    motor::network::create_tcp_client_info_rref_t info_in ) noexcept
{
    if ( info_in.handler == nullptr )
    {
        motor::log::global_t::error( "[win32_net_module] : client handler required" ) ;
        return motor::network::socket_id_t( -1 ) ;
    }

    motor::network::socket_id_t const sid = this_t::create_tcp_client_id() ;

    // start client thread
    {
        this_t::tcp_client_data_ptr_t tcpd = _tcp_clients[ sid ] ;
        tcpd->handler = std::move( info_in.handler ) ;
        tcpd->name = std::move( info_in.name ) ;
        tcpd->bp = std::move( info_in.bp ) ;

        tcpd->t = std::thread( [=] ( void )
        {
            motor::log::global_t::status( "[win32_net_module] : starting client thread : " + tcpd->name ) ;

            motor::network::connect_result connection_state = motor::network::connect_result::initial ;

            size_t connection_tryouts = 0 ;

            while ( true )
            {
                // try connect loop
                {
                    if ( tcpd->s == INVALID_SOCKET )
                    {
                        tcpd->s = this_t::connect_client( tcpd->bp ) ;

                        if ( tcpd->s == INVALID_SOCKET )
                            connection_state = motor::network::connect_result::failed ;
                        else
                            connection_state = motor::network::connect_result::established ;

                        auto const what_to_do = tcpd->handler->on_connect( connection_state, ++connection_tryouts ) ;
                        if ( what_to_do == motor::network::user_decision::shutdown )
                            break ;
                    }
                    
                    connection_tryouts = 0 ;
                }

                {
                    auto const res = tcpd->handler->on_update() ;
                    if ( res == motor::network::user_decision::shutdown ) break ;
                }
                
                // send
                {
                    byte_cptr_t buffer = nullptr ;
                    size_t sib = 0 ;

                    auto tres = motor::network::transmit_result::ok ;

                    tcpd->handler->on_send( buffer, sib ) ;

                    if ( sib != 0 )
                    {
                        auto const has_sent = send( tcpd->s, (const char *) buffer, (int) sib, 0 ) ;
                        if ( has_sent == SOCKET_ERROR )
                        {
                            auto const lerr = WSAGetLastError() ;
                            motor::log::global::error( "[win32_net_module::create_tcp_server] : send" ) ;
                            motor::log::global::error( "[win32_net_module::create_tcp_server] : WSAGetLastError " +
                                motor::to_string( lerr ) ) ;

                            if( lerr == WSAECONNRESET )
                            {
                                tres = motor::network::transmit_result::connection_reset ;
                            }
                            else 
                                tres = motor::network::transmit_result::failed ;
                            
                        }
                        else if ( has_sent != sib )
                        {
                            motor::log::global_t::status( "[win32_net_module::send] : has_sent != num_bytes" ) ;
                            tres = motor::network::transmit_result::failed ;
                        }
                    }
                    tcpd->handler->on_sent( tres ) ;
                }

                // receive loop
                {
                    size_t const buflen = 2048 ;
                    char_t buffer[ buflen ] ;

                    bool_t something_received = false ;

                    while( true )
                    {
                        auto const received = recvfrom( tcpd->s, buffer, buflen, 0, NULL, NULL );

                        // closed
                        if ( received == 0 )
                        {
                            tcpd->handler->on_connect( motor::network::connect_result::closed, 0 ) ;
                            closesocket( tcpd->s ) ;
                            tcpd->s = INVALID_SOCKET ;
                            break ;
                        }

                        // timeout and others
                        else if ( received == -1 ) break ;

                        something_received = true ;
                        tcpd->handler->on_receive( (byte_cptr_t) buffer, received ) ;

                        if( received < buflen ) break ;
                    }

                    if( tcpd->s == INVALID_SOCKET ) continue ;

                    if( something_received ) tcpd->handler->on_received() ;
                }

                #if 0 // maybe use for external sync
                {
                    auto const res = tcpd->handler->on_sync() ;
                    if( res == motor::network::user_decision::shutdown ) break ;
                }
                #endif
            }

            if( tcpd->s != INVALID_SOCKET )
            {
                closesocket( tcpd->s ) ;
                tcpd->handler->on_connect( motor::network::connect_result::closed, 0 ) ;
                motor::release( motor::move( tcpd->handler ) ) ;
                tcpd->used = false ;
            }
        } ) ;
    }

    return sid ;
}

//*****************************************************************
void_t win32_net_module::update( void_t ) noexcept
{

}

//*****************************************************************
size_t win32_net_module::create_tcp_client_id( void_t ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_tcp_clients ) ;
    
    for ( size_t i=0; i< _tcp_clients.size(); ++i )
    {
        if ( !_tcp_clients[i]->used )
        {
            auto & tcpd = *_tcp_clients[ i ] ;

            tcpd.used = true ;
            
            return i ;
        }
    }

    size_t const id = _tcp_clients.size() ;
    _tcp_clients.resize( _tcp_clients.size() + 1 ) ;
    _tcp_clients[ id ] = motor::memory::global_t::alloc( tcp_client_data(),
        "[win32_net_module] : tcp data" ) ;

    return id ;
}

//*****************************************************************
size_t win32_net_module::create_tcp_server_id( SOCKET s ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_tcp_servers ) ;

    for ( size_t i = 0; i < _tcp_servers.size(); ++i )
    {
        if ( _tcp_servers[ i ]->s == INVALID_SOCKET )
        {
            auto & tcpd = *_tcp_servers[ i ] ;

            tcpd.s = s ;

            return i ;
        }
    }

    size_t const id = _tcp_servers.size() ;
    _tcp_servers.resize( _tcp_servers.size() + 1 ) ;
    _tcp_servers[ id ] = motor::memory::global_t::alloc( tcp_server_data( s ),
        "[win32_net_module] : tcp data" ) ;

    return id ;
}

//*****************************************************************
void_t win32_net_module::release_all_tcp( void_t ) noexcept
{
    for ( auto * ptr : _tcp_clients )
    {

    }
}