
#include "twitch_irc_client.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

using namespace motor::social::twitch ;

namespace this_file
{
    using namespace motor::core::types ;
    bool_t check_for_status( nlohmann::json const & data )
    {
        if ( data.contains( "status" ) )
        {
            size_t const code = data[ "status" ] ;
            motor::string_t msg ;
            if ( data.contains( "message" ) )
            {
                msg = data[ "message" ] ;
            }
            motor::log::global_t::status( "Twitch Response Contains Status Code: " ) ;
            motor::log::global_t::status( "[" + motor::to_string( code ) + "] : " + msg ) ;

            return true ;
        }
        return false ;
    }
}

//*************************************************************
twitch_irc_bot::twitch_irc_bot( motor::io::database_mtr_safe_t db ) noexcept :
    _db( motor::move( db ) )
{
    assert( _db != nullptr ) ;

    // test if curl is present in the system
    {
        auto const sys_res = std::system( "curl --version -s " ) ;
        if ( sys_res != 0 )
        {
            motor::log::global::error( "curl required. Please install curl." ) ;
            exit( 1 ) ;
        }
    }

    this_t::update() ;
}

//*************************************************************
twitch_irc_bot::twitch_irc_bot( this_rref_t rhv ) noexcept
{
    _db = motor::move( rhv._db  ) ;
    _login_data = std::move( rhv._login_data ) ;
    _dcr = std::move( rhv._dcr ) ;
    _commands = std::move( rhv._commands ) ;
    _outs = std::move( rhv._outs  ) ;
    _ps = rhv._ps ;
}

//*************************************************************
twitch_irc_bot::~twitch_irc_bot( void_t ) noexcept
{
}

//*************************************************************
bool_t twitch_irc_bot::send_curl( motor::string_in_t curl, motor::string_in_t file_name, 
    motor::string_out_t response, bool_t const clear ) const noexcept
{
    // perform curl command
    {
        auto const com = curl + " -s -o " + file_name ;
        auto const sys_res = std::system( com.c_str() ) ;

        if ( sys_res != 0 )
        {
            return false ;
        }
    }

    // read content and load response
    {
        std::ifstream myfile( file_name.c_str() ) ;
        std::string line;
        if ( myfile.is_open() )
        {
            while ( std::getline ( myfile, line ) )
            {
                response += line ;
            }
            myfile.close();
        }
    }

    // clear content
    if( clear )
    {
        std::ofstream ofs;
        ofs.open( file_name.c_str(), std::ofstream::out | std::ofstream::trunc );
        ofs.close();
    }

    return true ;
}

//**********************************************************************************
validation_process_result twitch_irc_bot::validate_token( login_data_cref_t ld ) const noexcept
{
    motor::log::global_t::status( "Validating User Token" ) ;

    motor::string_t response ;

    #if 1
    auto const curl_validate_com =
        "curl -X GET https://id.twitch.tv/oauth2/validate "
        "-H \"Authorization: OAuth " + ld.user_token + "\" " ;
    if( !this_t::send_curl( curl_validate_com, "validate", response, true ) )
    {
        return motor::social::twitch::validation_process_result::curl_failed ;
    }

    #else
    auto const curl_validate_com =
        "curl -X GET https://id.twitch.tv/oauth2/validate "
        "-H \"Authorization: OAuth " + ld.user_token + "\" "
        "-s -o validate" ;

    auto const sys_res = std::system( curl_validate_com.c_str() ) ;

    if ( sys_res != 0 )
    {
        return motor::social::twitch::validation_process_result::curl_failed ;
    }
    // read content
    {
        std::ifstream myfile( "validate" );
        std::string line;
        if ( myfile.is_open() )
        {
            while ( std::getline ( myfile, line ) )
            {
                response += line ;
            }
            myfile.close();
        }
    }

    // clear content
    {
        std::ofstream ofs ;
        ofs.open( "validate", std::ofstream::out | std::ofstream::trunc );
        ofs.close();
    }

    #endif

    // investigate response
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;

        #if 1
        if( this_file::check_for_status( data ) )
        {
            return motor::social::twitch::validation_process_result::invalid_access_token ;
        }
        #else
        if ( data.contains( "status" ) && data.contains( "message" ) )
        {
            size_t const code = data[ "status" ] ;
            motor::string_t const msg = data[ "message" ] ;

            motor::log::global_t::status( "Twitch IRC Bot token validation failed: " ) ;
            motor::log::global_t::status( "[" + motor::to_string( code ) + "] : " + msg ) ;
            return motor::social::twitch::validation_process_result::invalid_access_token ;
        }
        #endif
    }

    return motor::social::twitch::validation_process_result::ok ;
}

