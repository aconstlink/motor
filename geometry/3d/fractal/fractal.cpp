
#include "fractal.h"

#include <cstring>

using namespace motor::geometry ;

//*************************************************************************************
fractal_3d::objects_t fractal_3d::menger_sponge( fractal_object_cref_t parent ) 
{
    objects_t objs ;

    float_t const step = parent.side_width / 3.0f ;
    float_t const child_width = step ;

    motor::math::vec3f_t const start_pos = 
        parent.pos - motor::math::vec3f_t(step) ;
    
    //////////////////////////////////////////////////////
    // left 8 cubes
    {
        motor::math::vec3f_t ls = start_pos ;

        //
        // front
        //
        {
            motor::math::vec3f_t pos = ls ;
            
            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.y() += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.y() += 2.0f * child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        //
        // middle
        //
        {
            motor::math::vec3f_t pos = ls ;
            pos.z() += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        /*{
            motor::math::vec3f_t pos = ls ;
            pos.y += child_width ;
            pos.z += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }*/

        {
            motor::math::vec3f_t pos = ls ;
            pos.y() += 2.0f * child_width ;
            pos.z() += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        //
        // back
        //
        {
            motor::math::vec3f_t pos = ls ;
            pos.z() += 2.0f*child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.y() += child_width ;
            pos.z() += 2.0f*child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.y() += 2.0f * child_width ;
            pos.z() += 2.0f*child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }
    }

    //////////////////////////////////////////////////////
    // middle 4 cubes
    {
        motor::math::vec3f_t ls = start_pos ;

        //
        // front
        //
        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += child_width ;
            
            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        // no pos here
        {}

        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += child_width ;
            pos.y() += 2.0f * child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        //
        // middle - middle has no positions
        //
        {}

        //
        // back
        //
        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += child_width ;
            pos.z() += 2.0f*child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        // no pos here
        {}

        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += child_width ;
            pos.y() += 2.0f * child_width ;
            pos.z() += 2.0f * child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }
    }

    //////////////////////////////////////////////////////
    // right 8 cubes
    {
        motor::math::vec3f_t ls = start_pos ;
    
        //
        // front
        //
        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;
            pos.y() += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;
            pos.y() += 2.0f * child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        //
        // middle
        //
        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;
            pos.z() += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        /*{
            motor::math::vec3f_t pos = ls ;
            pos.y += child_width ;
            pos.z += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }*/

        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;
            pos.y() += 2.0f * child_width ;
            pos.z() += child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        //
        // back
        //
        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;
            pos.z() += 2.0f*child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;
            pos.y() += child_width ;
            pos.z() += 2.0f*child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }

        {
            motor::math::vec3f_t pos = ls ;
            pos.x() += 2.0f * child_width ;
            pos.y() += 2.0f * child_width ;
            pos.z() += 2.0f*child_width ;

            fractal_object o ;
            o.pos = pos ;
            o.side_width = child_width ;

            objs.push_back( o ) ;
        }
    }

    return std::move( objs ) ;
}

//*************************************************************************************
fractal_3d::objects_t fractal_3d::sierpinski_tetra( fractal_object_cref_t parent ) 
{
    objects_t objs ;

    float_t const step = parent.side_width / 2.0f ;
    float_t const child_width = step ;

    motor::math::vec3f_t start_pos = parent.pos ;


    //////////////////////////////////////////////////////
    // left tet
    {
        motor::math::vec3f_t ls = start_pos ;
        ls.x() -= step ;
        ls.y() -= step ;
        ls.z() -= step ;

        fractal_object o ;
        o.pos = ls ;
        o.side_width = child_width ;

        objs.push_back( o ) ;
    }

    //////////////////////////////////////////////////////
    // right tet
    {
        motor::math::vec3f_t ls = start_pos ;
        ls.x() += step ;
        ls.y() -= step ;
        ls.z() -= step ;

        fractal_object o ;
        o.pos = ls ;
        o.side_width = child_width ;

        objs.push_back( o ) ;
    }

    //////////////////////////////////////////////////////
    // back tet
    {
        motor::math::vec3f_t ls = start_pos ;
        ls.y() -= step ;
        ls.z() += step ;

        fractal_object o ;
        o.pos = ls ;
        o.side_width = child_width ;

        objs.push_back( o ) ;
    }

    //////////////////////////////////////////////////////
    // top tet
    {
        motor::math::vec3f_t ls = start_pos ;
        ls.y() += step ;

        fractal_object o ;
        o.pos = ls ;
        o.side_width = child_width ;

        objs.push_back( o ) ;
    }
    return std::move( objs ) ;
}

//*************************************************************************************
fractal_3d::divider_funk_t fractal_3d::menger_sponge_funk( void_t ) 
{
    return [=]( fractal_object_cref_t parent )
    {
        return std::move(this_t::menger_sponge( parent )) ;
    } ;
}

//*************************************************************************************
fractal_3d::divider_funk_t fractal_3d::sierpinski_tetra_funk( void_t ) 
{
    return [=]( fractal_object_cref_t parent )
    {
        return std::move(this_t::sierpinski_tetra( parent )) ;
    } ;
}

//*************************************************************************************
fractal_3d::objects_t fractal_3d::iterate( divider_funk_t funk, fractal_object_cref_t parent, size_t num_divs ) 
{
    this_t::objects_t objs = { parent } ;

    this_t::objects_t store(10000000) ;

    for( size_t i=0; i<num_divs; ++i )
    {
        size_t cur_pos = 0 ;

        for( auto const & obj : objs )
        {
            this_t::objects_t tmps = std::move( funk( obj ) ) ;
            
            if( store.size() <= cur_pos )
            {
                store.resize( store.size() + 1000000 ) ;
            }            
            
            for( size_t j=0; j<tmps.size(); ++j )
            {
                store[cur_pos+j] = tmps[j] ;
            }
            cur_pos += tmps.size() ;
        }
        //objs = std::move( store ) ;

        objs.resize( cur_pos ) ;
        std::memcpy( objs.data(), store.data(), sizeof(fractal_object_t)*cur_pos ) ;
        
    }

    return std::move( objs ) ;
}
