
#pragma once

#include "../object.hpp"
#include "../shader/compilation_listener.h"

#include "render_object.h"

namespace motor
{
    namespace graphics
    {
        enum class msl_api_type
        {
            none,
            msl_4_0         // gen 4 shader and library
        };

        class MOTOR_GRAPHICS_API msl_object : public object
        {
            motor_this_typedefs( msl_object ) ;

        private:

            // msl shader itself
            // render object (variables, render states)
            // shader object (generated shaders and i/o)
            // geometry object maybe

            motor::string_t _name ;

            motor::vector< motor::string_t > _geo ;
            motor::vector< motor::string_t > _soo ;

            struct data
            {
                msl_api_type t ;
                motor::string_t code ;
            };

            motor::vector< data > _datas ;
            motor::vector< motor::graphics::variable_set_mtr_t > _vars ;

            // default compilation listener
            motor::graphics::compilation_listener_mtr_t _comp_lst = 
                motor::shared( motor::graphics::compilation_listener() ) ;

        private:

            motor::concurrent::mrsw_t _mtx_compilation_listeners ;
            motor::vector< motor::graphics::compilation_listener_mtr_t > _compilation_listeners ;

        public:

            msl_object( void_t ) noexcept 
            {
                this_t::register_listener( motor::share( _comp_lst ) ) ;
            }

            msl_object( motor::string_in_t name ) noexcept : _name( name ) 
            {
                this_t::register_listener( motor::share( _comp_lst ) ) ;
            }

            msl_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) )
            {
                _name = std::move( rhv._name ) ;
                _datas = std::move( rhv._datas ) ;
                _geo = std::move( rhv._geo ) ;
                _soo = std::move( rhv._soo ) ;
                _comp_lst = motor::move( rhv._comp_lst ) ;

                for( auto * vs : _vars ) motor::memory::release_ptr( vs ) ;
                _vars = std::move( rhv._vars ) ;

                for( auto * l : _compilation_listeners ) motor::memory::release_ptr( l ) ;
                _compilation_listeners = std::move( rhv._compilation_listeners ) ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                object::operator = ( std::move( rhv ) ) ;

                _name = std::move( rhv._name ) ;
                _datas = std::move( rhv._datas ) ;
                _geo = std::move( rhv._geo ) ;
                _soo = std::move( rhv._soo ) ;
                _comp_lst = motor::move( rhv._comp_lst ) ;

                for( auto * vs : _vars ) motor::memory::release_ptr( vs ) ;
                _vars = std::move( rhv._vars ) ;

                for ( auto * l : _compilation_listeners ) motor::memory::release_ptr( l ) ;
                _compilation_listeners = std::move( rhv._compilation_listeners ) ;

                return *this ;
            }

            #if 0
            msl_object( this_cref_t rhv ) = delete ;
            this_ref_t operator = ( this_cref_t rhv ) = delete ;
            #else

