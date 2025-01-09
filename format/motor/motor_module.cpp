
#include "motor_module.h"
#include "../module_registry.hpp"

#include <motor/io/database.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/graphics/texture/image.hpp>
#include <motor/math/vector/vector4.hpp>

#include <rapidxml.hpp>
#include <rapidxml_ext.hpp>
#include <rapidxml_utils.hpp>

#include <array>
#include <sstream>

using namespace motor::format ;

namespace this_file
{
    template< typename T, size_t N >
    static bool_t parse( motor::string_cref_t what, std::array< T, N > & ret  ) noexcept
    {
        size_t n = 0 ;
        size_t l = 0 ;
        size_t p = 0 ;  ;
        
        while( ( p = what.find_first_of( ' ', l ) )  != std::string::npos && n < N )
        {
            ret[ n++ ] = T( std::stoi( what.substr( l, p - l ).c_str() ) ) ;
            
            l = p + 1;
            //p = what.find_first_of( ' ', p + 1 ) ;
        }

        if( n < N ) ret[ n ] = T(std::stoi( what.substr( l ).c_str() )) ;

        return n == N ;
    }
}

// ***
void_t motor_module_register::register_module( motor::format::module_registry_mtr_t reg ) noexcept
{
    reg->register_import_factory( { "motor" }, motor::shared( motor_factory_t() ) ) ;
    reg->register_export_factory( { "motor" }, motor::shared( motor_factory_t() ) ) ;
}

// ***
motor::format::future_item_t motor_module::import_from( motor::io::location_cref_t loc, 
    motor::io::database_mtr_t db, motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return this_t::import_from( loc, db, motor::shared( motor::property::property_sheet_t() ), motor::move( mod_reg ) ) ;
}

