
#pragma once

#include "../icontext.h"

#include <natus/ntd/string.hpp>

namespace natus
{
    namespace graphics
    {
        class NATUS_GRAPHICS_API es_context : public natus::graphics::icontext
        {
        public:

            virtual ~es_context( void_t ) noexcept {}

        public:

            virtual bool_t is_extension_supported( natus::ntd::string_cref_t ) const noexcept = 0 ;
        };
        natus_res_typedef( es_context ) ;
    }
}