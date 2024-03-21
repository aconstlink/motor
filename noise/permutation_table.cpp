
#include "permutation_table.h"

using namespace motor::noise ;

//***********************************************************************************
permutation_table::permutation_table( uint_t const seed, uint_t const bit, uint_t num_mixes = 1 ) noexcept
{
    num_mixes = std::min( std::max( 0u, num_mixes), 10u ) ;
    _seed = seed == 0 ? 6273 : seed ;
    _bit = bit < 3 ? 8 : bit ;

    _upper_bound = ( 1 << _bit ) - 1 ;

    _indices.resize( this_t::get_num_entries() ) ;

    srand( _seed ) ;
            
    for( uint_t i=0; i<this_t::get_num_entries(); ++i )
    {
        _indices[i] = i ;
    }

    // swap 
    // values by real index with 
    // values by random index.
    for( uint_t mix=0; mix<num_mixes; ++mix )
    {
        for( size_t i=0; i<this_t::get_num_entries(); ++i )
        {
            uint_t const prn = rand() & _upper_bound ;
            uint_t const index = _indices[i] ;
            _indices[i] = _indices[prn] ;
            _indices[prn] = index ;
        }
    }
}

//***********************************************************************************
permutation_table::permutation_table( this_cref_t rhv ) noexcept
{
    *this = rhv ;
}

//***********************************************************************************
permutation_table::permutation_table( this_rref_t rhv ) noexcept
{
    *this = std::move( rhv ) ;
}

//***********************************************************************************
permutation_table::~permutation_table( void_t ) noexcept
{
}

//***********************************************************************************
permutation_table::this_ref_t permutation_table::operator = ( this_cref_t rhv ) noexcept 
{
    _seed = rhv._seed ;
    _upper_bound = rhv._upper_bound ;
    _bit = rhv._bit ;
    _indices = rhv._indices ;

    return *this ;
}

//***********************************************************************************
permutation_table::this_ref_t permutation_table::operator = ( this_rref_t rhv ) noexcept 
{
    _seed = rhv._seed ;
    _upper_bound = rhv._upper_bound ;
    _bit = rhv._bit ;
    _indices = std::move( rhv._indices ) ;

    return *this ;
}

//***********************************************************************************
uint_t permutation_table::permute_at( uint_t const i0 ) const noexcept
{
    uint_t const i = perm( i0 & _upper_bound ) ;
    return _indices[i] ;
}

//***********************************************************************************
uint_t permutation_table::permute_at( uint_t const i0, uint_t const i1 ) const noexcept
{
    const uint_t i = perm( i0 & _upper_bound ) ;
    const uint_t j = perm( (i+i1) & _upper_bound ) ;
    return _indices[j] ;
}

//***********************************************************************************
uint_t permutation_table::permute_at( uint_t const i0, uint_t const i1, uint_t const i2 ) const noexcept
{
    const uint_t i = perm( i0 & _upper_bound ) ;
    const uint_t j = perm( (i+i1) & _upper_bound ) ;
    const uint_t k = perm( (j+i2) & _upper_bound ) ;
    return _indices[k] ;
}

//***********************************************************************************
uint_t permutation_table::get_num_entries( void_t ) const noexcept
{ 
    return 1 << _bit ; 
}

//***********************************************************************************
uint_t permutation_table::get_bit( void_t ) const noexcept
{ 
    return _bit ; 
}

//***********************************************************************************
uint_t permutation_table::perm( uint_t i ) const noexcept
{
    return _indices[i] ;
}

