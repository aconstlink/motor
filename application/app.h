
#pragma once

#include "api.h"
#include "typedefs.h"
#include "result.h"
#include "protos.h"
#include "platform/window_info.h"
#include "platform/platform_window.h"
#include "platform/gfx_context.h"

#include <natus/device/device.hpp>
#include <natus/device/layouts/ascii_keyboard.hpp>
#include <natus/device/layouts/three_mouse.hpp>

#include <natus/audio/async.h>
#include <natus/graphics/async.h>
#include <natus/tool/imgui/imgui.h>
#include <natus/concurrent/typedefs.h>

namespace motor
{
    namespace application
    {
        class NATUS_APPLICATION_API app
        {
            motor_this_typedefs( app ) ;

            friend class platform_application ;

        public: // window info

            typedef size_t window_id_t ;

            struct window_info
            {
                int_t x = 100 ;
                int_t y = 100 ;
                int_t w = 400 ;
                int_t h = 400 ;

                bool_t vsync = true ;
                bool_t borders = true ;
                bool_t fullscreen = false ;
            };
            motor_typedef( window_info ) ;

            struct window_event_info
            {
                uint_t w = 400 ;
                uint_t h = 400 ;
            };
            motor_typedef( window_event_info ) ;

        public: // window accessor

            class NATUS_APPLICATION_API window_view
            {
                motor_this_typedefs( window_view ) ;

                friend class app ;

            private:

                window_id_t _id ;
                motor::application::window_message_receiver_res_t _msg_wnd ;
                motor::application::window_message_receiver_res_t _msg_gfx ;

            public:

                window_view( void_t ) ;
                window_view( this_rref_t ) ;
                window_view( this_cref_t ) ;
                ~window_view( void_t ) ;

                this_ref_t operator = ( this_rref_t ) ;
                this_ref_t operator = ( this_cref_t ) ;

            private:

                window_view( window_id_t id, motor::application::window_message_receiver_res_t wnd,
                    motor::application::window_message_receiver_res_t gfx ) ;

            public:

                window_id_t id( void_t ) const noexcept ;
                void_t position( int_t const x, int_t const y ) noexcept ;
                void_t resize( size_t const w, size_t const h ) noexcept ;
                void_t vsync( bool_t const ) noexcept ;
                void_t fullscreen( bool_t const ) noexcept ;
            };
            motor_res_typedef( window_view ) ;

        private: // per window info

            struct per_window_info
            {
                motor_this_typedefs( per_window_info ) ;

                motor::concurrent::mutex_t mtx ;
                motor::concurrent::thread_t rnd_thread ;
                motor::application::platform_window_res_t wnd ;
                motor::application::window_message_receiver_res_t msg_recv ;
                motor::application::window_message_receiver_res_t msg_send ;
                motor::application::window_message_receiver_res_t gfx_send ;
                motor::graphics::async_res_t async ;
                motor::tool::imgui_res_t imgui ;
                window_info_t wi ;
                bool_ptr_t run ;
                per_window_info( void_t ) noexcept {}
                per_window_info( this_cref_t ) = delete ;
                per_window_info( this_rref_t rhv ) noexcept
                {
                    imgui = std::move( rhv.imgui ) ;
                    rnd_thread = std::move( rhv.rnd_thread ) ;
                    wnd = std::move( rhv.wnd ) ;
                    async = std::move( rhv.async ) ;
                    wi = std::move( rhv.wi ) ;
                    msg_recv = std::move( rhv.msg_recv ) ;
                    msg_send = std::move( rhv.msg_send ) ;
                    motor_move_member_ptr( run, rhv ) ;
                }
                ~per_window_info( void_t ) {}
            };
            motor_typedef( per_window_info ) ;
            typedef motor::vector< per_window_info_t > windows_t ;
            motor::concurrent::mutex_t _wmtx ;
            windows_t _windows ;

        private: // audio engine

            struct per_audio_info
            {
                motor_this_typedefs( per_audio_info ) ;

                motor::concurrent::mutex_t mtx ;
                motor::concurrent::thread_t rnd_thread ;
                motor::audio::async_res_t async ;
                bool_ptr_t run ;
                per_audio_info( void_t ) {}
                per_audio_info( this_cref_t ) = delete ;
                per_audio_info( this_rref_t rhv )
                {
                    rnd_thread = ::std::move( rhv.rnd_thread ) ;
                    async = ::std::move( rhv.async ) ;
                    motor_move_member_ptr( run, rhv ) ;
                }
                ~per_audio_info( void_t ) {}
            };
            motor_typedef( per_audio_info ) ;
            typedef motor::vector< per_audio_info_t > audios_t ;
            motor::concurrent::mutex_t _amtx ;
            audios_t _audios ;

        private:

            // async view access
            bool_ptr_t _access = nullptr ;
            size_t _update_count = 0 ;
            size_t _render_count = 0 ;
            size_t _audio_count = 0 ;
            size_t _logic_count = 0 ;

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

