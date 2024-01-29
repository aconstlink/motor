#pragma once

#include "object.hpp"
#include "../enums.h"
#include "../buffer.hpp"

#include <motor/std/vector>
#include <motor/std/string>
#include <motor/math/vector/vector2.hpp>

#include <algorithm>
#include <limits>

namespace motor
{
    namespace audio
    {
        class buffer_object : public motor::audio::object
        {
            motor_this_typedefs( buffer_object ) ;
            
        private:
            
            // the frequency per second. how many samples are taken per second per channel.
            size_t _sample_rate = 0 ;
            size_t _channels = 0 ;

            motor_typedefs( motor::vector< float_t >, floats ) ;
            floats_t _samples ;

            motor::string_t _name ;

            // set by the run-time
            execution_state _es ;
            
        public:

            buffer_object( void_t ) noexcept {}
            buffer_object( motor::string_cref_t name ) noexcept : _name( name ) {}
            buffer_object( this_cref_t ) = delete ;
            buffer_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) )
            {
                _samples = std::move( rhv._samples ) ;
                _name = std::move( rhv._name ) ;
                _channels = rhv._channels;
                _sample_rate = rhv._sample_rate ;
            }

            virtual ~buffer_object( void_t ) noexcept {}

        public:

            this_ref_t operator = ( motor::audio::buffer_cref_t b ) noexcept
            {
                this_t::set_samples( b.get_channels(), b.get_sample_rate(), b.get_samples() ) ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _samples = std::move( rhv._samples ) ;
                _name = std::move( rhv._name ) ;
                _channels = rhv._channels ;
                _sample_rate = rhv._sample_rate ;
                _es = rhv._es ;
                return *this ;
            }

        public:

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

            motor::string_cref_t get_name( void_t ) const noexcept
            {
                return _name ;
            }

            // set the samples and the sample rate per channel
            // if 44.1 kHz is set, this is per channel.
            void_t set_samples( motor::audio::channels const channels, size_t const sample_rate, this_t::floats_cref_t buffer ) noexcept
            {
                _channels = motor::audio::to_number( channels ) ;
                _sample_rate = sample_rate ;
                _samples = buffer ;
            }

            void_t set_samples( motor::audio::channels const channels, size_t const sample_rate, motor::audio::buffer_cref_t buffer ) noexcept
            {
                _channels = motor::audio::to_number( channels ) ;
                _sample_rate = sample_rate ;
                buffer.copy_buffer( _samples ) ;
            }

            size_t get_sample_rate( void_t ) const noexcept
            {
                return _sample_rate ;
            }

            floats_cref_t get_samples( void_t ) const noexcept
            {
                return _samples ;
            }

            size_t get_sib( void_t ) const noexcept 
            {
                return _samples.size() * sizeof( float_t ) ;
            }

            size_t get_num_channels( void_t ) const noexcept
            {
                return _channels ;
            }

        public:

            void_t set_execution_state( motor::audio::execution_state const es ) noexcept
            {
                _es = es ;
            }

            motor::audio::execution_state get_state( void_t ) const noexcept
            {
                return _es ;
            }

        } ;
        motor_typedef( buffer_object ) ;
    }
}