//**********************************************************************************
motor::social::twitch::refresh_process_result twitch_irc_bot::refresh_token( login_data_ref_t ld ) const noexcept
{
    motor::log::global_t::status( "Refreshing User Token" ) ;

    motor::string_t response ;

    #if 1
    {
        auto const curl_com =
            "curl -X POST https://id.twitch.tv/oauth2/token "
            "-H \"Content-Type: application/x-www-form-urlencoded\" "
            "-d \"grant_type=refresh_token&refresh_token=" + ld.refresh_token +
            "&client_id=" + ld.client_id +
            "&client_secret=" + ld.client_secret + "\" " ;

        if ( !this_t::send_curl( curl_com, "refresh_token", response, true ) )
        {
            return motor::social::twitch::refresh_process_result::curl_failed ;
        }
    }

    #else
    {
        auto const curl_refresh_com =
            "curl -X POST https://id.twitch.tv/oauth2/token "
            "-H \"Content-Type: application/x-www-form-urlencoded\" "
            "-d \"grant_type=refresh_token&refresh_token=" + ld.refresh_token +
            "&client_id=" + ld.client_id +
            #if 1
            "&client_secret=" + ld.client_secret + "\" "
            #else
            "\" "
            #endif
            "-s -o refresh_token";

        auto const sys_res = std::system( curl_refresh_com.c_str() ) ;

        if ( sys_res != 0 )
        {
            return motor::social::twitch::refresh_process_result::curl_failed ;
        }

        // read content
        {
            std::ifstream myfile( "refresh_token" ) ;
            std::string line;
            if ( myfile.is_open() )
            {
                while ( std::getline ( myfile, line ) )
                {
                    response += line ;
                }
                myfile.close();
            }
        }

        // clear content
        {
            std::ofstream ofs;
            ofs.open( "refresh_token", std::ofstream::out | std::ofstream::trunc );
            ofs.close();
        }
    }
    #endif

    // json validate
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;
        #if 1
        if( this_file::check_for_status(data) )
        {
            return motor::social::twitch::refresh_process_result::invalid_token ;
        }
        #else
        if ( data.contains( "status" ) )
        {
            size_t const code = data[ "status" ] ;
            motor::string_t msg ;
            if ( data.contains( "message" ) )
            {
                msg = data[ "message" ] ;
            }
            motor::log::global_t::status( "Twitch IRC Bot token refresh failed: " ) ;
            motor::log::global_t::status( "[" + motor::to_string( code ) + "] : " + msg ) ;

            //return motor::social::twitch::refresh_process_result::invalid_client_id ;
            return motor::social::twitch::refresh_process_result::invalid_token ;
        }
        #endif

        if ( data.contains( "access_token" ) )
        {
            ld.user_token = data[ "access_token" ] ;
        }

        if ( data.contains( "refresh_token" ) )
        {
            ld.refresh_token = data[ "refresh_token" ] ;
        }
    }

    return motor::social::twitch::refresh_process_result::ok ;
}

