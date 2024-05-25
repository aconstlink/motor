
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
twitch_irc_bot::twitch_irc_bot( motor::io::database_mtr_safe_t db, motor::io::location_in_t conf_file ) noexcept :
    _db( motor::move( db ) ), _conf_file( conf_file )
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

    // create confidential file
    {
        if( conf_file.as_string().empty() )
        {
            _conf_file = motor::io::location_t("twitch.confidential") ;
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
    _conf_file = std::move( rhv._conf_file ) ;
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

    
    auto const curl_validate_com =
        "curl -X GET https://id.twitch.tv/oauth2/validate "
        "-H \"Authorization: OAuth " + ld.access_token + "\" " ;
    if( !this_t::send_curl( curl_validate_com, "validate", response, true ) )
    {
        return motor::social::twitch::validation_process_result::curl_failed ;
    }

    // investigate response
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;
        
        if( this_file::check_for_status( data ) )
        {
            return motor::social::twitch::validation_process_result::invalid_access_token ;
        }
    }

    return motor::social::twitch::validation_process_result::ok ;
}

//**********************************************************************************
motor::social::twitch::refresh_process_result twitch_irc_bot::refresh_token( login_data_ref_t ld ) const noexcept
{
    motor::log::global_t::status( "Refreshing User Token" ) ;

    motor::string_t response ;

    {
        auto const curl_com_conf =
            "curl -X POST https://id.twitch.tv/oauth2/token "
            "-H \"Content-Type: application/x-www-form-urlencoded\" "
            "-d \"grant_type=refresh_token&refresh_token=" + ld.refresh_token +
            "&client_id=" + ld.client_id +
            "&client_secret=" + ld.client_secret + "\" " ;

        auto const curl_com_pub =
            "curl -X POST https://id.twitch.tv/oauth2/token "
            "-H \"Content-Type: application/x-www-form-urlencoded\" "
            "-d \"grant_type=refresh_token&refresh_token=" + ld.refresh_token +
            "&client_id=" + ld.client_id + "\" " ;

        if ( !this_t::send_curl( ld.client_secret.empty() ? curl_com_pub : curl_com_conf, 
            "refresh_token", response, true ) )
        {
            return motor::social::twitch::refresh_process_result::curl_failed ;
        }
    }

    // json validate
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;
        
        if( this_file::check_for_status(data) )
        {
            return motor::social::twitch::refresh_process_result::invalid_token ;
        }

        if ( data.contains( "access_token" ) )
        {
            ld.access_token = data[ "access_token" ] ;
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

    motor::string_t response ;
    {
        auto const curl_com =
            "curl --location https://id.twitch.tv/oauth2/device "
            "--form \"client_id=" + ld.client_id + "\" "
            "--form \"scopes=" + ld.scopes_to_string() + "\" " ;

        if ( !this_t::send_curl( curl_com, "data_code_flow_request", response, true ) )
        {
            return motor::social::twitch::device_code_process_result::curl_failed ;
        }
    }

    // json 
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;
        
        if( this_file::check_for_status(data ) )
        {
            return motor::social::twitch::device_code_process_result::device_code_request_failed ;
        }

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

    motor::string_t response ;
    {
        auto const curl_com =
            "curl --location https://id.twitch.tv/oauth2/token "
            "--form \"client_id=" + ld.client_id + "\" "
            "--form \"scopes=" + ld.scopes_to_string() + "\" "
            "--form \"device_code=" + ld.device_code + "\" "
            "--form \"grant_type=urn:ietf:params:oauth:grant-type:device_code\" " ;

        if ( !this_t::send_curl( curl_com, "user_access_token", response, true ) )
        {
            return motor::social::twitch::request_user_token_result::curl_failed ;
        }
    }

    // json 
    if ( !response.empty() )
    {
        nlohmann::json data = nlohmann::json::parse( response ) ;

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

        if ( data.contains( "access_token" ) )
        {
            ld.access_token = data[ "access_token" ] ;
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
    motor::string_t content ;

    // load file
    {
        _db->load( _conf_file ).wait_for_operation(
            [&] ( char_cptr_t buf, size_t const sib, motor::io::result const res )
        {
            if ( res == motor::io::result::ok )
            {
                content = motor::string_t( buf, sib ) ;
            }
        } ) ;
    }

    // parse json
    if ( !content.empty() )
    {
        auto const json = nlohmann::json::parse( content ) ;

        if ( !json.contains( "required" ) )
        {
            motor::log::global_t::error( "missing required json object" ) ;
            exit( 1 ) ;
        }

        // handle required data
        {
            nlohmann::json const required = json[ "required" ] ;

            bool_t valid = true ;
            valid = valid || required.contains( "broadcaster_name" ) ;
            valid = valid || required.contains( "client_id" ) ;
            valid = valid || required.contains( "scopes" ) &&
                required[ "scopes" ].is_array() ;

            if ( !valid )
            {
                motor::log::global_t::error( "missing required json data" ) ;
                exit( 1 ) ;
            }

            ld.client_id = required[ "client_id" ] ;
            ld.broadcaster_name = required[ "broadcaster_name" ] ;


            auto const scopes = required[ "scopes" ] ;
            ld.scopes.reserve( scopes.size() ) ;

            for ( auto const & s : scopes )
            {
                if ( s.is_string() )
                {
                    ld.scopes.emplace_back( motor::string_t( s ) ) ;
                }
            }

            // print the scopes array as a string.
            {
                motor::log::global::status( ld.scopes_to_string() ) ;
            }
}

        if ( json.contains( "optional" ) )
        {
            nlohmann::json const optional = json[ "optional" ] ;

            if ( optional.contains( "bot_name" ) )
            {
                ld.bot_name = optional[ "bot_name" ] ;
            }

            if ( ld.bot_name.empty() )
            {
                ld.bot_name = ld.broadcaster_name ;
            }

            if ( optional.contains( "channel_name" ) )
            {
                ld.channel_name = optional[ "channel_name" ] ;
            }

            if ( ld.channel_name.empty() )
            {
                ld.channel_name = ld.broadcaster_name ;
            }

            if ( optional.contains( "nick_name" ) )
            {
                ld.nick_name = optional[ "nick_name" ] ;
            }

            if ( ld.nick_name.empty() )
            {
                ld.nick_name = ld.broadcaster_name ;
            }

            if ( optional.contains( "client_secret" ) )
            {
                ld.client_secret = optional[ "client_secret" ] ;
            }
        }
        else
        {
            ld.bot_name = ld.broadcaster_name ;
            ld.channel_name = ld.broadcaster_name ;
            ld.nick_name = ld.broadcaster_name ;
        }

        if ( json.contains( "internal" ) )
        {
            nlohmann::json const jdata = json[ "internal" ] ;

            if ( jdata.contains( "access_token" ) )
                ld.access_token = jdata[ "access_token" ] ;
            if ( jdata.contains( "device_code" ) )
                ld.device_code = jdata[ "device_code" ] ;
            if ( jdata.contains( "refresh_token" ) )
                ld.refresh_token = jdata[ "refresh_token" ] ;
        }
    }

    return !content.empty() ;
}

//**********************************************************************************
void_t twitch_irc_bot::write_credentials( this_t::login_data_in_t ld ) const noexcept
{
    nlohmann::json const output_data =
    {
        {
            "required",
            {
                { "broadcaster_name", ld.broadcaster_name },
                { "client_id", ld.client_id },
                { "scopes", ld.scopes }
            }
        },
        {
            "optional",
            {
                { "bot_name", ld.bot_name },
                { "channel_name", ld.channel_name },
                { "nick_name", ld.nick_name },
                { "client_secret", ld.client_secret }
            }
        },
        {
            "internal",
            {
                { "access_token", ld.access_token },
                { "device_code", ld.device_code },
                { "refresh_token", ld.refresh_token },
            }
        }
    } ;

    std::string dump = output_data.dump( 4 ) ;

    _db->store( _conf_file, dump.c_str(), dump.size() ).wait_for_operation( [&] ( motor::io::result const res )
    {
        if ( res == motor::io::result::ok )
        {
            motor::log::global_t::status( "[Twitch IRC Bot] : data written to : " + _conf_file.as_string() ) ;
        }
        else
        {
            motor::log::global_t::error( "[Twitch IRC Bot] : failed to write data to file : " + _conf_file.as_string() ) ;
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
            "PASS oauth:" + _login_data.access_token + "\r\n"
            "NICK "+ _login_data.nick_name + "\r\n"
            "JOIN #" + _login_data.channel_name + "\r\n"
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
        this_t::write_credentials( _login_data ) ;
        return motor::social::twitch::initial_process_result::credentials_failed ;
    }

    if ( _login_data.access_token.empty() )
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
motor::string_t twitch_irc_bot::get_user_id( motor::string_in_t user_name ) const noexcept 
{
    motor::string_t ret ;

    {
        motor::string_t response ;

        auto const curl_com =
            "curl -X GET \"https://api.twitch.tv/helix/users?login=" + user_name + "\" "

            "-H \"Authorization: Bearer " + _login_data.access_token + "\" "
            "-H \"Client-Id: " + _login_data.client_id + "\" " ;

        if ( !this_t::send_curl( curl_com, "get_user_id", response, true ) )
        {
            motor::log::global_t::error( "[send_message] : curl unavailable." ) ;
            return ret ;
        }

        // json validate
        if ( !response.empty() )
        {
            nlohmann::json root = nlohmann::json::parse( response ) ;
            if ( this_file::check_for_status( root ) )
            {
                return ret ;
            }

            if( root.contains("data") )
            {
                auto const data = root["data"] ;
                
                if( data.size() > 0 && data[0].contains("id" ) )
                {
                    ret = data[0]["id"] ;
                }
            }
        }
    }
    return ret ;
}

//**********************************************************************************
bool_t twitch_irc_bot::send_announcement( motor::string_in_t msg, 
    motor::social::twitch::announcement_color const c ) noexcept
{
    motor::string_t response ;

    auto const curl_validate_com =
        "curl -X POST \"https://api.twitch.tv/helix/chat/announcements?broadcaster_id="
            +_login_data.broadcaster_id+"&moderator_id="+_login_data.broadcaster_id+"\" "
        "-H \"Authorization: Bearer " + _login_data.access_token + "\" "
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
        "-H \"Authorization: Bearer " + _login_data.access_token + "\" "
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
        _login_data.broadcaster_id = this_t::get_user_id( _login_data.broadcaster_name ) ;
        _login_data.bot_id = this_t::get_user_id( _login_data.bot_name ) ;

        _ps = motor::social::twitch::program_state::bot_is_online ;
        //data_out = "PRIVMSG #aconstlink : HeyGuys " + _login_data.nick_name + " is online.\r\n" ;
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
            data_out += "PRIVMSG #" + _login_data.channel_name + " : " + o + "\r\n" ;
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