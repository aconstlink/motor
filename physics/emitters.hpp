
#pragma once

#include "particle.h"

#include <motor/math/interpolation/interpolate.hpp>
#include <motor/math/matrix/matrix2.hpp>
#include <motor/std/vector>

#include <random>
#include <functional>

namespace motor
{
    namespace physics
    {
        class emitter
        {
            motor_this_typedefs( emitter ) ;

        private:

            // amount of particles per interval 
            size_t _amount = 1 ;
            // times per second <=> interval
            float_t _rate = 1.0f ;

            float_t _mass = 1.0f ;

            motor::math::vec2f_t _pos ;
            motor::math::vec2f_t _dir = motor::math::vec2f_t( 1.0f, 0.0f ) ;

            float_t _acl = 0.0f ;
            float_t _vel = 0.0f ;

            spawn_location_type _slt = spawn_location_type::border ;
            spawn_distribution_type _sdt = spawn_distribution_type::uniform ;
        
        protected:

            typedef std::function< float_t ( void_t ) > variation_funk_t ;

        private:

            variation_type _mass_vt = variation_type::fixed ;
            variation_type _avt = variation_type::fixed ;
            variation_type _acl_vt = variation_type::fixed ;
            variation_type _vel_vt = variation_type::fixed ;
        

            variation_funk_t _mass_funk ;
            variation_funk_t _age_funk ;
            variation_funk_t _acl_funk ;
            variation_funk_t _vel_funk ;

            // in seconds
            float_t _age = 0.0f ;

        

            std::mt19937 _generator ;

        public:

            emitter( void_t ) noexcept
            {
                this_t::set_age_variation_type( _avt ) ;
                this_t::set_acceleration_variation_type( _acl_vt ) ;
                this_t::set_velocity_variation_type( _vel_vt ) ;
            }

            emitter( this_cref_t ) = delete ;
            emitter( this_rref_t rhv ) noexcept
            {
                this_t::operator=( std::move( rhv ) ) ;
            }
            virtual ~emitter( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _amount = rhv._amount ;
                _rate = rhv._rate ;
                _mass = rhv._mass ;
                _pos = rhv._pos ;
                _dir = rhv._dir ;
                _acl = rhv._acl ;
                _vel = rhv._vel ;
                _slt = rhv._slt ;
                _sdt = rhv._sdt ;
            
            
                _age = rhv._age ;
                _generator = std::move( rhv._generator ) ;
            
                this_t::set_mass_variation_type( rhv._mass_vt ) ;
                this_t::set_age_variation_type( rhv._avt ) ;
                this_t::set_acceleration_variation_type( rhv._acl_vt ) ;
                this_t::set_velocity_variation_type( rhv._vel_vt ) ;
                return *this ;
            }

        public:


            float_t get_age( void_t ) const noexcept { return _age ; }
            void_t set_age( float_t const s ) noexcept { _age = s ; }

            size_t get_amount( void_t ) const noexcept { return _amount ; }
            void_t set_amount( size_t const s ) noexcept { _amount = s ; }

            float_t get_rate( void_t ) const noexcept { return _rate ; }
            void_t set_rate( float_t const s ) noexcept { _rate = std::max( 1.0f, std::abs(s) ) ; }

            void_t set_mass( float_t const m ) noexcept { _mass = m ; }
            float_t get_mass( void_t ) const noexcept { return _mass ; }

            void_t set_position( motor::math::vec2f_cref_t v ) noexcept { _pos = v ; }
            void_t set_direction( motor::math::vec2f_cref_t v ) noexcept { _dir = v.normalized() ; }
            void_t set_velocity( float_t const v ) noexcept { _vel = v ; }
            void_t set_acceleration( float_t const v ) noexcept { _acl = v ; }
            void_t set_spawn_location_type( spawn_location_type const spt ) noexcept { _slt = spt ; }
            void_t set_spawn_distribution_type( spawn_distribution_type const sdt ) noexcept { _sdt = sdt ; }

            motor::math::vec2f_cref_t get_position( void_t ) const noexcept{ return _pos ; }
            float_t get_velocity( void_t ) const noexcept{ return _vel ; }
            float_t get_acceleration( void_t ) const noexcept{ return _acl ; }
            motor::math::vec2f_cref_t get_direction( void_t ) const noexcept{ return _dir ; }
            spawn_location_type get_spawn_location_type( void_t ) const noexcept { return _slt ; }
            spawn_distribution_type get_spawn_distribution_type( void_t ) const noexcept { return _sdt ; }

            variation_type get_mass_variation_type( void_t ) const noexcept { return _mass_vt ; }
            variation_type get_age_variation_type( void_t ) const noexcept { return _avt ; }
            variation_type get_acceleration_variation_type( void_t ) const noexcept { return _acl_vt ; }
            variation_type get_velocity_variation_type( void_t ) const noexcept { return _vel_vt ; }

            variation_funk_t get_mass_funk( void_t ) const noexcept { return _mass_funk ; }
            variation_funk_t get_age_funk( void_t ) const noexcept { return _age_funk ; }
            variation_funk_t get_acceleration_funk( void_t ) const noexcept { return _acl_funk ; }
            variation_funk_t get_velocity_funk( void_t ) const noexcept { return _vel_funk ; }

