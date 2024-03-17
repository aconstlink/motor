
#include "midi_module.h"


#include <motor/log/global.h>

#include <algorithm>

using namespace motor::platform ;
using namespace motor::platform::win32 ;


namespace this_file
{
    typedef std::function< void_t ( uint_t const idx, MIDIINCAPS const & ) > midi_in_caps_funk_t ;
    void_t for_each_device( midi_in_caps_funk_t funk )
    {
        for( uint_t i = 0; i < midiInGetNumDevs(); ++i )
        {
            MIDIINCAPS caps ;
            ZeroMemory( &caps, sizeof(MIDIINCAPS) ) ;

            MMRESULT const res = midiInGetDevCaps( i, &caps, sizeof(MIDIINCAPS) ) ;
            if( res != MMSYSERR_NOERROR )
            {
                motor::log::global::warning( 
                    "[midi_module::for_each_device] : unable to retrieve midi in caps" ) ;
                continue ;
            }

            funk( i, caps ) ;
        }
    }

    typedef std::function< void_t ( uint_t const idx, MIDIOUTCAPS const & ) > midi_out_caps_funk_t ;
    void_t for_each_device( midi_out_caps_funk_t funk )
    {
        for( uint_t i = 0; i < midiOutGetNumDevs(); ++i )
        {
            MIDIOUTCAPS caps ;
            ZeroMemory( &caps, sizeof(MIDIOUTCAPS) ) ;

            MMRESULT const res = midiOutGetDevCaps( i, &caps, sizeof(MIDIOUTCAPS) ) ;
            if( res != MMSYSERR_NOERROR )
            {
                motor::log::global::warning( 
                    "[midi_module::for_each_device] : unable to retrieve midi out caps" ) ;
                continue ;
            }

            funk( i, caps ) ;
        }
    }
}

//****************************************************************************************
void CALLBACK midi_in_proc(
    HMIDIIN   hMidiIn,
    UINT      wMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
    )
{
    motor::platform::win32::midi_module::global_self_ptr_t gsptr = 
        (motor::platform::win32::midi_module::global_self_ptr_t)(dwInstance) ;
    
    if( gsptr == nullptr )
    {
        motor::log::global::error( "[midi_in_proc] : nullptr " ) ;
        return ;
    }

    motor::platform::win32::midi_module_t::win32_midi_proc_accessor ac( gsptr->self_ptr ) ;

    if( wMsg == MIM_OPEN )
    {
        ac.handle_open_close( hMidiIn, true ) ;
    }
    else if( wMsg == MIM_CLOSE )
    {
        ac.handle_open_close( hMidiIn, false ) ;
    }
    else if( wMsg == MIM_DATA )
    {
        WORD const hiword = HIWORD( dwParam1 ) ;
        WORD const loword = LOWORD( dwParam1 ) ;

        BYTE const status = LOBYTE( loword ) ;
        BYTE const byte1 = HIBYTE( loword ) ;
        BYTE const byte2 = LOBYTE( hiword ) ;
        BYTE const byte3 = HIBYTE( hiword ) ;

        ac.handle_message( hMidiIn, 
            motor::controls::midi_message( status, byte1, byte2, byte3 ) ) ;
    }
    else if( wMsg == MIM_LONGDATA )
    {}
    else if( wMsg == MIM_ERROR )
    {}
    else if( wMsg == MIM_LONGERROR )
    {}
    else if( wMsg == MIM_MOREDATA )
    {}
}

//****************************************************************************************
void CALLBACK midi_out_proc(
    HMIDIOUT   hmo,
    UINT      wMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
    )
{
}

//****************************************************************************************
midi_module::midi_module( void_t ) noexcept
{
    _global_self_ptr = motor::memory::global_t::alloc<this_t::global_self_t>(
        "[platform::midi_module] : global self") ;
    _global_self_ptr->self_ptr = this ;

    this_t::check_for_new_devices() ;
}

//****************************************************************************************
midi_module::midi_module( this_rref_t rhv ) noexcept
{
    _ins = std::move( rhv._ins ) ;

    _global_self_ptr = std::move( rhv._global_self_ptr ) ;
    _global_self_ptr->self_ptr = this ;

    _devices = std::move( rhv._devices ) ;
    //_midi_notifies = std::move( rhv._midi_notifies ) ;
}

