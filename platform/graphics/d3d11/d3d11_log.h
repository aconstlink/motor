// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

#include <motor/log/global.h>

#if _DEBUG
#define D3D_DEBUG
#endif

#define d3d11_backend_log( text ) "[D3D11] : " text
#define d3d11_log_warningv( text, ... )                                                            \
    motor::log::global_t::warning< 2048 >( "[d3d11] : " text, __VA_ARGS__ );
#define d3d11_log_errorv( text, ... )                                                              \
    motor::log::global_t::error< 2048 >( "[d3d11] : " text, __VA_ARGS__ );