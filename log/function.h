// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include <motor/std/string>

#define motor_log_fn( text ) motor::string_t( "[" + motor::string_t( __FUNCTION__ ) + "] : " + motor::string_t(text) )
#define motor_log_fn2( text ) text ;
