
#include "generator.h"

#include <motor/log/global.h>

using namespace motor::msl ;

//*******************************************************************************
motor::msl::variable_mappings_t generator::generate( motor::msl::generated_code_inout_t genc ) noexcept 
{
    motor::msl::variable_mappings_t mappings = this_t::generate_mappings() ;

    // determine geometry stage vertex inputs/outputs
    {
        // last shader in geometry stage
        motor::msl::shader_type last_shader = motor::msl::shader_type::vertex_shader ; 

        for( auto const & var : mappings )
        {
            if( var.fq != motor::msl::flow_qualifier::out ) continue ;
            
            if( var.st == motor::msl::shader_type::geometry_shader ) 
                last_shader = motor::msl::shader_type::geometry_shader ;
        }

        // vertex attributes are vertex shader inputs
        for( auto const & vm : mappings )
        {
            motor::msl::generated_code_t::variable_t var =
            {
                vm.new_name,
                vm.binding, 
                vm.fq
            } ;

            // determine inputs
            if( vm.fq == motor::msl::flow_qualifier::in &&
                vm.st == motor::msl::shader_type::vertex_shader )
            {
                genc.geometry_ins.emplace_back( var ) ;
            }

            // determine outputs
            else if( vm.fq == motor::msl::flow_qualifier::out &&
                    vm.st == last_shader )
            {
                var.name = "streamout." + var.name ; // must be streamout
                genc.geometry_outs.emplace_back( var ) ;
            }
        }
    }

    // streamout type
    {
        bool_t has_pixel_shader = false ;

        for( auto const& s : _genable.config.shaders )
        {
            if( s.type == motor::msl::shader_type::pixel_shader )
            {
                has_pixel_shader = true ;
                break ;
            }
        }
        if( !has_pixel_shader ) genc.streamout = motor::msl::streamout_type::interleaved ; 
    }

    return mappings ;
}

    //*******************************************************************************
motor::msl::variable_mappings_t generator::generate_mappings( void_t ) const noexcept 
{
    motor::msl::variable_mappings_t mappings ;

    motor::msl::generated_code_t::shader_t shd ;
    {
        for( auto const& s : _genable.config.shaders )
        {
            motor::msl::shader_type const s_type = s.type ;

            if( s_type == motor::msl::shader_type::unknown )
            {
                motor::log::global_t::warning( "[generator] : unknown shader type" ) ;
                continue;
            }

            for( auto const& v : s.variables )
            {
                motor::msl::variable_mapping_t vm ;
                vm.st = s_type ;
                vm.t = v.type ;

                // everything is var first
                motor::string_t flow = "var_" ;

                if( v.fq == motor::msl::flow_qualifier::global )
                {
                    flow = "" ;
                } else if( s.type == motor::msl::shader_type::vertex_shader &&
                    v.fq == motor::msl::flow_qualifier::in )
                {
                    flow = "in_" ;
                } else if( s.type == motor::msl::shader_type::pixel_shader &&
                    v.fq == motor::msl::flow_qualifier::out )
                {
                    flow = "out_" ;
                }
                vm.new_name = flow + v.name ;
                vm.old_name = v.name ;
                vm.binding = v.binding ;
                vm.fq = v.fq ;
                mappings.emplace_back( std::move( vm ) ) ;
            }
        }
    }

    return mappings ;
}