// ***
motor::format::future_item_t motor_module::import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps, 
                motor::format::module_registry_mtr_safe_t mod_reg_ ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps ) ;
        motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ ) ;

        motor_document_t nd ;

        motor::memory::malloc_guard<char> content ;
        motor::io::database_t::cache_access_t ca = db->load( loc ) ;
        {
            auto const res = ca.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
            {
                motor::string_t file = motor::string_t( data, sib ) ;
                content = motor::memory::malloc_guard<char>( file.c_str(), file.size()+1 ) ;
            } ) ;

            if( !res )
            {
                motor::log::global_t::warning( "[motor_module] : failed to load file [" + loc.as_string() + "]" ) ;

                return motor::shared( motor::format::status_item_t( "failed to load file" ) ) ;
            }

            rapidxml::xml_document<> doc ;
            doc.parse<0>( content.get() ) ;

            rapidxml::xml_node<> * root = doc.first_node( "motor" ) ;
            if( root == nullptr )
            {
                motor::string_t msg = "[motor_module] : can not find node [motor] in document [" +
                    loc.as_string() + "]" ;

                motor::log::global_t::warning( msg ) ;

                return motor::shared( motor::format::status_item_t( msg ) ) ;
            }

            // handle sprite sheets
            for( rapidxml::xml_node<> * sss_xml = root->first_node( "sprite_sheets" ); sss_xml != nullptr; 
                    sss_xml = sss_xml->next_sibling() )
            {
                for( rapidxml::xml_node<> * ss_xml = sss_xml->first_node("sprite_sheet");
                    ss_xml != nullptr; ss_xml = ss_xml->next_sibling() )
                {
                    using sprite_sheet_t = motor::format::motor_document_t::sprite_sheet_t ;
                    sprite_sheet_t ss ;

                    // handle attributes
                    {
                        rapidxml::xml_attribute<> * attr = ss_xml->first_attribute( "name" ) ;
                        if( attr == nullptr )
                        {
                            motor::string_t msg = "[motor_module] : node [sprite_sheets] "
                                "requires [name] attribute in document [" + loc.as_string() + "]" ;

                            motor::log::global_t::warning( msg ) ;
                            continue ;
                        }
                        ss.name = motor::string_t( attr->value() ) ;
                    }

                    // image
                    {
                        sprite_sheet_t::image_t image ;

                        rapidxml::xml_node<> * image_xml = ss_xml->first_node( "image" ) ;
                        if( image_xml == nullptr )
                        {
                            motor::string_t msg = "[motor_module] : node [sprite_sheets] "
                                "requires [image] node in document [" + loc.as_string() + "]" ;
                            motor::log::global_t::warning( msg ) ;
                            continue ;
                        }

                        rapidxml::xml_attribute<> * attr = image_xml->first_attribute( "src" ) ;
                        if( attr == nullptr )
                        {
                            motor::string_t msg = "[motor_module] : node [image] "
                                "requires [src] attribute in document [" + loc.as_string() + "]" ;

                            motor::log::global_t::warning( msg ) ;
                            continue ;
                        }

                        image.src = motor::string_t( attr->value() ) ;

                        ss.image = std::move( image ) ;
                    }

                    // sprites
                    {
                        rapidxml::xml_node<> * sprites_xml = ss_xml->first_node( "sprites" ) ;
                        if( sprites_xml != nullptr )
                        {
                            for( rapidxml::xml_node<> * sprite_xml = sprites_xml->first_node("sprite"); 
                                sprite_xml != nullptr; sprite_xml = sprite_xml->next_sibling() )
                            {
                                sprite_sheet_t::sprite_t sprite ;

                                // attributes
                                {
                                    rapidxml::xml_attribute<> * attr = sprite_xml->first_attribute( "name" ) ;
                                    if( attr == nullptr ) 
                                    {
                                        motor::string_t msg = "[motor_module] : node [sprite] "
                                        "requires [name] attribute in document [" + loc.as_string() + "]" ;
                                        continue ;
                                    }
                                    sprite.name = motor::string_t( attr->value() ) ;
                                }

                                // animation
                                {
                                    sprite_sheet_t::sprite_t::animation_t animation ;
                                    rapidxml::xml_node<> * node = sprite_xml->first_node("animation") ;
                                    if( node != nullptr )
                                    {
                                        // attribute rect
                                        {
                                            rapidxml::xml_attribute<> * attr = node->first_attribute( "rect" ) ;
                                            if( attr == nullptr ) 
                                            {
                                                motor::string_t msg = "[motor_module] : node [animation] "
                                                "requires [rect] attribute in document [" + loc.as_string() + "]" ;
                                                continue ;
                                            }

                                            std::array< uint_t, 4 > parsed ;
                                            this_file::parse( motor::string_t(attr->value()), parsed ) ;
                                            animation.rect = motor::math::vec4ui_t( parsed[0], parsed[1],
                                                    parsed[2], parsed[3] ) ;
                                        }

                                        // attribute pivot
                                        {
                                            rapidxml::xml_attribute<> * attr = node->first_attribute( "pivot" ) ;
                                            if( attr != nullptr ) 
                                            {
                                                std::array< int_t, 2 > parsed ;
                                                this_file::parse( motor::string_t(attr->value()), parsed ) ;
                                                animation.pivot = motor::math::vec2i_t( parsed[0], parsed[1] ) ;
                                            }
                                        }
                                    }
                                    sprite.animation = std::move( animation ) ;
                                }

                                // collision
                                {
                                    sprite_sheet_t::sprite_t::collision_t collision ;
                                    rapidxml::xml_node<> * node = sprite_xml->first_node("collision") ;
                                    if( node != nullptr )
                                    {
                                        // attribute rect
                                        {
                                            rapidxml::xml_attribute<> * attr = node->first_attribute( "rect" ) ;
                                            if( attr != nullptr ) 
                                            {
                                                std::array< uint_t, 4 > parsed ;
                                                this_file::parse( motor::string_t(attr->value()), parsed ) ;
                                                collision.rect = motor::math::vec4ui_t( parsed[0], parsed[1],
                                                    parsed[2], parsed[3] ) ;
                                            }
                                        }
                                    }
                                    sprite.collision = std::move( collision ) ;
                                }

                                // hits
                                {
                                    rapidxml::xml_node<> * node = sprite_xml->first_node("hits") ;
                                    if( node != nullptr )
                                    {
                                        for( rapidxml::xml_node<> * hit = node->first_node("hit"); hit != nullptr; 
                                            hit = hit->next_sibling() )
                                        {
                                            sprite_sheet_t::sprite_t::hit_t h ;

                                            // attribute: name
                                            {
                                                rapidxml::xml_attribute<> * attr = hit->first_attribute( "name" ) ;
                                                if( attr != nullptr )
                                                {
                                                    h.name = motor::string_t( attr->value() ) ;
                                                }
                                            }

                                            // attribute: rect
                                            {
                                                rapidxml::xml_attribute<> * attr = hit->first_attribute( "rect" ) ;
                                                if( attr != nullptr )
                                                {
                                                    std::array< uint_t, 4 > parsed ;
                                                    this_file::parse( motor::string_t(attr->value()), parsed ) ;
                                                    h.rect = motor::math::vec4ui_t( parsed[0], parsed[1],
                                                        parsed[2], parsed[3] ) ;
                                                }
                                            }

                                            sprite.hits.emplace_back( std::move( h ) ) ;
                                        }
                                    }
                                }

                                ss.sprites.emplace_back( std::move( sprite ) ) ;
                            }
                        }
                    }

                    // animations
                    {
                        rapidxml::xml_node<> * animations_xml = ss_xml->first_node( "animations" ) ;
                        if( animations_xml != nullptr ) 
                        {
                            for( rapidxml::xml_node<> * ani_xml = animations_xml->first_node("animation"); 
                                ani_xml != nullptr; ani_xml = ani_xml->next_sibling() )
                            {
                                sprite_sheet_t::animation_t animation ;

                                // attribute: object
                                {
                                    rapidxml::xml_attribute<> * attr = ani_xml->first_attribute( "object" ) ;
                                    if( attr != nullptr )
                                    {
                                        animation.object = motor::string_t( attr->value() ) ;
                                    }
                                }

                                // attribute: name
                                {
                                    rapidxml::xml_attribute<> * attr = ani_xml->first_attribute( "name" ) ;
                                    if( attr != nullptr )
                                    {
                                        animation.name = motor::string_t( attr->value() ) ;
                                    }
                                }

                                for( rapidxml::xml_node<> * frame_xml = ani_xml->first_node("frame") ; 
                                frame_xml != nullptr; frame_xml = frame_xml->next_sibling() )
                                {
                                    sprite_sheet_t::animation_t::frame_t frame ;

                                    // attribute: sprite
                                    {
                                        rapidxml::xml_attribute<> * attr = frame_xml->first_attribute( "sprite" ) ;
                                        if( attr != nullptr )
                                        {
                                            frame.sprite = motor::string_t( attr->value() ) ;
                                        }
                                    }

                                    // attribute: duration
                                    {
                                        rapidxml::xml_attribute<> * attr = frame_xml->first_attribute( "duration" ) ;
                                        if( attr != nullptr )
                                        {
                                            frame.duration = std::stol( attr->value() ) ;
                                        }
                                    }

                                    animation.frames.emplace_back( std::move( frame ) ) ;
                                }

                                ss.animations.emplace_back( std::move( animation ) ) ;
                            }
                        }
                    }

                    nd.sprite_sheets.emplace_back( std::move( ss ) ) ;
                }
            }
        }

        return motor::shared( motor::format::motor_item_t( std::move( nd ) ) ) ;
    } ) ;
}