//**********************************************************************************
motor::social::twitch::device_code_process_result twitch_irc_bot::request_device_code( 
    login_data_inout_t ld, device_code_response & rt ) const noexcept
{
    motor::log::global_t::status( "Requesting Device Code" ) ;

    #if 1
    motor::string_t response ;
    {
        auto const curl_com =
            "curl --location https://id.twitch.tv/oauth2/device "
            "--form \"client_id=" + ld.client_id + "\" "
            "--form \"scopes=" + ld.scopes + "\" " ;

        if ( !this_t::send_curl( curl_com, "data_code_flow_request", response, true ) )
        {
            return motor::social::twitch::device_code_process_result::curl_failed ;
        }
    }

    #else
    auto const curl_request_com =
        "curl --location https://id.twitch.tv/oauth2/device "
        "--form \"client_id=" + ld.client_id + "\" "
        "--form \"scopes=" + ld.scopes + "\" "
        "-s -o data_code_flow_request" ;

    auto const sys_res = std::system( curl_request_com.c_str() ) ;
    if ( sys_res != 0 )
    {
        return motor::social::twitch::device_code_process_result::curl_failed ;
    }

    std::string response ;

    // read content
    {
        std::ifstream myfile( "data_code_flow_request" ) ;
        std::string line;
        if ( myfile.is_open() )
        {
            while ( std::getline ( myfile, line ) )
            {
                response += line ;
            }
            myfile.close();
        }
    }

    // clear content
    {
        std::ofstream ofs;
        ofs.open( "data_code_flow_request", std::ofstream::out | std::ofstream::trunc );
        ofs.close();
    }
    #endif

    // json 
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;

        #if 1
        if( this_file::check_for_status(data ) )
        {
            return motor::social::twitch::device_code_process_result::device_code_request_failed ;
        }
        #else
        if ( data.contains( "status" ) )
        {
            size_t const code = data[ "status" ] ;
            motor::string_t msg ;
            if ( data.contains( "message" ) )
            {
                msg = data[ "message" ] ;
            }
            motor::log::global_t::error( "Twitch IRC Bot device code request failed: " ) ;
            motor::log::global_t::error( "[" + motor::to_string( code ) + "] : " + msg ) ;

            return motor::social::twitch::device_code_process_result::device_code_request_failed ;
        }
        #endif

        if ( data.contains( "device_code" ) )
        {
            rt.device_code = data[ "device_code" ] ;
            ld.device_code = data[ "device_code" ] ;
        }
        if ( data.contains( "expires_in" ) )
        {
            rt.expires_in = data[ "expires_in" ] ;
        }
        if ( data.contains( "interval" ) )
        {
            rt.interval = data[ "interval" ] ;
        }
        if ( data.contains( "user_code" ) )
        {
            rt.user_code = data[ "user_code" ] ;
        }
        if ( data.contains( "verification_uri" ) )
        {
            rt.verification_uri = data[ "verification_uri" ] ;
        }
    }
    return motor::social::twitch::device_code_process_result::ok ;
}

