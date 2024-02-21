
#pragma once

#include "api.h"
#include "typedefs.h"
#include "result.h"
#include "window/window_info.h"
#include "window/window.h"

#include "window/window_message_listener.h"

#include <motor/device/device.hpp>
#include <motor/device/layouts/ascii_keyboard.hpp>
#include <motor/device/layouts/three_mouse.hpp>

//#include <motor/tool/imgui/imgui.h>
#include <motor/concurrent/typedefs.h>

namespace motor
{
    namespace application
    {
        class carrier ;
        using carrier_mtr_t = carrier * ;
        using carrier_ptr_t = carrier * ;

        class MOTOR_APPLICATION_API app
        {
            motor_this_typedefs( app ) ;

        private:

            carrier_mtr_t _carrier = nullptr ;

        private:

            typedef std::chrono::high_resolution_clock platform_clock_t ;
            platform_clock_t::time_point _tp_platform = platform_clock_t::now() ;
            
            std::chrono::microseconds _render_residual = std::chrono::microseconds( 0 ) ;
            std::chrono::microseconds _audio_residual = std::chrono::microseconds( 0 ) ;
            std::chrono::microseconds _update_interval = std::chrono::microseconds( 8000 ) ;
            std::chrono::microseconds _update_residual = std::chrono::microseconds( 0 ) ;
            std::chrono::microseconds _physics_interval = std::chrono::microseconds( 8000 ) ;
            std::chrono::microseconds _physics_residual = std::chrono::microseconds( 0 ) ;
            std::chrono::microseconds _logic_interval = std::chrono::microseconds( 8000 ) ;
            std::chrono::microseconds _logic_residual = std::chrono::microseconds( 0 ) ;
            std::chrono::microseconds _device_interval = std::chrono::microseconds( 10000 ) ;
            std::chrono::microseconds _device_residual = std::chrono::microseconds( 0 ) ;

        private:

            struct window_data
            {
                motor::application::iwindow_mtr_t wnd = nullptr ;
                motor::application::window_message_listener_mtr_t lst = nullptr ;
            };

            std::mutex _mtx_windows ;
            motor::vector< window_data > _windows ;

            bool_t _closed = false ;

        private: // device

            motor::device::three_device_mtr_t _dev_mouse ;
            motor::device::ascii_device_mtr_t _dev_ascii ;

        public:

            struct update_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };
            motor_typedef( update_data ) ;

            struct physics_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };
            motor_typedef( physics_data ) ;

            struct graphics_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };
            motor_typedef( graphics_data ) ;

            struct render_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };
            motor_typedef( render_data ) ;

            struct audio_data {} ;
            motor_typedef( audio_data ) ;

            struct device_data {};
            motor_typedef( device_data ) ;

            struct logic_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };
            motor_typedef( logic_data ) ;

            struct tool_data
            {
                //motor::tool::imgui_view_t imgui ;
            };
            motor_typedef( tool_data ) ;

        public:

            virtual motor::application::result on_init( void_t ) noexcept = 0 ;
            virtual motor::application::result on_update( motor::application::app::update_data_in_t ) noexcept = 0 ;
            virtual motor::application::result on_graphics( motor::application::app::graphics_data_in_t ) noexcept = 0 ;
            virtual motor::application::result on_render( motor::application::app::render_data_in_t ) noexcept = 0 ;
            virtual motor::application::result on_shutdown( void_t ) noexcept = 0 ;
            
            virtual motor::application::result on_audio( audio_data_in_t ) noexcept { return motor::application::result::ok ; }
            virtual motor::application::result on_device( device_data_in_t ) noexcept { return motor::application::result::ok ; }
            virtual motor::application::result on_logic( logic_data_in_t ) noexcept { return motor::application::result::ok ; }
            virtual motor::application::result on_tool( tool_data_ref_t ) noexcept { return motor::application::result::no_tool ;  }
            virtual motor::application::result on_physics( physics_data_in_t ) noexcept { return motor::application::result::ok ; }

        public:

            using window_id_t = size_t ;

            virtual motor::application::result on_event( window_id_t const, 
                motor::application::window_message_listener::state_vector_cref_t ) noexcept
            { return motor::application::result::ok ; }

            window_id_t create_window( motor::application::window_info_cref_t ) noexcept ;

            void_t close( void_t ) noexcept { _closed = true ; }

        public:

            app( void_t ) noexcept ;
            app( this_cref_t ) = delete ;
            app( this_rref_t ) noexcept ;
            virtual ~app( void_t ) noexcept ;

        public: // platform application interface

            struct carrier_accessor
            {
                this_ptr_t _app ;
                carrier_accessor( this_ptr_t a ) noexcept : _app(a) {}
                bool_t init( motor::application::carrier_ptr_t p ) noexcept { return _app->carrier_init(p) ; }
                bool_t update( void_t ) noexcept { return _app->carrier_update() ; }
                bool_t shutdown( void_t ) noexcept { return _app->carrier_shutdown() ; }
                bool_t has_closed( void_t ) noexcept { return _app->_closed ; }
            };

        private: 

            bool_t carrier_init( motor::application::carrier_ptr_t ) noexcept ;
            bool_t carrier_update( void_t ) noexcept ;
            bool_t carrier_shutdown( void_t ) noexcept ;

        private: // system specific update code

            bool_t before_update( std::chrono::microseconds const & ) noexcept ;
            bool_t after_update( size_t const iter ) ;
            bool_t before_physics( std::chrono::microseconds const & ) noexcept ;
            bool_t after_physics( size_t const iter ) ;
            bool_t before_render( std::chrono::microseconds const & ) noexcept ;
            bool_t after_render( size_t const iter ) ;
            bool_t before_audio( std::chrono::microseconds const & ) noexcept ;
            bool_t after_audio( size_t const iter ) ;
            bool_t before_tool( std::chrono::microseconds const & ) noexcept ;
            bool_t after_tool( size_t const iter ) noexcept ;
            bool_t before_device( std::chrono::microseconds const & ) noexcept ;
            bool_t after_device( size_t const iter ) noexcept ;
            bool_t before_logic( std::chrono::microseconds const & ) noexcept ;
            bool_t after_logic( size_t const iter ) noexcept ;

        };
        motor_typedef( app ) ;
    }
}
