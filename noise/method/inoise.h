#pragma once

#include "../typedefs.h"
#include "../api.h"

namespace motor { namespace noise
{
    class MOTOR_NOISE_API inoise
    {
    public:

        virtual float_t noise( float_t x ) const noexcept = 0 ;
        virtual float_t noise( float_t x, float_t y ) const noexcept = 0 ;
        virtual float_t noise( float_t x, float_t y, float_t z ) const noexcept = 0 ;
    };
} }
