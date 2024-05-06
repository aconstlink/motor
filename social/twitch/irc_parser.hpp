

#pragma once

#include "../typedefs.h"

#include <motor/std/string>
#include <motor/std/vector>
#include <motor/std/hash_map>

namespace motor { namespace social { namespace twitch
{
    enum class irc_command
    {
        invalid,
        join,
        nick,
        notice,
        part,
        pass,
        ping,
        pong,
        privmsg,
        num_commands
    };

    namespace detail
    {
        static char const * __irc_command_strings[] = {
            "INVALID", "JOIN", "NICK", "NOTICE", "PART", "PASS", "PING",
            "PONG", "PRIVMSG"
        } ;
    }

    static motor::string_t to_string( motor::social::twitch::irc_command const c ) noexcept
    {
        return detail::__irc_command_strings[ 
            size_t( c ) < size_t( irc_command::num_commands ) ? size_t(c) : 0 ] ;
    }

    static motor::social::twitch::irc_command from_string( motor::string_in_t s ) noexcept
    {
        for( size_t i=0; i<size_t(irc_command::num_commands); ++i )
        {
            if( detail::__irc_command_strings[i] == s ) return irc_command(i) ;
        }
        return irc_command::invalid ;
    }

    enum class twitch_commands
    {
        clear_chat,
        clear_msg,
        global_user_state,
        host_target,
        notice,
        reconnect,
        room_state,
        user_notice,
        user_state,
        whisper

    };

    class irc_parser
    {
        motor_this_typedefs( irc_parser ) ;

    private:

        motor::string_t _raw_msg ;

    private: // working on 

        using tags_t = motor::hash_map< motor::string_t, motor::string_t > ;

        struct command
        {
            motor::string_t name ;
            motor::string_t param ;
        };

        struct message_line
        {
            motor::string_t irc_message ;
            tags_t tags ;
            // prefix
            command com ;
        };

        motor::vector< message_line > _lines ;

    public:

        irc_parser( void_t ) noexcept {}
        irc_parser( motor::string_cref_t raw_msg ) noexcept : _raw_msg( raw_msg ) {}
        irc_parser( motor::string_rref_t raw_msg ) noexcept : _raw_msg( std::move( raw_msg ) ) {}
        irc_parser( this_rref_t rhv ) noexcept : _raw_msg( std::move( rhv._raw_msg ) ) {}
        ~irc_parser( void_t ) noexcept {}

        this_ref_t operator = ( this_rref_t rhv ) noexcept
        {
            _raw_msg = std::move( rhv._raw_msg ) ;
            return *this ;
        }

    public:

        void_t clear( void_t ) noexcept 
        {
            _raw_msg = "" ;
            _lines.clear() ;
        }

        bool_t parse( motor::string_cref_t new_msg ) noexcept
        {
            _raw_msg = new_msg ;
            return this_t::parse() ;
        }

        bool_t parse( motor::string_rref_t new_msg ) noexcept
        {
            _raw_msg = std::move( new_msg ) ;
            return this_t::parse() ;
        }

