#include "database.h"
#include "global.h"
#include "obfuscator.hpp"

#include <motor/std/filesystem>
#include <motor/std/string>
#include <motor/std/sstream>
#include <motor/std/string_split.hpp>
#include <motor/memory/malloc_guard.hpp>
#include <motor/log/global.h>

#include <streambuf>
#include <iomanip>
#include <array>
#include <fstream>

using namespace motor::io ;

static char_t const * const header_desc = "motor db file format 0.1" ;

namespace this_file_db
{
    using namespace motor::core::types ;

    struct length_validator
    {
        static bool_t make_ending_string( motor::string_cref_t si, motor::string_ref_t so ) noexcept
        {
            std::srand( uint_t(si.size()) ) ;

            if( si.size() >= length_validator::fixed_length() ) return false ;

            so = si ;
            so.resize( length_validator::fixed_length(), '-' ) ;
            for( size_t i=si.size(); i<so.size(); ++i )
            {
                so[ i ] = char_t( std::rand() ) ;
            }
            return true ;
        }

        static size_t fixed_length( void_t ) noexcept { return 1024 ; }
    };
}

struct database::record_cache
{
    motor_this_typedefs( record_cache ) ;
    motor_typedef( record_cache ) ;

    motor::concurrent::mrsw_t ac ;

    motor::io::system_t::load_handle_t _lh ;

    char_ptr_t _data = nullptr ;
    size_t _sib = 0 ;

    database* owner = nullptr ;
    size_t _idx = size_t( -1 ) ;
    

    record_cache( void_t )
    {
    }

    record_cache( database* owner_new, size_t const idx )
    {
        owner = owner_new ;
        _idx = idx ;
    }

    record_cache( this_rref_t rhv )
    {
        *this = std::move( rhv ) ;
    }

    ~record_cache( void_t )
    {
    }

    record_cache::this_ref_t operator = ( this_rref_t rhv )
    {
        _idx = rhv._idx ;
        rhv._idx = size_t( -1 ) ;
        owner = rhv.owner ;
        rhv.owner = nullptr ;

        motor_move_member_ptr( _data, rhv ) ;
        _sib = rhv._sib ;

        _lh = std::move( rhv._lh ) ;

        return *this ;
    }

    bool_t can_wait( void_t ) const noexcept { return _lh.can_wait() ; }
    bool_t has_data( void_t ) const noexcept { return _data != nullptr ; }

    void_t take_load_handle( motor::io::system_t::load_handle_rref_t hnd )
    {
        _lh = std::move( hnd ) ;
    }

    bool_t wait_for_operation( motor::io::database::load_completion_funk_t funk )
    {
        if( _idx == size_t( -1 ) )
        {
            motor::log::global_t::error( "[db cache] : invalid handle" ) ;
            return false ;
        }

        auto const res = _lh.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const err )
        {
            if( err != motor::io::result::ok )
            {
                motor::log::global_t::error( "[db] : failed to load data loc " + motor::io::to_string( err ) ) ;
                return ;
            }

            // @todo cache data...
            motor::memory::malloc_guard< char_t > const mg( data, sib ) ;

            funk( mg.get(), mg.size(), err ) ;
        } ) ;

        // there was no load operation, so take data from cache
        if( res == motor::io::result::invalid_handle )
        {
            if( _data == nullptr )
            {
                motor::log::global_t::error( "[db] : no load pending and no data cached. "
                    "This function requires a db load call." ) ;
                return false ;
            }

            funk( _data, _sib, motor::io::result::cached_data ) ;
            return true ;
        }
        return res == motor::io::result::ok ;
    }

    void_t change_database( motor::io::database* owner_new )
    {
        owner = owner_new ;
    }

    motor::io::database::record_cache_mtr_t load(
        motor::io::location_cref_t loc, bool_t const reload ) noexcept
    {
        return owner->load( loc, reload )._res ;
    }

    motor::io::database::record_cache_mtr_t load( bool_t const reload ) noexcept
    {
        motor::string_t loc = owner->location_for_index( _idx ) ;
        return owner->load( loc, reload )._res ;
    }
};

struct database::file_record
{
    motor::io::location_t location ;

