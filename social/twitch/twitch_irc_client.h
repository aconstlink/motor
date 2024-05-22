
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "irc_parser.hpp"

#include <motor/network/imodule.h>
#include <motor/io/database.h>
#include <motor/std/vector>

namespace motor { namespace social { namespace twitch {

    using namespace motor::core::types ;

    // the client code flow can not be used because it only grants a app access token,
    // but the irc bot requires a user access token. A user access token requires the
    // user to authorize the bot to access the chat and maybe other scopes.
    enum class code_grant_flow
    {
        none,
        device_code_flow, // no server, limited input(setopbox, gameconsode, engine) [user access token]
        //authorization_code_flow, // server/server [user access token]
        //implicit_code_flow, // no server, client-side apps(java script, modile app) [user access token]
        //client_code_flow // server/server like acf [app access token]
    } ;

    // The response when requesting a device code
    struct device_code_response
    {
        motor::string_t device_code ;
        int_t expires_in = 0 ;
        int_t interval = 0  ;
        motor::string_t user_code ;
        motor::string_t verification_uri ;

        device_code_response( void_t ) noexcept {}

        device_code_response( device_code_response const & rhv ) noexcept
        {
            device_code = ( rhv.device_code ) ;
            expires_in = rhv.expires_in ;
            interval = rhv.interval ;
            user_code = ( rhv.user_code ) ;
            verification_uri = ( rhv.verification_uri ) ;
        }

        device_code_response & operator = ( device_code_response  const & rhv ) noexcept
        {
            device_code = ( rhv.device_code ) ;
            expires_in = rhv.expires_in ;
            interval = rhv.interval ;
            user_code = ( rhv.user_code ) ;
            verification_uri = ( rhv.verification_uri ) ;

            return *this ;
        }

        device_code_response( device_code_response && rhv ) noexcept
        {
            device_code = std::move( rhv.device_code ) ;
            expires_in = rhv.expires_in ;
            interval = rhv.interval ;
            user_code = std::move( rhv.user_code ) ;
            verification_uri = std::move( rhv.verification_uri ) ;
        }

        device_code_response & operator = ( device_code_response && rhv ) noexcept
        {
            device_code = std::move( rhv.device_code ) ;
            expires_in = rhv.expires_in ;
            interval = rhv.interval ;
            user_code = std::move( rhv.user_code ) ;
            verification_uri = std::move( rhv.verification_uri ) ;

            return *this ;
        }
    } ;
    motor_typedef( device_code_response ) ;

    enum class program_state
    {
        load_credentials,
        need_device_code,
        need_token,
        need_refresh,
        need_token_validation,
        pending,
        need_login,
        login_pending,
        bot_is_online
    };

    enum class initial_process_result
    {
        invalid,
        ok,
        curl_failed,
        // there are no credentials or no client_id
        credentials_failed,
        // credentials has no token
        no_token,
        // twitch responded the token to be invalid
        invalid_token
    };

    enum class refresh_process_result
    {
        invalid,
        invalid_client_id,
        invalid_token,
        ok,
        curl_failed,

    };

    enum class request_user_token_result
    {
        invalid,
        curl_failed,
        invalid_device_code,
        request_failed,
        pending,
        ok
    };

    enum class validation_process_result
    {
        invalid,
        ok,
        invalid_access_token,
        curl_failed,
    };

    enum class device_code_process_result
    {
        curl_failed,
        device_code_request_failed,
        ok
    };

    class MOTOR_SOCIAL_API twitch_irc_bot : public motor::network::iclient_handler
    {
        motor_this_typedefs( twitch_irc_bot ) ;

    private:

        motor::string_t data_out ;
        motor::string_t data_in ;

        bool_t _require_pong = false ;
        motor::string_t _pong = "" ;

        motor::social::twitch::irc_parser_t parser ;


        motor::social::twitch::program_state _ps =
            motor::social::twitch::program_state::load_credentials ;

