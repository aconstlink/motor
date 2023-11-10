#pragma once

#define MOTOR_EXP_TO_CHAR( x ) #x

#define motor_save_call( ptr, fn ) \
    if( ptr != nullptr ) ptr->fn