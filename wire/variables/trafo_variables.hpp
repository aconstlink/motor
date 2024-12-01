
#pragma once

#include "vector_variables.hpp"
#include <motor/math/utility/3d/transformation.hpp>

namespace motor
{
    namespace wire
    {
        template< typename T >
        class variable< motor::math::m3d::transformation< T > > : public motor::wire::any
        {
            using base_t = motor::wire::any ;

            motor_this_typedefs( variable< motor::math::m3d::transformation< T > > ) ;

        public:

            motor_typedefs( motor::math::m3d::transformation< T >, value ) ;

        private:

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

            using vec3_is_t = motor::wire::input_slot< motor::math::vector3< T > > ;
            using type_is_t = motor::wire::input_slot< T > ;

        private:

            vec3_is_t * _pos = motor::shared( vec3_is_t() ) ;
            vec3_is_t * _scale = motor::shared( vec3_is_t() ) ;
            vec3_is_t * _axis = motor::shared( vec3_is_t() ) ;
            type_is_t * _angle = motor::shared( type_is_t() ) ;

        public:

            variable( char const * name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}
            variable( char const * name, value_cref_t v ) noexcept : base_t( name,
                motor::shared( in_t( v ) ), motor::shared( out_t( v ) ) )
            {
                this_t::propagate_value_to_sub( v ) ;
            }

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
            {
                this_t::disconnect_and_clear() ;

                _pos = motor::move( rhv._pos ) ;
                _scale = motor::move( rhv._scale ) ;
                _axis = motor::move( rhv._axis ) ;
                _angle = motor::move( rhv._angle ) ;
            }

            virtual ~variable( void_t ) noexcept 
            {
                this_t::disconnect_and_clear() ;
            }

            void_t disconnect_and_clear( void_t ) noexcept
            {
                if( _pos != nullptr ) _pos->disconnect() ;
                if( _scale != nullptr ) _scale->disconnect() ;
                if( _axis != nullptr ) _axis->disconnect() ;
                if( _angle != nullptr ) _angle->disconnect() ;

                motor::release( motor::move( _pos ) ) ;
                motor::release( motor::move( _scale ) ) ;
                motor::release( motor::move( _axis ) ) ;
                motor::release( motor::move( _angle ) ) ;
            }

        public:

            virtual bool_t update( void_t ) noexcept
            {
                auto in_ = base_t::borrow_is<in_t>() ;
                auto out_ = base_t::borrow_os<out_t>() ;

                // wire exchanged via "push"
                // pulling data is not required.
                //in_->exchange() ;

                if ( value_t v; in_->get_value_and_reset( v ) )
                {
                    this_t::set_output_value_and_exchange( v ) ;
                    this_t::propagate_value_to_sub( v ) ;
                    return true ;
                }
                else if ( motor::math::vec4b_t( _pos->has_changed(), _scale->has_changed(),
                    _axis->has_changed(), _angle->has_changed() ).any() )
                {
                    auto const p = _pos->get_value_and_reset() ;
                    auto const s = _scale->get_value_and_reset() ;
                    auto const ax = _axis->get_value_and_reset() ;
                    auto const an = _angle->get_value_and_reset() ;

                    this_t::value_t const trafo( s, motor::math::vector_is_normalized( motor::math::vector4<T>( ax, an ) ), p ) ;

                    this_t::set_value_and_exchange( trafo ) ;

                    return true ;
                }

                return false ;
            }

            virtual motor::wire::inputs_t inputs( void_t ) noexcept
            {
                motor::string_t const name = base_t::sname() ;

                return motor::wire::inputs_t( 
                    { 
                        { name, base_t::get_is() },
                        { name + ".position", motor::share( _pos ) },
                        { name + ".scale", motor::share( _scale ) },
                        { name + ".axis", motor::share( _axis ) }, 
                        { name + ".angle", motor::share( _angle ) } 
                    }, false ) ;
            }

            value_cref_t get_value( void_t ) const noexcept
            {
                return base_t::borrow_os<out_t>()->get_value() ;
            }
            
            void_t set_value( value_cref_t v ) noexcept
            {
                auto in_ = base_t::borrow_is<in_t>() ;
                auto out_ = base_t::borrow_os<out_t>() ;

                in_->set_value( v ) ;
                out_->set_value( v ) ;
            }

            void_t set_value_and_exchange( value_cref_t v ) noexcept
            {
                auto in_ = base_t::borrow_is<in_t>() ;
                auto out_ = base_t::borrow_os<out_t>() ;

                in_->set_value( v ) ;
                out_->set_and_exchange( v ) ;
            }

            void_t set_output_value_and_exchange( value_cref_t v ) noexcept
            {
                auto out_ = base_t::borrow_os<out_t>() ;
                out_->set_and_exchange( v ) ;
            }

            typename this_t::in_t * borrow_value_is( void_t ) noexcept
            {
                return base_t::borrow_is<in_t>() ;
            }

            typename this_t::out_t * borrow_value_os( void_t ) noexcept
            {
                return base_t::borrow_os<out_t>() ;
            }

            typename motor::core::mtr_safe< this_t::out_t > get_value_os( void_t ) noexcept
            {
                return motor::share( this_t::borrow_os<out_t>() ) ;
            }

            void_t connect( motor::core::mtr_safe< in_t > && is ) noexcept
            {
                out_t * os = base_t::borrow_os<out_t>() ;
                os->connect( motor::move( is ) ) ;
            }

            void_t connect( motor::core::mtr_safe< out_t > && s ) noexcept
            {
                in_t * is = base_t::borrow_is<in_t>() ;
                is->connect( motor::move( s ) ) ;
                this_t::propagate_value_to_sub( is->get_value() ) ;
            }

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _pos->set_value( v.get_translation() ) ;

                // some cost involved!
                _scale->set_value( v.get_scale() ) ;

                // some cost involved!
                {
                    auto const orientation = v.get_orientation() ;
                    _axis->set_value( orientation.xyz() ) ;
                    _angle->set_value( orientation.w() ) ;
                }
            }
        } ;
        motor_typedefs( variable< motor::math::m3d::transformation< float_t > >, trafo3fv ) ;
    }
}
