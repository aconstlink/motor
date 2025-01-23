
#pragma once

#include "particle.h"

#include <motor/concurrent/parallel_for.hpp>
#include <motor/std/vector_pod>
#include <motor/math/utility/constants.hpp>

namespace motor
{
    namespace physics
    {
        class force_field
        {
        private:
                
            motor::math::vec2f_t _pos ;
            float_t _radius = -1.0f ;
            float_t _radius2 = 0.0f ;

        public:

            void_t set_radius( float_t const r ) noexcept { _radius = r ; _radius2 = r * r ; }
            float_t get_radius( void_t ) const noexcept { return _radius ; }

            bool_t is_inside( particle_in_t p ) const noexcept
            {
                return _radius < 0.0f || (p.pos - _pos).length2() < _radius2 ;
            }

        public:

            virtual void_t apply( size_t const beg, size_t const n, motor::vector_pod< particle_t > & particles ) const noexcept = 0 ;
        } ;
        motor_typedef( force_field ) ;

        class constant_force_field : public force_field
        {
            motor_this_typedefs( constant_force_field ) ;

        private:

            motor::math::vec2f_t _force ;

        public:

            void_t set_force( motor::math::vec2f_cref_t v ) noexcept
            {
                _force = v ;
            }

        public:

            virtual void_t apply( size_t const beg, size_t const n, motor::vector_pod< particle_t > & particles ) const noexcept 
            {
                #if !MOTOR_PHYSICS_USE_PARALLEL_FOR
                for( size_t i=beg; i<beg+n; ++i )
                {
                    physics::particle_ref_t p = particles[i] ;
                    if( !this_t::is_inside( p ) ) continue ;
                    p.force += _force ;
                }
                #else
                motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>( beg, beg+n ),
                    [&]( motor::concurrent::range_1d<size_t> const & r )
                    {
                        for( size_t i=r.begin(); i<r.end(); ++i )
                        {
                            physics::particle_ref_t p = particles[i] ;
                            if( !this_t::is_inside( p ) ) continue ;
                            p.force += _force ;
                        }
                    } ) ;
                #endif
            }
        } ;
        motor_typedef( constant_force_field ) ;

        class acceleration_field : public force_field
        {
            motor_this_typedefs( acceleration_field ) ;

        private:

            motor::math::vec2f_t _accl ;

        public:

            acceleration_field( void_t ) noexcept {}
            acceleration_field( motor::math::vec2f_cref_t accl ) noexcept : _accl( accl ) {}

            void_t set_acceleration( motor::math::vec2f_cref_t v ) noexcept
            {
                _accl = v ;
            }

        public:

