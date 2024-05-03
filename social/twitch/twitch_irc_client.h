
#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor { namespace social { namespace twitch {

    class MOTOR_SOCIAL_API twitch_irc_client
    {
        motor_this_typedefs( twitch_irc_client ) ;

    public: 

        twitch_irc_client( void_t ) noexcept ;
        ~twitch_irc_client( void_t ) noexcept ;
    };
} } }