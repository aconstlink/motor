
#include "particle.h"
#include "emitters.hpp"
#include "force_fields.hpp"

#include <motor/math/vector/vector4.hpp>
#include <array>

namespace motor
{
    namespace physics
    {
        class MOTOR_PHYSICS_API particle_system
        {
            motor_this_typedefs( particle_system ) ;

        public:
        
            particle_system( void_t ) noexcept ;
            particle_system( this_cref_t rhv ) noexcept = delete ;
            particle_system( this_rref_t rhv ) noexcept ;
            ~particle_system( void_t ) noexcept ;

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept  ;

        private:

            // particle array
            motor::vector< particle_t > _particles ;

            struct emitter_data
            {
                emitter_mtr_t emt ;

                // time since attached
                float_t seconds ;

                size_t emitted ;

                size_t emit ;
            };
            motor_typedef( emitter_data ) ;

            // emitter array
            motor::vector< emitter_data_t > _emitter ;

            struct force_field_data
            {
                force_field_mtr_t ff ;
            };
            motor_typedef( force_field_data ) ;

            // force fields
            motor::vector< force_field_data_t > _forces ;

            motor::math::vec4f_t _extend ;

        public:

            void_t attach_emitter( motor::physics::emitter_mtr_safe_t emt ) noexcept ;

            void_t detach_emitter( motor::physics::emitter_mtr_safe_t emt ) noexcept ;

            void_t attach_force_field( motor::physics::force_field_mtr_safe_t ff ) noexcept ;

            void_t detach_force_field( motor::physics::force_field_mtr_safe_t ff ) noexcept ;

            void_t clear( void_t ) noexcept ;

            motor::math::vec4f_cref_t get_extend( void_t ) const noexcept { return _extend ; }
            std::array< motor::math::vec2f_t, 4 > get_extend_rect( void_t ) const noexcept ;
            
        public: 

            void_t update( float_t const dt ) noexcept ;

            typedef std::function< void_t ( motor::vector< particle_t > const &  ) > on_particles_funk_t ;
            void_t on_particles( on_particles_funk_t funk ) noexcept ;
        };
        motor_typedef( particle_system ) ;
    }
}