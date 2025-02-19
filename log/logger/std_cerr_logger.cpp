
#include "std_cerr_logger.h"

#include <iostream>

using namespace motor::log ;

// attributes
#define RESET       0
#define BRIGHT      1
#define DIM         2
#define UNDERLINE   3
#define BLINK       4
#define REVERSE     7
#define HIDDEN      8

// colors
#define BLACK   0
#define RED     1
#define GREEN   2
#define YELLOW  3
#define BLUE    4
#define MAGENTA 5
#define CYAN    6
#define WHITE   7


namespace this_file
{
    static int const __log_level_color[] = {
        WHITE, WHITE, YELLOW, MAGENTA, RED, BLUE
    } ;

    static constexpr int to_color( log_level const ll )  noexcept
    {
        return this_file::__log_level_color[ 
            size_t(ll) >= size_t( log_level::num_levels ) ? 0 : size_t(ll) ] ;
    }

    static void_t text_color( int const attr, int const fg, int const bg ) noexcept
    {
        char buffer[13] ;
        std::snprintf( buffer, 13, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40 ) ;
        printf( "%s", buffer ) ;
    }

    static void_t text_color( motor::log::log_level const ll ) noexcept
    {
        int const fg = this_file::to_color( ll ) ;
        int const bg = 0 ;
        int const attr = 1 ;
        this_file::text_color( attr, fg, bg ) ;
    }

    static void_t text_color_reset( void_t ) noexcept
    {
        this_file::text_color( motor::log::log_level::raw ) ;
    }
}

//*************************************************************************************
std_cerr_logger::std_cerr_logger( void_t ) noexcept {}

//*************************************************************************************
std_cerr_logger::~std_cerr_logger( void_t ) noexcept {}

//*************************************************************************************
motor::log::result std_cerr_logger::log( motor::log::log_level const ll, char const * msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    
    /* Command is the control command to the terminal */

    this_file::text_color( ll ) ;
    printf( "%s", motor::log::to_string_short( ll ) ) ;
    this_file::text_color_reset() ;
    printf( "%s\n", msg ) ;

    return motor::log::result::ok ;
}

//*************************************************************************************
motor::log::result std_cerr_logger::log( motor::log::log_level const ll, motor::string_cref_t msg ) noexcept
{
    return std_cerr_logger::log( ll, msg.c_str() ) ;
}