        using clk_t = std::chrono::high_resolution_clock ;
        clk_t::time_point _login_tp ;

        // this is when update needs to wait for something
        std::chrono::seconds _timeout = std::chrono::seconds( 0 ) ;

        motor::io::database_mtr_t _db ;

        struct login_data
        {
            motor::string_t channel_name ;
            motor::string_t nick_name ;
            motor::string_t user_token ;
            motor::string_t refresh_token ;
            motor::string_t client_id ;
            motor::string_t client_secret ;
            motor::string_t scopes ;
            motor::string_t device_code ;
        };
        motor_typedef( login_data ) ;

        login_data_t _login_data ;

        device_code_response _dcr ;

        code_grant_flow _mode = motor::social::twitch::code_grant_flow::device_code_flow ;

    public: // commands

        struct command
        {
            motor::string_t user ;
            motor::string_t name ;
            motor::string_t params[4] ;
        };

        motor_typedefs( motor::vector< command >, commands ) ;

        motor_typedefs( motor::vector< motor::string_t >, out_messages ) ;

    private:

        std::mutex _mtx_commands ;
        commands_t _commands ;

        std::mutex _mtx_outs ;
        out_messages_t _outs ;

    private:

        // validates the user token via twitch validate endpoint
        validation_process_result validate_token( login_data_cref_t ld ) const noexcept ;
        
        // Refresh the user token using the refresh token.
        motor::social::twitch::refresh_process_result refresh_token( login_data_ref_t ld ) const noexcept ;

        // requests a device code for the "Device Code Flow". It is required in order to 
        // request the user/refresh token
        motor::social::twitch::device_code_process_result request_device_code( login_data_inout_t ld, device_code_response & rt ) const noexcept ;

        // requests the user token. Requires a device_code.
        motor::social::twitch::request_user_token_result request_user_token(
            this_t::login_data_inout_t ld ) const noexcept ;

        // load credentials from file and do validation.
        motor::social::twitch::initial_process_result load_credentials_from_file( void_t ) noexcept ;

        // load credentials from the default location
        bool_t load_credentials( this_t::login_data_out_t ld ) const noexcept ;

        // simply credentials validation after loading it from the file.
        bool_t validate_credentials( this_t::login_data_out_t ld ) const noexcept
        {
            return !ld.client_id.empty() ;
        }

        // write credentials to default location on disk.
        void_t write_credentials( this_t::login_data_in_t ld ) const noexcept ;

        // do the update loop. Drives the state-machine.
        void_t update( void_t ) noexcept ;

    public: // user send/recv section

        bool_t swap_commands( this_t::commands_inout_t ) noexcept ;
        void_t send_response( motor::string_in_t ) noexcept ;
        void_t send_response( motor::string_rref_t ) noexcept ;

    public: // network interface

        //**********************************************************************************
        twitch_irc_bot( motor::io::database_mtr_safe_t db ) noexcept ;
        twitch_irc_bot( this_cref_t ) = delete ;
        twitch_irc_bot( this_rref_t ) noexcept ;

        //**********************************************************************************
        virtual ~twitch_irc_bot( void_t ) noexcept ;

        //**********************************************************************************
        virtual motor::network::user_decision on_connect( 
            motor::network::connect_result const res, size_t const tried ) noexcept ;

        //**********************************************************************************
        virtual motor::network::user_decision on_sync( void_t ) noexcept ;

        //**********************************************************************************
        virtual motor::network::user_decision on_update( void_t ) noexcept ;

        //**********************************************************************************
        virtual void_t on_receive( byte_cptr_t buffer, size_t const sib ) noexcept ;

        //**********************************************************************************
        virtual void_t on_received( void_t ) noexcept ;

        //**********************************************************************************
        virtual void_t on_send( byte_cptr_t & buffer, size_t & num_sib ) noexcept ;

        //**********************************************************************************
        virtual void_t on_sent( motor::network::transmit_result const ) noexcept ;
    };
} } }