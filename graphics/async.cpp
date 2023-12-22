
#include "async.h"
#include "backend/gen4/null.h"

using namespace motor::graphics ;

//***********************************************************
async::async( void_t ) noexcept
{}

//***********************************************************
async::async( motor::graphics::gen4::backend_mtr_shared_t rptr ) noexcept : _backend( motor::memory::copy_ptr(rptr) ) 
{}

//***********************************************************
async::async( this_rref_t rhv ) noexcept
{
    _runtimes[0] = std::move( rhv._runtimes[0] ) ;
    _runtimes[1] = std::move( rhv._runtimes[1] ) ;
    _configures[0] = std::move( rhv._configures[0] ) ;
    _configures[1] = std::move( rhv._configures[1] ) ;
    _backend = motor::move( rhv._backend ) ;   
    _num_enter = rhv._num_enter ;
}

//***********************************************************
async::~async( void_t ) noexcept
{}

//***********************************************************
motor::graphics::result async::set_window_info( motor::graphics::gen4::backend_t::window_info_cref_t wi ) noexcept 
{
    motor::concurrent::lock_guard_t lk( _window_info_mtx ) ;
    _window_info_set = true ;
    _window_info = wi ;
    return motor::graphics::result::ok ;
}

//***********************************************************
async::this_ref_t async::configure( motor::graphics::geometry_object_mtr_delay_t gconfig, 
    motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable 
        { 
            auto const ires = be->configure( gconfig ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }
    return *this ;
}

//***********************************************************
async::this_ref_t async::configure( motor::graphics::render_object_mtr_delay_t rc, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->configure( rc ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }
    
    return *this ;
}

//***********************************************************
async::this_ref_t async::configure( motor::graphics::shader_object_mtr_delay_t sc,
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->configure( sc ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

//***********************************************************
async::this_ref_t async::configure( motor::graphics::image_object_mtr_delay_t sc,
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->configure( sc ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

//***********************************************************
async::this_ref_t async::configure( motor::graphics::framebuffer_object_mtr_delay_t fb, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->configure( fb ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }
    return *this ;
}

async::this_ref_t async::configure( motor::graphics::state_object_mtr_delay_t s, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->configure( s ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::configure( motor::graphics::array_object_mtr_delay_t obj, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->configure( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::configure( motor::graphics::streamout_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->configure( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::geometry_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::render_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::shader_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::image_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::framebuffer_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::state_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::array_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::release( motor::graphics::streamout_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->release( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ; 
}

async::this_ref_t async::update( motor::graphics::geometry_object_mtr_delay_t gs, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;

        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->update( gs ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

//***********************************************************
async::this_ref_t async::update( motor::graphics::array_object_mtr_delay_t obj, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;

        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->update( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

//***********************************************************
async::this_ref_t async::update( motor::graphics::image_object_mtr_delay_t obj, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;

        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->update( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

//***********************************************************
async::this_ref_t async::use( motor::graphics::framebuffer_object_mtr_delay_t fb,
    motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _runtimes_mtx ) ;

        _runtimes[_runtimes_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->use( fb ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }
    return *this ;
}

async::this_ref_t async::use( motor::graphics::streamout_object_mtr_delay_t obj, motor::graphics::result_mtr_t res ) noexcept 
{
    {
        motor::concurrent::lock_guard_t lk( _runtimes_mtx ) ;

        _runtimes[_runtimes_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->use( obj ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    return *this ;
}

async::this_ref_t async::unuse( motor::graphics::gen4::backend::unuse_type const t, motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _runtimes_mtx ) ;

        _runtimes[_runtimes_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->unuse( t ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }
    return *this ;
}

//***********************************************************
async::this_ref_t async::push( motor::graphics::state_object_mtr_delay_t s, size_t const sid, bool_t const push, 
    motor::graphics::result_mtr_t res ) noexcept 
{
    motor::concurrent::lock_guard_t lk( _runtimes_mtx ) ;

    _runtimes[_runtimes_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
    {
        auto const ires = be->push( std::move( s ), sid, push ) ;
        if( res != nullptr ) *res = ires ;
    } ) ;
    return *this ;
}
//***********************************************************
async::this_ref_t async::pop( motor::graphics::gen4::backend::pop_type const t, motor::graphics::result_mtr_t res ) noexcept
{
    motor::concurrent::lock_guard_t lk( _runtimes_mtx ) ;

    _runtimes[_runtimes_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
    {
        auto const ires = be->pop( t ) ;
        if( res != nullptr ) *res = ires ;
    } ) ;

    return *this ;
}

//***********************************************************
async::this_ref_t async::render( motor::graphics::render_object_mtr_delay_t obj, motor::graphics::gen4::backend::render_detail_cref_t detail,
    motor::graphics::result_mtr_t res ) noexcept
{
    {
        motor::concurrent::lock_guard_t lk( _configures_mtx ) ;

        _configures[_configures_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        {
            auto const ires = be->update( obj, detail.varset ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }

    {
        motor::concurrent::lock_guard_t lk( _runtimes_mtx ) ;

        _runtimes[_runtimes_id].push_back( [=] ( motor::graphics::gen4::backend_ptr_t be ) mutable
        { 
            auto const ires = be->render( obj, detail ) ;
            if( res != nullptr ) *res = ires ;
        } ) ;
    }
    return *this ;
}

//***********************************************************
void_t async::system_update( void_t ) noexcept 
{
    // window info
    if( _window_info_set )
    {
        motor::graphics::gen4::backend_t::window_info_t wi ;
        {
            motor::concurrent::lock_guard_t lk( _window_info_mtx ) ;
            wi = _window_info ;
            _window_info_set = false ;
        }
        _backend->set_window_info( wi ) ;
    }

    // configures
    {
        {
            motor::concurrent::lock_guard_t lk( _configures_mtx ) ;
            _configures_id = (_configures_id+1) % 2 ;
        }

        size_t const use_id = (_configures_id + 1) % 2 ;
        this_t::commands_t & coms = _configures[use_id] ;

        for( auto& rtz : coms )
        {
            rtz( _backend ) ;
        }
        coms.clear() ;
    }

    // runtime functions
    {
        {
            motor::concurrent::lock_guard_t lk( _runtimes_mtx ) ;
            _runtimes_id = (_runtimes_id+1) % 2 ;
        }

        {
            motor::concurrent::lock_guard_t lk( _frame_mtx ) ;
            _num_enter = 0 ;
        }
        
        {
            auto const use_id = (_runtimes_id+1)%2 ;
            auto & coms = _runtimes[ use_id ] ;

            _backend->render_begin() ;
            
            for( auto& rtz : coms )
            {
                rtz( _backend ) ;
            }

            _backend->render_end() ;

            coms.clear() ;
        }
    }
}

void_t async::end_of_time( void_t ) noexcept 
{
    _backend = nullptr ;
}

//***
bool_t async::enter_frame( void_t ) 
{
    motor::concurrent::lock_guard_t lk( _frame_mtx ) ;
    if( _num_enter > 0 ) return false ;
    return true ;
}

//***
void_t async::leave_frame( void_t ) 
{
    {
        motor::concurrent::lock_guard_t lk( _frame_mtx ) ;
        _num_enter = 1 ;
    }
    _frame_cv.notify_all() ;
}

//***
void_t async::wait_for_frame( void_t ) 
{
    std::unique_lock< std::mutex > lk( _frame_mtx ) ;
    while( _num_enter == 0 ) _frame_cv.wait( lk ) ;
}
