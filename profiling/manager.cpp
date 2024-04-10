
#include "manager.h"

using namespace motor::profiling ;

size_t motor::profiling::manager::_probe_id = size_t( -1 ) ;
std::mutex motor::profiling::manager::_mtx_probe_id ;
motor::profiling::manager::per_probe_id_datas_t motor::profiling::manager::_per_probe_id_datas( 1000 ) ;

//************************************************************
manager::manager( void_t ) noexcept 
{}

//************************************************************
manager::manager( this_rref_t rhv ) noexcept
{
    _pt_timings = std::move( rhv._pt_timings ) ;
    _data_points[ 0 ] = std::move( rhv._data_points[ 0 ] ) ;
    _data_points[ 1 ] = std::move( rhv._data_points[ 1 ] ) ;

    _probes = std::move( rhv._probes ) ;
}

//************************************************************
manager::~manager( void_t ) noexcept
{}

//************************************************************
size_t manager::gen_id( char_cptr_t cat, char_cptr_t name ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_probe_id ) ;
    
    size_t const id = ++_probe_id ;
    if ( id >= _per_probe_id_datas.size() )
    {
        _per_probe_id_datas.resize( _per_probe_id_datas.size() + 1000 ) ;
    }

    _per_probe_id_datas[ id ] = this_t::per_probe_id_data { cat, name }  ;

    return id ;
}

//************************************************************
void_t manager::push( motor::string_rref_t name ) noexcept
{
    this_t::push( name ) ;
} 

//************************************************************
void_t manager::push( motor::string_cref_t name ) noexcept
{
    auto const tid = std::this_thread::get_id() ;
    
    std::lock_guard< std::mutex > lk( _mtx_ptt ) ;
    auto iter = std::find_if( _pt_timings.begin(), _pt_timings.end(),
        [&] ( per_thread_data_cref_t d )
    {
        return d.tid == tid ;
    } ) ;

    size_t idx = std::distance( _pt_timings.begin(), iter ) ;

    if ( idx == _pt_timings.size() )
    {
        idx = _pt_timings.size() ;

        this_t::per_thread_data_t ptd { tid } ;
        _pt_timings.emplace_back( std::move( ptd ) ) ;
    }
    _pt_timings[ idx ].timings.push( this_t::per_thread_data_t::stack_data_t 
        { name, motor::profiling::clock_t::now() } ) ;
}

//************************************************************
void_t manager::pop( void_t ) noexcept
{
    auto const tid = std::this_thread::get_id() ;
    
    size_t idx = 0 ;

    this_t::per_thread_data_t::stack_data_t data ;

    {
        std::lock_guard< std::mutex > lk( _mtx_ptt ) ;
        auto iter = std::find_if( _pt_timings.begin(), _pt_timings.end(), 
            [&] ( per_thread_data_cref_t d )
        {
            return d.tid == tid ;
        } ) ;

        assert( iter != _pt_timings.end() ) ;
        data = std::move( iter->timings.pop() ) ;
    }
     
    #if 0
    auto dur = motor::profiling::clock_t::now() - data.tp ;

    // place data point
    {
        std::lock_guard< std::mutex > lk( _mtx_dp ) ;
        _data_points[ this_t::dp_widx() ].emplace_back(this_t::data_point_t {std::move(data.name), dur}) ;
    }
    #endif
}

//************************************************************
char_cptr_t manager::get_category( size_t const probe_id ) noexcept
{
    assert( probe_id <= _per_probe_id_datas.size() ) ;

    std::lock_guard< std::mutex > lk( _mtx_probe_id ) ;
    return _per_probe_id_datas[ probe_id ].category ;
}

//************************************************************
char_cptr_t manager::get_name( size_t const probe_id ) noexcept
{
    assert( probe_id <= _per_probe_id_datas.size() ) ;

    std::lock_guard< std::mutex > lk( _mtx_probe_id ) ;
    return _per_probe_id_datas[ probe_id ].name ;
}

//************************************************************
size_t manager::begin_probe( size_t const id ) noexcept
{
    assert( id <= _probe_id ) ;

    this_t::probe_data const pd { true, id, std::this_thread::get_id(),
        motor::profiling::clock_t::now() } ;

    std::lock_guard< std::mutex > lk( _mtx_probes ) ;
    auto iter = std::find_if( _probes.begin(), _probes.end(), [&] ( this_t::probe_data const & d ) 
    {
        return d.used == false ;
    } ) ;

    if ( iter == _probes.end() )
    {
        iter = _probes.insert( _probes.end(), std::move( pd ) ) ;
    }
    else
    {
        *iter = std::move( pd ) ;
    }

    return std::distance( _probes.begin(), iter ) ;
}

//************************************************************
void_t manager::end_probe( size_t const idx ) noexcept
{
    assert( idx < _probes.size() ) ;

    this_t::probe_data pd ;

    // #1 invalidate probe
    {
        std::lock_guard< std::mutex > lk( _mtx_probes ) ;
        pd = _probes[ idx ] ;
        _probes[ idx ].used = false ;
    }

    // #2 place data point
    auto dur = motor::profiling::clock_t::now() - pd.tp ;
    
    // place data point
    {
        std::lock_guard< std::mutex > lk( _mtx_dp ) ;
        _data_points[ this_t::dp_widx() ].emplace_back( this_t::data_point_t { pd.probe_id, dur } ) ;
    }
}

//************************************************************
manager::data_points_t manager::swap_and_clear( void_t ) noexcept
{
    _data_points[ _dp_ridx ].clear() ;

    {
        std::lock_guard< std::mutex > lk( _mtx_dp ) ;
        this_t::swap_data_points() ;

    }

    return _data_points[ _dp_ridx ] ;
}