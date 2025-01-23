
#include "particle_system.h"

using namespace motor::physics ;

particle_system::particle_system( void_t ) noexcept {}
particle_system::particle_system( this_rref_t rhv ) noexcept 
{
    _particles = std::move( rhv._particles ) ;
    _forces = std::move( rhv._forces ) ;
    _emitter = std::move( rhv._emitter ) ;
}

particle_system::~particle_system( void_t ) noexcept 
{
    for( size_t i=0; i<_emitter.size(); ++i ) 
        motor::release( motor::move( _emitter[i].emt ) ) ;

    for ( size_t i = 0; i < _forces.size(); ++i )
        motor::release( motor::move( _forces[ i ].ff ) ) ;
}

            
particle_system::this_ref_t particle_system::operator = ( this_rref_t rhv ) noexcept 
{
    _particles = std::move( rhv._particles ) ;
    _forces = std::move( rhv._forces ) ;
    _emitter = std::move( rhv._emitter ) ;
    return *this ;
}

void_t particle_system::attach_emitter( motor::physics::emitter_mtr_safe_t emt ) noexcept
{
    size_t i=size_t(-1);
    while( ++i<_emitter.size() && _emitter[i].emt != emt ) ;
    if( i != _emitter.size() ) 
    {
        motor::release( emt ) ;
        return ;
    }

    _emitter.emplace_back( this_t::emitter_data_t{ motor::move( emt ), 0, 0, 0 } ) ;
}

void_t particle_system::detach_emitter( motor::physics::emitter_mtr_safe_t emt ) noexcept
{
    size_t i = size_t( -1 );
    while ( ++i < _emitter.size() && _emitter[ i ].emt != emt ) ;
    if ( i == _emitter.size() )
    {
        motor::release( emt ) ;
        return ;
    }

    motor::release( motor::move( _emitter[i].emt ) ) ;
    _emitter.erase( _emitter.begin() + i ) ;
}

void_t particle_system::attach_force_field( motor::physics::force_field_mtr_safe_t ff ) noexcept
{
    size_t i = size_t( -1 );
    while ( ++i < _forces.size() && _forces[ i ].ff != ff ) ;
    if ( i != _forces.size() )
    {
        motor::release( ff ) ;
        return ;
    }

    _forces.emplace_back( this_t::force_field_data{ motor::move( ff ) } ) ;
}

void_t particle_system::detach_force_field( motor::physics::force_field_mtr_safe_t ff ) noexcept
{
    size_t i = size_t( -1 );
    while ( ++i < _forces.size() && _forces[ i ].ff != ff ) ;
    if ( i == _forces.size() )
    {
        motor::release( ff ) ;
        return ;
    }

    motor::release( motor::move( _forces[ i ].ff ) ) ;
    _forces.erase( _forces.begin() + i ) ;
}

void_t particle_system::clear( void_t ) noexcept
{
    _particles.clear() ;
    for( auto & e : _emitter )
    {
        e.emitted = 0 ;
        e.emit = 0 ;
        e.seconds = 0.0f ;
        // do not release this here. this 
        // clear is meant differently
        //motor::release( motor::move( e.emt ) ) ;
    }
}

void_t particle_system::update( float_t const dt ) noexcept 
{
    // update particle
    {
        #if !MOTOR_PHYSICS_USE_PARALLEL_FOR
        for( auto & p : _particles )
        {
            p.age -= dt ;
        }
        #else
        motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>( 0, _particles.size() ),
            [&]( motor::concurrent::range_1d<size_t> const & r )
            {
                for( size_t i=r.begin(); i<r.end(); ++i )
                {
                    _particles[i].age -= dt ;
                }
            } ) ;
        #endif
    }

    // reorder particles
    {
        size_t last = _particles.size() ;
        for( size_t i=0; i<last; ++i )
        {
            auto const & p = _particles[i] ;
            if( p.age <= 0.0f )
            {
                _particles[i--] = _particles[ --last ] ;
            }
        }
        _particles.resize( last ) ;
    }

    // emit new particles
    {
        size_t new_emit = 0 ;
        for( auto & e : _emitter )
        {
            auto const emit = e.emt->calc_emits( e.emitted, e.seconds ) ;
            e.seconds += dt ;
            e.emitted += emit ;
            e.emit = emit ;
            new_emit += emit ;
        }

        size_t begin = _particles.size() ;
        _particles.resize( _particles.size() + new_emit ) ;

        for( auto & e : _emitter )
        {
            if( e.emit == 0 ) continue ;
            e.emt->emit( begin, e.emit, _particles ) ; begin += e.emit ;
            e.emit = 0 ;
        }
    }
            
    // reset particle physics here so 
    // value can be read elsewhere during a 
    // cycle of physics
    {
        //for( auto & p : _particles )
        {
            //p.force.negate();//motor::math::vec2f_t() ;
            //p.acl = motor::math::vec2f_t() ;
            //p.vel = motor::math::vec2f_t() ;
        }
    }

    // do force fields
    {
        for( auto & f : _forces )
        {
            f.ff->apply( 0, _particles.size(), _particles ) ;
        }
    }
            
    // do physics
    {
        #if !MOTOR_PHYSICS_USE_PARALLEL_FOR
        for( auto & p : _particles )
        {
            p.acl = p.force / p.mass ;
            p.vel += motor::math::vec2f_t( dt ) * p.acl ;
            p.pos += motor::math::vec2f_t( dt ) * p.vel ;
        }
        #else
        motor::concurrent::parallel_for<size_t>( motor::concurrent::range_1d<size_t>( 0, _particles.size() ),
            [&]( motor::concurrent::range_1d<size_t> const & r )
            {
                for( size_t i=r.begin(); i<r.end(); ++i )
                {
                    auto & p = _particles[i] ;
                    p.acl = p.force / p.mass ;
                    p.vel += motor::math::vec2f_t( dt ) * p.acl ;
                    p.pos += motor::math::vec2f_t( dt ) * p.vel ;
                }
            } ) ;
        #endif
    }

    // compute extend
    {
        motor::math::vec2f_t min_xy( std::numeric_limits<float_t>::max(), std::numeric_limits<float_t>::max() ) ;
        motor::math::vec2f_t max_xy( std::numeric_limits<float_t>::min(), std::numeric_limits<float_t>::min() ) ;

        for( auto & p : _particles )
        {
            min_xy = min_xy.greater_than( p.pos ).select( p.pos, min_xy ) ;
            max_xy = max_xy.less_than( p.pos ).select( p.pos, max_xy ) ;
        }

        _extend = motor::math::vec4f_t( min_xy, max_xy ) ;
    }
}

void_t particle_system::on_particles( on_particles_funk_t funk ) noexcept
{
    funk( _particles ) ;
}

std::array< motor::math::vec2f_t, 4 > particle_system::get_extend_rect( void_t ) const noexcept 
{
    return 
    {
        _extend.xy() ,
        _extend.xw() ,
        _extend.zw() ,
        _extend.zy() 
    } ;
}