            float_t random_real_number( float_t const a, float_t const b ) noexcept
            {
                std::uniform_real_distribution<float_t> distribution(a, b) ;
                return distribution( _generator ) ;
            }

            void_t set_mass_variation_type( variation_type const avt ) noexcept 
            { 
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_mass() ;
                } ;

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        return this_t::random_real_number( this_t::get_mass() * 0.2f, this_t::get_mass() ) ;
                    } ;
                }
                _mass_vt = avt ; 
                _mass_funk = funk ;
            }

            void_t set_age_variation_type( variation_type const avt ) noexcept 
            {             
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_age() ;
                };

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        return this_t::random_real_number( this_t::get_age() * 0.2f, this_t::get_age() ) ;
                    } ;
                }

                _avt = avt ; 
                _age_funk = funk ;
            }

            void_t set_acceleration_variation_type( variation_type const avt ) noexcept 
            { 
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_acceleration() ;
                } ;

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        return this_t::random_real_number( this_t::get_acceleration() * 0.2f, this_t::get_acceleration() ) ;
                    } ;
                }

                _acl_vt = avt ; 
                _acl_funk = funk ;
            }

            void_t set_velocity_variation_type( variation_type const avt ) noexcept 
            {
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_velocity() ;
                } ;

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        return this_t::random_real_number( this_t::get_velocity() * 0.2f, this_t::get_velocity() ) ;
                    } ;
                }

                _vel_vt = avt ; 
                _vel_funk = funk ;
            }
        

        public:

            // the default function returns amount particles per rate * second
            virtual size_t calc_emits( size_t const emitted, float_t const secs ) const noexcept 
            {
                float_t const s = secs * this_t::get_rate() ;
                size_t const tq = size_t( s - motor::math::fn<float_t>::fract( s ) ) ;
                size_t const eq = emitted / this_t::get_amount() ;

                return tq >= eq ? this_t::get_amount() : 0 ;
            }

            virtual void_t emit( size_t const, size_t const, motor::vector< particle_t > & ) noexcept = 0 ;
        };
        motor_typedef( emitter ) ;

    
        class radial_emitter : public emitter
        {
            motor_this_typedefs( radial_emitter ) ;

        private:

            /// radius of 0 is a point emitter
            float_t _radius = 0.0f ;

            // around direction in [-angle, angle]
            float_t _angle = 0.0f ;


            variation_type _radius_vt = variation_type::fixed ;
            variation_type _angle_vt = variation_type::fixed ;

            emitter::variation_funk_t _radius_funk ;
            emitter::variation_funk_t _angle_funk ;

        public:

            radial_emitter( void_t ) noexcept 
            {
                this_t::set_angle_variation_type( _angle_vt ) ;
                this_t::set_radius_variation_type( _radius_vt ) ;
            }

            radial_emitter( this_cref_t ) = delete ;
            radial_emitter( this_rref_t rhv ) noexcept : emitter( std::move( rhv ) )
            {
                this_t::operator=( std::move( rhv ) ) ;
            }
            virtual ~radial_emitter( void_t ) noexcept{}
        
            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _radius = rhv._radius ;
                _angle = rhv._angle ;

                this_t::set_radius_variation_type( rhv._radius_vt ) ;
                this_t::set_angle_variation_type( rhv._angle_vt ) ;

                return *this ;
            }

            void_t set_radius_variation_type( variation_type const avt ) noexcept 
            { 
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_radius() ;
                } ;

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        return this_t::random_real_number( this_t::get_radius() * 0.2f, this_t::get_radius() ) ;
                    } ;
                }
                _radius_vt = avt ; 
                _radius_funk = funk ;
            }

            void_t set_angle_variation_type( variation_type const avt ) noexcept 
            { 
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_angle() ;
                } ;

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        return this_t::random_real_number( this_t::get_angle() * 0.2f, this_t::get_angle() ) ;
                    } ;
                }
                _angle_vt = avt ; 
                _angle_funk = funk ;
            }

            variation_type get_radius_variation_type( void_t ) const noexcept { return _radius_vt ; }
            variation_funk_t get_radius_funk( void_t ) const noexcept { return _radius_funk ; }
            variation_type get_angle_variation_type( void_t ) const noexcept { return _angle_vt ; }
            variation_funk_t get_angle_funk( void_t ) const noexcept { return _angle_funk ; }

        public:

            float_t get_radius( void_t ) const noexcept { return _radius ; }
            void_t set_radius( float_t const r ) noexcept { _radius = std::abs( r ) ; }

            float_t get_angle( void_t ) const noexcept { return _angle ; }
            void_t set_angle( float_t const v ) noexcept { _angle = v ; }

        public:

            virtual void_t emit( size_t const beg, size_t const n, motor::vector< particle_t > & particles ) noexcept 
            {
                float_t const angle = this_t::get_angle_funk()() ;

                float_t const angle_step = ( 2.0f * angle ) / float_t(n-1) ;
                motor::math::mat2f_t uniform = motor::math::mat2f_t::rotation( angle_step ) ;

                motor::math::vec2f_t dir = motor::math::mat2f_t::rotation( n == 1 ? 0.0f : -angle ) * 
                    this_t::get_direction() ;

                for( size_t i=beg; i<beg+n; ++i )
                {
                    particle_t p ;
                
                    p.age = this_t::get_age_funk()() ;
                    p.mass = this_t::get_mass_funk()() ;
                    p.pos = this_t::get_position() + dir * this_t::get_radius_funk()() ;
                    p.vel = dir * this_t::get_velocity_funk()() ;
                    p.acl = dir * this_t::get_acceleration_funk()() ;

                    particles[ i ] = p ;

                    dir = uniform * dir ;
                }
            }
        };
        motor_typedef( radial_emitter ) ;

        class line_emitter : public emitter
        {
            motor_this_typedefs( line_emitter ) ;

        private:

            /// distance from origin on the line
            float_t _parallel_dist = 0.0f ;
        
            /// distance away from line in ortho direction
            float_t _ortho_dist = 0.0f ;
        
            variation_type _parallel_vt = variation_type::fixed ;
            variation_type _ortho_vt = variation_type::fixed ;

            emitter::variation_funk_t _parallel_funk ;
            emitter::variation_funk_t _ortho_funk ;

        public:

            line_emitter( void_t ) noexcept 
            {
                this_t::set_parallel_variation_type( _parallel_vt ) ;
                this_t::set_ortho_variation_type( _ortho_vt ) ;
            }
            line_emitter( this_cref_t ) = delete ;
            line_emitter( this_rref_t rhv ) noexcept : emitter( std::move( rhv ) )
            {
                this_t::operator=( std::move( rhv ) ) ;
            }
            virtual ~line_emitter( void_t ) noexcept{}

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _parallel_dist = rhv._parallel_dist ;
                _ortho_dist = rhv._ortho_dist ;

                this_t::set_parallel_variation_type( rhv._parallel_vt ) ;
                this_t::set_ortho_variation_type( rhv._ortho_vt ) ;
                return *this ;
            }

            void_t set_parallel_variation_type( variation_type const avt ) noexcept 
            { 
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_parallel_distance() ;
                } ;

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        auto const r = this_t::random_real_number( 0.0f, 1.0f ) ;
                        return motor::math::interpolation<float_t>::linear( this_t::get_parallel_distance() * 0.2f, this_t::get_parallel_distance(), r ) ;
                    } ;
                }
                _parallel_vt = avt ; 
                _parallel_funk = funk ;
            }

            void_t set_ortho_variation_type( variation_type const avt ) noexcept 
            { 
                variation_funk_t funk = [=]( void_t )
                {
                    return this_t::get_ortho_distance() ;
                } ;

                if( avt == physics::variation_type::random )
                {
                    funk = [=]( void_t )
                    {
                        auto const r = this_t::random_real_number( 0.0f, 1.0f ) ;
                        return motor::math::interpolation<float_t>::linear( this_t::get_ortho_distance() * 0.2f, this_t::get_ortho_distance(), r ) ;
                    } ;
                }
                _ortho_vt = avt ; 
                _ortho_funk = funk ;
            }

            variation_type get_parallel_variation_type( void_t ) const noexcept { return _parallel_vt ; }
            variation_funk_t get_parallel_funk( void_t ) const noexcept { return _parallel_funk ; }
            variation_type get_ortho_variation_type( void_t ) const noexcept { return _ortho_vt ; }
            variation_funk_t get_ortho_funk( void_t ) const noexcept { return _ortho_funk ; }

        public:

            float_t get_parallel_distance( void_t ) const noexcept { return _parallel_dist ; }
            void_t set_parallel_distance( float_t const r ) noexcept { _parallel_dist = std::abs( r ) ; }

            float_t get_ortho_distance( void_t ) const noexcept { return _ortho_dist ; }
            void_t set_ortho_distance( float_t const v ) noexcept { _ortho_dist = v ; }

        public:

            virtual void_t emit( size_t const beg, size_t const n, motor::vector< particle_t > & particles ) noexcept 
            {
                float_t const parallel = this_t::get_parallel_funk()() ;
                float_t const step = (2.0f * parallel) / float_t(n-1) ;

                motor::math::vec2f_t ortho_dir = this_t::get_direction() ; 
                motor::math::vec2f_t parallel_dir( ortho_dir.y(), -ortho_dir.x() ) ; 

                motor::math::vec2f_t pos = this_t::get_position() + parallel_dir * parallel ;

                for( size_t i=beg; i<beg+n; ++i )
                {
                    particle_t p ;
                
                    p.age = this_t::get_age_funk()() ;
                    p.mass = this_t::get_mass_funk()() ;
                    p.pos = pos + ortho_dir * this_t::get_ortho_funk()() ;
                    p.vel = ortho_dir * this_t::get_velocity_funk()() ;
                    p.acl = ortho_dir * this_t::get_acceleration_funk()() ;

                    particles[ i ] = p ;

                    pos += parallel_dir.negated() * step ;
                }
            }
        };
        motor_typedef( line_emitter ) ;
    }
}