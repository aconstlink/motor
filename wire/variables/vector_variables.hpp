


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

            using type_is_t = motor::wire::input_slot< T > ;

            type_is_t * _x = motor::shared( type_is_t( T(0) ) ) ;
            type_is_t * _y = motor::shared( type_is_t( T(0) ) ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( char const * const name, value_cref_t v ) noexcept : base_t( name,
                motor::shared( in_t( v ) ), motor::shared( out_t( v ) ) ),
                _x( motor::shared( type_is_t( v.x() ) ) ),
                _y( motor::shared( type_is_t( v.y() ) ) ) {}

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
            {
                this_t::disconnect_and_clear() ;

                _x = motor::move( rhv._x ) ;
                _y = motor::move( rhv._y ) ;
            }

            virtual ~variable( void_t ) noexcept 
            {
                this_t::disconnect_and_clear() ;
            }

            void_t disconnect_and_clear( void_t ) noexcept
            {
                if( _x != nullptr ) _x->disconnect() ;
                if( _y != nullptr ) _y->disconnect() ;
                motor::release( motor::move( _x ) ) ;
                motor::release( motor::move( _y ) ) ;
            }

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
                    _x->has_changed(), _y->has_changed() ).any() )
                {
                    value_t const v ( _x->get_value_and_reset(), _y->get_value_and_reset() ) ;
                    in_->set_value( v ) ;
                    out_->set_and_exchange( v ) ;
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
                        { name + ".x", motor::share( _x ) },
                        { name + ".y", motor::share( _y ) }
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

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _x->set_value( v.x() ) ;
                _y->set_value( v.y() ) ;
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

            using type_is_t = motor::wire::input_slot< T > ;

            type_is_t * _x = motor::shared( type_is_t( T(0) ) ) ;
            type_is_t * _y = motor::shared( type_is_t( T(0) ) ) ;
            type_is_t * _z = motor::shared( type_is_t( T(0) ) ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( char const * const name, value_cref_t v ) noexcept : base_t( name,
                motor::shared( in_t( v ) ), motor::shared( out_t( v ) ) ), 
                _x( motor::shared( type_is_t( v.x() ) ) ),
                _y( motor::shared( type_is_t( v.y() ) ) ),
                _z( motor::shared( type_is_t( v.z() ) ) ) {}

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
            {
                this_t::disconnect_and_clear() ;

                _x = motor::move( rhv._x ) ;
                _y = motor::move( rhv._y ) ;
                _z = motor::move( rhv._z ) ;
            }

            virtual ~variable( void_t ) noexcept
            {
                this_t::disconnect_and_clear() ;
            }

            void_t disconnect_and_clear( void_t ) noexcept
            {
                if ( _x != nullptr ) _x->disconnect() ;
                if ( _y != nullptr ) _y->disconnect() ;
                if ( _z != nullptr ) _z->disconnect() ;
                motor::release( motor::move( _x ) ) ;
                motor::release( motor::move( _y ) ) ;
                motor::release( motor::move( _z ) ) ;
            }

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
                    _x->has_changed(), _y->has_changed(), _z->has_changed() ).any() )
                {
                    value_t const v ( 
                        _x->get_value_and_reset(), 
                        _y->get_value_and_reset(),
                        _z->get_value_and_reset() ) ;
                    
                    in_->set_value( v ) ;
                    out_->set_and_exchange( v ) ;

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
                    { name + ".x", motor::share( _x ) },
                    { name + ".y", motor::share( _y ) },
                    { name + ".z", motor::share( _z ) }
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

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _x->set_value( v.x() ) ;
                _y->set_value( v.y() ) ;
                _z->set_value( v.z() ) ;
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

            using type_is_t = motor::wire::input_slot< T > ;

            type_is_t * _x = motor::shared( type_is_t( T(0) ) ) ;
            type_is_t * _y = motor::shared( type_is_t( T(0) ) ) ;
            type_is_t * _z = motor::shared( type_is_t( T(0) ) ) ;
            type_is_t * _w = motor::shared( type_is_t( T(0) ) ) ;

        public:

            variable( char const * const name ) noexcept : base_t( name,
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( char const * const name, value_cref_t v ) noexcept : base_t( name,
                motor::shared( in_t( v ) ), motor::shared( out_t( v ) ) ),
                _x( motor::shared( type_is_t( v.x() ) ) ),
                _y( motor::shared( type_is_t( v.y() ) ) ),
                _z( motor::shared( type_is_t( v.z() ) ) ),
                _w( motor::shared( type_is_t( v.w() ) ) ){}

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
            {
                this_t::disconnect_and_clear() ;

                _x = motor::move( rhv._x ) ;
                _y = motor::move( rhv._y ) ;
                _z = motor::move( rhv._z ) ;
                _w = motor::move( rhv._w ) ;
            }

            virtual ~variable( void_t ) noexcept
            {
                this_t::disconnect_and_clear() ;
            }

            void_t disconnect_and_clear( void_t ) noexcept
            {
                if ( _x != nullptr ) _x->disconnect() ;
                if ( _y != nullptr ) _y->disconnect() ;
                if ( _z != nullptr ) _z->disconnect() ;
                if ( _w != nullptr ) _w->disconnect() ;
                motor::release( motor::move( _x ) ) ;
                motor::release( motor::move( _y ) ) ;
                motor::release( motor::move( _z ) ) ;
                motor::release( motor::move( _w ) ) ;
            }

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
                    _x->has_changed(), _y->has_changed(), _z->has_changed(), _w->has_changed() ).any() )
                {
                    value_t const v (
                        _x->get_value_and_reset(),
                        _y->get_value_and_reset(),
                        _z->get_value_and_reset(),
                        _w->get_value_and_reset() ) ;

                    in_->set_value( v ) ;
                    out_->set_and_exchange( v ) ;
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
                    { name + ".x", motor::share( _x ) },
                    { name + ".y", motor::share( _y ) },
                    { name + ".z", motor::share( _z ) },
                    { name + ".w", motor::share( _w ) }
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

        private:

            void_t propagate_value_to_sub( value_cref_t v ) noexcept
            {
                _x->set_value( v.x() ) ;
                _y->set_value( v.y() ) ;
                _z->set_value( v.z() ) ;
                _w->set_value( v.w() ) ;
            }
        };
        
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