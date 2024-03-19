
#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/controls/imodule.h>
#include <motor/controls/midi_message.hpp>
#include <motor/controls/midi_observer.hpp>
#include <motor/controls/layouts/midi_controller.hpp>

#include <motor/std/vector>
#include <motor/math/vector/vector2.hpp>

#include <windows.h>
#include <mmeapi.h>
#include <chrono>

namespace motor
{
    namespace platform
    {
        namespace win32
        {
            class MOTOR_PLATFORM_API midi_module : public motor::controls::imodule
            {
                motor_this_typedefs( midi_module ) ;

            public:

                struct global_self
                {
                    this_ptr_t self_ptr = nullptr ;
                };
                motor_typedef( global_self ) ;

            private:

                global_self_ptr_t _global_self_ptr = nullptr ;

            private: // midi devices

                struct midi_data
                {
                    motor_this_typedefs( midi_data );

                    HMIDIIN inh = NULL ;
                    HMIDIOUT outh = NULL ;
                    LPSTR in_buffer = NULL ;

                    midi_data( void_t ) noexcept {}
                    
                    midi_data( this_rref_t rhv ) noexcept
                    {
                        *this = std::move(rhv) ;
                    }

                    midi_data( this_cref_t rhv ) noexcept
                    {
                        *this = rhv ;
                    }

                    
                    this_ref_t operator = ( this_cref_t rhv ) noexcept
                    {
                        inh = rhv.inh;
                        outh = rhv.outh;
                        in_buffer = rhv.in_buffer;

                        return *this ;
                    }

                    this_ref_t operator = ( this_rref_t rhv ) noexcept
                    {
                        inh = rhv.inh ;
                        rhv.inh = NULL ;

                        outh = rhv.outh ;
                        rhv.outh = NULL ;

                        in_buffer = motor::move( rhv.in_buffer ) ;
                    
                        return *this ;
                    }
                };
                motor_typedef( midi_data ) ;

                motor_typedefs( motor::vector<midi_data_t>, midi_datas ) ;

                struct device_data
                {
                    motor_this_typedefs( device_data );

                    motor::string_t name ;
                    motor::controls::midi_device_mtr_t dev_ptr ;

                    midi_data mdata ;

                    #if 0
                    this_ref_t operator = ( this_rref_t rhv ) noexcept
                    {
                        name = rhv.name;
                        dev_ptr = motor::move( rhv.dev_ptr) ;
                        mdata = rhv.mdata;
                        return *this;
                    }

                    this_ref_t operator = ( this_cref_t rhv ) noexcept
                    {
                        name = rhv.name;
                        dev_ptr = motor::memory::copy_ptr( rhv.dev_ptr ) ;
                        mdata = rhv.mdata;
                        return *this;
                    }
                    #endif
                };
                motor_typedef( device_data ) ;
                motor_typedefs( motor::vector< device_data >, midi_devices ) ;
                

                std::mutex _devices_mtx ;
                /// will contain a active midi devices
                midi_devices_t _devices ;

                /// will contain a once active midi devices
                /// those are probably unplugged. 
                struct open_close_msg
                {
                    bool_t open ;
                    HMIDIIN hid ;
                };
                std::mutex _mtx_open_close ;
                motor::vector< open_close_msg > _need_open_close ;

                using __clock_t = std::chrono::high_resolution_clock ;
                __clock_t::time_point _tp_check_devices = __clock_t::now() ;

            private: // messages

                struct in_message
                {
                    HMIDIIN inh ;
                    motor::controls::midi_message message ;
                };
                motor_typedef( in_message ) ;
                motor_typedefs( motor::vector<in_message>, in_messages ) ;

                std::mutex _mtx_in ;
                in_messages_t _ins ;
                in_messages_t _ins_from_proc ;

            private: // observers
                
                motor_typedefs( motor::vector< motor::controls::midi_observer_mtr_t>, observers ) ;

                std::mutex _mtx_observers ;
                observers_t _observers ;

            public:

                midi_module( void_t ) noexcept ;
                midi_module( this_rref_t ) noexcept ;
                virtual ~midi_module( void_t ) noexcept ;

            public: // imodule interface

                virtual void_t search( search_funk_t ) noexcept ;
                virtual void_t update( void_t ) noexcept ;
                virtual void_t release( void_t ) noexcept ;
                
                virtual void_t install( motor::controls::iobserver_mtr_t ) noexcept ;


            public: // interface for win32 midi proc

                struct win32_midi_proc_accessor
                {
                    this_ptr_t _p ;
                    win32_midi_proc_accessor( this_ptr_t p ) noexcept : _p( p ) {}
                    void_t handle_message( HMIDIIN hin, motor::controls::midi_message_cref_t msg ) noexcept { _p->handle_message( hin, msg ) ; }
                    void_t handle_open_close( HMIDIIN hin, bool_t const open ) noexcept { _p->handle_open_close(hin,open) ; }
                };
                
            private: // interface through accessor

                void_t handle_message( HMIDIIN, motor::controls::midi_message_cref_t ) noexcept ;
                void_t handle_open_close( HMIDIIN, bool_t const open ) noexcept ;

            private:
                
                void_t create_devices( void_t ) noexcept ;
                
                // updates a time point and then every 3 seconds checks for new devices
                void_t check_for_new_devices( void_t ) noexcept ;
                void_t unregister_device( this_t::device_data_ref_t, bool_t const closed ) noexcept ;


                void_t open_midi_in( uint_t const idx, midi_data_out_t ) noexcept ;
                void_t open_midi_out( uint_t const idx, midi_data_out_t ) noexcept ;
            } ;
            motor_typedef( midi_module ) ;
        }
    }
}