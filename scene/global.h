// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

#include "api.h"
#include "typedefs.h"
#include <motor/core/id_registry.hpp>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API global
        {
            motor_this_typedefs( global ) ;

        private:

            using internal_id_registry_t = motor::core::id_registry<0, 100> ;
            using external_id_registry_t = motor::core::id_registry<100> ;
            static internal_id_registry_t _idr_internal;
            static external_id_registry_t _idr_external;

        public:

            static void_t init( void_t ) noexcept ;
            static void_t deinit( void_t ) noexcept ;

            static uint64_t register_internal_type( char const * const ) noexcept ;
            static uint64_t register_external_type( char const * const ) noexcept ;

        private:


        };

    }
}