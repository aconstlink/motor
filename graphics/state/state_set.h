#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "render_states.h"
#include <motor/math/vector/vector4.hpp>

namespace motor
{
    namespace graphics
    {
        template< typename T >
        struct state_set
        {
            motor_this_typedefs( state_set< T > ) ;
            bool_t do_change = false ;
            T ss ;

            state_set( void_t ) {}
            state_set( this_cref_t rhv ) noexcept 
            {
                do_change = rhv.do_change ;
                ss = rhv.ss ;
            }
            state_set( this_rref_t rhv ) noexcept 
            {
                do_change = rhv.do_change ;
                ss = std::move( rhv.ss ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                do_change = rhv.do_change ;
                ss = rhv.ss ;
                return *this ;
            }

            this_ref_t operator += ( this_cref_t rhv ) noexcept
            {
                if( rhv.do_change ) *this = rhv ;
                return *this ;
            }

            this_t operator + ( this_cref_t rhv ) const noexcept
            {
                return rhv.do_change ? rhv : this_t( *this ) ;
            }

            // this  |  other
            //   0   |    0  -> do nothing
            //   1   |    0  -> negate effect of this
            //   0   |    1  -> do nothing
            //   1   |    1  -> do change to other
            this_t operator - ( this_cref_t rhv ) const noexcept
            {
                this_t ret ;

                if( do_change && !rhv.do_change )
                {
                    ret = *this ;
                    ret.ss.do_activate = !ret.ss.do_activate ;
                }
                else if( do_change && rhv.do_change )
                {
                    ret = rhv ;
                }

                return ret ;
            }
        };

        struct clear_states
        {
            motor::math::vec4f_t clear_color = motor::math::vec4f_t( 0.4f, 0.1f, 0.2f, 1.0f ) ;
            bool_t do_activate = false ;
            bool_t do_color_clear = false ;
            bool_t do_depth_clear = false ;
        };
        motor_typedefs( state_set< clear_states >, clear_states_set ) ;

        struct blend_states
        {
            bool_t do_activate = false ;
            blend_factor src_blend_factor = motor::graphics::blend_factor::one ;
            blend_factor dst_blend_factor = motor::graphics::blend_factor::zero ;
            blend_function blend_func = motor::graphics::blend_function::add ;

            // separate section
            /*
            motor::math::vec4f_t constant_blend_color = motor::math::vec4f_t( 0.0f ) ;

            bool_t blend_factor_separate = false ;
            
            blend_factor src_color_blend_factor = motor::graphics::blend_factor::one ;
            blend_factor dst_color_blend_factor = motor::graphics::blend_factor::zero ;
            blend_factor src_alpha_blend_factor = motor::graphics::blend_factor::one ;
            blend_factor dst_alpha_blend_factor = motor::graphics::blend_factor::zero ;


            bool_t blend_func_separate = false ;
            
            blend_function color_blend_func = motor::graphics::blend_function::add ;
            blend_function alpha_blend_func = motor::graphics::blend_function::add ;*/
        };
        motor_typedefs( state_set< blend_states >, blend_states_set ) ;

        struct depth_states
        {
            bool_t do_activate = false ;
            bool_t do_depth_write = true ;

            // depth buffer test func
        };
        motor_typedefs( state_set< depth_states >, depth_states_set ) ;

        struct stencil_states
        {
            bool_t do_activate = false ;
            // missing
        };
        motor_typedefs( state_set< stencil_states >, stencil_states_set ) ;

        struct scissor_states
        {
            bool_t do_activate = false ;
            motor::math::vec4ui_t rect ;
        };
        motor_typedefs( state_set< scissor_states >, scissor_states_set ) ;

        struct polygon_states
        {
            bool_t do_activate = false ;

            cull_mode cm = motor::graphics::cull_mode::back ;
            front_face ff = motor::graphics::front_face::clock_wise ;
            fill_mode fm = motor::graphics::fill_mode::fill ;
        };
        motor_typedefs( state_set< polygon_states >, polygon_states_set ) ;

        struct viewport_states
        {
            bool_t do_activate = false ;