    // -1 : invalid
    // -2 : external
    // otherwise : stored in .motor db file
    uint64_t offset = uint64_t( -1 ) ;
    uint64_t sib = uint64_t( 0 ) ;

    motor::io::path_t rel ;
    motor::filesystem::file_time_type stamp ;

    motor::vector< motor::io::monitor_mtr_t > monitors ;

    // the handle prototype/origin which is copied to the user
    record_cache_mtr_t cache = nullptr ;

    file_record( void_t ) {}
    file_record( file_record const & rhv ) 
    {
        location = rhv.location ;
        offset = rhv.offset ;
        sib = rhv.sib ;
        rel = rhv.rel ;
        stamp = rhv.stamp ;
        monitors = rhv.monitors ;
        cache = motor::memory::copy_ptr(rhv.cache) ;
    }

    file_record( file_record && rhv )
    {
        location = std::move( rhv.location ) ;
        offset = rhv.offset ;
        sib = rhv.sib ;
        rel = std::move( rhv.rel ) ;
        stamp = std::move( rhv.stamp ) ;
        monitors = std::move( rhv.monitors ) ;
        cache = motor::move( rhv.cache ).mtr() ;
    }
    ~file_record( void_t ) 
    {
        motor::memory::release_ptr( motor::move( cache ) ) ;
    }

    file_record & operator = ( file_record const& rhv ) noexcept
    {
        location = rhv.location ;
        offset = rhv.offset ;
        sib = rhv.sib ;
        rel = rhv.rel ;
        stamp = rhv.stamp ;
        monitors = rhv.monitors ;
        cache = motor::memory::copy_ptr(rhv.cache) ;
        return *this ;
    }

    file_record & operator = ( file_record&& rhv ) noexcept
    {
        location = std::move( rhv.location ) ;
        offset = rhv.offset ;
        sib = rhv.sib ;
        rel = std::move( rhv.rel ) ;
        stamp = std::move( rhv.stamp ) ;
        monitors = std::move( rhv.monitors ) ;
        cache = motor::move( rhv.cache ).mtr() ;
        return *this ;
    }
};


struct database::file_header
{
    // encryption method
    // some infos
    // start of file records
    // number of file records
    char_t padding[ 20 ] ;
};

struct database::db
{
    motor::io::path_t base ;
    motor::io::path_t working ;
    motor::io::path_t name ;
    motor::vector< file_record > records ;
    motor::vector< motor::io::monitor_mtr_t > monitors ;
    size_t offset = 0 ;

    db( void_t ) noexcept {}
    db( db const& rhv ) noexcept { *this = rhv ; }
    db( db && rhv ) noexcept { *this = std::move( rhv ) ; }
    ~db( void_t ) noexcept
    {
        for( auto * mtr : monitors ) motor::memory::release_ptr( mtr ) ;
    }

    db & operator = ( db const& rhv ) noexcept
    {
        base = rhv.base ;
        records = rhv.records ;
        monitors = rhv.monitors ;
        working = rhv.working ;
        name = rhv.name ;
        offset = rhv.offset ;
        return *this ;
    }

    db & operator = ( db && rhv ) noexcept
    {
        base = std::move( rhv.base ) ;
        records = std::move( rhv.records ) ;
        monitors = std::move( rhv.monitors ) ;
        working = std::move( rhv.working ) ;
        name = std::move( rhv.name ) ;
        offset = rhv.offset ;
        return *this ;
    }

    
};


database::cache_access::cache_access( database::record_cache_mtr_t res ) noexcept : _res( motor::memory::copy_ptr(res) ) {}
database::cache_access::cache_access( this_cref_t rhv ) noexcept : _res( motor::memory::copy_ptr(rhv._res) ) {}
database::cache_access::cache_access( this_rref_t rhv ) noexcept : _res( motor::move( rhv._res ) ) {}
database::cache_access::~cache_access( void_t ) noexcept 
{
    motor::memory::release_ptr( _res ) ;
}

database::cache_access_ref_t database::cache_access::operator = ( this_cref_t rhv ) noexcept 
    { _res = motor::memory::copy_ptr(rhv._res) ; return *this ; }

database::cache_access_ref_t database::cache_access::operator = ( this_rref_t rhv ) noexcept 
    { _res = motor::move( rhv._res ).mtr() ; return *this ; }


