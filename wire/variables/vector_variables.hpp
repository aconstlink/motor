


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
        class variable< motor::math::vector2< T > > : public motor::wire::any
        {
            using base_t = motor::wire::any ;

            motor_this_typedefs( variable< motor::math::vector2< T > > ) ;
            motor_typedefs( motor::math::vector2< T >, value ) ;

        private:

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

            using x_t = motor::wire::variable< T > ;
            using y_t = motor::wire::variable< T > ;

            x_t _x = x_t( "x" ) ;
            y_t _y = y_t( "y" ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( char const * const name, motor::wire::sub_update_strategy const us ) noexcept :
                base_t( name, motor::shared( in_t() ), motor::shared( out_t() ), us ) {}

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

            virtual void_t update_strat_changed( motor::wire::sub_update_strategy const us ) noexcept
            {
                base_t::set_update_strategy( us ) ;
                _x.set_update_strategy( us ) ;
                _y.set_update_strategy( us ) ;
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
        class variable< motor::math::vector3< T > > : public motor::wire::any
        {
            using base_t = motor::wire::any ;

            motor_this_typedefs( variable< motor::math::vector3< T > > ) ;
            motor_typedefs( motor::math::vector3< T >, value ) ;

        private:

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

            using x_t = motor::wire::variable< T > ;
            using y_t = motor::wire::variable< T > ;
            using z_t = motor::wire::variable< T > ;

            x_t _x = x_t( "x" ) ;
            y_t _y = y_t( "y" ) ;
            z_t _z = z_t( "z" ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( char const * const name, motor::wire::sub_update_strategy const us ) noexcept :
                base_t( name, motor::shared( in_t() ), motor::shared( out_t() ), us ) {}

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

            virtual void_t update_strat_changed( motor::wire::sub_update_strategy const us ) noexcept
            {
                base_t::set_update_strategy( us ) ;
                _x.set_update_strategy( us ) ;
                _y.set_update_strategy( us ) ;
                _z.set_update_strategy( us ) ;
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
        class variable< motor::math::vector4< T > > : public motor::wire::any
        {
            using base_t = motor::wire::any ;

            motor_this_typedefs( variable< motor::math::vector4< T > > ) ;
            motor_typedefs( motor::math::vector4< T >, value ) ;

        private:

            using in_t = motor::wire::input_slot< value_t > ;
            using out_t = motor::wire::output_slot< value_t > ;

            using x_t = motor::wire::variable< T > ;
            using y_t = motor::wire::variable< T > ;
            using z_t = motor::wire::variable< T > ;
            using w_t = motor::wire::variable< T > ;

            x_t _x = x_t( "x" ) ;
            y_t _y = y_t( "y" ) ;
            z_t _z = z_t( "z" ) ;
            w_t _w = z_t( "w" ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( char const * const name, motor::wire::sub_update_strategy const us ) noexcept :
                base_t( name, motor::shared( in_t() ), motor::shared( out_t() ), us ) {}

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

            virtual void_t update_strat_changed( motor::wire::sub_update_strategy const us ) noexcept
            {
                base_t::set_update_strategy( us ) ;
                _x.set_update_strategy( us ) ;
                _y.set_update_strategy( us ) ;
                _z.set_update_strategy( us ) ;
                _w.set_update_strategy( us ) ;
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

        template< typename T >
        using vec2v = motor::wire::variable< motor::math::vector2< T > > ;

        template< typename T >
        using vec3v = motor::wire::variable< motor::math::vector3< T > > ;

        template< typename T >
        using vec4v = motor::wire::variable< motor::math::vector4< T > > ;

        motor_typedefs( variable< motor::math::vector2< float_t > >, vec2fv ) ;
        motor_typedefs( variable< motor::math::vector3< float_t > >, vec3fv ) ;
        motor_typedefs( variable< motor::math::vector4< float_t > >, vec4fv ) ;

        motor_typedefs( variable< motor::math::vector2< int_t > >, vec2iv ) ;
        motor_typedefs( variable< motor::math::vector3< int_t > >, vec3iv ) ;
        motor_typedefs( variable< motor::math::vector4< int_t > >, vec4iv ) ;

        motor_typedefs( variable< motor::math::vector2< uint_t > >, vec2uiv ) ;
        motor_typedefs( variable< motor::math::vector3< uint_t > >, vec3uiv ) ;
        motor_typedefs( variable< motor::math::vector4< uint_t > >, vec4uiv ) ;
    }
}