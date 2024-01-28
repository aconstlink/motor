
#pragma once

#include "command_engine.h"
#include "ifrontend.h"
#include "backend.h"

namespace motor
{
    namespace audio
    {
        class MOTOR_AUDIO_API frontend : public motor::audio::ifrontend
        {
            motor_this_typedefs( frontend ) ;

        private:

            motor::audio::command_engine_ptr_t _re ;
            motor::audio::backend_ptr_t _be ;

        public:

            frontend( motor::audio::command_engine_ptr_t re, motor::audio::backend_ptr_t be ) noexcept :
                _re(re), _be(be){}

            frontend( this_cref_t ) = delete ;
            frontend( this_rref_t rhv ) noexcept
            {
                _re = motor::move( rhv._re ) ;
                _be = motor::move( rhv._be ) ;
            }
            virtual ~frontend( void_t ) noexcept {}

        public:

            template< typename T >
            this_ref_t release( motor::core::mtr_delay< T > o ) noexcept 
            {
                _re->send_execute( [=]( void_t )
                {
                    _be->release( o.mtr() ) ;
                } ) ;

                return *this ;
            }

            // takes the managed pointer and releases it after
            // the backend function is called.
            template< typename T >
            this_ref_t release( motor::core::mtr_unique< T > o ) noexcept 
            {
                _re->send_execute( [=, mtr = o.mtr() ]( void_t )
                {
                    _be->release( mtr ) ;
                    motor::memory::release_ptr( mtr ) ;
                } ) ;

                return *this ;
            }

            void_t configure( motor::audio::capture_type const ct, 
                motor::audio::capture_object_mtr_t o ) noexcept 
            {
                _re->send_execute( [=]( void_t )
                {
                    _be->configure( ct, o ) ;
                } ) ;
            }

            void_t capture( motor::audio::capture_object_mtr_t o, bool_t const b = true ) noexcept 
            {
                _re->send_execute( [=]( void_t )
                {
                    _be->capture( o, b ) ;
                } ) ;
            }

            void_t configure( motor::audio::buffer_object_mtr_t o ) noexcept
            {
                _re->send_execute( [=]( void_t )
                {
                    _be->configure( o ) ;
                } ) ;
            }

            void_t update( motor::audio::buffer_object_mtr_t o ) noexcept 
            {
                _re->send_execute( [=]( void_t )
                {
                    _be->update( o ) ;
                } ) ;
            }

            void_t execute( motor::audio::buffer_object_mtr_t o, motor::audio::backend::execute_detail_cref_t ed ) noexcept 
            {
                _re->send_execute( [=]( void_t )
                {
                    _be->execute( o, ed ) ;
                } ) ;
            }

            using fence_funk_t = std::function< void_t ( void_t ) > ;
            void_t fence( fence_funk_t funk ) noexcept
            {
                _re->send_execute( [=]( void_t )
                {
                    funk() ;
                } ) ;
            }
        };
        motor_typedef( frontend ) ;
    }
}