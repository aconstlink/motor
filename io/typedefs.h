#pragma once

#include "result.h"

#include <motor/memory/global.h>

#include <motor/std/string>
#include <motor/std/filesystem>

//#include <motor/string/utf8.hpp>

#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>

#include <functional>

namespace motor
{
    namespace io
    {
        using namespace motor::core::types ;

        using memory = motor::memory::global ;
        motor_typedefs( motor::filesystem::path, path ) ;

        typedef std::function< void_t( char_cptr_t, size_t const, motor::io::result const ) > load_completion_funk_t ;
        typedef std::function< void_t( motor::io::result const ) > store_completion_funk_t ;

        typedef void_mtr_t internal_item_mtr_t ;
    }
}