bool_t database::cache_access::wait_for_operation( motor::io::database::load_completion_funk_t funk ) 
{
    if( _res == nullptr ) return false ;
    return _res->wait_for_operation( funk ) ;
}

motor::io::database::cache_access_t::this_t database::cache_access::load( 
    motor::io::location_cref_t loc, bool_t const reload ) noexcept
{
    this->_res = _res->load( loc, reload ) ;
    return std::move( *this ) ;
}

motor::io::database::cache_access_t::this_t database::cache_access::load( bool_t const reload ) noexcept
{
    return this_t( _res->load( reload ) ) ;
}

//******************************************************************
database::database( void_t ) 
{
}

//******************************************************************
database::database( motor::io::path_cref_t base, motor::io::path_cref_t name,
    motor::io::path_cref_t working_rel )
{
    this_t::init( base, name, working_rel ) ;
    this_t::spawn_update() ;
}

//******************************************************************
database::database( this_rref_t rhv )
{
    *this = std::move( rhv ) ;
}

//******************************************************************
database::~database( void_t )
{
    this_t::join_update() ;
    motor::memory::global_t::dealloc( _db ) ;
}

//******************************************************************
database::this_ref_t database::operator = ( this_rref_t rhv ) noexcept 
{
    this_t::join_update() ;
    _db = motor::move( rhv._db ).mtr() ;

    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;
    for( auto& rec : _db->records )
    {
        rec.cache->change_database( this ) ;
    }

    this_t::spawn_update() ;
    return *this ;
}

//******************************************************************
bool_t database::init( motor::io::path_cref_t base, motor::io::path_cref_t working, motor::io::path_cref_t name )
{
    this_t::db_t db ;

    // this is where the db is located
    db.base = base ;
    // this is where the data stored should be located
    db.working = db.base / working ;
    // this is the db name
    db.name = name ;

    // look for db file
    {
        motor::io::path_t const loc = db.base / motor::io::path_t(name).replace_extension( motor::io::path_t( ".mdb" ) ) ;
        auto const res = motor::filesystem::exists( loc ) ;
        
        if( res )
        {
            this_t::load_db_file( db, loc ) ;
        }
    }

    // look for db file system
    if( motor::filesystem::exists( db.working ) )
    {
        for( auto& i : motor::filesystem::recursive_directory_iterator( db.working ) )
        {
            // do not go into .xyz directories
            if( i.is_directory() && i.path().stem().string().find_first_of(".",0) == 0 )
            {
                continue ;
            }

            // files
            else if( i.is_regular_file() )
            {
                // do not track self
                if( i.path().stem() == name && i.path().extension() == ".mdb" ) continue ;

                auto fr = this_t::create_file_record( db, i.path() ) ;
                
                // check other files' existence
                {
                    this_t::file_record_t fr2 ;
                    if( this_t::lookup( db, fr.location, fr2 ) )
                    {
                        // only unique data entries
                        if( fr2.offset == size_t(-2) )
                        {
                            motor::stringstream_t ss ;
                            ss
                                << "[" << name << ".mdb] : "
                                << "Only unique file names supported. See [" << fr.location.as_string() << "] with extensions "
                                << "[" << fr.location.extension() << ", " << fr2.location.extension() << "] "
                                << "where [" << fr2.location.extension() << "] already stored" ;

                            motor::log::global_t::error( ss.str() ) ;
                        }

                        // file system data wins
                        else
                        {
                            this_t::file_change_external( db, fr ) ;
                        }
                        continue ;
                    }
                }
                
                db.records.emplace_back( std::move( fr ) ) ;
            }
        }
    }
    else
    {
        motor::log::global_t::warning( "Path does not exist : " + motor::from_std( motor::io::path_t( base / working.string() ).string() ) ) ;
    }

    // spawn monitor thread for file system changes
    {
        _update_thread = motor::concurrent::thread_t([=](){}) ;
    }

    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;
        _db = motor::memory::global_t::alloc( std::move( db ), 
            "[motor::io::database::init] : db" ) ;
    }

    return true ;
}

