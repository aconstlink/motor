

#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "iwindow_message_listener.h"

#include <motor/graphics/frontend/ifrontend.h>

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API iwindow
        {
        public:

            virtual ~iwindow( void_t ) {}

        public: // ins

            virtual void_t register_in( motor::application::iwindow_message_listener_mtr_safe_t l ) noexcept = 0 ;
            virtual void_t unregister_in( motor::application::iwindow_message_listener_mtr_t l ) noexcept = 0 ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_listener_funk_t ;
            virtual void_t foreach_in( foreach_listener_funk_t funk ) noexcept = 0 ;

        public: // outs

            virtual void_t register_out( motor::application::iwindow_message_listener_mtr_safe_t l ) noexcept = 0 ;
            virtual void_t unregister_out( motor::application::iwindow_message_listener_mtr_t l ) noexcept = 0 ;

            typedef std::function< void ( motor::application::iwindow_message_listener_mtr_t ) > foreach_out_funk_t ;
            virtual void_t foreach_out( foreach_out_funk_t funk ) noexcept = 0 ;

        public:

            virtual void_t send_message( motor::application::show_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::resize_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::create_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::close_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::screen_dpi_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::screen_size_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::vsync_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::fullscreen_message_cref_t ) noexcept = 0 ;
            virtual void_t send_message( motor::application::cursor_message_cref_t ) noexcept = 0 ;


        protected:

            using render_frame_funk_t = std::function< void_t ( motor::graphics::ifrontend* ) > ;
            virtual bool_t render_frame_virt( render_frame_funk_t ) noexcept = 0 ;

        public:

            template< typename frontend_t >
            bool_t render_frame( std::function< void_t ( frontend_t * ) > funk ) noexcept
            {
                return this->render_frame_virt( [&]( motor::graphics::ifrontend* ife )
                {
                    if( frontend_t * fe = dynamic_cast<frontend_t *>(ife); fe!= nullptr )
                    {
                        funk( fe ) ;

                        if( !fe->has_commands() )
                        {
                            // holds back render loop 
                            fe->fence( [=]( void_t ){for( size_t i=0; i<10000; ++i ) ;} ) ;
                        }
                    }
                } ) ;
            }
            
        } ;
        motor_typedef( iwindow ) ;
    }
}