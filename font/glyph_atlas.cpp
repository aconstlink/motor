#include "glyph_atlas.h"

#include <cstring>

using namespace motor::font ;

//*******************************************************************
glyph_atlas::image::image( void_t ) noexcept {}

//*******************************************************************
glyph_atlas::image::image( size_t width, size_t height ) noexcept : _width( width ), _height( height )
{
    _plane_ptr = motor::memory::global_t::alloc_raw<uint8_t>( width * height ) ;
    std::memset( ( void_ptr_t ) _plane_ptr, 0, width * height * sizeof( uint8_t ) ) ;
}

//*******************************************************************
glyph_atlas::image::image( this_rref_t rhv ) noexcept
{
    motor_move_member_ptr( _plane_ptr, rhv ) ;
    _width = rhv._width ;
    rhv._width = 0 ;
    _height = rhv._height ;
    rhv._height = 0 ;
}

//*******************************************************************
glyph_atlas::image::~image( void_t ) noexcept
{
    motor::memory::global_t::dealloc_raw<uint8_t>( _plane_ptr ) ;
}

//*******************************************************************
glyph_atlas::image::this_ref_t glyph_atlas::image::operator = ( this_rref_t rhv ) noexcept
{
    motor_move_member_ptr( _plane_ptr, rhv ) ;
    _width = rhv._width ;
    rhv._width = 0 ;
    _height = rhv._height ;
    rhv._height = 0 ;
    return *this ;
}

//*******************************************************************
glyph_atlas::glyph_atlas( void_t )
{

}

//*******************************************************************
glyph_atlas::glyph_atlas( this_rref_t rhv )
{
    _glyph_infos = std::move( rhv._glyph_infos ) ;
    _atlas = std::move( rhv._atlas ) ;
    _face_name_to_id = std::move( rhv._face_name_to_id ) ;
}

//*******************************************************************
glyph_atlas::~glyph_atlas( void_t )
{
    for( auto * iptr : _atlas )
        motor::memory::global_t::dealloc( iptr ) ;
}

//*******************************************************************
glyph_atlas::this_ref_t glyph_atlas::operator = ( this_rref_t rhv ) noexcept 
{
    _glyph_infos = std::move( rhv._glyph_infos ) ;
    _atlas = std::move( rhv._atlas ) ;
    _face_name_to_id = std::move( rhv._face_name_to_id ) ;
    return *this ;
}

//*******************************************************************
bool_t glyph_atlas::add_glyph( glyph_info_cref_t gi )
{
    utf32_t const cp = gi.code_point ;
    size_t const fid = gi.face_id ;

    // check if already in glyph infos
    {
        auto const iter = std::find_if( _glyph_infos.begin(), _glyph_infos.end(), 
            [&]( glyph_info_cref_t rhv )
        {
            return rhv.code_point == cp && rhv.face_id == fid ;
        } ) ;

        if( iter != _glyph_infos.end() )
            return false ;
    }

    _glyph_infos.push_back( gi ) ;

    return true ;
}

//*******************************************************************
bool_t glyph_atlas::add_image( this_t::image_rref_t img )
{
    _atlas.push_back( motor::memory::global_t::alloc( std::move(img), "[glyph_atlas::add_image]" ) ) ;
    return true ;
}

//*******************************************************************
void_t glyph_atlas::map_face_id( motor::string_cref_t name, size_t const id ) noexcept 
{
    auto iter = std::find_if( _face_name_to_id.begin(), _face_name_to_id.end(), [&]( name_id_t const & nid )
    {
        return nid.first == name ;
    } ) ;

    if( iter != _face_name_to_id.end() ) 
    {
        iter->second = id ;
        return ;
    }

    _face_name_to_id.emplace_back( std::make_pair( name, id ) ) ;
}

//*******************************************************************
bool_t glyph_atlas::face_id_for_name( motor::string_cref_t name, size_t & id ) const noexcept 
{
    auto iter = std::find_if( _face_name_to_id.begin(), _face_name_to_id.end(), [&]( name_id_t const & nid )
    {
        return nid.first == name ;
    } ) ;

    if( iter != _face_name_to_id.end() )
    {
        id = iter->second ;
        return true ;
    }

    return false ;
}

//*******************************************************************
size_t glyph_atlas::get_num_glyphs( void_t ) const
{
    return _glyph_infos.size() ;
}

//*******************************************************************
bool_t glyph_atlas::get_glyph_info( size_t const i, glyph_info_out_t gi ) const
{
    if( i >= _glyph_infos.size() ) return false ;
    gi = _glyph_infos[ i ] ;
    return true ;
}
//*******************************************************************
size_t glyph_atlas::get_num_images( void_t ) const
{
    return _atlas.size() ;
}

//*******************************************************************
bool_t glyph_atlas::find_glyph( size_t const fid, utf32_t const cp, 
    size_t & i_out, this_t::glyph_info_ref_t gi_out ) const
{
    auto const iter = std::find_if( _glyph_infos.begin(), _glyph_infos.end(), 
        [&]( glyph_info_cref_t gi_in )
    {
        return gi_in.face_id == fid && gi_in.code_point == cp ;
    } ) ;

    if( iter == _glyph_infos.end() )
        return false ;

    i_out = iter - _glyph_infos.begin() ;
    gi_out = *iter ;

    return true ;
}

//*******************************************************************
glyph_atlas::image_cptr_t glyph_atlas::get_image( size_t const id ) const
{
    if( id >= _atlas.size() ) return nullptr ;
    return _atlas[ id ] ;
}

//*******************************************************************
size_t glyph_atlas::get_width( void_t ) const
{
    return _atlas.size() > 0 ? _atlas[ 0 ]->width() : 0 ;
}

//*******************************************************************
size_t glyph_atlas::get_height( void_t ) const
{
    return _atlas.size() > 0 ? _atlas[ 0 ]->height() : 0 ;
}