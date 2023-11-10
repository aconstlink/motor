#pragma once

#include <motor/core/std/string>

#define motor_log_fn( text ) motor::core::string_t( "[" + motor::core::string_t( __FUNCTION__ ) + "] : " + motor::core::string_t(text) )