#pragma once

#include <natus/memory/allocator.hpp>
#include <list>

namespace natus
{
    namespace ntd
    {
        //template< typename T >
        //using list = ::std::list< T, natus::memory::allocator<T> > ;

        // for now, we use the default allocator
        template< typename T >
        using list = ::std::list< T > ;
    }
}