//**********************************************************************************
motor::social::twitch::request_user_token_result twitch_irc_bot::request_user_token(
    this_t::login_data_inout_t ld ) const noexcept
{
    motor::log::global_t::status( "Requesting User Token" ) ;

    #if 1 
    motor::string_t response ;
    {
        auto const curl_com =
            "curl --location https://id.twitch.tv/oauth2/token "
            "--form \"client_id=" + ld.client_id + "\" "
            "--form \"scopes=" + ld.scopes + "\" "
            "--form \"device_code=" + ld.device_code + "\" "
            "--form \"grant_type=urn:ietf:params:oauth:grant-type:device_code\" " ;

        if ( !this_t::send_curl( curl_com, "user_access_token", response, true ) )
        {
            return motor::social::twitch::request_user_token_result::curl_failed ;
        }
    }
    #else
    auto const curl_request_com =
        "curl --location https://id.twitch.tv/oauth2/token "
        "--form \"client_id=" + ld.client_id + "\" "
        "--form \"scopes=" + ld.scopes + "\" "
        "--form \"device_code=" + ld.device_code + "\" "
        "--form \"grant_type=urn:ietf:params:oauth:grant-type:device_code\" "
        "-s -o user_access_token" ;

    auto const sys_res = std::system( curl_request_com.c_str() ) ;
    if ( sys_res != 0 )
    {
        return motor::social::twitch::request_user_token_result::curl_failed ;
    }

    std::string response ;

    // read content
    {
        std::ifstream myfile( "user_access_token" ) ;
        std::string line;
        if ( myfile.is_open() )
        {
            while ( std::getline ( myfile, line ) )
            {
                response += line ;
            }
            myfile.close();
        }
    }

    // clear content
    {
        std::ofstream ofs;
        ofs.open( "user_access_token", std::ofstream::out | std::ofstream::trunc );
        ofs.close();
    }
    #endif

    // json 
    if ( !response.empty() )
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;

        #if 1
        if( this_file::check_for_status(data) )
        {
            auto const msg = data[ "message" ] ;

            if ( msg == "authorization_pending" )
            {
                return motor::social::twitch::request_user_token_result::pending ;
            }
            else if ( msg == "invalid device code" )
            {
                return motor::social::twitch::request_user_token_result::invalid_device_code ;
            }
            else if ( msg == "missing device_code" )
            {
                return motor::social::twitch::request_user_token_result::invalid_device_code ;
            }

            return motor::social::twitch::request_user_token_result::request_failed ;
        }
        #else
        if ( data.contains( "status" ) )
        {
            size_t const code = data[ "status" ] ;
            motor::string_t msg ;

            if ( data.contains( "message" ) )
            {
                msg = data[ "message" ] ;
            }

            motor::log::global_t::error( "Twitch IRC Bot device code request failed: " ) ;
            motor::log::global_t::error( "[" + motor::to_string( code ) + "] : " + msg ) ;

            if ( msg == "authorization_pending" )
            {
                return motor::social::twitch::request_user_token_result::pending ;
            }
            else if ( msg == "invalid device code" )
            {
                return motor::social::twitch::request_user_token_result::invalid_device_code ;
            }
            else if ( msg == "missing device_code" )
            {
                return motor::social::twitch::request_user_token_result::invalid_device_code ;
            }

            return motor::social::twitch::request_user_token_result::request_failed ;
        }
        #endif

        if ( data.contains( "access_token" ) )
        {
            ld.user_token = data[ "access_token" ] ;
        }
        if ( data.contains( "refresh_token" ) )
        {
            ld.refresh_token = data[ "refresh_token" ] ;
        }
    }
    else
    {
        motor::log::global_t::error( "response was empty. Please check curl request." ) ;
        return motor::social::twitch::request_user_token_result::request_failed ;
    }
    return motor::social::twitch::request_user_token_result::ok ;
}

//**********************************************************************************
bool_t twitch_irc_bot::load_credentials( this_t::login_data_out_t ld ) const noexcept
{
    bool_t ret = false ;

    _db->load( motor::io::location( "twitch.credentials" ) ).wait_for_operation(
        [&] ( char_cptr_t buf, size_t const sib, motor::io::result const res )
    {
        if ( res == motor::io::result::ok )
        {
            auto const content = std::string( buf, sib ) ;
            auto const json = nlohmann::json::parse( content ) ;
            
            if ( !json.contains( "client_id" ) || 
                !json.contains( "channel_name" ) ||
                !json.contains( "nick_name" ) ||
                !json.contains( "broadcaster_id" ) ||
                !json.contains( "bot_id" ) )
            {
                motor::log::global_t::error( "client_id, channel name or nick name missing." ) ;
                ret = false ;
                return ;
            }

            if ( json.contains( "access_token" ) )
            {
                ld.user_token = json[ "access_token" ] ;
            }
            if ( json.contains( "refresh_token" ) )
            {
                ld.refresh_token = json[ "refresh_token" ] ;
            }
            
            if ( json.contains( "client_secret" ) )
            {
                ld.client_secret = json[ "client_secret" ] ;
            }
            if ( json.contains( "scopes" ) )
            {
                ld.scopes = json[ "scopes" ] ;
            }
            if ( json.contains( "device_code" ) )
            {
                ld.device_code = json[ "device_code" ] ;
            }

            ld.client_id = json[ "client_id" ] ;
            ld.channel_name = json[ "channel_name" ] ;
            ld.nick_name = json[ "nick_name" ] ;
            ld.broadcaster_id = json["broadcaster_id"] ;
            ld.bot_id = json[ "bot_id" ] ;

            ret = true ;
        }
    } ) ;
    return ret ;
}

