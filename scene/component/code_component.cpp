// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT




#include "code_component.h"

using namespace motor::scene ;


//**********************************************************************
void_t code_component::execute( motor::scene::node_mtr_t n ) noexcept
{
    _funk( n ) ;
}