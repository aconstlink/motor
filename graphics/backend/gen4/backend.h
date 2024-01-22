
#pragma once

#include "types.h"

#include "../../api.h"
#include "../../result.h"
#include "../../protos.h"
#include "../bid_generator.h"

#include "../../object/msl_object.h"
#include "../../object/shader_object.h"
#include "../../object/render_object.h"
#include "../../object/geometry_object.h"
#include "../../object/image_object.h"
#include "../../object/framebuffer_object.h"
#include "../../object/state_object.h"
#include "../../object/array_object.h"
#include "../../object/streamout_object.h"
#include "../../variable/variable_set.hpp"
#include "../../state/state_set.h"

namespace motor
{
    namespace graphics
    {
        namespace gen4
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

                struct render_detail
                {
                    size_t start = size_t( 0 ) ;
                    size_t num_elems = size_t( -1 ) ;
                    size_t varset = size_t( 0 ) ;
                    size_t geo = size_t( 0 ) ;
                    size_t state_set = size_t( -1 ) ;
                    bool_t feed_from_streamout = false ;
                    bool_t use_streamout_count = false ;

                    // only used if msl is rendered
                    size_t ro_idx = size_t( 0 ) ;
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

                virtual motor::graphics::result configure( motor::graphics::msl_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::geometry_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::render_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::shader_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::image_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::framebuffer_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::state_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::array_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result configure( motor::graphics::streamout_object_mtr_t ) noexcept = 0 ;

                virtual motor::graphics::result release( motor::graphics::geometry_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result release( motor::graphics::render_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result release( motor::graphics::shader_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result release( motor::graphics::image_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result release( motor::graphics::framebuffer_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result release( motor::graphics::state_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result release( motor::graphics::array_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result release( motor::graphics::streamout_object_mtr_t ) noexcept = 0 ;

                virtual motor::graphics::result update( motor::graphics::geometry_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result update( motor::graphics::array_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result update( motor::graphics::streamout_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result update( motor::graphics::image_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result update( motor::graphics::render_object_mtr_t, size_t const varset ) noexcept = 0 ;

                virtual motor::graphics::result use( motor::graphics::framebuffer_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result use( motor::graphics::streamout_object_mtr_t ) noexcept = 0 ;
                virtual motor::graphics::result unuse( motor::graphics::gen4::backend::unuse_type const ) noexcept = 0 ;
                virtual motor::graphics::result push( motor::graphics::state_object_mtr_t, size_t const, bool_t const ) noexcept = 0 ;
                virtual motor::graphics::result pop( pop_type const ) noexcept = 0 ;

                virtual motor::graphics::result render( motor::graphics::render_object_mtr_t, motor::graphics::gen4::backend::render_detail_cref_t ) noexcept = 0 ;
                virtual motor::graphics::result render( motor::graphics::msl_object_mtr_t, motor::graphics::gen4::backend::render_detail_cref_t ) noexcept = 0 ;

            public: // engine interface

                virtual void_t render_begin( void_t ) noexcept = 0 ;
                virtual void_t render_end( void_t ) noexcept = 0 ;

                static motor::graphics::render_state_sets_t default_render_states( void_t )
                {
                    return motor::graphics::render_state_sets_t() ;
                }

            public:
                
                backend( void_t ) noexcept : _bid( motor::graphics::bid_gen.generate() ) {}

                // can override bid. Only use if bid is unique and known
                backend( size_t const bid ) noexcept : _bid( bid ) {}
                backend( this_rref_t rhv ) noexcept : _bid( rhv._bid ) 
                {
                    rhv._bid = size_t(-1) ;
                }

                virtual ~backend( void_t ) noexcept {}

                size_t get_bid( void_t ) const noexcept { return _bid ; }

            private:

                size_t _bid ; 

            };
            motor_typedef( backend ) ;
        }
    }
}