        bool_t parse( void_t ) noexcept
        {
            _lines.reserve( 100 ) ;

            // separate \r\n lines
            {
                size_t start = 0 ;
                while ( true )
                {
                    size_t const pos = _raw_msg.find_first_of( '\r', start ) ;
                    if ( pos != std::string::npos )
                    {
                        auto const line = _raw_msg.substr( start, pos - start ) ;
                        _lines.emplace_back( this_t::message_line { line } ) ;
                        start = pos + 2 ;

                        continue ;
                    }

                    // last line
                    auto const line = _raw_msg.substr( start, _raw_msg.size() - start ) ;
                    if ( line.size() != 0 )
                    {
                        _lines.emplace_back( this_t::message_line { line } ) ;
                    }
                    break ;
                }
            }

            // STAGE 1 : identify tags
            {
                for ( auto & line : _lines )
                {
                    if ( line.irc_message[0] != '@' ) continue ;

                    auto const & ircm = line.irc_message ;

                    size_t start = 1 ;
                    while ( true )
                    {
                        size_t const pos = ircm.find_first_of( ';', start ) ;
                        if ( pos != std::string::npos )
                        {
                            motor::string_t const prop = ircm.substr( start, pos - start ) ;
                            {
                                size_t const inner_pos = prop.find_first_of( '=', 0 ) ;
                                if( inner_pos != std::string::npos )
                                {
                                    motor::string_t const key = prop.substr( 0, inner_pos ) ;
                                    motor::string_t const value = prop.substr( inner_pos+1, 
                                        prop.size() - ( inner_pos + 1 )) ;
                                    line.tags[key] = value ;
                                }
                            }
                            start = pos + 1 ;
                            continue ;
                        }

                        // last one
                        {
                            motor::string_t const prop = ircm.substr( start, ircm.size() - start ) ;
                            size_t const inner_beg = prop.find_first_of( '=', 0 ) ;
                            size_t const inner_end = prop.find_first_of( ' ', 0 ) ;

                            if ( inner_beg != std::string::npos &&
                                inner_end != std::string::npos )
                            {
                                motor::string_t const key = prop.substr( 0, inner_beg ) ;
                                motor::string_t const value = prop.substr( inner_beg + 1, 
                                    inner_end - (inner_beg + 1) ) ;
                                line.tags[ key ] = value ;
                            }

                            if( inner_end != std::string::npos )
                            {
                                line.irc_message = prop.substr( inner_end + 1, 
                                    prop.size() - ( inner_end + 1 ) ) ;
                            }

                            break ;
                        }
                    }
                }
            }

            // STAGE 2 : parse prefix
            {
                for ( auto & line : _lines )
                {
                    auto const & ircm = line.irc_message ;

                    // is it a prefix
                    if( ircm[0] != ':' ) continue ;

                    // analyze user name
                    {
                        size_t const start = ircm.find_first_of( '!', 1 ) ;
                        size_t const end = ircm.find_first_of( '@', 1 ) ;

                        if ( start != std::string::npos && 
                             end != std::string::npos )
                        {
                            size_t const p0 = start + 1 ;
                            size_t const p1 = end ;

                            motor::string_t const user = ircm.substr( p0, p1 - p0 ) ;
                        }
                    }

                    size_t const start = ircm.find_first_of( ' ', 1 ) ;
                    line.irc_message = ircm.substr( start + 1, ircm.size() - (start + 1) ) ;
                }
            }

            // STAGE 3 : parse command
            {
                motor::vector< motor::string_t > tokens ;
                for ( auto & line : _lines )
                {
                    auto const & ircm = line.irc_message ;

                    size_t const double_end = ircm.find_first_of( ':', 1 ) ;

                    if( double_end == std::string::npos ) continue ;

                    auto com = ircm.substr( 0, ircm.substr( 0, double_end ).find_first_of( ' ' ) ) ;

                    line.com = { com, ircm.substr(double_end+1, ircm.size() - ( double_end + 1 ) ) } ;
                }
                
            }

            return true ;
        }


        using for_each_line_funk_t = std::function< void_t ( message_line const & l ) > ;
        void_t for_each( for_each_line_funk_t funk ) const noexcept
        {
            for( auto const & line : _lines ) funk( line ) ;
        }

        using for_each_command_funk_t = std::function< bool_t ( motor::social::twitch::irc_command const, 
            motor::string_in_t param  ) > ;

        void_t for_each( for_each_command_funk_t funk ) const noexcept
        {
            for ( auto const & line : _lines ) 
            {
                irc_command const c = motor::social::twitch::from_string( line.com.name ) ;
                if( c == motor::social::twitch::irc_command::invalid ) continue ;
                auto const res = funk( c, line.com.param ) ;
                if( !res ) break ;
            }
        }
    };
    motor_typedef( irc_parser ) ;

}}} 