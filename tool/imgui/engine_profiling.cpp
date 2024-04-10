#include "engine_profiling.h"

#include "custom_widgets.h"

#include <motor/profiling/global.h>

using namespace motor::tool ;

//******************************************************************************
void_t engine_profiling::update( void_t ) noexcept
{
    #if MOTOR_MEMORY_OBSERVER
    {
        motor::memory::observer_t::observable_data_t od =
            motor::memory::global_t::get_observer()->swap_and_clear() ;

        _profiling_data.memory_allocations.clear() ;
        _profiling_data.memory_deallocations.clear() ;

        for ( auto const & m : od.messages )
        {
            if ( m.type == motor::memory::observer_t::alloc_type::allocation ||
                m.type == motor::memory::observer_t::alloc_type::managed )
            {
                _profiling_data.memory_allocations.insert( m.sib ) ;
                _profiling_data.memory_current.insert( m.sib ) ;
            }
            else if ( m.type == motor::memory::observer_t::alloc_type::deallocation )
            {
                _profiling_data.memory_deallocations.insert( m.sib ) ;
                _profiling_data.memory_current.remove( m.sib ) ;
            }
        }
    }
    #endif

    #if MOTOR_PROFILING
    _profiling_data.profiling_data_points.clear() ;
    _profiling_data.profiling_data_points = motor::profiling::global_t::manager().swap_and_clear() ;
    
    {
        for ( auto & dp : _profiling_data.profiling_data_points  )
        {
            size_t const id = dp.probe_id ;

            auto & d = _profiling_data.tmp[ motor::profiling::manager_t::get_name(id) ] ;
            ++d.num_calls ;
            d.duration += dp.dur ;
        }

        for ( auto const & [key, value] : _profiling_data.tmp )
        {
            auto & vec = _profiling_data.function_timings[ key ] ;
            vec.append( value ) ;
        }
    }

    {
        for ( auto & dp : _profiling_data.profiling_data_points )
        {
            size_t const pid = dp.probe_id ;
            auto & cat_item = _by_categories.by_cat[ motor::profiling::manager_t::get_category( pid ) ] ;
            auto & name_item = cat_item[ motor::profiling::manager_t::get_name( pid ) ] ;
            ++name_item ;
            
        }
    }

    _profiling_data.tmp.clear() ;
    #endif
}

