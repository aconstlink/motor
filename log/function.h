#pragma once

#include <motor/std/string>

#define motor_log_fn( text ) motor::string_t( "[" + motor::string_t( __FUNCTION__ ) + "] : " + motor::string_t(text) )
#define motor_log_fn2( text ) text ;
