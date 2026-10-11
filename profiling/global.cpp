// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT



#include "global.h"

using namespace motor::profiling ;

//******************************************************
global::global( void_t ) noexcept
{

}

//******************************************************
global::global( this_rref_t /*rhv*/ ) noexcept
{
}

//******************************************************
global::~global( void_t ) noexcept
{

}

//******************************************************
void_t global::deinit( void_t ) noexcept
{
    _mgr.~manager() ;
}