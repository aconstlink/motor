

#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/std/stack>
#include <motor/std/string>
#include <motor/std/vector>

#include <thread>

namespace motor{ namespace profiling { 

    class MOTOR_PROFILING_API manager
    {
        motor_this_typedefs( manager ) ;
        
    public:

        struct data_point
        {
            size_t probe_id ;
            std::chrono::nanoseconds dur ;
        };
        motor_typedef( data_point ) ;
        motor_typedefs( motor::vector< data_point_t >, data_points ) ;

    private:

        std::mutex _mtx_dp ;
        size_t _dp_ridx = 0 ;
        data_points_t _data_points[2] ;

        size_t dp_widx( void_t ) const noexcept
        {
            return (_dp_ridx + 1) % 2 ;
        }

        void_t swap_data_points( void_t ) noexcept
        {
            _dp_ridx = (++_dp_ridx) % 2 ;
        }

    private:

        struct per_thread_data
        {
            using stack_t = motor::stack< size_t, 20 > ;

            std::thread::id tid ;
            stack_t the_stack ;
        };
        motor_typedef( per_thread_data ) ;

        std::mutex _mtx_ptt ;
        motor::vector< per_thread_data_t > _pt_stacks ;

    private: // probe run-time data

        struct probe_data
        {
            bool_t used = false ;

            // ref into _per_probe_id_datas
            size_t probe_id ;

            std::thread::id tid ;
            motor::profiling::clock_t::time_point tp ;
        };

        std::mutex _mtx_probes ;
        motor::vector< probe_data > _probes ;

    private: // probe static data

        struct per_probe_id_data
        {
            char_cptr_t category ;
            char_cptr_t name ;
        };

        static std::mutex _mtx_probe_id ;
        static size_t _probe_id ;

        using per_probe_id_datas_t = std::vector< per_probe_id_data > ;
        static per_probe_id_datas_t _per_probe_id_datas ;

    public:

        manager( void_t ) noexcept ;
        manager( this_rref_t ) noexcept ;
        ~manager( void_t ) noexcept ;

    public: // probe interface

        static size_t gen_id( char_cptr_t cat, char_cptr_t name ) noexcept ;
        static char_cptr_t get_category( size_t const probe_id ) noexcept ;
        static char_cptr_t get_name( size_t const probe_id ) noexcept ;

        size_t begin_probe( size_t const id ) noexcept ;
        void_t end_probe( size_t const idx ) noexcept ;

    public:

        data_points_cref_t swap_and_clear( void_t ) noexcept ;
    };
    motor_typedef( manager ) ;
} }