            msl_object( this_cref_t rhv ) noexcept : object( rhv )
            {
                _name = rhv._name ;
                _datas = rhv._datas ;
                _geo = rhv._geo ;
                _soo = rhv._soo ;
                _comp_lst = motor::share( rhv._comp_lst ) ;

                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;

                _vars.resize( rhv._vars.size() ) ;
                for( size_t i=0; i<rhv._vars.size(); ++i )
                    _vars[i] = motor::memory::copy_ptr( rhv._vars[i] ) ;

                for ( auto * l : _compilation_listeners ) 
                    motor::memory::release_ptr( l ) ;

                _compilation_listeners.resize( rhv._compilation_listeners.size() ) ;
                for( size_t i=0; i<rhv._compilation_listeners.size(); ++i )
                    _compilation_listeners[ i ] = motor::share( rhv._compilation_listeners[ i ] ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                object::operator = ( rhv ) ;

                _name = rhv._name ;
                _datas = rhv._datas ;
                _geo = rhv._geo ;
                _soo = rhv._soo ;
                _comp_lst = motor::share( rhv._comp_lst ) ;

                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;

                _vars.resize( rhv._vars.size() ) ;
                for( size_t i=0; i<rhv._vars.size(); ++i )
                    _vars[i] = motor::memory::copy_ptr( rhv._vars[i] ) ;

                _compilation_listeners.resize( rhv._compilation_listeners.size() ) ;
                for ( size_t i = 0; i < rhv._compilation_listeners.size(); ++i )
                    _compilation_listeners[ i ] = motor::share( rhv._compilation_listeners[ i ] ) ;

                return *this ;
            }

            #endif

            virtual ~msl_object( void_t ) noexcept 
            {
                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;

                for( auto * lst : _compilation_listeners )
                    motor::release( motor::move( lst ) ) ;

                motor::release( motor::move( _comp_lst ) ) ;

            }

            motor::string_cref_t name( void_t ) const noexcept
            { 
                return _name ;
            }

            this_ref_t add( motor::graphics::msl_api_type const t, motor::string_in_t msl_code ) noexcept 
            {
                _datas.emplace_back( this_t::data{ t, msl_code } ) ;
                return *this ;
            }

            this_ref_t clear_shaders( void_t ) noexcept 
            {
                _datas.clear() ;
                return *this ;
            }

            typedef std::function< void_t ( motor::string_in_t ) > foreach_funk_t ;
            void_t for_each_msl( motor::graphics::msl_api_type const t, foreach_funk_t funk ) const noexcept 
            {
                for( auto & d : _datas )
                {
                    if( d.t == t ) funk( d.code ) ;
                }
            }

            this_ref_t link_geometry( motor::string_cref_t name ) noexcept 
            {
                _geo.emplace_back( name ) ;
                return *this ;
            }

            this_ref_t link_geometry( std::initializer_list< motor::string_t > const & names ) noexcept 
            {
                for( auto const & name : names ) _geo.emplace_back( name ) ;
                return *this ;
            }

            // link to stream out object so geometry can be fed from there.
            // the geometry is then mainly used for geometry layout.
            this_ref_t link_geometry( motor::string_cref_t name, motor::string_cref_t soo_name ) noexcept 
            {
                _geo.emplace_back( name ) ;
                _soo.emplace_back( soo_name ) ;
                return *this ;
            }

            motor::vector< motor::string_t > const & get_geometry( void_t ) const noexcept{ return _geo ; }
            motor::vector< motor::string_t > const & get_streamout( void_t ) const noexcept{ return _soo ; }

        public: // variable sets

            this_ref_t add_variable_set( motor::graphics::variable_set_mtr_safe_t vs ) noexcept
            {
                _vars.emplace_back( vs ) ;
                return *this ;
            }

            motor::vector< motor::graphics::variable_set_mtr_safe_t > get_varibale_sets( void_t ) const noexcept
            {
                motor::vector< motor::graphics::variable_set_mtr_safe_t > ret ;

                for( auto * mtr : _vars ) ret.emplace_back( motor::share( mtr ) ) ;

                return ret ;
            }

            motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > borrow_varibale_sets( void_t ) const noexcept
            {
                return _vars ;
            }

            typedef std::function< void_t ( size_t const i, motor::graphics::variable_set_mtr_t ) > for_each_var_funk_t ;

            void_t for_each( for_each_var_funk_t funk ) noexcept
            {
                size_t i = 0 ;
                for( auto const & v : _vars )
                {
                    funk( i++, v ) ;
                }
            }

        public:

            void_t register_listener( motor::graphics::compilation_listener_mtr_safe_t lst ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_compilation_listeners ) ;

                auto iter = std::find_if( _compilation_listeners.begin(), _compilation_listeners.end(),
                    [&] ( motor::graphics::compilation_listener_mtr_t l )
                {
                    return l == lst ;
                } ) ;

                if ( iter != _compilation_listeners.end() )
                {
                    motor::release( lst ) ;
                    return ;
                }

                _compilation_listeners.push_back( motor::move( lst ) ) ;
            }

            void_t unregister_listener( motor::graphics::compilation_listener_mtr_t lst ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_compilation_listeners ) ;

                auto iter = std::find_if( _compilation_listeners.begin(), _compilation_listeners.end(),
                    [&] ( motor::graphics::compilation_listener_mtr_t l )
                {
                    return l == lst ;
                } ) ;

                if ( iter == _compilation_listeners.end() ) return ;

                motor::release( motor::move( *iter ) ) ;
                _compilation_listeners.erase( iter ) ;
            }

            using for_each_change_listerner_funk_t = std::function< void_t ( motor::graphics::compilation_listener_mtr_t ) > ;
            void_t for_each( for_each_change_listerner_funk_t f ) noexcept
            {
                motor::concurrent::mrsw_t::reader_lock lk( _mtx_compilation_listeners ) ;
                for ( auto * lst : _compilation_listeners ) f( lst ) ;
            }

            motor::graphics::compilation_listener_mtr_t borrow_compilation_listener( void_t ) const noexcept
            {
                return _comp_lst ;
            }

            bool_t has_shader_changed( void_t ) const noexcept
            {
                return _comp_lst->has_changed() ;
            }

            bool_t reset_and_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
            {
                return _comp_lst->reset_and_successful( sb ) ;
            }
        };
        motor_typedef( msl_object ) ;
    }
}
