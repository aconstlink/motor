#pragma once

//#include <natus/memory/allocator.hpp>
#include <natus/core/macros/typedef.h>

#include <string>

namespace natus
{
    namespace ntd
    {
        //template< typename T >
        //using string = ::std::vector< T, natus::memory::allocator<T> > ;

        // for now, we use the default allocator
        using string = ::std::string ;

        natus_typedefs( string, string ) ;
    }
}
