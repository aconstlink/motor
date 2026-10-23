

#include "light_component.hpp"
#include "name_component.hpp"
#include "graphics/msl_set_component.hpp"
#include "graphics/render_settings_component.hpp"
#include "graphics/geometry_name_component.hpp"

using namespace motor::scene;

uint64_t const name_component::tid =
    motor::scene::global::register_internal_type( "motor::scene::name_component" );

uint64_t const light_component::tid =
    motor::scene::global::register_internal_type( "motor::scene::light_component" );

uint64_t const msl_set_component::tid =
    motor::scene::global::register_internal_type( "motor::scene::msl_set_component" );

uint64_t const render_settings_component::tid =
    motor::scene::global::register_internal_type( "motor::scene::render_settings_component" );

uint64_t const geometry_name_component::tid =
    motor::scene::global::register_internal_type( "motor::scene::geometry_name_component" );