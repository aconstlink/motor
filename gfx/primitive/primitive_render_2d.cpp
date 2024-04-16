
#include "primitive_render_2d.h"

using namespace motor::gfx ;

//**********************************************************************************************************
primitive_render_2d::primitive_render_2d( void_t ) noexcept
{
   
}

//**********************************************************************************************************
primitive_render_2d::primitive_render_2d( this_rref_t rhv ) noexcept
{
    _lr = std::move( rhv._lr ) ;
    _tr = std::move( rhv._tr ) ;
}

//**********************************************************************************************************
primitive_render_2d::~primitive_render_2d( void_t ) noexcept
{
}

//**********************************************************************************************************
void_t primitive_render_2d::init( motor::string_cref_t name ) noexcept 
{
    _name = name ;

    _lr.init( name + ".line_render" ) ;
    _tr.init( name + ".tri_render" ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::release( void_t ) noexcept 
{
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_line( size_t const l, motor::math::vec2f_cref_t p0, 
    motor::math::vec2f_cref_t p1, motor::math::vec4f_cref_t color ) noexcept
{
    _lr.draw( l, p0, p1, color ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_lines( size_t const layer, size_t const num_lines, motor::gfx::line_render_2d::draw_lines_funk_t funk ) noexcept
{
    _lr.draw_lines( layer, num_lines, funk ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_tri( size_t const l, motor::math::vec2f_cref_t p0, 
    motor::math::vec2f_cref_t p1, motor::math::vec2f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept 
{
    _tr.draw( l, p0, p1, p2, color ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_circles( size_t const layer, size_t const segs,
    size_t const num_circles, this_t::draw_circles_funk_t funk ) noexcept
{
    motor::gfx::tri_render_2d::draw_circles_funk_t tri_funk = [&] ( size_t const i ) 
    {
        auto ret = funk( i ) ;
        return motor::gfx::tri_render_2d::circle_md_t { ret.pos, ret.radius, ret.color } ;
    } ;

    _tr.draw_circles( layer, segs, num_circles, tri_funk ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_circles_border( size_t const layer, size_t const segs,
    size_t const num_circles, this_t::draw_circles_funk_t funk ) noexcept
{
    motor::gfx::tri_render_2d::draw_circles_funk_t tri_funk = [&] ( size_t const i )
    {
        auto ret = funk( i ) ;
        return motor::gfx::tri_render_2d::circle_md_t { ret.pos, ret.radius, ret.color } ;
    } ;

    motor::gfx::line_render_2d::draw_circles_funk_t line_funk = [&] ( size_t const i )
    {
        auto ret = funk( i ) ;
        return motor::gfx::line_render_2d::circle_t { ret.pos, ret.radius, ret.border } ;
    } ;

    _tr.draw_circles( layer, segs, num_circles, tri_funk ) ;
    _lr.draw_circles( layer + 1, segs, num_circles, line_funk ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_tris( size_t const layer, size_t const num_tris, motor::gfx::tri_render_2d::draw_tris_funk_t funk ) noexcept
{
    _tr.draw_tris( layer, num_tris, funk ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_rect( size_t const l, motor::math::vec2f_cref_t p0, 
    motor::math::vec2f_cref_t p1, motor::math::vec2f_cref_t p2, motor::math::vec2f_cref_t p3, motor::math::vec4f_cref_t color, 
    motor::math::vec4f_cref_t border_color ) noexcept 
{
    auto const layer = l << 1 ;
    _tr.draw_rect( layer+0, p0, p1, p2, p3, color ) ;
    _lr.draw_rect( layer+1, p0, p1, p2, p3, border_color ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_rects( size_t const l, size_t const num_rects, this_t::draw_rects_funk_t funk ) noexcept
{
    motor::gfx::tri_render_2d::draw_rects_funk_t tri_funk = [&] ( size_t const i ) 
    {
        auto ret = funk( i ) ;
        return motor::gfx::tri_render_2d::rect_md_t {
            { ret.points[ 0 ], ret.points[ 1 ], ret.points[ 2 ], ret.points[ 3 ] }, ret.color } ;
    } ;

    _tr.draw_rects( l, num_rects, tri_funk ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_rects_border( size_t const l, size_t const num_rects, this_t::draw_rects_funk_t funk ) noexcept
{
    motor::gfx::tri_render_2d::draw_rects_funk_t tri_funk = [&] ( size_t const i )
    {
        auto ret = funk( i ) ;
        return motor::gfx::tri_render_2d::rect_md_t {
            { ret.points[ 0 ], ret.points[ 1 ], ret.points[ 2 ], ret.points[ 3 ] }, ret.color } ;
    } ;

    motor::gfx::line_render_2d::draw_rects_funk_t line_funk = [&] ( size_t const i )
    {
        auto ret = funk( i ) ;
        return motor::gfx::line_render_2d::rect_t {
            { ret.points[ 0 ], ret.points[ 1 ], ret.points[ 2 ], ret.points[ 3 ] }, ret.border } ;
    } ;

    _tr.draw_rects( l, num_rects, tri_funk ) ;
    _lr.draw_rects( l + 1, num_rects, line_funk ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::draw_circle( size_t const l, size_t const s, motor::math::vec2f_cref_t p0, 
    float_t const r, motor::math::vec4f_cref_t color, motor::math::vec4f_cref_t border_color ) noexcept 
{
    auto const layer = l << 1 ;
    _tr.draw_circle( layer+0, s, p0, r, color ) ;
    _lr.draw_circle( layer+1, s, p0, r, border_color ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    _lr.configure( fe ) ;
    _tr.configure( fe ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::prepare_for_rendering( void_t ) noexcept 
{
    _lr.prepare_for_rendering() ;
    _tr.prepare_for_rendering() ;
}

//**********************************************************************************************************
void_t primitive_render_2d::prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    _lr.prepare_for_rendering( fe ) ;
    _tr.prepare_for_rendering( fe ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::render( motor::graphics::gen4::frontend_mtr_t fe, size_t const l ) noexcept 
{
    _lr.render( fe, l ) ;
    _tr.render( fe, l ) ;
}

//**********************************************************************************************************
void_t primitive_render_2d::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept 
{
    _tr.set_view_proj( view, proj ) ;
    _lr.set_view_proj( view, proj ) ;
}