//**********************************************************************************
void_t twitch_irc_bot::write_credentials( this_t::login_data_in_t ld ) const noexcept
{
    std::string const content = nlohmann::json (
    {
        { "access_token", _login_data.user_token },
        { "refresh_token", _login_data.refresh_token },
        { "client_id", _login_data.client_id },
        { "client_secret", _login_data.client_secret },
        { "scopes", _login_data.scopes },
        { "device_code", _login_data.device_code },
        { "nick_name", _login_data.nick_name },
        { "channel_name", _login_data.channel_name },
        { "broadcaster_id", _login_data.broadcaster_id },
        { "bot_id", _login_data.bot_id }
        } ).dump() ;

    _db->store( motor::io::location_t( "twitch.credentials" ), content.c_str(), content.size() ).
        wait_for_operation( [&] ( motor::io::result const res )
    {
        if ( res == motor::io::result::ok )
        {
            motor::log::global_t::error( "New Twitch Tokens written to : twitch.credentials" ) ;
        }
        else
        {
            motor::log::global_t::error( "failed to write new tokens" ) ;
        }
    } ) ;
}

//**********************************************************************************
void_t twitch_irc_bot::update( void_t ) noexcept
{
    std::this_thread::sleep_for( _timeout ) ;
    _timeout = std::chrono::seconds( 0 ) ;

    switch ( _ps )
    {
    case motor::social::twitch::program_state::load_credentials:
    {
        auto const res = this_t::load_credentials_from_file() ;
        switch ( res )
        {
        case motor::social::twitch::initial_process_result::curl_failed:
            exit( 1 ) ;
            break ;
        case motor::social::twitch::initial_process_result::invalid_token:
            _ps = motor::social::twitch::program_state::need_refresh ;
            break ;
        case motor::social::twitch::initial_process_result::no_token:
            _ps = motor::social::twitch::program_state::need_device_code ;
            break ;
        case motor::social::twitch::initial_process_result::ok:
            _ps = motor::social::twitch::program_state::need_token_validation ;
            break ;
        }

        break ;
    }
    case motor::social::twitch::program_state::need_token_validation:
    {
        auto const res = this_t::validate_token( _login_data ) ;
        switch ( res )
        {
        case motor::social::twitch::validation_process_result::curl_failed:
            exit( 1 ) ;
            break ;
        case motor::social::twitch::validation_process_result::invalid_access_token:
            _ps = motor::social::twitch::program_state::need_refresh ;
            break ;
        case motor::social::twitch::validation_process_result::ok:
            _ps = motor::social::twitch::program_state::need_login ;
            break ;
        }
        break ;
    }
    case motor::social::twitch::program_state::need_device_code:
    {
        auto const res = this_t::request_device_code( _login_data, _dcr ) ;
        switch ( res )
        {
        case motor::social::twitch::device_code_process_result::curl_failed:
            exit( 1 ) ;
            break ;

        case motor::social::twitch::device_code_process_result::device_code_request_failed:
        {
            motor::log::global_t::error( "Please check the client_id in the credentials." ) ;
            motor::log::global_t::error( "Will reinitialize in 10 seconds." ) ;
            _timeout = std::chrono::seconds( 10 ) ;
            _ps = motor::social::twitch::program_state::load_credentials ;
            break ;
        }
        case motor::social::twitch::device_code_process_result::ok:
            _ps = motor::social::twitch::program_state::need_token ;
            break ;
        }
        break ;
    }
    case motor::social::twitch::program_state::need_token:
    {
        auto const res = this_t::request_user_token( _login_data ) ;
        switch ( res )
        {
        case motor::social::twitch::request_user_token_result::curl_failed:
            exit( 1 ) ;
            break ;
        case motor::social::twitch::request_user_token_result::invalid_device_code:
            _ps = motor::social::twitch::program_state::need_device_code ;
            break ;
            // lets give the user some time to authorize.
            // then re-try user token request.
        case motor::social::twitch::request_user_token_result::pending:
            motor::log::global_t::status( "Please visit for authorization:" ) ;
            motor::log::global_t::status( _dcr.verification_uri ) ;
            _timeout = std::chrono::seconds( 10 ) ;
            break ;
            // unknown failure. Just redo everything and hope for the best.
            // Twitch error status codes are not documented or are changing 
            // based on error, so if the message can not be parsed correctly...
        case motor::social::twitch::request_user_token_result::request_failed:
            motor::log::global::error( "Some error occurred when requesting a user token." ) ;
            motor::log::global::error( "Reinitializing process." ) ;
            _ps = motor::social::twitch::program_state::load_credentials ;
            break ;
            // Yay. Worked.
        case motor::social::twitch::request_user_token_result::ok:
            this_t::write_credentials( _login_data ) ;
            _ps = motor::social::twitch::program_state::need_login ;
            break ;
        }
        break ;
    }
    case motor::social::twitch::program_state::need_refresh:
    {
        auto const res = this_t::refresh_token( _login_data ) ;
        switch ( res )
        {
        case motor::social::twitch::refresh_process_result::ok:
            this_t::write_credentials( _login_data ) ;
            _ps = motor::social::twitch::program_state::need_login ;
            break ;
        case motor::social::twitch::refresh_process_result::curl_failed: break ;
        case motor::social::twitch::refresh_process_result::invalid_client_id: break ;
        case motor::social::twitch::refresh_process_result::invalid_token:
            _ps = motor::social::twitch::program_state::need_token ;
            break ;
        case motor::social::twitch::refresh_process_result::invalid: break ;

        }
        break ;
    }
    case motor::social::twitch::program_state::need_login:
    {
        data_out =
            "PASS oauth:" + _login_data.user_token + "\r\n"
            "NICK "+ _login_data.nick_name + "\r\n"
            "JOIN " + _login_data.channel_name + "\r\n"
            "CAP REQ :twitch.tv/membership twitch.tv/tags twitch.tv/commands\r\n" ;

        _login_tp = clk_t::now() ;
        _ps = motor::social::twitch::program_state::login_pending ;
        break ;
    }
    case motor::social::twitch::program_state::login_pending:
    {
        if ( std::chrono::duration_cast<std::chrono::seconds>( clk_t::now() - _login_tp ) >=
            std::chrono::seconds( 5 ) )
        {
            //_ps = motor::social::twitch::program_state::need_login ;
        }
        break ;
    }
    }
}

