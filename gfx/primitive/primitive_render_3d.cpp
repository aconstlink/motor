
#include "primitive_render_3d.h"

using namespace motor::gfx ;

//*****************************************************************
primitive_render_3d::primitive_render_3d( void_t ) 
{
   
}

//*****************************************************************
primitive_render_3d::primitive_render_3d( this_rref_t rhv ) 
{
    _lr = std::move( rhv._lr ) ;
    _tr = std::move( rhv._tr ) ;
}

//*****************************************************************
primitive_render_3d::~primitive_render_3d( void_t ) 
{
}

//*****************************************************************
void_t primitive_render_3d::init( motor::string_cref_t name ) noexcept 
{
    _name = name ;

    _lr = motor::gfx::line_render_3d_t() ;
    _lr.init( name + ".line_render" ) ;

    _tr = motor::gfx::tri_render_3d_t() ;
    _tr.init( name + ".tri_render" ) ;
}

//*****************************************************************
void_t primitive_render_3d::release( void_t ) noexcept 
{
    _lr.release() ;
    _tr.release() ;
}

//*****************************************************************
void_t primitive_render_3d::draw_line( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec4f_cref_t color ) noexcept
{
    _lr.draw( p0, p1, color ) ;
}

//*****************************************************************
void_t primitive_render_3d::draw_tri( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec3f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept 
{
    _tr.draw( p0, p1, p2, color ) ;
}

#if 0
//*****************************************************************
void_t primitive_render_3d::draw_rect(  motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec3f_cref_t p2, motor::math::vec3f_cref_t p3, motor::math::vec4f_cref_t color, 
    motor::math::vec4f_cref_t border_color ) noexcept 
{
    _tr.draw_rect( p0, p1, p2, p3, color ) ;
    _lr.draw_rect( p0, p1, p2, p3, border_color ) ;
}
#endif

//*****************************************************************
void_t primitive_render_3d::draw_circle( motor::math::mat3f_cref_t o, motor::math::vec3f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color, 
    motor::math::vec4f_cref_t border_color, size_t const s) noexcept 
{
    _tr.draw_circle( o, p0, r, color, s ) ;
    //_lr.draw_circle( s, p0, r, border_color ) ;
}

//*****************************************************************
void_t primitive_render_3d::configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    _lr.configure( fe ) ;
    _tr.configure( fe ) ;
}

//*****************************************************************
void_t primitive_render_3d::prepare_for_rendering( void_t ) noexcept 
{
    _lr.prepare_for_rendering() ;
    _tr.prepare_for_rendering() ;
}

//*****************************************************************
void_t primitive_render_3d::prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    _lr.prepare_for_rendering( fe ) ;
    _tr.prepare_for_rendering( fe ) ;
}

//*****************************************************************
void_t primitive_render_3d::render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    _lr.render( fe ) ;
    _tr.render( fe ) ;
}

//*****************************************************************
void_t primitive_render_3d::set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept 
{
    _tr.set_view_proj( view, proj ) ;
    _lr.set_view_proj( view, proj ) ;
}