        private: // device

            motor::device::three_device_res_t _dev_mouse ;
            motor::device::ascii_device_res_t _dev_ascii ;

        public:

            app( void_t ) ;
            app( this_cref_t ) = delete ;
            app( this_rref_t ) ;
            virtual ~app( void_t ) ;

        public:

            struct update_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };

            struct physics_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };

            struct render_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };
            struct audio_data {} ;
            struct device_data {};

            struct logic_data 
            {
                float_t sec_dt ;
                size_t micro_dt ;
                size_t milli_dt ;
            };

            struct tool_data
            {
                motor::tool::imgui_view_t imgui ;
            };

            motor_typedef( update_data ) ;
            motor_typedef( physics_data ) ;
            motor_typedef( render_data ) ;
            motor_typedef( audio_data ) ;
            motor_typedef( device_data ) ;
            motor_typedef( logic_data ) ;
            motor_typedef( tool_data ) ;

        public:

            virtual motor::application::result on_init( void_t ) noexcept = 0 ;
            virtual motor::application::result on_update( update_data_in_t ) noexcept { return motor::application::result::ok ; }
            virtual motor::application::result on_graphics( render_data_in_t ) noexcept = 0 ;
            virtual motor::application::result on_audio( audio_data_in_t ) noexcept { return motor::application::result::ok ; }
            virtual motor::application::result on_device( device_data_in_t ) noexcept { return motor::application::result::ok ; }
            virtual motor::application::result on_shutdown( void_t ) noexcept = 0 ;
            virtual motor::application::result on_logic( logic_data_in_t ) noexcept { return motor::application::result::ok ; }

            virtual motor::application::result on_event( window_id_t const, this_t::window_event_info_in_t ) noexcept
            { return motor::application::result::ok ; }

            virtual motor::application::result on_tool( tool_data_ref_t ) noexcept
            { return motor::application::result::no_tool ;  }

            virtual motor::application::result on_physics( physics_data_in_t ) noexcept 
            { return motor::application::result::ok ; }

        protected:

            struct window_async
            {
                friend class app ;

            private:

                this_t::window_view_t _wv ;
                motor::graphics::async_view_t _av ;

                window_async( this_t::window_view_t wv, motor::graphics::async_view_t p ) :
                    _wv( std::move(wv) ), _av( std::move(p) ) {}

            public:

                window_async( void_t ) {}
                window_async( window_async && rhv )
                {
                    _wv = std::move( rhv._wv ) ;
                    _av = std::move( rhv._av ) ;
                }

                ~window_async( void_t ) {}

                window_async & operator = ( window_async&& rhv )
                {
                    _wv = std::move( rhv._wv ) ;
                    _av = std::move( rhv._av ) ;
                    return *this ;
                }

            public:

                this_t::window_view_t window( void_t ) const noexcept { return _wv ; }
                motor::graphics::async_view_t async( void_t ) const noexcept { return _av ; }

            };
            motor_typedef( window_async ) ;

            this_t::window_async_t create_window( 
                motor::string_cref_t name, this_t::window_info_in_t, motor::vector< motor::graphics::backend_type > types = { } ) ;

            motor::application::result request_change( this_t::window_info_in_t ) ;


            motor::audio::async_access_t create_audio_engine( motor::audio::backend_type bt = motor::audio::backend_type::unknown ) noexcept ;

        protected:

            enum class time_state
            {
                unknown, 
                run, 
                hold,
                step_forward
            };

            time_state get_time_state( void_t ) const noexcept ;
            void_t set_time_state( time_state const ) noexcept ;

        private:

            time_state _time_state = time_state::run ;
            time_state _time_state_user = time_state::run ;

        private:

            void_t destroy_window( this_t::per_window_info_ref_t ) ;
            void_t destroy_audio( this_t::per_audio_info_ref_t ) ;

        private: // platform application interface

            bool_t platform_init( void_t ) ;
            bool_t platform_update( void_t ) ;

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

        private:

            motor::application::gfx_context_res_t create_wgl_window( motor::application::window_info_in_t, motor::application::window_message_receiver_res_t, this_t::per_window_info_inout_t ) noexcept ;

            motor::application::gfx_context_res_t create_egl_window( motor::application::window_info_in_t, motor::application::window_message_receiver_res_t, this_t::per_window_info_inout_t ) noexcept ;

            motor::application::gfx_context_res_t create_d3d_window( motor::application::window_info_in_t, motor::application::window_message_receiver_res_t, this_t::per_window_info_inout_t ) noexcept ;

            motor::application::gfx_context_res_t create_glx_window( motor::application::window_info_in_t, motor::application::window_message_receiver_res_t, this_t::per_window_info_inout_t ) noexcept ;

            motor::application::gfx_context_res_t create_null_window( motor::application::window_info_in_t, motor::application::window_message_receiver_res_t, this_t::per_window_info_inout_t ) noexcept ;

        };
        motor_res_typedef( app ) ;
    }
}