//**********************************************************************************
motor::social::twitch::initial_process_result twitch_irc_bot::load_credentials_from_file( void_t ) noexcept
{
    if ( !this_t::load_credentials( _login_data ) ||
        !this_t::validate_credentials( _login_data ) )
    {
        motor::log::global_t::status( "[twitch_irc_bot] : Twitch Client ID missing." ) ;
        return motor::social::twitch::initial_process_result::credentials_failed ;
    }

    if ( _login_data.user_token.empty() )
    {
        return motor::social::twitch::initial_process_result::no_token ;
    }

    return motor::social::twitch::initial_process_result::ok ;
}

//**********************************************************************************
bool_t twitch_irc_bot::swap_commands( this_t::commands_inout_t comms_out ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_commands ) ;

    {
        auto tmp = std::move( comms_out ) ;
        comms_out = std::move( _commands ) ;
        _commands = std::move( tmp ) ;
        _commands.clear() ;
    }
    
    return !comms_out.empty() ;
}

//**********************************************************************************
void_t twitch_irc_bot::send_response( motor::string_in_t v ) noexcept
{
    std::lock_guard< std::mutex > lk(_mtx_outs ) ;
    _outs.emplace_back( v ) ;
}

//**********************************************************************************
void_t twitch_irc_bot::send_response( motor::string_rref_t v ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_outs ) ;
    _outs.emplace_back( std::move( v ) ) ;
}

