
#include "primitive_render_2d.h"

using namespace motor::gfx ;

primitive_render_2d::primitive_render_2d( void_t ) 
{
   
}

primitive_render_2d::primitive_render_2d( this_rref_t rhv ) 
{
    _lr = std::move( rhv._lr ) ;
    _tr = std::move( rhv._tr ) ;
}
            
primitive_render_2d::~primitive_render_2d( void_t ) 
{
}

void_t primitive_render_2d::init( motor::string_cref_t name, motor::graphics::async_views_t asyncs ) noexcept 
{
    _name = name ;
    _asyncs = asyncs ;

    _lr = motor::gfx::line_render_2d_res_t( motor::gfx::line_render_2d_t() ) ;
    _lr->init( name + ".line_render", _asyncs ) ;

    _tr = motor::gfx::tri_render_2d_res_t( motor::gfx::tri_render_2d_t() ) ;
    _tr->init( name + ".tri_render", _asyncs ) ;
}

void_t primitive_render_2d::release( void_t ) noexcept 
{
}

void_t primitive_render_2d::draw_line( size_t const l, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, motor::math::vec4f_cref_t color ) noexcept
{
    _lr->draw( l, p0, p1, color ) ;
}

void_t primitive_render_2d::draw_tri( size_t const l, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, motor::math::vec2f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept 
{
    _tr->draw( l, p0, p1, p2, color ) ;
}

void_t primitive_render_2d::draw_rect( size_t const l, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, motor::math::vec2f_cref_t p2, motor::math::vec2f_cref_t p3, motor::math::vec4f_cref_t color, 
    motor::math::vec4f_cref_t border_color ) noexcept 
{
    auto const layer = l << 1 ;
    _tr->draw_rect( layer+0, p0, p1, p2, p3, color ) ;
    _lr->draw_rect( layer+1, p0, p1, p2, p3, border_color ) ;
}

void_t primitive_render_2d::draw_circle( size_t const l, size_t const s, motor::math::vec2f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color, motor::math::vec4f_cref_t border_color ) noexcept 
{
    auto const layer = l << 1 ;
    _tr->draw_circle( layer+0, s, p0, r, color ) ;
    _lr->draw_circle( layer+1, s, p0, r, border_color ) ;
}


void_t primitive_render_2d::prepare_for_rendering( void_t ) noexcept 
{
    _lr->prepare_for_rendering() ;
    _tr->prepare_for_rendering() ;
}

void_t primitive_render_2d::render( size_t const l ) noexcept 
{
    _lr->render( l ) ;
    _tr->render( l ) ;
}

void_t primitive_render_2d::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept 
{
    _tr->set_view_proj( view, proj ) ;
    _lr->set_view_proj( view, proj ) ;
}