            // x, y, width, height
            motor::math::vec4ui_t vp = motor::math::vec4ui_t( 0, 0, 0, 0 ) ;
            
            // depth range
        };
        motor_typedefs( state_set< viewport_states >, viewport_states_set ) ;

        // something that can be turned on/off
        struct render_state_sets
        {
            motor_this_typedefs( render_state_sets ) ;

            clear_states_set_t clear_s ;
            viewport_states_set_t view_s ;
            blend_states_set_t blend_s ;
            depth_states_set_t depth_s ;
            stencil_states_set_t stencil_s ;
            polygon_states_set_t polygon_s ;
            scissor_states_set_t scissor_s ;

            render_state_sets( void_t ) {}

            render_state_sets( this_cref_t rhv ) noexcept
            {
                view_s = rhv.view_s ;
                blend_s = rhv.blend_s ;
                depth_s = rhv.depth_s ;
                stencil_s = rhv.stencil_s ;
                polygon_s = rhv.polygon_s ;
                scissor_s = rhv.scissor_s ;
                clear_s = rhv.clear_s ;
            }

            render_state_sets( this_rref_t rhv ) noexcept
            {
                view_s = std::move( rhv.view_s ) ;
                blend_s = std::move( rhv.blend_s ) ;
                depth_s = std::move( rhv.depth_s ) ;
                stencil_s = std::move( rhv.stencil_s ) ;
                polygon_s = std::move( rhv.polygon_s ) ;
                scissor_s = std::move( rhv.scissor_s ) ;
                clear_s = std::move( rhv.clear_s ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                view_s = rhv.view_s ;
                blend_s = rhv.blend_s ;
                depth_s = rhv.depth_s ;
                stencil_s = rhv.stencil_s ;
                polygon_s = rhv.polygon_s ;
                scissor_s = rhv.scissor_s ;
                clear_s = rhv.clear_s ;

                return *this ;
            }

            this_ref_t operator += ( this_cref_t rhv ) noexcept
            {
                blend_s += rhv.blend_s ;
                depth_s += rhv.depth_s ;
                stencil_s += rhv.stencil_s ;
                polygon_s += rhv.polygon_s ;
                scissor_s += rhv.scissor_s ;
                view_s += rhv.view_s ;
                clear_s += rhv.clear_s ;

                return *this ;
            }

            // returns a new state set where rhv is added to this.
            // adding render states means only changed states are
            // assigned(added).
            this_t operator + ( this_cref_t rhv ) const noexcept
            {
                this_t ret = *this ;

                
                ret.view_s = this_t::view_s + rhv.view_s ;
                ret.blend_s = this_t::blend_s + rhv.blend_s ;
                ret.depth_s = this_t::depth_s + rhv.depth_s ;
                ret.stencil_s = this_t::stencil_s + rhv.stencil_s ;
                ret.polygon_s = this_t::polygon_s + rhv.polygon_s ;
                ret.scissor_s = this_t::scissor_s + rhv.scissor_s ;
                
                // clear state needs special handling
                // there will only be a clear if the incoming
                // state will clear
                {
                    ret.clear_s = this_t::clear_s + rhv.clear_s ;
                    ret.clear_s.do_change = rhv.clear_s.do_change ;
                }

                return this_t( std::move( ret ) ) ;
            }

            this_t operator - ( this_cref_t rhv ) const noexcept
            {
                this_t ret = *this ;
                
                ret.view_s = this_t::view_s - rhv.view_s ;
                ret.blend_s = this_t::blend_s - rhv.blend_s ;
                ret.depth_s = this_t::depth_s - rhv.depth_s ;
                ret.stencil_s = this_t::stencil_s - rhv.stencil_s ;
                ret.polygon_s = this_t::polygon_s - rhv.polygon_s ;
                ret.scissor_s = this_t::scissor_s - rhv.scissor_s ;

                // clear state needs special handling
                // never recover the clear state
                {
                    ret.clear_s.do_change = false ;
                }

                return this_t( std::move( ret ) ) ;
            }


        };
        motor_typedef( render_state_sets ) ;
    }
}