//**********************************************************************************
bool_t twitch_irc_bot::send_announcement( motor::string_in_t msg, 
    motor::social::twitch::announcement_color const c ) noexcept
{
    motor::string_t response ;

    auto const curl_validate_com =
        "curl -X POST \"https://api.twitch.tv/helix/chat/announcements?broadcaster_id="
            +_login_data.broadcaster_id+"&moderator_id="+_login_data.bot_id+"\" "
        "-H \"Authorization: Bearer " + _login_data.user_token + "\" "
        "-H \"Client-Id: " + _login_data.client_id + "\" "
        "-H \"Content-Type: application/json\" "
        "-d \"{"        
        "\\\"message\\\":\\\"" + msg + "\\\","
        "\\\"color\\\":\\\"" + motor::social::twitch::to_string( c ) + "\\\""
        "}\"" ;

    if ( !this_t::send_curl( curl_validate_com, "send_message", response, true ) )
    {
        motor::log::global_t::error( "[send_message] : curl unavailable." ) ;
        return false ;
    }

    //motor::log::global_t::status( response ) ;

    // json validate
    if( !response.empty() )
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;
        if ( this_file::check_for_status( data ) )
        {
            return false ;
        }
    }
    return true ;
}

//**********************************************************************************
bool_t twitch_irc_bot::send_message( motor::string_in_t msg ) noexcept
{
    motor::string_t response ;

    auto const curl_validate_com =
        "curl -X POST https://api.twitch.tv/helix/chat/messages "
        "-H \"Authorization: Bearer " + _login_data.user_token + "\" "
        "-H \"Client-Id: " + _login_data.client_id + "\" "
        "-H \"Content-Type: application/json\" "
        "-d \"{"
        "\\\"broadcaster_id\\\":\\\""+_login_data.broadcaster_id+"\\\","
        "\\\"sender_id\\\":\\\"" + _login_data.bot_id + "\\\","
        "\\\"message\\\":\\\"" + msg + "\\\""
        "}\"" ;

    if ( !this_t::send_curl( curl_validate_com, "send_message", response, true ) )
    {
        motor::log::global_t::error("[send_message] : curl unavailable.") ;
        return false ;
    }

    //motor::log::global_t::status( response ) ;
    
    // json validate
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;
        if( this_file::check_for_status( data ) )
        {
            return false ;
        }
    }
    return true ;
}

//**********************************************************************************
motor::network::user_decision twitch_irc_bot::on_connect( motor::network::connect_result const res, size_t const tried ) noexcept
{
    motor::log::global_t::status( "Connection : " + motor::network::to_string( res ) ) ;
    if ( tried > 2 ) std::this_thread::sleep_for( std::chrono::seconds( 2 ) ) ;

    if ( res == motor::network::connect_result::established )
    {
        this_t::update() ;
    }
    else if ( res == motor::network::connect_result::closed )
    {
        if( _ps == motor::social::twitch::program_state::bot_is_online )
            _ps = motor::social::twitch::program_state::need_login ;
    }

    return motor::network::user_decision::keep_going ;
}

//**********************************************************************************
motor::network::user_decision twitch_irc_bot::on_sync( void_t ) noexcept
{
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ) ;
    return motor::network::user_decision::keep_going ;
}

//**********************************************************************************
motor::network::user_decision twitch_irc_bot::on_update( void_t ) noexcept
{
    this_t::update() ;

    return motor::network::user_decision::keep_going ;
}

