
#pragma once

#include "../../render_engine.h"
#include "../ifrontend.h"
#include "../../backend/gen4/backend.h"

namespace motor
{
    namespace graphics
    {
        namespace gen4
        {
            class MOTOR_GRAPHICS_API frontend : public motor::graphics::ifrontend
            {
                motor_this_typedefs( frontend ) ;

            private:

                motor::graphics::render_engine_ptr_t _re ;
                motor::graphics::gen4::backend_ptr_t _be ;

            public:

                frontend( motor::graphics::render_engine_ptr_t re, motor::graphics::gen4::backend_ptr_t be ) noexcept :
                    _re(re), _be(be){}

                frontend( this_cref_t ) = delete ;
                frontend( this_rref_t rhv ) noexcept
                {
                    _re = motor::move( rhv._re ) ;
                    _be = motor::move( rhv._be ) ;
                }
                virtual ~frontend( void_t ) noexcept ;

            public:

                this_ref_t configure( motor::graphics::state_object_mtr_delay_t so ) noexcept 
                {
                    _re->send_execute( [=]( void_t )
                    {
                        _be->configure( so.mtr() ) ;
                    } ) ;

                    return *this ;
                }

                this_ref_t push( motor::graphics::state_object_mtr_delay_t so, size_t const sid = 0, bool_t const push = true ) noexcept 
                {
                    _re->send_execute( [=]( void_t )
                    {
                        _be->push( so.mtr(), sid, push ) ;
                    } ) ;

                    return *this ;
                }
                
                this_ref_t pop( motor::graphics::gen4::backend::pop_type const pt ) noexcept 
                {
                    _re->send_execute( [=]( void_t )
                    {
                        _be->pop( pt ) ;
                    } ) ;

                    return *this ;
                }

                void_t force_clear_color( motor::math::vec4f_cref_t color ) noexcept 
                {
                    _re->send_execute( [=]( void_t ){} ) ;
                }

                void_t force_context_swap( void_t ) noexcept
                {
                    _re->send_execute( [=]( void_t ){} ) ;
                }

            };
            motor_typedef( frontend ) ;
        }
    }
}