//******************************************************************
bool_t database::pack( this_t::encryption const )
{
    struct __file_record
    {
        motor::io::location_t location ;
        uint64_t offset ;
        uint64_t sib ;
        motor::io::path_t path ;
        bool_t external ;

        motor::string_t to_string( void_t ) const 
        {
            motor::stringstream_t ss ;
            ss  << location.as_string() << ":" << location.extension() << ":"
                << std::to_string( offset ) << ":" << std::to_string( sib ) << ": " ;
            return ss.str() ;
        }

        size_t size( void_t ) const 
        {
            return to_string().size() ;
        }
    };

    motor::vector< __file_record > records ;
    
    // base offset to the first data written
    // (number of records + header ) * length
    uint64_t const offset = (_db->records.size() + 1) * this_file_db::length_validator::fixed_length() ;

    {
        uint64_t lo = offset ;

        for( auto& fr : _db->records )
        {
            __file_record fr__ ;
            fr__.location = fr.location ;
            fr__.offset = lo ;
            fr__.sib = fr.sib ;
            fr__.path = _db->base / fr.rel ;
            fr__.external = fr.offset == uint64_t( -2 ) ;

            lo += fr__.sib ;
            records.push_back( fr__ ) ;
        }
    }

    motor::io::path_t db_new = _db->base / motor::io::path_t( _db->name ).replace_extension( ".tmp.mdb" ) ;
    motor::io::path_t db_old = _db->base / motor::io::path_t( _db->name ).replace_extension( ".mdb" ) ;
    
    // write file
    {
        std::ofstream outfile ( db_new, std::ios::binary ) ;

        // header info
        {
            motor::string_t so ;
            motor::string_t const si = this_t::obfuscator().
                encode( motor::string_t(header_desc) + ":" + motor::from_std( std::to_string( _db->records.size() ) ) + ": " ) ;

            auto const res = this_file_db::length_validator::make_ending_string( si, so ) ;
            if( res ) outfile << so ;
            motor::log::global_t::error( !res, "[db] : header does not fit into fixed length." ) ;
            
        }
        
        // records
        {
            motor::string_t so ;

            for( auto& fr : records )
            {
                motor::string_t const si = this_t::obfuscator().encode( fr.to_string() ) ;

                auto const res = this_file_db::length_validator::make_ending_string( si, so ) ;
                if( res ) outfile << so ;
                motor::log::global_t::warning( !res, "[db] : file record entry too long. Please reduce name length." ) ;
            }
        }
       
        // file content 
        {
            for( auto & fr : records )
            {
                this_t::load( fr.location ).wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
                {
                    motor::string_t const wdata = this_t::obfuscator().encode( data, sib ) ;

                    outfile.seekp( fr.offset ) ;
                    outfile.write( wdata.c_str(), sib ) ;

                } ) ;
            }
            outfile.flush() ;
        }
        
        outfile.flush() ;
        outfile.close() ;
    }

    if( motor::filesystem::exists( db_old ) )
    {
        motor::filesystem::remove( db_old ) ;
    }
    motor::filesystem::rename( db_new, db_old ) ;
    return true ;
}

