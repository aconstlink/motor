
#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/profiling/manager.h>

#include <motor/std/histogram.hpp>
#include <motor/std/hash_map>
#include <motor/std/ring_buffer>
#include <motor/std/string>

namespace motor{ namespace tool {
    
    class MOTOR_TOOL_API engine_profiling
    {
        motor_this_typedefs( engine_profiling ) ;

        struct profiling_data
        {
            bool_t display_allocations = false ;
            bool_t display_deallocations = false ;
            motor::mstd::histogram< size_t > memory_allocations ;
            motor::mstd::histogram< size_t > memory_deallocations ;
            motor::mstd::histogram< size_t > memory_current ;

            struct function_timing_data
            {
                size_t num_calls = 0 ;
                std::chrono::nanoseconds duration = std::chrono::nanoseconds( 0 ) ;
            };

            using ring_of_function_timings_t = motor::ring_buffer< function_timing_data, 1000 >;

            motor::hash_map< char_cptr_t, ring_of_function_timings_t > function_timings ;

            motor::hash_map< char_cptr_t, this_t::profiling_data::function_timing_data > tmp ;


            motor::profiling::manager_t::data_points_t profiling_data_points ;
        };
        motor_typedef( profiling_data ) ;

        profiling_data_t _profiling_data ;

        struct by_category_data
        {
            using by_name_t= motor::hash_map< char_cptr_t, size_t > ;
            motor::hash_map < char_cptr_t, by_name_t > by_cat ;

            
        };
        motor_typedef( by_category_data ) ;

        by_category_data_t _by_categories ;

    public:

        void_t update( void_t ) noexcept ;
        bool_t display( void_t ) noexcept ;
        
    };
    motor_typedef( engine_profiling ) ;

} }