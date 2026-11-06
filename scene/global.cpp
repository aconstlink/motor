// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "global.h"

using namespace motor::scene;

global::internal_id_registry_t global::_idr_internal;
global::external_id_registry_t global::_idr_external;

//***********************************************************************
void_t global::init( void_t ) noexcept {}

//***********************************************************************
void_t global::deinit( void_t ) noexcept {}

uint64_t global::register_internal_type( char const * const name ) noexcept
{
    return _idr_internal.register_type( name );
}

uint64_t global::register_external_type( char const * const name ) noexcept
{
    return _idr_external.register_type( name );
}