//******************************************************************
void_t database::load_db_file( this_t::db_ref_t db_, motor::io::path_cref_t p ) 
{
    std::ifstream ifs( p, std::ios::binary ) ;

    size_t num_records = 0 ;

    motor::memory::malloc_guard< char_t > data( this_file_db::length_validator::fixed_length() ) ;

    // check file header description
    {
        ifs.read( data, this_file_db::length_validator::fixed_length() ) ;

        motor::string_t const buffer = this_t::obfuscator().
            decode( motor::string_t( data.get(), this_file_db::length_validator::fixed_length() ) ) ;
        
        motor::vector< motor::string_t > token ;
        size_t const num_elems = motor::mstd::string_ops::split( buffer, ':', token ) ;

        if( num_elems >= 3 )
        {
            if( token[0] != header_desc )
            {
                motor::log::global_t::error( "[db] : Invalid db file. Header description mismatch. Should be " + 
                    motor::string_t(header_desc) + " for file " + motor::from_std( p.string() ) ) ;

                motor::log::global_t::error( "[db] : Just repack the data with the appropriate packer version." ) ;
                return ;
            }

            num_records = std::stol( token[ 1 ].c_str() ) ;
        }
        else 
        {
            motor::log::global_t::error( "[db] : can not read db for file " + motor::from_std( p.string() ) ) ;
            return ;
        }
        
        motor::log::global_t::status( "[db] : Loading db file from " + motor::from_std(  p.string() ) ) ;
    }

    // add file records
    {
        for( size_t i = 0; i < num_records; ++i )
        {
            ifs.read( data, this_file_db::length_validator::fixed_length() ) ;
            motor::string_t const buffer = this_t::obfuscator().
                decode( motor::string_t( data.get(), this_file_db::length_validator::fixed_length() ) ) ;

            motor::vector< motor::string_t > token ;
            size_t const num_elems = motor::mstd::string_ops::split( buffer, ':', token ) ;

            if( num_elems < 5 ) 
            {
                motor::log::global_t::warning( "[db] : invalid file record" ) ;
                continue;
            }

            this_t::file_record_t fr ;
            fr.location = motor::io::location_t( token[ 0 ] ) ;
            fr.offset = std::stol( token[ 2 ].c_str() ) ;
            fr.sib = std::stol( token[ 3 ].c_str() ) ;
            fr.cache = motor::memory::create_ptr( this_t::record_cache( const_cast< this_ptr_t >( this ), db_.records.size() ),
                "[io::db] : file_record" ) ;

            db_.records.emplace_back( std::move( fr ) ) ;
        }
    }
}

//******************************************************************
bool_t database::unpack( void_t )
{
    return true ;
}

//******************************************************************
motor::io::system_t::store_handle_t database::store( motor::io::location_cref_t loc, char_cptr_t d, size_t const s ) noexcept
{
    auto p  = _db->working ;
    p.append( loc.as_path() ) ;
    return motor::io::global_t::store( p, d, s ) ;
}

//******************************************************************
database::cache_access_t database::load( motor::io::location_cref_t loc, bool_t const reload )
{
    this_t::file_record_t fr ;
    if( !this_t::lookup( loc, fr ) )
    {
        motor::log::global_t::warning( "resource location not found : " + loc.as_string() ) ;
        return this_t::cache_access_t( nullptr ) ;
    }
    
    this_t::cache_access_t ret = this_t::cache_access_t( fr.cache ) ;

    // need write lock so no concurrent task is doing the load in parallel.
    motor::concurrent::mrsw_t::writer_lock_t lk( fr.cache->ac ) ;

    // is load going on?
    if( fr.cache->can_wait() ) return std::move( ret ) ;

    if( !fr.cache->has_data() || reload )
    {
        motor::io::system_t::load_handle_t lh ;
        
        // load from filesystem
        if( fr.offset == uint64_t( -2 ) )
        {
            lh = motor::io::global_t::load( _db->working / fr.rel, std::launch::async ) ;
        }
        // load from .mdb file
        else if( fr.offset != uint64_t( -1 ) )
        {
            size_t const offset = fr.offset + _db->offset ;
            auto const p = _db->base / motor::io::path_t( _db->name ).replace_extension( motor::io::path_t( ".mdb" ) ) ;
            lh = motor::io::global_t::load( p, offset, fr.sib, std::launch::async, this_t::obfuscator() ) ;
        }

        fr.cache->take_load_handle( std::move( lh ) ) ;
    }

    return std::move( ret ) ;
}

//******************************************************************
void_t database::dump_to_std( void_t ) const noexcept 
{
    motor::log::global_t::status( "Printing Database" ) ;
    motor::log::global_t::status( "***************************************************" ) ;
    
    // db infos
    {
        motor::log::global_t::status( "Database Infos" ) ;
        motor::log::global_t::status( "**************" ) ;
        motor::log::global_t::status( "@" + motor::from_std( _db->base.string() ) ) ;
    }

    // file records
    {
        motor::log::global_t::status( "File Records" ) ;
        motor::log::global_t::status( "**************" ) ;
        for( auto& fr : _db->records )
        {
            motor::stringstream_t ss ;
            
            ss << "[" << std::to_string(fr.sib) << " @ " 
                << (fr.offset != uint64_t(-2) ? std::to_string(fr.offset) : "extern") 
                << "] "
                << fr.location.as_string() ;

            motor::log::global_t::status( ss.str() ) ;
        }
    }

    motor::log::global_t::status( "***************************************************" ) ;
}

