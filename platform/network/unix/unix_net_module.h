
#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/network/imodule.h>

#include <future>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using SOCKET = int ;        
#define INVALID_SOCKET -1

namespace motor { namespace platform { namespace unx
{
    class unix_net_module : public motor::network::imodule
    {
        motor_this_typedefs( unix_net_module ) ;

        

    private:
        
        bool_t _is_init = false ;

    private: // tcp server data

        struct tcp_server_data
        {
            struct client_info
            {
                SOCKET s ;
                motor::network::ipv4::binding_point bp ;
            };

            SOCKET s ;
            std::thread t ;
            motor::vector< client_info > clients ;
            bool_t running = false ;
            uint_t port ; // server port

            motor::network::iserver_handler_mtr_t handler ;
            
        public:

            tcp_server_data( void_t ) noexcept : s( INVALID_SOCKET ){}
            tcp_server_data( SOCKET s_ ) noexcept : s(s_){}
            tcp_server_data( tcp_server_data && rhv ) noexcept : s( rhv.s), t( std::move( rhv.t) ),
                clients( std::move( rhv.clients ) ) 
            { rhv.s = INVALID_SOCKET ; }
            ~tcp_server_data( void_t ) noexcept { assert( s == INVALID_SOCKET ) ; }

            motor::network::client_id_t get_client_id( client_info && ci ) noexcept
            {
                for ( size_t i=0; i<clients.size(); ++i )
                {
                    auto & d = clients[ i ] ;
                    if ( d.s == INVALID_SOCKET )
                    {
                        d.s = ci.s ;
                        d.bp = std::move( ci.bp ) ;
                        return i ;
                    }
                }

                clients.emplace_back( std::move( ci ) ) ;
                return clients.size() - 1  ;
            }

            void_t invalidate_client( motor::network::client_id_t const cid ) noexcept
            {
                clients[ cid ].s = INVALID_SOCKET ;
            }

        };
        motor_typedef( tcp_server_data ) ;

        std::mutex _mtx_tcp_servers ;
        motor::vector< tcp_server_data_ptr_t > _tcp_servers ;

    private: // tcp client data

        struct tcp_client_data
        {
            SOCKET s ;
            std::thread t ;
            motor::string_t name ;
            motor::network::ipv4::binding_point_host bp ;

            bool_t used = true ;

            motor::network::iclient_handler_mtr_t handler ;

            tcp_client_data( void_t ) noexcept : s( INVALID_SOCKET  ){}
            tcp_client_data( tcp_client_data && rhv ) noexcept : s( rhv.s ), handler( motor::move( rhv.handler ) )
            { rhv.s = INVALID_SOCKET ;}

            ~tcp_client_data( void_t ) noexcept { assert( s == INVALID_SOCKET ) ; }

            tcp_client_data & operator = ( tcp_client_data && rhv ) noexcept
            {
                assert( s == INVALID_SOCKET ) ;

                s = rhv.s ;
                rhv.s = INVALID_SOCKET ;
                t = std::move( rhv.t ) ;
                used = rhv.used ;
                rhv.used = false ;
                handler = motor::move( rhv.handler ) ;
                return *this ;
            }
        };
        motor_typedef( tcp_client_data ) ;

        std::mutex _mtx_tcp_clients ;
        motor::vector< tcp_client_data_ptr_t > _tcp_clients ;

    public:
        
        unix_net_module( void_t ) noexcept ;
        unix_net_module( this_cref_t ) = delete ;
        unix_net_module( this_rref_t ) noexcept ;
        ~unix_net_module( void_t ) noexcept ;

    public: // interface

        virtual motor::network::socket_id_t create_tcp_client( 
            motor::network::create_tcp_client_info_rref_t ) noexcept ;

        virtual motor::network::socket_id_t create_tcp_server( 
            motor::network::create_tcp_server_info_rref_t ) noexcept ;

        virtual void_t update( void_t ) noexcept ;

    private:

        size_t create_tcp_client_id( void_t ) noexcept ;
        size_t create_tcp_server_id( SOCKET ) noexcept ;

        SOCKET connect_client( motor::network::ipv4::binding_point_host const & bp ) const noexcept ;

        void_t release_all_tcp( void_t ) noexcept ;
    };
    motor_typedef( unix_net_module ) ;
} } }
