// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#define MOTOR_EXP_TO_CHAR( x ) #x

#define motor_save_call( ptr, fn ) if( ptr != nullptr ) ptr->fn