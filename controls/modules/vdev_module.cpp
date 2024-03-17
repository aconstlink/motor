

#include "vdev_module.h"

#include "../mapping.hpp"

#include <motor/controls/layouts/xbox_controller.hpp>
#include <motor/controls/layouts/game_controller.hpp>
#include <motor/controls/layouts/ascii_keyboard.hpp>
#include <motor/controls/layouts/three_mouse.hpp>

#include <motor/log/global.h>

using namespace motor::controls ;

//***
vdev_module::vdev_module( void_t ) noexcept  
{
    for( size_t i=0; i<4; ++i )
    {
        this_t::data d ;
        d.dev = motor::memory::create_ptr( motor::controls::game_device_t( 
            "logical game controller " + motor::from_std( std::to_string(i) ) ) ) ;

        _games.emplace_back( std::move( d ) ) ;
    }
}

//***
vdev_module::vdev_module( this_rref_t rhv ) noexcept 
{
    _games = std::move( rhv._games ) ;
}

vdev_module::~vdev_module( void_t ) noexcept
{
}

//***
vdev_module::this_ref_t vdev_module::operator = ( this_rref_t rhv ) noexcept
{
    _games = std::move( rhv._games ) ;
    return *this ;
}

//***
void_t vdev_module::search( motor::controls::imodule::search_funk_t funk ) noexcept
{
    for( auto & g : _games )
    {
        funk( motor::share( g.dev ) ) ;
    }
}

//***
void_t vdev_module::update( void_t ) noexcept
{
    for( auto & g : _games )
    {
        g.dev->update() ;
    }
}

void_t vdev_module::release( void_t ) noexcept 
{
    _games.clear() ;
}

//***
void_t vdev_module::search( this_t::mapping_searach_ft ) noexcept
{
}

//***
void_t vdev_module::check_devices( motor::controls::imodule_mtr_t mod ) noexcept 
{
    this_t::init_controller_1( mod ) ;
}

//***
void_t vdev_module::init_controller_1( motor::controls::imodule_mtr_t mod ) noexcept
{
    this_t::data & g = _games[ 0 ] ;

    {
        motor::controls::xbc_device_mtr_t xbc_dev ;
        motor::string_t const name = "[system] : xbox" ;

        bool_t have_xbox = false ;
        g.dev->search( [&] ( motor::controls::imapping_mtr_t r )
        {
            if( r->name() == name ) have_xbox = true ;
        } ) ;

        mod->search( [&] ( motor::controls::idevice_mtr_t dev_in )
        {
            if( auto * ptr = dynamic_cast<motor::controls::xbc_device_mtr_t>( dev_in ); ptr != nullptr )
            {
                xbc_dev = ptr ;
            }
        } ) ;

        // do mappings for xbox
        if( !have_xbox && xbc_dev != nullptr )
        {
            using a_t = motor::controls::game_device_t ;
            using b_t = motor::controls::xbc_device_t ;

            using ica_t = a_t::layout_t::input_component ;
            using icb_t = b_t::layout_t::input_component ;

            using mapping_t = motor::controls::mapping< a_t, b_t > ;
            mapping_t m( name, g.dev, xbc_dev ) ;

            {
                auto const res = m.insert( ica_t::jump, icb_t::button_a ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }
            {
                auto const res = m.insert( ica_t::shoot, icb_t::button_b ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }
            {
                auto const res = m.insert( ica_t::action_a, icb_t::button_x ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }
            {
                auto const res = m.insert( ica_t::action_b, icb_t::button_y ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }
            {
                auto const res = m.insert( ica_t::aim, icb_t::stick_right ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }
            {
                auto const res = m.insert( ica_t::movement, icb_t::stick_left ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }

            g.dev->add_mapping( motor::memory::create_ptr( mapping_t(m) ) ) ;
        }
    }

    {
        motor::controls::ascii_device_mtr_t ascii_dev ;
        motor::string_t const name = "[system] : ascii" ;

        bool_t have_ascii = false ;
        g.dev->search( [&] ( motor::controls::imapping_mtr_t r )
        {
            if( r->name() == name ) have_ascii = true ;
        } ) ;

        mod->search( [&] ( motor::controls::idevice_mtr_t dev_in )
        {
            if( auto * ptr = dynamic_cast<motor::controls::ascii_device_mtr_t>( dev_in ); ptr != nullptr )
            {
                ascii_dev = ptr ;
            }
        } ) ;

        // do mappings for ascii
        if( !have_ascii && ascii_dev != nullptr )
        {
            using a_t = motor::controls::game_device_t ;
            using b_t = motor::controls::ascii_device_t ;

            using ica_t = a_t::layout_t::input_component ;
            //using icb_t = b_t::layout_t::input_component ;

            using mapping_t = motor::controls::mapping< a_t, b_t > ;
            mapping_t m( name, g.dev, ascii_dev ) ;

            {
                auto const res = m.insert( ica_t::jump,
                    b_t::layout_t::ascii_key_to_input_component( b_t::layout_t::ascii_key::space ) ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }

            {
                auto const res = m.insert( ica_t::movement,
                    b_t::layout_t::ascii_key_to_input_component( b_t::layout_t::ascii_key::a ),
                    motor::controls::mapping_detail::negative_x ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }

            {
                auto const res = m.insert( ica_t::movement,
                    b_t::layout_t::ascii_key_to_input_component( b_t::layout_t::ascii_key::d ),
                    motor::controls::mapping_detail::positive_x ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }

            {
                auto const res = m.insert( ica_t::movement,
                    b_t::layout_t::ascii_key_to_input_component( b_t::layout_t::ascii_key::w ),
                    motor::controls::mapping_detail::positive_y ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }

            {
                auto const res = m.insert( ica_t::movement,
                    b_t::layout_t::ascii_key_to_input_component( b_t::layout_t::ascii_key::s ),
                    motor::controls::mapping_detail::negative_y ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }

            g.dev->add_mapping( motor::memory::create_ptr( mapping_t(m) ) ) ;
        }
    }

    // mouse
    {
        motor::controls::three_device_mtr_t three_dev ;
        motor::string_t const name = "[system] : mouse" ;

        bool_t have_mouse = false ;
        g.dev->search( [&] ( motor::controls::imapping_mtr_t r )
        {
            if( r->name() == name ) have_mouse = true ;
        } ) ;

        mod->search( [&] ( motor::controls::idevice_mtr_t dev_in )
        {
            if( auto * ptr = dynamic_cast<motor::controls::three_device_mtr_t>( dev_in ); ptr != nullptr )
            {
                three_dev = ptr ;
            }
        } ) ;

        // do mappings for mouse
        if( !have_mouse && three_dev != nullptr )
        {
            using a_t = motor::controls::game_device_t ;
            using b_t = motor::controls::three_device_t ;

            using ica_t = a_t::layout_t::input_component ;
            using icb_t = b_t::layout_t::input_component ;

            using mapping_t = motor::controls::mapping< a_t, b_t > ;
            mapping_t m( name, g.dev, three_dev ) ;

            {
                auto const res = m.insert( ica_t::aim, icb_t::local_coords ) ;
                motor::log::global_t::warning( !res, "can not do mapping." ) ;
            }

            g.dev->add_mapping( motor::memory::create_ptr( mapping_t(m) ) ) ;
        }
    }
}

//***
