// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#include "backend.h"

using namespace motor::audio ;

backend::backend( void_t ) noexcept : _backend_id( this_t::create_backend_id() )
{}