//******************************************************************
void_t database::attach( motor::io::location_cref_t loc, motor::io::monitor_mtr_safe_t mon ) noexcept 
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;
    
    for( auto & fr : _db->records )
    {
        if( fr.location == loc )
        {
            fr.monitors.emplace_back( mon ) ;
        }
    }

}

//******************************************************************
void_t database::detach( motor::io::location_cref_t loc, motor::io::monitor_mtr_t mon ) noexcept 
{
    bool_t found = false ;
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;

        this_t::file_record_t fr ;
        if( this_t::lookup( *_db, loc, fr ) )
        {
            auto iter = std::remove_if( fr.monitors.begin(), fr.monitors.end(),
                [&] ( motor::io::monitor_mtr_t r )
            {
                return r == mon ;
            } ) ;

            if( iter != fr.monitors.end() ) 
            {
                fr.monitors.erase( iter ) ;
                found = true ;
            }
        }
    }
    if( found ) motor::memory::release_ptr( mon ) ;
}

//******************************************************************
void_t database::attach( motor::io::monitor_mtr_t moni ) noexcept 
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;
    auto iter = std::find_if( _db->monitors.begin(), _db->monitors.end(), 
        [&]( motor::io::monitor_mtr_t m )
    {
        return m == moni ;
    } ) ;

    if( iter != _db->monitors.end() ) return ;
        
    _db->monitors.emplace_back( motor::memory::copy_ptr(moni) ) ;
}

//******************************************************************
void_t database::detach( motor::io::monitor_mtr_t moni ) noexcept 
{
    size_t found = 0 ;
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;

        // remove from global monitors
        {
            auto iter = std::remove_if( _db->monitors.begin(), _db->monitors.end(),
                [&] ( motor::io::monitor_mtr_t r )
            {
                return r == moni ;
            } ) ;

            if( iter != _db->monitors.end() ) 
            {
                _db->monitors.erase( iter ) ;
                found++ ;
            }
        }
    
        // remove from local monitors
        {
            for( auto& fr : _db->records )
            {
                auto iter = std::remove_if( fr.monitors.begin(), fr.monitors.end(),
                    [&] ( motor::io::monitor_mtr_t r )
                {
                    return r == moni ;
                } ) ;

                if( iter != fr.monitors.end() ) 
                {
                    fr.monitors.erase( iter ) ;
                    found++ ;
                }
            }
        }
    }

    for( size_t i=0; i<found; ++i ) motor::memory::release_ptr( moni ) ;
}

//******************************************************************
bool_t database::lookup_extension( motor::string_cref_t loc, motor::string_ref_t ext ) const noexcept 
{
    this_t::file_record_t fr ;
    if( !this_t::lookup( loc, fr ) ) return false ;

    ext = fr.location.extension() ;

    return true;
}

//******************************************************************
database::file_record_t database::create_file_record( this_t::db_ref_t db, motor::io::path_cref_t path ) const noexcept
{
    this_t::file_record_t fr ;
 
    // the files' extension
    {
        //fr.extension = path.extension() ;
    }

    // determine file locator
    {
        // the relative path to the base path       
        fr.rel = motor::filesystem::relative( path, db.working ) ;
        fr.location = motor::io::location_t::from_path( fr.rel ) ;
    }

    {
        // external
        fr.offset = uint64_t( -2 ) ;

        {
            std::error_code ec ;
            fr.sib = motor::filesystem::file_size( path, ec ) ;
            motor::log::global_t::error( ( bool_t ) ec,
                motor_log_fn( "file_size with [" + ec.message() + "]" ) ) ;
        }
    }

    // store the last write time so 
    // monitoring can  take place.
    {
        fr.stamp = motor::filesystem::last_write_time( path ) ;
    }

    {
        fr.cache = motor::memory::create_ptr( this_t::record_cache( const_cast< this_ptr_t >( this ), db.records.size() ),
            "[io::db] : file_record" ) ;
    }

    return std::move( fr ) ;
}

//******************************************************************
bool_t database::lookup( motor::io::location_cref_t loc, this_t::file_record_out_t fro ) const noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;
    return this_t::lookup( *_db, loc, fro ) ;
}

