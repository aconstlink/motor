// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#define gl4_log( text ) "[GL4] : " text
#define gl4_log_error( text ) motor::ogl::error::check_and_log( "[GL4] : " text )
#define gl4_log_error2( text, name ) motor::ogl::error::check_and_log( "[GL4] : " text,  name )
#define gl4_log_error3( text, obj ) motor::ogl::error::check_and_log<decltype(obj)>( "[GL4] : " text,  obj )