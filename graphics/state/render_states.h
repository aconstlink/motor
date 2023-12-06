
#pragma once

namespace motor
{
    namespace graphics
    {
        enum class blend_factor
        {
            zero,
            one,
            src_color,
            one_minus_src_color,
            dest_color,
            one_minus_dst_color,
            src_alpha,
            one_minus_src_alpha,
            dst_alpha,
            one_minus_dst_alpha,
            const_color,
            one_minus_const_color,
            const_alpha,
            one_minus_cost_alpha,
            src_alpha_saturate
        } ;

        enum class blend_function
        {
            add,
            sub,
            reverse_sub,
            min,
            max
            //,logic_op
        } ;

        enum class clear_mode
        {
            color_buffer = 1,
            depth_buffer = 2,
            stencil_buffer = 4
        } ;

        enum class cull_mode
        {
            front,
            back,
            none
        } ;

        enum class fill_mode
        {
            point,
            line,
            fill
        } ;

        enum class front_face
        {
            clock_wise,
            counter_clock_wise
        } ;

        enum class render_state
        {
            undefined,
            clearscreen, // ?
            cullmode,
            frontface,
            fillmode,
            blend,
            depth_test,
            depth_write,
            scissor_test
        } ;
    }
}