//******************************************************************
bool_t database::lookup( this_t::db_cref_t db_, motor::io::location_cref_t loc, file_record_out_t fro ) const noexcept
{
    for( auto& fr : db_.records )
    {
        if( fr.location == loc )
        {
            fro = fr ;
            return true ;
        }
    }

    return false ;
}

//******************************************************************
void_t database::file_change_stamp( this_t::file_record_cref_t fri ) noexcept 
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;
    auto iter = std::find_if( _db->records.begin(), _db->records.end(), [&]( this_t::file_record_cref_t fr )
    {
        return fr.location == fri.location ;
    } ) ;

    if( iter == _db->records.end() ) return ;
    iter->stamp = fri.stamp ;
}

//******************************************************************
void_t database::file_remove( this_t::file_record_cref_t fri ) noexcept 
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;

    auto iter = std::remove_if( _db->records.begin(), _db->records.end(), [&] ( this_t::file_record_cref_t fr )
    {
        return fr.location == fri.location ;
    } ) ;
    if( iter != _db->records.end() ) _db->records.erase( iter ) ;
}

//******************************************************************
void_t database::file_change_external( this_t::file_record_cref_t fr ) noexcept 
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _ac ) ;
    return this_t::file_change_external( *_db, fr ) ;
}

//******************************************************************
motor::io::location_t database::location_for_index( size_t const idx ) const 
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;
    if( _db->records.size() <= idx ) return motor::io::location_t( motor::string_t( "" ) ) ;
    return _db->records[ idx ].location ;
}

//******************************************************************
void_t database::file_change_external( this_t::db_t & db,  this_t::file_record_cref_t fri ) noexcept 
{
    auto iter = std::find_if( db.records.begin(), db.records.end(), [&] ( this_t::file_record_cref_t fr )
    {
        return fr.location == fri.location ;
    } ) ;

    if( iter == db.records.end() ) return ;
    
    iter->offset = fri.offset ;
    iter->sib = fri.sib ;
    iter->rel = fri.rel ;
}

//******************************************************************
void_t database::spawn_update( void_t ) noexcept
{
    this_t::join_update() ;

    _update_thread = motor::concurrent::thread_t( [&] ( void_t )
    { 
        size_t const ms = 500 ;

        motor::log::global_t::status("[db] : update thread going up @ " + motor::from_std( std::to_string(ms) ) + " ms") ;

        while( !_isleep.sleep_for( std::chrono::milliseconds(ms) ) )
        {
            this_t::db_ptr_t db ;
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _ac ) ;
                db = _db ;
            }

            for( auto & fr : db->records )
            {
                // is internal? At the moment, there is no
                // internal db file data change tracking.
                if( fr.offset != uint64_t( -2 ) ) continue ;

                motor::io::path_t const p = db->working / fr.rel ;
                if( !motor::filesystem::exists( p ) )
                {
                    for( auto & r : db->monitors ) 
                    {
                        r->trigger_changed( fr.location, motor::io::monitor_t::notify::deletion ) ;
                    }

                    for( auto & r : fr.monitors )
                    {
                        r->trigger_changed( fr.location, motor::io::monitor_t::notify::deletion ) ;
                    }

                    this_t::file_remove( fr ) ;
                    continue ;
                }
                
                auto const tp = motor::filesystem::last_write_time( p ) ;
                if( fr.stamp != tp )
                {
                    for( auto& r : db->monitors )
                    {
                        r->trigger_changed( fr.location, motor::io::monitor_t::notify::change ) ;
                    }

                    for( auto& r : fr.monitors )
                    {
                        r->trigger_changed( fr.location, motor::io::monitor_t::notify::change ) ;
                    }

                    fr.stamp = tp ;

                    this_t::file_change_stamp( fr ) ;
                    continue ;
                }
            }
        }

        motor::log::global_t::status("[db] : update thread shutting down") ;
    } ) ;
}

//******************************************************************
void_t database::join_update( void_t ) noexcept 
{
    if( _update_thread.joinable() ) 
    { 
        _isleep.interrupt() ; 
        _update_thread.join() ; 
        _isleep.reset() ;
    }
}