            virtual void_t apply( size_t const beg, size_t const n, motor::vector_pod< particle_t > & particles ) const noexcept 
            {
                #if !MOTOR_PHYSICS_USE_PARALLEL_FOR
                for( size_t i=beg; i<beg+n; ++i )
                {
                    physics::particle_ref_t p = particles[i] ;
                    if( !this_t::is_inside( p ) ) continue ;
                    p.force += _accl * p.mass ;
                }
                #else
                motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>( beg, beg+n ),
                    [&]( motor::concurrent::range_1d<size_t> const & r )
                    {
                        for( size_t i=r.begin(); i<r.end(); ++i )
                        {
                            physics::particle_ref_t p = particles[i] ;
                            if( !this_t::is_inside( p ) ) continue ;
                            p.force += _accl * p.mass ;
                        }
                    } ) ;
                #endif
            }
        } ;
        motor_typedef( acceleration_field ) ;

        class friction_force_field : public force_field
        {
            motor_this_typedefs( friction_force_field ) ;

        private:

            float_t _friction = 0.4f ;
        
        public:

            virtual void_t apply( size_t const beg, size_t const n, motor::vector_pod< particle_t > & particles ) const noexcept 
            {
                #if !MOTOR_PHYSICS_USE_PARALLEL_FOR
                for( size_t i=beg; i<beg+n; ++i )
                {
                    physics::particle_ref_t p = particles[i] ;
                    if( !this_t::is_inside( p ) ) continue ;
                    p.force -= motor::math::vec2f_t( _friction ) * p.force ;
                }
                #else
                motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>( beg, beg+n ),
                    [&]( motor::concurrent::range_1d<size_t> const & r )
                    {
                        for( size_t i=r.begin(); i<r.end(); ++i )
                        {
                            physics::particle_ref_t p = particles[i] ;
                            if( !this_t::is_inside( p ) ) continue ;
                            p.force -= motor::math::vec2f_t( _friction ) * p.force ;
                        }
                    } ) ;
                #endif
            }
        } ;
        motor_typedef( friction_force_field ) ;

        class viscosity_force_field : public force_field
        {
            motor_this_typedefs( viscosity_force_field ) ;

        private:

            float_t _friction = 0.1f ;
        
        public:

            viscosity_force_field( void_t ) noexcept {}
            viscosity_force_field( float_t const friction ) noexcept { _friction = friction ; }

        public:

            virtual void_t apply( size_t const beg, size_t const n, motor::vector_pod< particle_t > & particles ) const noexcept 
            {
                #if !MOTOR_PHYSICS_USE_PARALLEL_FOR
                for( size_t i=beg; i<beg+n; ++i )
                {
                    physics::particle_ref_t p = particles[i] ;
                    if( !this_t::is_inside( p ) ) continue ;
                    p.force -= motor::math::vec2f_t( _friction ) * p.vel  ;
                }
                #else
                motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>( beg, beg+n ),
                    [&]( motor::concurrent::range_1d<size_t> const & r )
                    {
                        for( size_t i=r.begin(); i<r.end(); ++i )
                        {
                            physics::particle_ref_t p = particles[i] ;
                    if( !this_t::is_inside( p ) ) continue ;
                    p.force -= motor::math::vec2f_t( _friction ) * p.vel  ;
                        }
                    } ) ;
                #endif
            }
        } ;
        motor_typedef( viscosity_force_field ) ;

        class sin_velocity_field : public force_field
        {
            motor_this_typedefs( sin_velocity_field ) ;

        private:

            float_t _amplitude = 1.0f ;
            float_t _frequency = 1.0f ;
            float_t _shift = 1.0f ;
        
        public:

            sin_velocity_field( void_t ) noexcept {}
            sin_velocity_field( float_t const a, float_t const f, float_t const s ) noexcept 
            { 
                _amplitude = a ;
                _frequency = f ;
                _shift = s ;
            }

            float_t get_amplitude( void_t ) const noexcept
            {
                return _amplitude ;
            }

            float_t get_frequency( void_t ) const noexcept
            {
                return _frequency ;
            }

            float_t get_shift( void_t ) const noexcept
            {
                return _shift ;
            }

            void_t set_amplitude( float_t const a ) noexcept
            {
                _amplitude = a ;
            }

            void_t set_frequency( float_t const a ) noexcept
            {
                _frequency = a ;
            }

            void_t set_shift( float_t const a ) noexcept
            {
                _shift = a ;
            }

        public:

            virtual void_t apply( size_t const beg, size_t const n, motor::vector_pod< particle_t > & particles ) const noexcept 
            {
                #if !MOTOR_PHYSICS_USE_PARALLEL_FOR
                for( size_t i=beg; i<beg+n; ++i )
                {
                    physics::particle_ref_t p = particles[i] ;
                    if( !this_t::is_inside( p ) ) continue ;
                    
                    auto const v = (p.pos / _frequency).fracted() ;
                    auto const f = v.dot(v) * motor::math::constants<float_t>::pi() * 2.0f ;

                    float_t const s = _amplitude * std::sin(  f   ) ;                    
                    motor::math::vec2f_t ortho = p.vel.ortho() ;
                    //p.force += ortho.normalize() * s * p.mass;
                    p.vel += ortho.normalize() * s ;

                }
                #else
                motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>( beg, beg+n ),
                    [&]( motor::concurrent::range_1d<size_t> const & r )
                    {
                        for( size_t i=r.begin(); i<r.end(); ++i )
                        {
                            physics::particle_ref_t p = particles[i] ;
                            if( !this_t::is_inside( p ) ) continue ;
                    
                            auto const v = (p.pos / _frequency).fracted() ;
                            auto const f = v.dot(v) * motor::math::constants<float_t>::pi() * 2.0f ;

                            float_t const s = _amplitude * std::sin(  f   ) ;                    
                            motor::math::vec2f_t ortho = p.vel.ortho() ;
                            //p.force += ortho.normalize() * s * p.mass;
                            p.vel += ortho.normalize() * s ;
                        }
                    } ) ;
                #endif

            }
        } ;
        motor_typedef( sin_velocity_field ) ;
    }
}