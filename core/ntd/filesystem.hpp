#pragma once

#include <ghc/filesystem.hpp>
namespace natus
{
    namespace ntd
    {
        namespace filesystem = ghc::filesystem ;
        namespace fs = ghc::filesystem ;
    }
}
/*
#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
#if __has_include( <filesystem> )
#define GHC_USE_STD_FS
#include <filesystem>
namespace natus
{
    namespace ntd
    {
        namespace filesystem = ::std::filesystem ;
        namespace fs = ::std::filesystem ;
    }
}

#endif
#endif
#ifndef GHC_USE_STD_FS
#include <ghc/filesystem.hpp>
namespace natus
{
    namespace ntd
    {
        namespace filesystem = ghc::filesystem ;
        namespace fs = ghc::filesystem ;
    }
}


#endif
*/
