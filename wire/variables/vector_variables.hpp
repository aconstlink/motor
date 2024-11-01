


#pragma once

#include "../variable.hpp"
#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

namespace motor
{
    namespace wire
    {
        template< typename T >
        class variable< motor::math::vector2< T >, motor::wire::detailed_trait > : public motor::wire::any, public motor::wire::detailed_trait
        {
            using base_t = motor::wire::any ;

            motor_this_typedefs( variable< motor::math::vector2< T > motor_comma motor::wire::detailed_trait > ) ;
            motor_typedefs( motor::math::vector2< T >, value ) ;

            using trait_t = motor::wire::detailed_trait ;
        private:

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

            using x_t = motor::wire::variable< T, trait_t > ;
            using y_t = motor::wire::variable< T, trait_t > ;

            x_t _x = x_t( "x" ) ;
            y_t _y = y_t( "y" ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept :
                base_t( std::move( rhv ) ),
                _x( std::move( rhv._x ) ), _y( std::move( rhv._y ) ) {}

            virtual ~variable( void_t ) noexcept {}

        public:

            virtual bool_t update( void_t ) noexcept
            {
                auto in_ = base_t::borrow_is<in_t>() ;
                auto out_ = base_t::borrow_os<out_t>() ;

                // wire exchanged via "push"
                // pulling data is not required.
                //in_->exchange() ;

                if ( value_t a; in_->get_value_and_reset( a ) )
                {
                    *out_ = a ;
                    this_t::propagate_value_to_sub( a ) ;
                    return true ;
                }
                else if ( motor::math::vec2b_t(
                    _x.update(), _y.update() ).any() )
                {
                    value_t const v ( _x.get_value(), _y.get_value() ) ;
                    in_->set_value( v ) ;
                    *out_ = v ;
                    return true ;
                }

                return false ;
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

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _x.set_value( v.x() ) ;
                _y.set_value( v.y() ) ;
            }

        private:

            virtual void_t for_each_member( for_each_funk_t f, motor::wire::any::member_info_in_t ifo ) noexcept
            {
                f( _x, { ifo.level + 1, ifo.full_name + "." + _x.name() } ) ;
                f( _y, { ifo.level + 1, ifo.full_name + "." + _y.name() } ) ;
            }
        };

        template< typename T >
        class variable< motor::math::vector3< T >, motor::wire::detailed_trait > : public motor::wire::any, public motor::wire::detailed_trait
        {
            using base_t = motor::wire::any ;

            motor_this_typedefs( variable< motor::math::vector3< T > motor_comma motor::wire::detailed_trait > ) ;
            motor_typedefs( motor::math::vector3< T >, value ) ;

            using trait_t = motor::wire::detailed_trait ;

        private:

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

            using x_t = motor::wire::variable< T, trait_t> ;
            using y_t = motor::wire::variable< T, trait_t > ;
            using z_t = motor::wire::variable< T, trait_t > ;

            x_t _x = x_t( "x" ) ;
            y_t _y = y_t( "y" ) ;
            z_t _z = z_t( "z" ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept :
                base_t( std::move( rhv ) ),
                _x( std::move( rhv._x ) ), _y( std::move( rhv._y ) ),
                _z( std::move( rhv._z ) ) {}

            virtual ~variable( void_t ) noexcept {}

        public:

            virtual bool_t update( void_t ) noexcept
            {
                auto in_ = base_t::borrow_is<in_t>() ;
                auto out_ = base_t::borrow_os<out_t>() ;

                // wire exchanged via "push"
                // pulling data is not required.
                //in_->exchange() ;

                if ( value_t a; in_->get_value_and_reset( a ) )
                {
                    *out_ = a ;
                    this_t::propagate_value_to_sub( a ) ;
                    return true ;
                }
                else if ( motor::math::vec3b_t(
                    _x.update(), _y.update(), _z.update() ).any() )
                {
                    value_t const v ( _x.get_value(), _y.get_value(), _z.get_value() ) ;
                    in_->set_value( v ) ;
                    *out_ = v ;
                    return true ;
                }

                return false ;
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

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _x.set_value( v.x() ) ;
                _y.set_value( v.y() ) ;
                _z.set_value( v.z() ) ;
            }

        private:

            virtual void_t for_each_member( for_each_funk_t f, motor::wire::any::member_info_in_t ifo ) noexcept
            {
                f( _x, { ifo.level + 1, ifo.full_name + "." + _x.name() } ) ;
                f( _y, { ifo.level + 1, ifo.full_name + "." + _y.name() } ) ;
                f( _z, { ifo.level + 1, ifo.full_name + "." + _z.name() } ) ;
            }
        };

        template< typename T >
        class variable< motor::math::vector4< T >, motor::wire::detailed_trait > : public motor::wire::any, public motor::wire::detailed_trait
        {
            using base_t = motor::wire::any ;

            motor_this_typedefs( variable< motor::math::vector4< T > motor_comma motor::wire::detailed_trait > ) ;
            motor_typedefs( motor::math::vector4< T >, value ) ;

            using trait_t = motor::wire::detailed_trait ;

        private:

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

