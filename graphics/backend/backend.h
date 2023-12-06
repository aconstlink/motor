
#pragma once

#include "../api.h"
#include "../result.h"
#include "../protos.h"


#include "../id.hpp"
#include "../object/shader_object.h"
#include "../object/render_object.h"
#include "../object/geometry_object.h"
#include "../object/image_object.h"
#include "../object/framebuffer_object.h"
#include "../object/state_object.h"
#include "../object/array_object.h"
#include "../object/streamout_object.h"
#include "../variable/variable_set.hpp"
#include "../state/state_set.h"

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API backend
        {
            motor_this_typedefs( backend ) ;

        public:

            struct window_info
            {
                size_t width = 0 ;
                size_t height = 0 ;
            };
            motor_typedef( window_info ) ;
            virtual void_t set_window_info( window_info_cref_t ) noexcept = 0 ;

            motor::graphics::backend_type _bt = backend_type::unknown ;

            struct render_detail
            {
                size_t start = size_t( 0 ) ;
                size_t num_elems = size_t( -1 ) ;
                size_t varset = size_t( 0 ) ;
                size_t geo = size_t( 0 ) ;
                size_t state_set = size_t( -1 ) ;
                bool_t feed_from_streamout = false ;
                bool_t use_streamout_count = false ;
            };
            motor_typedef( render_detail ) ;

            enum class pop_type
            {
                render_state
            };

            enum class unuse_type
            {
                framebuffer,
                streamout
            };

        public: // user interface

            virtual motor::graphics::result configure( motor::graphics::geometry_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result configure( motor::graphics::render_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result configure( motor::graphics::shader_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result configure( motor::graphics::image_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result configure( motor::graphics::framebuffer_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result configure( motor::graphics::state_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result configure( motor::graphics::array_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result configure( motor::graphics::streamout_object_mtr_shared_t ) noexcept = 0 ;

            virtual motor::graphics::result release( motor::graphics::geometry_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result release( motor::graphics::render_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result release( motor::graphics::shader_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result release( motor::graphics::image_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result release( motor::graphics::framebuffer_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result release( motor::graphics::state_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result release( motor::graphics::array_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result release( motor::graphics::streamout_object_mtr_shared_t ) noexcept = 0 ;

            // @obsolete
            virtual motor::graphics::result connect( motor::graphics::render_object_mtr_shared_t, motor::graphics::variable_set_mtr_shared_t ) noexcept = 0 ;

            virtual motor::graphics::result update( motor::graphics::geometry_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result update( motor::graphics::array_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result update( motor::graphics::streamout_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result update( motor::graphics::image_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result update( motor::graphics::render_object_mtr_shared_t, size_t const varset ) noexcept = 0 ;

            virtual motor::graphics::result use( motor::graphics::framebuffer_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result use( motor::graphics::streamout_object_mtr_shared_t ) noexcept = 0 ;
            virtual motor::graphics::result unuse( motor::graphics::backend::unuse_type const ) noexcept = 0 ;
            virtual motor::graphics::result push( motor::graphics::state_object_mtr_shared_t, size_t const, bool_t const ) noexcept = 0 ;
            virtual motor::graphics::result pop( pop_type const ) noexcept = 0 ;

            virtual motor::graphics::result render( motor::graphics::render_object_mtr_shared_t, motor::graphics::backend::render_detail_cref_t ) noexcept = 0 ;

        public: // engine interface

            virtual void_t render_begin( void_t ) noexcept = 0 ;
            virtual void_t render_end( void_t ) noexcept = 0 ;

        public: // 

            backend( void_t ) noexcept ;
            backend( motor::graphics::backend_type const bt ) ;
            backend( this_rref_t ) noexcept ;
            virtual ~backend( void_t ) noexcept ;

            motor::graphics::backend_type get_type( void_t ) const noexcept { return _bt ; }

            static motor::graphics::render_state_sets_t default_render_states( void_t )
            {
                return motor::graphics::render_state_sets_t() ;
            }

        protected:

            size_t get_bid( void_t ) const { return size_t( _bt ) ; }
            void_t set_type( motor::graphics::backend_type const bt ) { _bt = bt ; }

        private:
            
            static size_t create_backend_id( void_t ) noexcept ;

        };
        motor_typedef( backend ) ;
    }
}