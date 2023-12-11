//------------------------------------------------------------
// snakeoil (c) Alexis Constantin Link
// Distributed under the MIT license
//------------------------------------------------------------
#ifndef _SNAKEOIL_APPLICATION_EVENT_RESIZE_EVENT_H_
#define _SNAKEOIL_APPLICATION_EVENT_RESIZE_EVENT_H_

#include "../typedefs.h"

#include <snakeoil/math/vector/vector4.hpp>

namespace so_app
{
    class resize_event
    {
    public:

        typedef so_math::vector4< size_t > size_vec_t ;

    private:

        size_vec_t _sizes ;

    public:
        
        resize_event( void_t ) {}
        resize_event( size_t x, size_t y, size_t w, size_t h ) : _sizes(x,y,w,h) {}
        resize_event( size_vec_t sizes ) : _sizes(sizes) {}

        size_vec_t const & get_sizes( void_t ) const { return _sizes ; }
    };
}

#endif

