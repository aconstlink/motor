
#pragma once

#include "../object.hpp"

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

        private: // api data feeding back information to the user

            struct api_data
            {
                bool_t is_compiled ;
                motor::graphics::shader_bindings_t bindings ;
            };
            motor_typedef( api_data ) ;

            motor::concurrent::mrsw_t _mtx_api_datas ;
            motor::vector< api_data_t > _api_datas ;

        public:

            msl_object( void_t ) noexcept {}
            msl_object( motor::string_in_t name ) noexcept : _name( name ) {}

            msl_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) )
            {
                _name = std::move( rhv._name ) ;
                _datas = std::move( rhv._datas ) ;
                _geo = std::move( rhv._geo ) ;
                _soo = std::move( rhv._soo ) ;
                _api_datas = std::move( rhv._api_datas ) ;

                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;
                _vars = std::move( rhv._vars ) ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                object::operator = ( std::move( rhv ) ) ;

                _name = std::move( rhv._name ) ;
                _datas = std::move( rhv._datas ) ;
                _geo = std::move( rhv._geo ) ;
                _soo = std::move( rhv._soo ) ;
                _api_datas = std::move( rhv._api_datas ) ;

                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;
                _vars = std::move( rhv._vars ) ;

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

                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;

                _vars.resize( rhv._vars.size() ) ;
                for( size_t i=0; i<rhv._vars.size(); ++i )
                    _vars[i] = motor::memory::copy_ptr( rhv._vars[i] ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                object::operator = ( rhv ) ;

                _name = rhv._name ;
                _datas = rhv._datas ;
                _geo = rhv._geo ;
                _soo = rhv._soo ;

                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;

                _vars.resize( rhv._vars.size() ) ;
                for( size_t i=0; i<rhv._vars.size(); ++i )
                    _vars[i] = motor::memory::copy_ptr( rhv._vars[i] ) ;

                return *this ;
            }

            #endif

            virtual ~msl_object( void_t ) noexcept 
            {
                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;
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

            class backend_accessor
            {
                size_t _bid ;
                this_ptr_t _msl ;

            public:

                backend_accessor( size_t const bid, this_ptr_t msl ) noexcept : 
                    _bid( bid ), _msl( msl ) {}

                void_t set_api_data( this_t::api_data_ref_t d ) noexcept
                {
                    _msl->set_api_data( _bid, d ) ;
                }

            };
            friend class backend_accessor ;

        private:

            void_t set_api_data( size_t const bid, this_t::api_data_ref_t d ) noexcept
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_api_datas ) ;
                if( _api_datas.size() <= bid )
                {
                    _api_datas.resize( bid + 1 ) ;
                }

                _api_datas[bid] = d ;
            }

        public:

            using for_each_api_data_t = std::function < bool_t ( api_data_cref_t ) > ;

            // iterate over each api datat until f returns true ;
            // can be used to inspect generated shader data
            void_t for_each( for_each_api_data_t f ) const noexcept
            {
                for ( auto const & d : _api_datas )
                {
                    if ( f( d ) ) break ;
                }
            }
        };
        motor_typedef( msl_object ) ;
    }
}