motor::format::future_item_t motor_module::export_to( motor::io::location_cref_t loc, 
    motor::io::database_mtr_t db, motor::format::item_mtr_safe_t what,
    motor::format::module_registry_mtr_safe_t mod_reg_ ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > releaser( what ) ;
        motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ ) ;

        motor::format::motor_item_mtr_t ntm = 
            dynamic_cast<motor::format::motor_item_mtr_t>( what.mtr() ) ;

        if( ntm == nullptr )
        {
            return motor::shared( motor::format::status_item_t( "Invalid item type. motor_item expected." ) ) ;
        }

        auto const & ndoc = ntm->doc ;
        rapidxml::xml_document<> doc ;

        auto * root = doc.allocate_node( rapidxml::node_element, "motor" ) ;
        
        {
            auto * rsheets = doc.allocate_node( rapidxml::node_element, "sprite_sheets" ) ;
            
            for( auto const & ss : ndoc.sprite_sheets )
            {
                auto * rsheet = doc.allocate_node( rapidxml::node_element, "sprite_sheet" ) ;
                {
                    rsheet->append_attribute( doc.allocate_attribute( "name", ss.name.c_str() ) ) ;
                }

                {
                    auto * img = doc.allocate_node( rapidxml::node_element, "image" ) ;
                    img->append_attribute( doc.allocate_attribute( "src", ss.image.src.c_str() ) ) ;
                    rsheet->append_node( img ) ;
                }

                // sprites
                {
                    auto * n = doc.allocate_node( rapidxml::node_element, "sprites" ) ;
                    for( auto const & s : ss.sprites )
                    {
                        auto * n2 = doc.allocate_node( rapidxml::node_element, "sprite" ) ;
                        {
                            n2->append_attribute( doc.allocate_attribute( "name", s.name.c_str() ) ) ;
                            {
                                auto * n3 = doc.allocate_node( rapidxml::node_element, "animation" ) ;
                                {
                                    auto const value = std::to_string( s.animation.rect.x() ) + " " +
                                        std::to_string( s.animation.rect.y() ) + " " + 
                                        std::to_string( s.animation.rect.z() ) + " " + 
                                        std::to_string( s.animation.rect.w() ) ;

                                    n3->append_attribute( doc.allocate_attribute( "rect", doc.allocate_string( value.c_str() ) ) ) ;
                                }
                                {
                                    auto const value = std::to_string( s.animation.pivot.x() ) + " " +
                                        std::to_string( s.animation.pivot.y() ) ;

                                    n3->append_attribute( doc.allocate_attribute( "pivot", doc.allocate_string( value.c_str() ) ) ) ;
                                }
                                n2->append_node( n3 ) ;
                            }
                            {
                                auto * n3 = doc.allocate_node( rapidxml::node_element, "collision" ) ;
                                {
                                    auto const value = std::to_string( s.collision.rect.x() ) + " " +
                                        std::to_string( s.collision.rect.y() ) + " " + 
                                        std::to_string( s.collision.rect.z() ) + " " + 
                                        std::to_string( s.collision.rect.w() ) ;

                                    n3->append_attribute( doc.allocate_attribute( "rect", doc.allocate_string( value.c_str() ) ) ) ;
                                }
                                n2->append_node( n3 ) ;
                            }
                            {
                                auto * n3 = doc.allocate_node( rapidxml::node_element, "hits" ) ;
                                for( auto const & h : s.hits )
                                {
                                    auto * n4 = doc.allocate_node( rapidxml::node_element, "hit" ) ;
                                    {
                                        n4->append_attribute( doc.allocate_attribute( "name", h.name.c_str() ) ) ;
                                    }
                                    {
                                        auto const value = std::to_string( h.rect.x() ) + " " +
                                            std::to_string( h.rect.y() ) + " " + 
                                            std::to_string( h.rect.z() ) + " " + 
                                            std::to_string( h.rect.w() ) ;

                                        n4->append_attribute( doc.allocate_attribute( "rect", doc.allocate_string( value.c_str() ) ) ) ;
                                    }
                                    n3->append_node( n4 ) ;
                                }
                                n2->append_node( n3 ) ;
                            }
                            n->append_node( n2 ) ;
                        }
                    }
                    rsheet->append_node( n ) ;
                }

                // animations
                {
                    auto * n = doc.allocate_node( rapidxml::node_element, "animations" ) ;
                    for( auto const & a : ss.animations )
                    {
                        auto * ani = doc.allocate_node( rapidxml::node_element, "animation" ) ;
                        {
                            ani->append_attribute( doc.allocate_attribute( "name", doc.allocate_string( a.name.c_str() ) ) ) ;
                        }

                        for( auto const & f : a.frames )
                        {
                            auto * fr = doc.allocate_node( rapidxml::node_element, "frame" ) ;
                            {
                                fr->append_attribute( doc.allocate_attribute( "sprite", f.sprite.c_str() ) ) ;
                            }
                            {
                                fr->append_attribute( doc.allocate_attribute( "duration", doc.allocate_string( std::to_string( f.duration ).c_str() ) ) ) ;
                            }
                            ani->append_node( fr ) ;
                        }
                        n->append_node( ani ) ;
                    }
                    rsheet->append_node( n ) ;
                }
                rsheets->append_node( rsheet ) ;
            }
            root->append_node( rsheets ) ;
        }

        doc.append_node( root ) ;

        motor::string_t output ;
        print(std::back_inserter(output), doc, 0);
        
        // tabs to spaces
        {
            size_t o = 0 ;
            while( true )
            {
                auto const a = output.find_first_of( '\t', o ) ;
                if( a == std::string::npos ) break ;

                output.replace( a, 1, "    " ) ;
                o = a ;
            }
        }

        motor::io::result res = motor::io::result::invalid ;
        
        db->store( loc.as_string(), output.c_str(), output.size() ).wait_for_operation( 
            [&]( motor::io::result const r ) { res = r ; } ) ;

        return motor::shared( motor::format::status_item_t( 
            res == motor::io::result::ok ? "Exported motor document" : "Failed to export motor document ") ) ;
    } ) ;
}