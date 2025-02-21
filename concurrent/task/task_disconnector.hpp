
#pragma once

#include "task.hpp"

namespace motor
{
    namespace concurrent
    {
        class task_disconnector
        {

        public:

            static void_t disconnect_everyting( motor::concurrent::task_mtr_safe_t t0 ) noexcept
            {
                motor::concurrent::task_t::tasks_t ts[2] ;
                motor::concurrent::task_t::cleaner_accessor::move_out_all( t0, ts[0] ) ;

                size_t ridx = 0 ;
                size_t widx = 1 ;

                while( ts[ridx].size() != 0 ) 
                {
                    for( size_t i=0; i<ts[ ridx ].size(); ++i )
                    //for ( auto * t : ts[ ridx ] )
                    {
                        auto * t = ts[ridx][i] ;

                        motor::concurrent::task_t::cleaner_accessor::move_out_all( t, ts[ widx ] ) ;

                        t->disconnect() ;
                        motor::memory::release_ptr( t ) ;
                    }
                    ts[ ridx ].clear() ;

                    widx = ridx ;
                    ridx = ++ridx % 2 ;
                }

                t0->disconnect() ;
                motor::release( t0 ) ;
            }
        };
        motor_typedef( task_disconnector ) ;
    }
}