//****************************************************************************************
midi_module::~midi_module( void_t ) noexcept
{
    this_t::release() ;
}

//****************************************************************************************
void_t midi_module::release( void_t ) noexcept 
{
    for( auto & d : _devices )
    {
        motor::memory::release_ptr( d.dev_ptr ) ;
        this_t::unregister_device( d, false ) ;
    }
    _devices.clear() ;

    for( auto * ptr : _observers )
    {
        motor::memory::release_ptr( ptr ) ;
    }
    _observers.clear() ;

    motor::memory::global_t::dealloc( motor::move( _global_self_ptr  ) ) ;
}

//****************************************************************************************
void_t midi_module::install( motor::controls::iobserver_mtr_t ptr ) noexcept 
{
    if( motor::controls::midi_observer_ptr_t ptr_ = dynamic_cast<motor::controls::midi_observer_ptr_t>( ptr ); ptr_!= nullptr  )
    {
        std::lock_guard< std::mutex > lk( _mtx_observers ) ;
        auto iter = std::find_if( _observers.begin(), _observers.end(), [&]( motor::controls::midi_observer_mtr_t ptr_in )
        {
            return ptr_in == ptr_ ;
        } ) ;
        
        if( iter == _observers.end() ) 
        {
            _observers.push_back( motor::share( ptr_ ) ) ;
        }
    }
}

//****************************************************************************************
void_t midi_module::search( search_funk_t funk ) noexcept 
{
    std::lock_guard< std::mutex > lk( _devices_mtx ) ;
    for( auto & item : _devices )
    {
        funk( item.dev_ptr ) ;
    }
}

//****************************************************************************************
void_t midi_module::unregister_device( midi_module::device_data_ref_t item, bool_t const closed ) noexcept
{
    if( item.mdata.inh != NULL )
    {
        // reset according to msdn. Otherwise midiInClose will fail due to
        // midiInAddBuffer used here.
        {
            auto res = midiInReset( item.mdata.inh ) ;
            motor::log::global::error( res != MMSYSERR_NOERROR,
                "[motor::controls::win32::midi_module::unregister_device] : midiInReset" ) ;
        }

        {
            MIDIHDR mhdr ;
            ZeroMemory( &mhdr, sizeof(MIDIHDR) ) ;

            auto res = midiInUnprepareHeader( item.mdata.inh, &mhdr, sizeof(MIDIHDR) ) ;
            motor::log::global::error( res != MMSYSERR_NOERROR,
                "[motor::controls::win32::midi_module::unregister_device] : midiInUnprepareHeader" ) ;

            motor::memory::global_t::dealloc_raw( item.mdata.in_buffer ) ;
        }
        {
            auto res = midiInClose( item.mdata.inh ) ;
            motor::log::global::error( res != MMSYSERR_NOERROR,
                "[motor::controls::win32::midi_module::unregister_device] : midiInClose" ) ;
        }
        item.mdata.inh = NULL ;
    }

    if( item.mdata.outh != NULL )
    {
        // can not remember what this was for, maybe resetting the device?
        if( !closed )
        {
            auto res = midiOutShortMsg( item.mdata.outh, byte_t(255) ) ;
            motor::log::global::error( res != MMSYSERR_NOERROR,
                "[motor::controls::win32::midi_module::unregister_device] : sending -1 failed" ) ;
        }

        auto res = midiOutClose( item.mdata.outh ) ;
        motor::log::global::error( res != MMSYSERR_NOERROR,
            "[motor::controls::win32::midi_module::unregister_device] : midiInClose" ) ;
        item.mdata.outh = NULL ;
    }
}

