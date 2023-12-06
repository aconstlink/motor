#pragma once

#include "../types.h"
#include "../object.hpp"

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API streamout_object : public object
        {
            motor_this_typedefs( streamout_object ) ;
           
        private:

            motor::string_t _name ;

            typedef motor::vector< motor::graphics::vertex_buffer_t > buffers_t ;
            buffers_t _dbs ;

            // used for gpu only memory. So buffers do not require any CPU memory.
            size_t _ne = 0 ;

        public:

            streamout_object( void_t ) {}

            streamout_object( motor::string_cref_t name ) : _name(name)
            {}

            streamout_object( motor::string_cref_t name, motor::graphics::vertex_buffer_cref_t vb ) : _name(name)
            {
                _dbs.emplace_back( vb ) ;
            }

            streamout_object( motor::string_cref_t name, motor::graphics::vertex_buffer_rref_t vb ) : _name(name)
            {
                _dbs.emplace_back( std::move( vb ) ) ;
            }

            streamout_object( this_cref_t rhv ) noexcept : object( rhv ) 
            {
                _dbs = rhv._dbs ;
                _name = rhv._name ;
                _ne = rhv._ne ;
            }

            streamout_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) ) 
            {
                _dbs = std::move( rhv._dbs ) ;
                _name = std::move( rhv._name ) ;
                _ne = rhv._ne ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                object::operator=( rhv ) ;

                _dbs = rhv._dbs ;
                _name = rhv._name ;
                _ne = rhv._ne ;

                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                object::operator=( std::move( rhv ) ) ;

                _dbs = std::move( rhv._dbs ) ;
                _name = std::move( rhv._name ) ;
                _ne = rhv._ne ;

                return *this ;
            }

        public:

            size_t num_buffers( void_t ) const noexcept{ return _dbs.size() ; }

            typedef std::function< void_t ( size_t const i, motor::graphics::vertex_buffer_ref_t vb ) > for_each_buffer_funk_t ;

            this_ref_t for_each( for_each_buffer_funk_t f ) noexcept
            {
                size_t i = 0 ;
                for( auto & vb : _dbs ) { f( i++, vb ) ; }
                return *this ;
            }

            this_ref_t on_buffer( size_t const i, for_each_buffer_funk_t f ) noexcept
            {
                if( i >= _dbs.size() ) return *this ;
                f( i, _dbs[i] ) ;
                return *this ;
            }

            vertex_buffer_ref_t get_buffer( size_t const i ) noexcept { return _dbs[i] ; }

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }

            // allows to resize the GPU memory without resizing the buffers gpu memory.
            // @param ne number of elements defined by the buffers layout.
            this_ref_t resize( size_t const ne ) noexcept { _ne = ne ; return *this ; }

            // @return number of elementy. The elements structure is defined by the buffers layout.
            size_t size( void_t ) const noexcept { return _ne ; }

        };
        motor_typedef( streamout_object ) ;
    }
}