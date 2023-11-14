
#include "store_logger.h"

using namespace motor::log ;

//********************************************************************
store_logger::store_logger( void_t ) noexcept : _stores( this_t::stores_t::allocator_type(
    "[log::store_logger] : stored messages") )
{
    _stores.resize( max_elems ) ;
}

//********************************************************************
store_logger::store_logger( this_rref_t rhv ) noexcept
{
    _stores = std::move( rhv._stores ) ;
    _cp_start = rhv._cp_start ;
    _cp_end = rhv._cp_end ;
}

//********************************************************************
store_logger::this_ref_t store_logger::operator = ( this_rref_t rhv ) noexcept
{
    _stores = std::move( rhv._stores ) ;
    _cp_start = rhv._cp_start ;
    _cp_end = rhv._cp_end ;
    return *this ;
}

//********************************************************************
store_logger::~store_logger( void_t ) noexcept
{}

//********************************************************************
motor::log::result store_logger::log( motor::log::log_level const ll, motor::core::string_cref_t msg ) noexcept
{
    this_t::store_data_t sd ;
    sd.ll = ll ;
    sd.msg = msg ;

    std::lock_guard< ::std::mutex > lk( _mtx ) ;

    ++_cp_end ;

    _cp_end = _cp_end >= max_elems ? 0 : _cp_end ;
    _cp_start = _cp_start >= _cp_end ? _cp_end + 1 : _cp_start ;

    _cp_start = _cp_start > _stores.size() ? 0 : _cp_start ;

    //_stores[ _cp_end ] = sd ;
    _stores.emplace_back( sd ) ;

    return motor::log::result::ok ;
}

//********************************************************************
size_t store_logger::get_max_items( void_t ) const noexcept
{
    return max_elems ;
}

//********************************************************************
size_t store_logger::get_num_items( void_t ) const noexcept
{
    return _cp_end > _cp_start ? _cp_end - _cp_start : _cp_start - _cp_end ;
}

//********************************************************************
void_t store_logger::for_each( size_t const begin, size_t const end, funk_t f ) const noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;

    auto const a = _cp_start + begin ;
    auto const b = _cp_start + end ;

    for( size_t i = a; i < b; ++i )
    {
        f( _stores[ i % _stores.size() ] ) ;
    }
}

//********************************************************************
void_t store_logger::copy( size_t const begin, size_t const end, stores_ref_t sout ) const noexcept
{
    auto const a = _cp_start + begin ;
    auto const b = _cp_start + end ;

    sout.resize( b - a ) ;

    for( size_t i = a; i < b; ++i )
    {
        sout[i] = _stores[ i ]  ;
    }
}