            using x_t = motor::wire::variable< T, trait_t > ;
            using y_t = motor::wire::variable< T, trait_t > ;
            using z_t = motor::wire::variable< T, trait_t > ;
            using w_t = motor::wire::variable< T, trait_t > ;

            x_t _x = x_t( "x" ) ;
            y_t _y = y_t( "y" ) ;
            z_t _z = z_t( "z" ) ;
            w_t _w = z_t( "w" ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept :
                base_t( std::move( rhv ) ),
                _x( std::move( rhv._x ) ), _y( std::move( rhv._y ) ),
                _z( std::move( rhv._z ) ), _w( std::move( rhv._w ) ) {}

            virtual ~variable( void_t ) noexcept {}

        public:

            virtual bool_t update( void_t ) noexcept
            {
                auto in_ = base_t::borrow_is<in_t>() ;
                auto out_ = base_t::borrow_os<out_t>() ;

                // wire exchanged via "push"
                // pulling data is not required.
                //in_->exchange() ;

                if ( value_t a; in_->get_value_and_reset( a ) )
                {
                    *out_ = a ;
                    this_t::propagate_value_to_sub( a ) ;
                    return true ;
                }
                else if ( motor::math::vec4b_t(
                    _x.update(), _y.update(), _z.update(), _w.update() ).any() )
                {
                    value_t const v ( _x.get_value(), _y.get_value(), _z.get_value(), , _w.get_value() ) ;
                    in_->set_value( v ) ;
                    *out_ = v ;
                    return true ;
                }

                return false ;
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

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _x.set_value( v.x() ) ;
                _y.set_value( v.y() ) ;
                _z.set_value( v.z() ) ;
                _w.set_value( v.w() ) ;
            }

        private:

            virtual void_t for_each_member( for_each_funk_t f, motor::wire::any::member_info_in_t ifo ) noexcept
            {
                f( _x, { ifo.level + 1, ifo.full_name + "." + _x.name() } ) ;
                f( _y, { ifo.level + 1, ifo.full_name + "." + _y.name() } ) ;
                f( _z, { ifo.level + 1, ifo.full_name + "." + _z.name() } ) ;
                f( _w, { ifo.level + 1, ifo.full_name + "." + _w.name() } ) ;
            }
        };
                
        template< typename T, typename trait_t >
        using vec2v = motor::wire::variable< motor::math::vector2< T >, trait_t > ;

        template< typename T, typename trait_t >
        using vec3v = motor::wire::variable< motor::math::vector3< T >, trait_t > ;

        template< typename T, typename trait_t>
        using vec4v = motor::wire::variable< motor::math::vector4< T >, trait_t > ;

        motor_typedefs( variable< motor::math::vector2< float_t > motor_comma motor::wire::detailed_trait >, vec2fvd ) ;
        motor_typedefs( variable< motor::math::vector3< float_t > motor_comma motor::wire::detailed_trait >, vec3fvd ) ;
        motor_typedefs( variable< motor::math::vector4< float_t > motor_comma motor::wire::detailed_trait >, vec4fvd ) ;

        motor_typedefs( variable< motor::math::vector2< int_t > motor_comma motor::wire::detailed_trait >, vec2ivd ) ;
        motor_typedefs( variable< motor::math::vector3< int_t > motor_comma motor::wire::detailed_trait >, vec3ivd ) ;
        motor_typedefs( variable< motor::math::vector4< int_t > motor_comma motor::wire::detailed_trait >, vec4ivd ) ;

        motor_typedefs( variable< motor::math::vector2< uint_t > motor_comma motor::wire::detailed_trait >, vec2uivd ) ;
        motor_typedefs( variable< motor::math::vector3< uint_t > motor_comma motor::wire::detailed_trait >, vec3uivd ) ;
        motor_typedefs( variable< motor::math::vector4< uint_t > motor_comma motor::wire::detailed_trait >, vec4uivd ) ;

        motor_typedefs( variable< motor::math::vector2< float_t > motor_comma motor::wire::simple_trait >, vec2fv ) ;
        motor_typedefs( variable< motor::math::vector3< float_t > motor_comma motor::wire::simple_trait >, vec3fv ) ;
        motor_typedefs( variable< motor::math::vector4< float_t > motor_comma motor::wire::simple_trait >, vec4fv ) ;

        motor_typedefs( variable< motor::math::vector2< int_t > motor_comma motor::wire::simple_trait >, vec2iv ) ;
        motor_typedefs( variable< motor::math::vector3< int_t > motor_comma motor::wire::simple_trait >, vec3iv ) ;
        motor_typedefs( variable< motor::math::vector4< int_t > motor_comma motor::wire::simple_trait >, vec4iv ) ;

        motor_typedefs( variable< motor::math::vector2< uint_t > motor_comma motor::wire::simple_trait >, vec2uiv ) ;
        motor_typedefs( variable< motor::math::vector3< uint_t > motor_comma motor::wire::simple_trait >, vec3uiv ) ;
        motor_typedefs( variable< motor::math::vector4< uint_t > motor_comma motor::wire::simple_trait >, vec4uiv ) ;
    }
}