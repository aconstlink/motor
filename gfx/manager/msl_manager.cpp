
#include "msl_manager.h"

#include <motor/io/global.h>

using namespace motor::gfx;

//**************************************************************************
msl_manager::msl_manager( motor::io::database_mtr_safe_t db ) noexcept : _db( motor::move( db ) )
{    
    this_t::start_thread();
}

//**************************************************************************
msl_manager::msl_manager( this_rref_t rhv ) noexcept : _db( motor::move( rhv._db ) )
{
    rhv.stop_thread();
    this_t::start_thread();
}

//**************************************************************************
msl_manager::~msl_manager( void_t ) noexcept
{
    this_t::stop_thread();
}

//**************************************************************************
bool_t msl_manager::add( motor::string_cref_t name, motor::io::location_cref_t loc ) noexcept
{
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

        auto iter = _location_to_id.find( loc.as_string() );
        if( iter != _location_to_id.end() )
        {
            // return what?
            // id
            // msl
        }

        auto ca = _db->load( loc );
        _loads.emplace_back( load_operation{ loc, name, std::move( ca ) } );
    }

    {
        std::lock_guard< std::mutex > lk( _sd->mtx );
        _sd->has_work = true;
    }
    _sd->cv.notify_one();

    return true;
}

//**************************************************************************
void_t msl_manager::on_update( void_t ) noexcept {}

//**************************************************************************
void_t msl_manager::for_each_configure_done( on_configure_funk_t funk ) noexcept 
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );
    
    auto iter = _configures_in_process.begin() ;
    while( iter != _configures_in_process.end() )
    {
        size_t const id = *iter ;
        auto res = _msls[id].cs->is_configured() ;
        if( res )
        {
            funk( _msls[id].msl->name(), _msls[id].msl, _msls[id].cs ) ;
            iter = _configures_in_process.erase( iter ) ;
            continue ;
        }
        ++iter ;
    }
}

//**************************************************************************
void_t msl_manager::on_render_init( motor::graphics::gen4::frontend_ptr_t ) noexcept {}

//**************************************************************************
void_t msl_manager::on_render_release( motor::graphics::gen4::frontend_ptr_t ) noexcept {}

//**************************************************************************
void_t msl_manager::on_render( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

    for( auto const idx : _msls_config )
    {
        auto & item = _msls[ idx ];
        fe->configure< motor::graphics::msl_object_t >( item.msl, item.cs );
    }

    for( auto const idx : _msls_release )
    {
        auto & item = _msls[ idx ];
        fe->release< motor::graphics::msl_object_t >( item.msl, item.cs );
    }
}

//**************************************************************************
void_t msl_manager::on_frame_done( void_t ) noexcept
{
    _msls_config.clear();
    _msls_release.clear();
}

//**************************************************************************
void_t msl_manager::start_thread( void_t ) noexcept
{
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

        if( _sd != nullptr ) return;

        {
            this_t::shared_data sd;
            sd.has_work = false;
            sd.owner = this;
            sd.running = true;

            _sd = motor::shared( std::move( sd ) );
        }
    }

    _th = std::thread( [ & ]( void_t )
    {
        motor::log::global_t::status( "[msl_manager] : thread started" );

        while( _sd->running )
        {
            // little shortcut
            this_ptr_t owner = _sd->owner;

            {
                std::unique_lock< std::mutex > lk( _sd->mtx );
                while( !_sd->has_work && _sd->running ) _sd->cv.wait( lk );
            }

            decltype( _loads ) tmp;
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _sd->owner->_mutex );
                tmp = std::move( _sd->owner->_loads );
            }

            for( auto & item : tmp )
            {
                motor::string_t shd;
                item.ca.wait_for_operation(
                    [ & ]( char_cptr_t data, size_t const sib, motor::io::result const res )
                {
                    if( res != motor::io::result::ok )
                    {
                        motor::log::global_t::warning(
                            "[msl_manager] : could not load file : " + item.loc.as_string() );
                        return;
                    }

                    shd = motor::string_t( data, sib );
                } );

                {
                    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

                    auto iter = _name_to_id.find( item.name );
                    if( iter != _name_to_id.end() )
                    {
                        size_t const idx = ( *iter ).second;

                        if( _msls[ idx ].cs->is_in_transit() )
                        {
                            motor::log::global_t::warning(
                                "[msl_manager] : the shader is currently being processed: " +
                                item.name );
                        }
                        else
                        {
                            _msls[ idx ].msl->clear_shaders().add(
                                motor::graphics::msl_api_type::msl_4_0, shd );
                            _msls_config.emplace_back( idx );
                            _configures_in_process.emplace_back( idx ) ;
                        }
                    }
                    else
                    {
                        motor::graphics::msl_object_t msl( item.name );
                        msl.add( motor::graphics::msl_api_type::msl_4_0, shd );

                        this_t::msl_data md;
                        md.cs = motor::shared( motor::graphics::command_status_t() );
                        md.msl = motor::shared( std::move( msl ) );
                        md.mon = motor::shared( motor::io::monitor_t() ) ;
                        _msls_config.emplace_back( _msls.size() );
                        _configures_in_process.emplace_back( _msls.size() ) ;
                        _msls.emplace_back( std::move( md ) );
                    }
                }
            }
        }

        motor::log::global_t::status( "[msl_manager] : thread stopped" );
    } );
}

//**************************************************************************
void_t msl_manager::stop_thread( void_t ) noexcept
{
    if( _sd == nullptr ) return;

    {
        std::lock_guard< std::mutex > _lk( _sd->mtx );
        _sd->running = false;
    }
    _sd->cv.notify_one();

    if( _th.joinable() ) _th.join();

    motor::release( motor::move( _sd ) );
}