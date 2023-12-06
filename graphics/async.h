
#pragma once

#include "async_id.hpp"
#include "backend/backend.h"

#include <motor/concurrent/typedefs.h>
#include <motor/std/vector>

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API async
        {
            motor_this_typedefs( async ) ;

        private:

            /// decorated backend.
            backend_mtr_t _backend ;

            bool_t _window_info_set = false  ;
            motor::graphics::backend_t::window_info_t _window_info ;
            motor::concurrent::mutex_t _window_info_mtx ;

            typedef std::function< void_t ( motor::graphics::backend_ptr_t ) > runtime_command_t ;
            motor_typedefs( motor::vector< runtime_command_t >, commands ) ;
            size_t _configures_id = 0 ;
            commands_t _configures[2] ;
            motor::concurrent::mutex_t _configures_mtx ;
            size_t _runtimes_id = 0 ;
            commands_t _runtimes[2] ;
            motor::concurrent::mutex_t _runtimes_mtx ;

        private: // sync

            motor::concurrent::mutex_t _frame_mtx ;
            std::condition_variable _frame_cv ;
            size_t _num_enter = 0 ;

        public:

            async( void_t ) noexcept ;
            async( backend_mtr_shared_t ) noexcept ;
            async( this_cref_t ) = delete ;
            async( this_rref_t ) noexcept ;
            ~async( void_t ) noexcept ;

        public:

            motor::graphics::result set_window_info( motor::graphics::backend_t::window_info_cref_t ) noexcept ;

            this_ref_t configure( motor::graphics::geometry_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t configure( motor::graphics::render_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t configure( motor::graphics::shader_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t configure( motor::graphics::image_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t configure( motor::graphics::framebuffer_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t configure( motor::graphics::state_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t configure( motor::graphics::array_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t configure( motor::graphics::streamout_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;

            this_ref_t release( motor::graphics::geometry_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t release( motor::graphics::render_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t release( motor::graphics::shader_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t release( motor::graphics::image_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t release( motor::graphics::framebuffer_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t release( motor::graphics::state_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t release( motor::graphics::array_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t release( motor::graphics::streamout_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;

            this_ref_t update( motor::graphics::geometry_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t update( motor::graphics::array_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t update( motor::graphics::image_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;

            this_ref_t use( motor::graphics::framebuffer_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t use( motor::graphics::streamout_object_mtr_shared_t, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t unuse( motor::graphics::backend::unuse_type const, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;

            this_ref_t push( motor::graphics::state_object_mtr_shared_t, size_t const = 0, bool_t const = true, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t pop( motor::graphics::backend::pop_type const, motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;
            this_ref_t render( motor::graphics::render_object_mtr_shared_t, motor::graphics::backend::render_detail_cref_t, 
                motor::graphics::result_mtr_shared_t = nullptr ) noexcept ;

        public:

            /// render thread update function - DO NOT USE.
            void_t system_update( void_t ) noexcept ;
            void_t end_of_time( void_t ) noexcept ;

        public: // sync

            bool_t enter_frame( void_t ) ;
            void_t leave_frame( void_t ) ;
            void_t wait_for_frame( void_t ) ;
        };
        motor_typedef( async ) ;

        // restricts access to the async
        class async_view
        {
            motor_this_typedefs( async_view ) ;

        private:

            motor::graphics::async_mtr_t _async = nullptr ;

        public: 

            async_view( motor::graphics::async_mtr_shared_t r ) noexcept : 
                _async( motor::memory::copy_ptr(r) ) {}

            async_view( motor::graphics::async_mtr_unique_t r ) noexcept : 
                _async( r ){}

        public:

            async_view( void_t ) noexcept {}
            async_view( this_cref_t rhv ) noexcept :
                _async( motor::memory::copy_ptr(rhv._async) ){}

            async_view( this_rref_t rhv ) noexcept :
                _async( rhv._async ){}

            ~async_view( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _async = motor::memory::copy_ptr(rhv._async) ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _async = motor::move( rhv._async ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::geometry_object_mtr_shared_t config,
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->configure( std::move(config), std::move(res) ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::render_object_mtr_shared_t config, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->configure( std::move(config), std::move(res) ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::shader_object_mtr_shared_t config,
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->configure( std::move(config), std::move(res) ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::image_object_mtr_shared_t config,
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->configure( std::move(config), std::move(res) ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::framebuffer_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->configure( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::state_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->configure( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::array_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->configure( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t configure( motor::graphics::streamout_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->configure( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::geometry_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::render_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::shader_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::image_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::framebuffer_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::state_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::array_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t release( motor::graphics::streamout_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->release( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t update( motor::graphics::geometry_object_mtr_shared_t obj,
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->update( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t update( motor::graphics::array_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->update( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t update( motor::graphics::image_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept 
            {
                _async->update( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t use( motor::graphics::framebuffer_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->use( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t use( motor::graphics::streamout_object_mtr_shared_t obj, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->use( std::move(obj), std::move(res) ) ;
                return *this ;
            }

            this_ref_t unuse( motor::graphics::backend::unuse_type const t, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->unuse( t, std::move(res) ) ;
                return *this ;
            }

            this_ref_t push( motor::graphics::state_object_mtr_shared_t obj, 
                size_t const sid = 0 , bool_t const push = true, motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->push( std::move(obj), sid, push, std::move(res) ) ;
                return *this ;
            }

            this_ref_t pop( motor::graphics::backend::pop_type const t, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->pop( t, std::move(res) ) ;
                return *this ;
            }

            this_ref_t render( motor::graphics::render_object_mtr_shared_t config, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                motor::graphics::backend_t::render_detail_t detail ;
                _async->render( std::move(config), detail, std::move(res) ) ;
                return *this ;
            }

            this_ref_t render( motor::graphics::render_object_mtr_shared_t config, motor::graphics::backend::render_detail_cref_t detail, 
                motor::graphics::result_mtr_shared_t res = nullptr ) noexcept
            {
                _async->render( std::move(config), detail, std::move(res) ) ;
                return *this ;
            }

        public:

        };
        motor_typedef( async_view ) ;


        class MOTOR_GRAPHICS_API async_views
        {
            motor_this_typedefs( async_views ) ;

            motor_typedefs( motor::vector< async_view_t >, asyncs ) ;
            asyncs_t _asyncs ;

        public:

            async_views( void_t ) noexcept {}
            async_views( this_cref_t rhv ) noexcept
            {
                _asyncs = rhv._asyncs ;
            }
            async_views( this_rref_t rhv ) noexcept
            {
                _asyncs = std::move( rhv._asyncs ) ;
            }
            async_views( asyncs_t asyncs ) noexcept
            {
                _asyncs = std::move( asyncs ) ;
            }
            ~async_views( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _asyncs = rhv._asyncs ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _asyncs = std::move( rhv._asyncs ) ;
                return *this ;
            }

        public:

            typedef std::function< void_t ( motor::graphics::async_view_t ) > for_each_async_t ;
            void_t for_each( for_each_async_t funk )
            {
                for( auto & a : _asyncs ) funk( a ) ;
            }
        };
        motor_typedef( async_views ) ;
    }
}
