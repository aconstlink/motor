
#include "xaudio2.h"

#include <xaudio2.h>

using namespace motor::audio ;

struct motor::audio::xaudio2_backend::pimpl
{
    pimpl( void_t )
    {
    }

    ~pimpl( void_t )
    {
    }

    bool_t init( void_t ) noexcept
    {
        return true ;
    }

    void_t release( void_t ) 
    {
        
    }

    size_t construct_buffer_object( size_t oid, motor::string_cref_t name, motor::audio::buffer_object_ref_t )
    {
        return oid ;
    }

    void_t release_buffer_object( size_t const oid ) noexcept
    {
    }

    bool_t update( size_t const oid, motor::audio::buffer_object_ref_t buffer ) noexcept
    {
        return true ;
    }

    void_t execute( size_t const oid, motor::audio::buffer_object_ref_t obj, 
        motor::audio::backend::execute_detail_cref_t det ) noexcept
    {
    }

    void_t update( void_t ) noexcept
    {
        // stuff can be done here per frame
        // track state of playing sources
    }
} ;
//**************************************************************************
xaudio2_backend::xaudio2_backend( void_t ) noexcept
{
}

//**************************************************************************
xaudio2_backend::xaudio2_backend( this_rref_t rhv ) noexcept : backend( std::move( rhv ) )
{
    motor_move_member_ptr( _pimpl, rhv ) ;
}

//**************************************************************************
xaudio2_backend::~xaudio2_backend( void_t ) noexcept 
{
    this_t::release() ;
}

//**************************************************************************
xaudio2_backend::this_ref_t xaudio2_backend::operator = ( this_rref_t rhv ) noexcept 
{
    motor_move_member_ptr( _pimpl, rhv ) ;
    return *this ;
}

//**************************************************************************
motor::audio::result xaudio2_backend::configure( motor::audio::capture_type const, 
    motor::audio::capture_object_mtr_t ) noexcept 
{
    return motor::audio::result::ok ;
}

//**************************************************************************
motor::audio::result xaudio2_backend::capture( motor::audio::capture_object_mtr_t, bool_t const b ) noexcept 
{
    return motor::audio::result::ok ;
}

//**************************************************************************
motor::audio::result xaudio2_backend::release( motor::audio::capture_object_mtr_t ) noexcept 
{
    return motor::audio::result::ok ;
}

//**************************************************************************
motor::audio::result xaudio2_backend::configure( motor::audio::buffer_object_mtr_t ) noexcept 
{
    return motor::audio::result::ok ;
}

//**************************************************************************
motor::audio::result xaudio2_backend::update( motor::audio::buffer_object_mtr_t ) noexcept 
{
    return motor::audio::result::ok ;
}

//**************************************************************************
motor::audio::result xaudio2_backend::execute( motor::audio::buffer_object_mtr_t, motor::audio::backend::execute_detail_cref_t ) noexcept 
{
    return motor::audio::result::ok ;
}

//**************************************************************************
motor::audio::result xaudio2_backend::release( motor::audio::buffer_object_mtr_t ) noexcept 
{
    return motor::audio::result::ok ;
}

//**************************************************************************
void_t xaudio2_backend::init( void_t ) noexcept 
{
    if( _pimpl == nullptr )
    {
        _pimpl = motor::memory::global_t::alloc( pimpl(), "[motor::audio::oal_backend::pimpl]" ) ;
        _pimpl->init() ;
    }
}

//**************************************************************************
void_t xaudio2_backend::release( void_t ) noexcept 
{
    if( _pimpl != nullptr )
    {
        _pimpl->release() ;
    }
    motor::memory::global_t::dealloc( _pimpl ) ;
    _pimpl = nullptr ;
}

//**************************************************************************
void_t xaudio2_backend::begin( void_t ) noexcept 
{
    _pimpl->update() ;
}

//**************************************************************************
void_t xaudio2_backend::end( void_t ) noexcept 
{
}

//**************************************************************************
size_t xaudio2_backend::create_backend_id(  void_t ) noexcept 
{
    static size_t const id = size_t( motor::audio::backend_type::xaudio2 ) ;
    return id ;
}

//**************************************************************************