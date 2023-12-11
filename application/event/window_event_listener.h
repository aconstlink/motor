//------------------------------------------------------------
// snakeoil (c) Alexis Constantin Link
// Distributed under the MIT license
//------------------------------------------------------------
#ifndef _SNAKEOIL_APPLICATION_WINDOW_EVENT_LISTENER_H_
#define _SNAKEOIL_APPLICATION_WINDOW_EVENT_LISTENER_H_

#include "../typedefs.h"
#include "resize_event.h"

namespace so_app
{
    /// will be used by a window application to transport 
    /// application specific events.
    class window_event_listener
    {
    public:

        virtual void_t on_close( void_t ) = 0 ;
        virtual void_t on_show( bool_t shown ) = 0 ;
        virtual void_t on_focus( bool_t focus ) = 0 ;
        virtual void_t on_resize( resize_event const & ev ) = 0 ;
    };
}

#endif

