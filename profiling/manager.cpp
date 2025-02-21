
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
    _pt_stacks = std::move( rhv._pt_stacks ) ;
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

    auto const tid = std::this_thread::get_id() ;

    // prepare for per thread call stack and parent child
    // function call relation : disabled for now and still not .
    #if EXPERIMENTAL_PER_THREAD_CLASS_STACK_PROFILING
    // place probe id on per thread call stack
    {
        bool_t found = false ;

        std::lock_guard< std::mutex > lk( _mtx_ptt ) ;

        for( size_t i=0; i<_pt_stacks.size(); ++i )
        {
            auto & item = _pt_stacks[i] ;

            if( item.tid == tid )
            {
                item.the_stack.push( id ) ;
                found = true ;
                break ;
            }
        }

        if( !found )
        {
            // find a free stack
            for ( size_t i = 0; i < _pt_stacks.size(); ++i )
            {
                auto & item = _pt_stacks[i] ;

                if ( item.tid == std::thread::id() )
                {
                    assert( item.the_stack.size() == 0 ) ;

                    item.tid = tid ;
                    item.the_stack.push( id ) ;
                    found = true ;
                    break ;
                }
            }

            if( !found )
            {
                _pt_stacks.emplace_back( this_t::per_thread_data
                    {
                        tid,
                        this_t::per_thread_data::stack_t::create_with_first_item( id )
                    } ) ;
            }
        }
    }
    #endif

    this_t::probe_data pd { true, id, tid,
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

    #if EXPERIMENTAL_PER_THREAD_CLASS_STACK_PROFILING
    // #2 clear thread call stack
    {
        auto const tid = std::this_thread::get_id() ;

        assert( tid == pd.tid ) ;

        std::lock_guard< std::mutex > lk( _mtx_ptt ) ;
        for ( auto & i : _pt_stacks )
        {
            if( i.tid == tid ) 
            {
                assert( i.the_stack.top() == pd.probe_id ) ;
                i.the_stack.pop() ;

                if( i.the_stack.size() == 0 )
                    i.tid = std::thread::id() ;
                break ;
            }
        }
    }
    #endif

    // #3 place data point
    auto dur = motor::profiling::clock_t::now() - pd.tp ;
    
    // place data point
    {
        std::lock_guard< std::mutex > lk( _mtx_dp ) ;

        auto & dps = _data_points[ this_t::dp_widx() ] ;

        if( dps.capacity() == dps.size() ) dps.reserve( dps.size() + 1000 ) ;
        dps.emplace_back( this_t::data_point_t { pd.probe_id, dur } ) ;
    }
}

//************************************************************
manager::data_points_cref_t manager::swap_and_clear( void_t ) noexcept
{
    _data_points[ _dp_ridx ].clear() ;

    {
        std::lock_guard< std::mutex > lk( _mtx_dp ) ;
        this_t::swap_data_points() ;

    }

    return _data_points[ _dp_ridx ] ;
}