//****************************************************************************************
void_t midi_module::update( void_t ) noexcept
{
    // time interval
    // check periodically, so devices can be 
    // connected afterwards
    {
        this_t::check_for_new_devices() ;
    }

    // handle all closed devices
    {
        std::lock_guard< std::mutex > lk( _mtx_open_close ) ;
        for( auto & d : _need_open_close )
        {
            auto iter = std::find_if( _devices.begin(), _devices.end(), [&]( this_t::device_data_cref_t dd )
            {
                return dd.mdata.inh == d.hid ;
            } ) ;

            if( !d.open )
            {
                this_t::unregister_device( *iter, true ) ;
                motor::log::global_t::status( "Midi device is unplugged : " + iter->name ) ;
            }
            else 
            {
                motor::log::global_t::status( "Midi device ready for use : " + iter->name ) ;
            }
        }
        _need_open_close.clear() ;
    }

    for( auto & item : _devices )
    {
        // was once registered but was plugged out.
        if( item.mdata.inh == NULL || item.mdata.outh == NULL ) continue ;

        #if 0
        motor::controls::midi_device_t::midi_messages_t msgs ;
        item.dev_ptr->update( msgs ) ;
        

        for( auto & msg : msgs )
        {
            MMRESULT res = midiOutShortMsg( item.mdata.outh, (DWORD) midi_message::to_32bit( msg ) ) ;
            motor::log::global::error( res != MMSYSERR_NOERROR,
                "[midi_module::transmit_message] : midiOutShortMsg " ) ;
        }
        #endif

        {
            auto res = midiInReset( item.mdata.inh ) ;
            motor::log::global_t::error( res != MMSYSERR_NOERROR,
                "[motor::controls::win32::midi_module::update] : midiInReset" ) ;
        }

        {
            auto res = midiInStart( item.mdata.inh ) ;
            motor::log::global::error( res != MMSYSERR_NOERROR,
                "[motor::controls::win32::midi_module::update] : midiInStart" ) ;
        }

        //midiInStart( item.mdata.inh ) ;
        //midiInReset( item.mdata.inh ) ;
    }
}

//****************************************************************************************
void_t midi_module::handle_message( HMIDIIN hin, motor::controls::midi_message_cref_t msg ) noexcept
{
    for( auto & item : _devices )
    {
        if( item.mdata.inh == hin )
        {
            #if 0
            for( auto * ptr : _midi_notifies )
            {
                ptr->on_message( item.dev_ptr->get_device_info().device_name, msg ) ;
            }

            item.dev_ptr->receive_message( msg ) ;
            #endif
        }
    }

    {
        
        this_t::observers_t tmp ;
        
        {
            std::lock_guard< std::mutex > lk( _mtx_observers ) ;
            tmp = std::move( _observers ) ;
        }

        UINT id = 0 ;
        MIDIINCAPS caps ;
        ZeroMemory( &caps, sizeof(MIDIINCAPS) ) ;
        midiInGetID( hin, &id ) ;
        midiInGetDevCaps( id, &caps, sizeof(MIDIINCAPS) ) ;

        for( auto * obs : tmp )
        {
            obs->on_message( motor::string_t( caps.szPname ), msg ) ;
        }
        
        {
            std::lock_guard< std::mutex > lk( _mtx_observers ) ;
            assert( _observers.size() == 0 ) ;
            _observers = std::move( tmp ) ;
        }
    }
}

//****************************************************************************************
void_t midi_module::handle_open_close( HMIDIIN hin, bool_t const open ) noexcept 
{
    #if 0 // remember this snippet. Get caps by midi handle
    UINT id = 0 ;
    MIDIINCAPS caps ;
    ZeroMemory( &caps, sizeof(MIDIINCAPS) ) ;
    midiInGetID( hin, &id ) ;
    midiInGetDevCaps( id, &caps, sizeof(MIDIINCAPS) ) ; 
    #endif

    std::lock_guard< std::mutex > lk( _mtx_open_close ) ;
    _need_open_close.emplace_back( this_t::open_close_msg{ open, hin } ) ;
}