//******************************************************************************
bool_t engine_profiling::display( void_t ) noexcept
{
    if ( !ImGui::Begin( "Profiling Data" ) ) return false ;


    if ( ImGui::BeginTabBar( "Profiling Data##TabBar" ) )
    {
        using histogram_t = decltype( _profiling_data.memory_current ) ;

        auto common_histogram = [&] ( void_t )
        {
            motor::vector< int_t > xs( _profiling_data.memory_current.get_num_entries() )  ;
            motor::vector< int_t > ys( _profiling_data.memory_current.get_num_entries() )  ;

            _profiling_data.memory_current.for_each_entry( [&] ( size_t const i, histogram_t::data_cref_t d )
            {
                xs[ i ] = (int_t) d.value ;
                ys[ i ] = (int_t) d.count ;
            } ) ;

            //int_t const max_value = (int_t) _profiling_data.memory_current.get_max_count() ;


            ImPlot::PlotBars( "Current Footprint", xs.data(), ys.data(), (int) ys.size() / sizeof( int ), 1 );
        } ;

        if ( ImGui::BeginTabItem( "CPU Memory" ) )
        {
            if ( ImPlot::BeginPlot( "CPU Memory##the_plot" ) )
            {
                motor::vector< int_t > xs ;
                motor::vector< int_t > ys ;
                int_t max_value = 0 ;

                // draw allocation difference
                {
                    common_histogram() ;
                }
                // draw allocations
                {
                    if ( _profiling_data.display_allocations )
                    {
                        xs = motor::vector< int_t >( _profiling_data.memory_allocations.get_num_entries() )  ;
                        ys = motor::vector< int_t >( _profiling_data.memory_allocations.get_num_entries() )  ;

                        _profiling_data.memory_allocations.for_each_entry( [&] ( size_t const i, histogram_t::data_cref_t d )
                        {
                            xs[ i ] = (int_t) d.value ;
                            ys[ i ] = (int_t) d.count ;
                        } ) ;

                        max_value = (int_t) _profiling_data.memory_allocations.get_max_count() ;
                    }

                    ImPlot::HideNextItem( true ) ;
                    ImPlot::PlotBars( "Relative Allocations", xs.data(), ys.data(), (int) ys.size() / sizeof( int ), 1 );

                    if ( ImPlot::IsLegendEntryHovered( "Relative Allocations" ) )
                    {
                        if ( ImGui::IsItemClicked() )
                        {
                            _profiling_data.display_allocations = !_profiling_data.display_allocations ;
                        }
                    }
                }

                // draw deallocations
                {
                    if ( _profiling_data.display_deallocations )
                    {
                        xs = motor::vector< int_t >( _profiling_data.memory_deallocations.get_num_entries() )  ;
                        ys = motor::vector< int_t >( _profiling_data.memory_deallocations.get_num_entries() )  ;

                        _profiling_data.memory_deallocations.for_each_entry( [&] ( size_t const i, histogram_t::data_cref_t d )
                        {
                            xs[ i ] = (int_t) d.value ;
                            ys[ i ] = (int_t) d.count ;
                        } ) ;

                        max_value = (int_t) _profiling_data.memory_deallocations.get_max_count() ;
                    }

                    ImPlot::HideNextItem( true ) ;
                    ImPlot::PlotBars( "Relative Deallocations", xs.data(), ys.data(), (int) ys.size() / sizeof( int ), 1 );

                    if ( ImPlot::IsLegendEntryHovered( "Relative Deallocations" ) )
                    {
                        if ( ImGui::IsItemClicked() )
                        {
                            _profiling_data.display_deallocations = !_profiling_data.display_deallocations ;
                        }
                    }
                }
                ImPlot::EndPlot() ;
            }
            ImGui::EndTabItem() ;
        }

        size_t const max_entries = 1000 ;

        uint_t num_calls[ max_entries ] ;
        uint_t spend_micro[ max_entries ] ;

        if ( ImGui::BeginTabItem( "CPU Probes" ) )
        {
            if ( ImGui::TreeNode( "Unordered" ) )
            {
                for ( auto & [key, value] : _profiling_data.function_timings )
                {
                    for ( size_t i = value.cur_pos(); i < max_entries; ++i )
                    {
                        num_calls[ i ] = 0 ;
                        spend_micro[ i ] = 0 ;
                    }

                    for ( size_t i = 0; i < std::min( size_t( max_entries ), value.cur_pos()+1 ) ; ++i )
                    {
                        num_calls[ i ] = value[ i ].num_calls ;
                        spend_micro[ i ] = uint_t( std::chrono::duration_cast<std::chrono::microseconds>(
                            value[ i ].duration ).count() )  ;
                    }

                    if ( ImGui::TreeNode( key.c_str() ) )
                    {
                        if ( ImPlot::BeginPlot( key.c_str() ) )
                        {
                            ImPlot::PlotLine( "Num Calls", num_calls, max_entries ) ;
                            ImPlot::PlotLine( "Microsecs", spend_micro, max_entries ) ;
                            ImPlot::EndPlot() ;
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
            if ( ImGui::TreeNode( "By Category" ) )
            {
                for ( auto & [category, by_name] : _by_categories.by_cat )
                {
                    if ( ImGui::TreeNode( category ) )
                    {
                        for ( auto & [name, not_used_var] : by_name )
                        {
                            this_t::profiling_data::ring_of_function_timings_t & value =
                                _profiling_data.function_timings[name] ;

                            if ( ImGui::TreeNode( name ) )
                            {
                                for ( size_t i = value.cur_pos(); i < max_entries; ++i )
                                {
                                    num_calls[ i ] = 0 ;
                                    spend_micro[ i ] = 0 ;
                                }

                                for ( size_t i = 0; i < std::min( size_t( max_entries ), value.cur_pos() ) ; ++i )
                                {
                                    num_calls[ i ] = value[ i ].num_calls ;
                                    spend_micro[ i ] = uint_t( std::chrono::duration_cast<std::chrono::microseconds>(
                                        value[ i ].duration ).count() )  ;
                                }

                                if ( ImPlot::BeginPlot( name ) )
                                {
                                    ImPlot::PlotLine( "Num Calls", num_calls, max_entries ) ;
                                    ImPlot::PlotLine( "Microsecs", spend_micro, max_entries ) ;
                                    ImPlot::EndPlot() ;
                                }

                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                
                ImGui::TreePop();
            }

            if ( ImGui::TreeNode( "Functions" ) )
            {
                if ( ImGui::TreeNode( "Category 1" ) )
                {
                    for ( int i = 0; i < 5; i++ )
                    {
                        if ( ImGui::TreeNode( (void *) (intptr_t) i, "Child %d", i ) )
                        {
                            #if 0
                            ImGui::SameLine() ;

                            if ( ImPlot::BeginPlot( "Stock Prices" ) )
                            {
                                ImPlot::PlotLine( "Stock 1", xs1, ys1, 101 );
                                ImPlot::EndPlot() ;
                            }
                            #endif
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if ( ImGui::TreeNode( "Category 2" ) )
                {
                    for ( int i = 0; i < 5; i++ )
                    {
                        if ( ImGui::TreeNode( (void *) (intptr_t) i, "Child %d", i ) )
                        {
                            ImGui::Text( "blah blah" );
                            ImGui::SameLine();
                            if ( ImGui::SmallButton( "button" ) ) {}
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }

            ImGui::EndTabItem() ;
        }

        if ( ImGui::BeginTabItem( "GPU Memory" ) )
        {
            ImGui::EndTabItem() ;
        }

        if ( ImGui::BeginTabItem( "GPU Performace" ) )
        {
            ImGui::EndTabItem() ;
        }

        ImGui::EndTabBar() ;
    }

    ImGui::End() ;

    return true ;
}