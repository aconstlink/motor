

#include "null.h"

using namespace motor::graphics ;
using namespace motor::graphics::gen4 ;

//***
null_backend::null_backend( void_t ) noexcept : base_t( size_t(-1) ){}
null_backend::null_backend( this_rref_t rhv ) noexcept : backend( std::move( rhv ) ){}
null_backend::~null_backend( void_t ) noexcept {}

//****
void_t null_backend::set_window_info( window_info_cref_t ) noexcept
{
}

//***
motor::graphics::result null_backend::configure( motor::graphics::geometry_object_mtr_t ) noexcept
{
    //static size_t number = 0 ;
    return motor::graphics::result::ok ;
}

//***
motor::graphics::result null_backend::configure( motor::graphics::render_object_mtr_t ) noexcept 
{
    //static size_t number = 0 ;
    return motor::graphics::result::ok ;
}

//***
motor::graphics::result null_backend::configure( motor::graphics::shader_object_mtr_t ) noexcept 
{
    //static size_t number = 0 ;
    return motor::graphics::result::ok ;
}

//***
motor::graphics::result null_backend::configure( motor::graphics::image_object_mtr_t ) noexcept 
{
    // static size_t number = 0 ;
    return motor::graphics::result::ok ;
}

//***
motor::graphics::result null_backend::configure( motor::graphics::framebuffer_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::configure( motor::graphics::state_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::configure( motor::graphics::array_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::configure( motor::graphics::streamout_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::msl_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::geometry_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::render_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::shader_object_mtr_t ) noexcept
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::image_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::framebuffer_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::state_object_mtr_t ) noexcept
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::array_object_mtr_t ) noexcept
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::release( motor::graphics::streamout_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

//***
motor::graphics::result null_backend::update( motor::graphics::geometry_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::update( motor::graphics::array_object_mtr_t ) noexcept
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::update( motor::graphics::streamout_object_mtr_t ) noexcept
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::update( motor::graphics::image_object_mtr_t ) noexcept
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::update( motor::graphics::render_object_mtr_t, size_t const /*varset*/ ) noexcept 
{
    return motor::graphics::result::ok ;
}

//****
motor::graphics::result null_backend::use( motor::graphics::framebuffer_object_mtr_t ) noexcept
{
    return motor::graphics::result::ok ;
}

motor::graphics::result null_backend::use( motor::graphics::streamout_object_mtr_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

//****
motor::graphics::result null_backend::unuse( motor::graphics::gen4::backend::unuse_type const ) noexcept
{
    return motor::graphics::result::ok ;
}

//****
motor::graphics::result null_backend::push( motor::graphics::state_object_mtr_t, size_t const, bool_t const ) noexcept 
{
    return motor::graphics::result::ok ;
}

//****
motor::graphics::result null_backend::pop( pop_type const ) noexcept 
{
    return motor::graphics::result::ok ;
}

//***
motor::graphics::result null_backend::render( motor::graphics::render_object_mtr_t, motor::graphics::gen4::backend::render_detail_cref_t ) noexcept 
{
    return motor::graphics::result::ok ;
}

//***
void_t null_backend::render_begin( void_t ) noexcept 
{
}

//***
void_t null_backend::render_end( void_t ) noexcept 
{
}
