
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

            using pos_t = motor::wire::vec3v< T > ;
            using scale_t = motor::wire::vec3v< T > ;
            using axis_t = motor::wire::vec3v< T > ;
            using angle_t = motor::wire::variable< T > ;

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

        private:

            pos_t _pos = pos_t( "position" ) ;
            scale_t _scale = scale_t( "scale" ) ;
            axis_t _axis = axis_t( "axis" ) ;
            angle_t _angle = angle_t( "angle" ) ;

        public:

            variable( char const * name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}
            variable( char const * name, value_cref_t v ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) )
            {
                this_t::set_value( v ) ;
            }

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept :
                base_t( std::move( rhv ) ),
                _pos( std::move( rhv._pos ) ), _scale( std::move( rhv._scale ) ),
                _axis( std::move( rhv._axis ) ), _angle( std::move( rhv._angle ) ) {}

            virtual ~variable( void_t ) noexcept {}

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
                    *out_ = v ;
                    this_t::propagate_value_to_sub( v ) ;
                    return true ;
                }
                else if ( motor::math::vec4b_t( _pos.update(), _scale.update(),
                    _axis.update(), _angle.update() ).any() )
                {
                    auto const p = _pos.get_value() ;
                    auto const s = _scale.get_value() ;
                    auto const ax = _axis.get_value() ;
                    auto const an = _angle.get_value() ;

                    this_t::value_t const trafo( s, motor::math::vector_is_normalized( motor::math::vector4<T>( ax, an ) ), p ) ;

                    this_t::set_value( trafo ) ;

                    return true ;
                }

                return false ;
            }

            value_cref_t get_value( void_t ) const noexcept
            {
                return base_t::borrow_os<out_t>()->get_value() ;
            }

            // if the transformation is updated, other
            // costly functions are called. So only use
            // if really required.
            void_t set_value( value_cref_t v ) noexcept
            {
                auto in_ = base_t::borrow_is<in_t>() ;
                auto out_ = base_t::borrow_os<out_t>() ;

                in_->set_value( v ) ;
                out_->set_value( v ) ;
            }

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _pos.set_value( v.get_translation() ) ;

                // some cost involved!
                _scale.set_value( v.get_scale() ) ;

                // some cost involved!
                {
                    auto const orientation = v.get_orientation() ;
                    _axis.set_value( orientation.xyz() ) ;
                    _angle.set_value( orientation.w() ) ;
                }

                {
                    auto const r = v.get_rotation_matrix() ;
                    _axis.set_value( r.rotation_axis() ) ;
                    _angle.set_value( r.angle() ) ;
                }
            }

        private:

            virtual void_t for_each_member( for_each_funk_t f, motor::wire::any::member_info_in_t ifo ) noexcept
            {
                {
                    motor::string_t const full_name = ifo.full_name + "." + _pos.name() ;
                    f( _pos, { ifo.level, full_name } ) ;
                    motor::wire::any::derived_accessor( _pos ).for_each_member( f, { ifo.level + 1, full_name } ) ;
                }

                {
                    motor::string_t const full_name = ifo.full_name + "." + _axis.name() ;
                    f( _axis, { ifo.level, full_name } ) ;
                    motor::wire::any::derived_accessor( _axis ).for_each_member( f, { ifo.level + 1, full_name } ) ;
                }

                {
                    motor::string_t const full_name = ifo.full_name + "." + _angle.name() ;
                    f( _angle, { ifo.level, full_name } ) ;
                    motor::wire::any::derived_accessor( _angle ).for_each_member( f, { ifo.level + 1, full_name } ) ;
                }
            }

        } ;
        motor_typedefs( variable< motor::math::m3d::transformation< float_t > >, trafo3fv ) ;
    }
}