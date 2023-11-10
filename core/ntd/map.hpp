#pragma once

//#include <natus/memory/allocator.hpp>
#include <map>

namespace natus
{
    namespace ntd
    {
        //template< typename T >
        //using map = ::std::map< T, natus::memory::allocator<T> > ;

        // for now, we use the default allocator
        template< typename T, typename G >
        using map = ::std::map< T, G > ;
    }
    
}