//**********************************************************************************
void_t twitch_irc_bot::on_receive( byte_cptr_t buffer, size_t const sib ) noexcept
{
    data_in += motor::string_t( (char_cptr_t) buffer, sib ) ;
}

//**********************************************************************************
void_t twitch_irc_bot::on_received( void_t ) noexcept
{
    parser.parse( data_in ) ;

    if ( _ps == motor::social::twitch::program_state::login_pending )
    {
        _ps = motor::social::twitch::program_state::bot_is_online ;
        data_out = "PRIVMSG #aconstlink : HeyGuys " + _login_data.nick_name + " is online.\r\n" ;
    }

    parser.for_each( [&] ( motor::social::twitch::irc_command const c,
        motor::social::twitch::tags_cref_t tags, motor::string_in_t param )
    {
        if ( c == motor::social::twitch::irc_command::ping )
        {
            _pong = "PONG " + motor::string_t( ":" ) + param + "\r\n" ;
            _require_pong = true ;
            return false ;
        }
        else if ( c == motor::social::twitch::irc_command::notice )
        {
            auto const r0 = param.find( "Login" ) ;
            auto const r1 = param.find( "authentication" ) ;
            auto const r2 = param.find( "failed" ) ;

            if ( r1 != std::string::npos && r2 != std::string::npos )
            {
                _ps = motor::social::twitch::program_state::need_refresh ;
            }

            return false ;
        }
        else if ( c == motor::social::twitch::irc_command::privmsg )
        {
            size_t const pos = param.find_first_of( '!' ) ;
            if ( pos != std::string::npos && pos == 0 )
            {
                auto const user = tags.find( "display-name" ) ;
                
                motor::string_t tokens[ 9 ] ;

                {
                    size_t idx = size_t(-1) ;
                    size_t p0 = 1 ;
                    while ( true )
                    {
                        size_t const p1 = param.find_first_of( ' ', p0 ) ;
                        tokens[++idx] = param.substr( p0, p1 - p0 ) ;
                        if( p1 == std::string::npos || idx == 7 ) break ;
                        p0 = p1 + 1 ;
                    }
                }

                auto const com = tokens[0] ; //param.substr( 1, param.size() - 1 ) ;
                if ( com == "echo" )
                {
                    if ( user != tags.end() )
                        data_out = "PRIVMSG #aconstlink : HeyGuys " + user->second + "\r\n" ;
                }
                else
                {
                    _commands.emplace_back( this_t::command { user->second, com,
                        { tokens[ 1 ], tokens[ 2 ], tokens[ 3 ], tokens[ 4 ],
                        tokens[ 5 ], tokens[ 6 ], tokens[ 7 ], tokens[ 8 ] }
                        } ) ;
                    
                }
            }
        }
        else if ( c == motor::social::twitch::irc_command::part )
        {
        }
        else
        {
            int bp = 0 ;
        }

        return true ;
    } ) ;

    parser.clear() ;

    motor::log::global_t::status( data_in ) ;
    data_in.clear() ;
}

//**********************************************************************************
void_t twitch_irc_bot::on_send( byte_cptr_t & buffer, size_t & num_sib ) noexcept
{
    if( _require_pong )
    {
        buffer = byte_ptr_t( _pong.c_str() ) ;
        num_sib = _pong.size() ;
        _require_pong = false ;
        return ;
    }

    // construct response 
    {
        std::lock_guard< std::mutex > lk( _mtx_outs ) ;
        for( auto const & o : _outs )
        {
            data_out += "PRIVMSG " + _login_data.channel_name + " : " + o + "\r\n" ;
        }
        _outs.clear() ;
    }

    if ( data_out.size() != 0 )
    {
        buffer = byte_ptr_t( data_out.c_str() );
        num_sib = data_out.size() ;

        return ;
    }
}

//**********************************************************************************
void_t twitch_irc_bot::on_sent( motor::network::transmit_result const res ) noexcept
{
    if( res != motor::network::transmit_result::connection_reset )
        data_out.clear() ;
}