//****************************************************************************************
void_t midi_module::create_devices( void_t ) noexcept
{
    this_file::for_each_device( [&]( uint_t const idx, MIDIINCAPS const & caps )
    {
        // check if the device is already in the list by checking the name.
        // unfortunately, only the name is usable from the caps data.
        // !manufacturer id and product id does not work!
        for( auto & d : _devices )
        {
            if( d.name == motor::string_t( caps.szPname ) )
            {
                if( d.mdata.inh == NULL )
                {
                    motor::log::global_t::status( "Open Midi In : " + d.name ) ;
                    this_t::open_midi_in( idx, d.mdata ) ;
                }
                return ;
            }
        }

        // create a new device
        // fill in the components based on the devices' name 
        // e.g nanoPAD2, MIDI Mix, etc
        // need to create an actual device based on the device name.
        {
            motor::controls::midi_device_t mdev = motor::controls::midi_device_t( motor::string_t( caps.szPname ) ) ;

            this_t::device_data sd ;
            sd.name = motor::string_t( caps.szPname ) ;
            sd.dev_ptr = motor::shared( std::move(mdev) ) ;
            this_t::open_midi_in( idx, sd.mdata ) ;

            _devices.emplace_back( std::move( sd ) ) ;
        }
    } ) ;

    this_file::for_each_device( [&]( uint_t const idx, MIDIOUTCAPS const & caps )
    {
        // only create an output device if we have an input 
        // device already created.
        for( auto & d : _devices )
        {
            if( d.name == motor::string_t( caps.szPname ) )
            {
                if( d.mdata.outh == NULL )
                {
                    motor::log::global_t::status( "Open Midi Out : " + d.name ) ;
                    this_t::open_midi_out( idx, d.mdata ) ;
                }
                return ;
            }
        }
    } ) ;
}

//****************************************************************************************
void_t midi_module::open_midi_in( uint_t const idx, midi_data_out_t md ) noexcept 
{
    HMIDIIN hmin ;
        
    auto const res = midiInOpen( &hmin, idx, (DWORD_PTR) midi_in_proc, 
        (DWORD_PTR)_global_self_ptr, CALLBACK_FUNCTION | MIDI_IO_STATUS ) ;

    motor::log::global::warning( res != MMSYSERR_NOERROR, 
        "[motor::controls::midi_module::check_handle_for_device] : \
        Input MIDI device can not be opended" ) ;
        
    if( res == MMSYSERR_NOERROR )
    {
        MIDIHDR midihdr ;
        memset( &midihdr, 0, sizeof(MIDIHDR) );

        midihdr.dwBufferLength = 10 ;
        midihdr.lpData = (LPSTR)motor::memory::global_t::alloc_raw<uchar_t>(10, 
            "[motor::controls::win32::midi_module::check_handle_for_device] : MIDIHDR.lpData" ) ;
        midihdr.dwFlags = 0 ;
            
        md.in_buffer = midihdr.lpData ;

        auto const prep_res = midiInPrepareHeader( hmin, &midihdr, sizeof( MIDIHDR ) ) ;
        motor::log::global::error( prep_res != MMSYSERR_NOERROR, 
            "[motor::controls::midi_module::check_handle_for_device] : midiInPrepareHeader" ) ;

        auto const add_res = midiInAddBuffer( hmin, &midihdr, sizeof( MIDIHDR ) ) ;
        motor::log::global::error( prep_res != MMSYSERR_NOERROR, 
            "[motor::controls::midi_module::check_handle_for_device] : midiInAddBuffer" ) ;
            
        if( prep_res == MMSYSERR_NOERROR && add_res == MMSYSERR_NOERROR )
        {
            midiInStart( hmin ) ;
            md.inh = hmin ;
        }
    } 
}

//****************************************************************************************
void_t midi_module::open_midi_out( uint_t const idx, midi_data_out_t md ) noexcept 
{
    HMIDIOUT hmout ;
    {
        auto const res = midiOutOpen( &hmout, idx, (DWORD_PTR)midi_out_proc, 
            (DWORD_PTR)_global_self_ptr, CALLBACK_FUNCTION ) ;

        motor::log::global::error( res != MMSYSERR_NOERROR, 
            "[motor::controls::midi_module::check_handle_for_device] : \
            Output MIDI device can not be opended" ) ;
        if( res == MMSYSERR_NOERROR )
        {
            md.outh = hmout ;
        }
    }
}

//****************************************************************************************
void_t midi_module::check_for_new_devices( void_t ) noexcept
{
    auto d = this_t::__clock_t::now() - _tp_check_devices ;
    if( d > std::chrono::seconds( 3 ) )
    {
        this_t::create_devices() ;
        _tp_check_devices = this_t::__clock_t::now() ;
    }
}