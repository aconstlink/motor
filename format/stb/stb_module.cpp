
#include "stb_module.h"

#include "../future_items.hpp"

#include <motor/font/structs.h>
#include <motor/io/database.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/graphics/texture/image.hpp>
#include <motor/math/vector/vector4.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stb_vorbis.c>
#include <stb_truetype.h>

#include <motor/font/stb/stb_glyph_atlas_creator.h>

using namespace motor::format ;

// ***
void_t stb_module_register::register_module( motor::format::module_registry_mtr_t reg ) 
{
    reg->register_import_factory( { "png", "jpg" }, motor::shared( stb_image_factory_t(), "stb_image_factory" ) ) ;
    reg->register_export_factory( { "png" }, motor::shared( stb_image_factory_t(), "stb_image_factory" ) ) ;

    reg->register_import_factory( { "ogg" }, motor::shared( stb_audio_factory_t(), "stb_image_factory" ) ) ;
    reg->register_import_factory( { "ttf" }, motor::shared( stb_font_factory_t(), "stb_image_factory" ) ) ;
}

// ***
motor::format::future_item_t stb_image_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db ) noexcept 
{
    return stb_image_module::import_from( loc, db, motor::shared( motor::property::property_sheet_t() ) ) ;
}

// ***
motor::format::future_item_t stb_image_module::import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    { 
        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps ) ;

        motor::memory::malloc_guard<char_t> data_buffer ;

        motor::io::database_t::cache_access_t ca = db->load( loc ) ;
        auto const res = ca.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const ) 
        { 
            data_buffer = motor::memory::malloc_guard<char_t>( data, sib ) ;
        } ) ;

        if( !res ) 
        {
            motor::log::global_t::error( "[wav_import] : can not load location " + loc.as_string() ) ;
            return motor::shared( motor::format::status_item_t( "error" ) ) ;
        }

        int width, height, comp ;

        //
        // loading the image in uchar format
        // @todo float images
        //
        
        uchar_ptr_t stb_data_ptr = stbi_load_from_memory( uchar_cptr_t( data_buffer.get() ),
            int_t( data_buffer.size() ), &width, &height, &comp, 0 ) ;

        if( motor::log::global::error( stb_data_ptr == nullptr,
            motor_log_fn( "stbi_load_from_memory" ) ) )
        {
            char_cptr_t stbi_err_msg = stbi_failure_reason() ;
            motor::log::global::error( motor_log_fn( "stb_image" )
                + motor::string_t( stbi_err_msg ) ) ;

            return motor::shared( motor::format::status_item_t("Can not load from memory.") ) ;
        }

        motor::graphics::image_format imf = motor::graphics::image_format::unknown ;
        switch( comp )
        {
        case 1: imf = motor::graphics::image_format::intensity ; break ;
        case 3: imf = motor::graphics::image_format::rgba ; break ;
        case 4: imf = motor::graphics::image_format::rgba; break ;
        default: break  ;
        }

        motor::graphics::image_t img( imf, motor::graphics::image_element_type::uint8,
            motor::graphics::image_t::dims_t( size_t( width ), size_t( height ), 1 ) ) ;

        if( comp == 3 )
        {
            img.update( [&] ( motor::graphics::image_ptr_t, motor::graphics::image_t::dims_in_t dims, void_ptr_t data_in )
            {
                typedef motor::math::vector3< uint8_t > rgb_t ;
                typedef motor::math::vector4< uint8_t > rgba_t ;
                auto* dst = reinterpret_cast< rgba_t* >( data_in ) ;
                auto* src = reinterpret_cast< rgb_t* >( stb_data_ptr ) ;

                size_t const ne = dims.x() * dims.y() * dims.z() ;
                for( size_t i=0; i<ne; ++i ) 
                {
                    // mirror y
                    #if 1
                    {
                        size_t const start = ne - width * ( ( i / width ) + 1 ) ;
                        dst[ i ] = rgba_t( src[ start + i % width ], 255 );
                    }
                    #else
                    {
                        dst[ i ] = rgba_t( src[ i ], 255 );
                    }
                    #endif
                }
            } ) ;
        }
        else if( comp == 4 )
        {
            img.update( [&] ( motor::graphics::image_ptr_t, motor::graphics::image_t::dims_in_t dims, void_ptr_t data_in )
            {
                typedef motor::math::vector4< uint8_t > rgba_t ;
                auto* dst = reinterpret_cast< rgba_t* >( data_in ) ;
                auto* src = reinterpret_cast< rgba_t* >( stb_data_ptr ) ;

                size_t const ne = dims.x() * dims.y() * dims.z() ;
                for( size_t i = 0; i < ne; ++i )
                {
                    // mirror y
                    #if 1
                    {
                        size_t const start = ne - width * ( ( i / width ) + 1 ) ;
                        dst[ i ] = rgba_t( src[ start + i % width ] );
                    }
                    #else
                    {
                        dst[ i ] = rgba_t( src[ i ] );
                    }
                    #endif
                    
                }

            } ) ;
        }
        stbi_image_free( stb_data_ptr ) ;

        return motor::shared( motor::format::image_item_t( motor::shared( std::move( img ) ) ), "fitem" ) ;
    } ) ;
}

// ***************************************************************************
motor::format::future_item_t stb_image_module::export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t what ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > rel( what ) ;
        return motor::shared( motor::format::status_item_t( "Export not implemented" ) ) ;
    } ) ;
}

// ***************************************************************************
motor::format::future_item_t stb_audio_module::import_from( motor::io::location_cref_t loc, 
    motor::io::database_mtr_t db ) noexcept
{
    return stb_audio_module::import_from( loc, std::move( db ), 
        motor::shared( motor::property::property_sheet_t() ) ) ;
}

// ***************************************************************************
motor::format::future_item_t stb_audio_module::import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps ) ;

        motor::memory::malloc_guard<char_t> data_buffer ;

        motor::io::database_t::cache_access_t ca = db->load( loc ) ;
        auto const res = ca.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
        {
            data_buffer = motor::memory::malloc_guard<char_t>( data, sib ) ;
        } ) ;

        if( !res )
        {
            motor::log::global_t::error( "[stb_module] : can not load location " + loc.as_string() ) ;
            return motor::shared( motor::format::status_item_t( "error" ) ) ;
        }

        motor::audio::buffer_t bo ;

        {
            int_t error ;
            stb_vorbis* stbv = stb_vorbis_open_memory( uchar_cptr_t( data_buffer.get() ), data_buffer.size(), &error, nullptr ) ;

            if( stbv == nullptr )
            {
                motor::log::global_t::error( "[stb_module] : failed to import .ogg file [" + 
                    loc.as_string() + "] with stb error code [" + motor::to_string( error ) + "]" ) ;

                return motor::shared( motor::format::status_item_t( "Error loading .ogg file" ) ) ;
            }

            size_t sample_rate = 0 ;
            size_t channels = 0 ;
            size_t max_frames = 0 ;

            {
                stb_vorbis_info const info = stb_vorbis_get_info( stbv ) ;
                sample_rate = info.sample_rate ;
                channels = info.channels ;
                max_frames = info.max_frame_size ;
            }

            {
                motor::vector< float_t > samples ;
                motor::vector< float_t > tmp( 1 << 14 ) ;

                int_t num_samples = -1 ;
                size_t s = 0 ;

                while( num_samples != 0 )
                {
                    num_samples = stb_vorbis_get_samples_float_interleaved(
                        stbv, channels, tmp.data(), tmp.size() ) ;

                    size_t const start = samples.size() ;
                    samples.resize( samples.size() + size_t( num_samples * channels) ) ;
                    for( size_t i=0; i<num_samples<<1; i+=channels )
                    {
                        for( size_t j=0; j<channels; ++j )
                        {
                            samples[ start + i + j ] = tmp[ i + j ] ;
                        }
                    }

                    ++s ;
                }
                
                motor::log::global_t::status( "[stb_module] : Loaded (" + motor::to_string( s ) + " x " + 
                    motor::to_string( tmp.size() ) + ") samples : " + motor::to_string( samples.size() ) ) ;

                bo.set_samples( motor::audio::to_channels( channels ), sample_rate, std::move( samples ) ) ;
            }
            stb_vorbis_close( stbv ) ;
        }

        return motor::shared( motor::format::audio_item_t( motor::shared( std::move( bo ) ) ) ) ;
    } ) ;
}

// ***
motor::format::future_item_t stb_audio_module::export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t what ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > rel( what ) ;
        return motor::shared( motor::format::status_item_t( "Export not implemented" ) ) ;
    } ) ;
}

// ***
motor::format::future_item_t stb_font_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db ) noexcept
{
    return stb_font_module::import_from( loc, std::move( db ), motor::shared( motor::property::property_sheet_t() ) ) ;
}

// ***
motor::format::future_item_t stb_font_module::import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps ) ;

        motor::memory::malloc_guard<char_t> data_buffer ;

        motor::font::font_files_t ffs ;

        motor::vector< motor::io::database_t::cache_access_t > caches = 
        {
            db->load( loc )
        } ;
        
        // check additional locations
        {
            motor::vector< motor::io::location_t > additional_locations ;
            if( ps->get_value( "additional_locations", additional_locations ) )
            {
                for( auto const & l : additional_locations )
                {
                    caches.emplace_back( db->load( l ) ) ;
                }
            }
        }

        for( auto & cache : caches )
        {
            auto const res = cache.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
            {
                ffs.emplace_back( motor::font::font_file_t( loc.as_string(), (uchar_cptr_t)data, sib ) ) ;
            }) ;

            if( !res ) 
            {
                return motor::shared( motor::format::status_item_t( "Error loading .ttf file [" + loc.as_string() + "]" ) ) ;
            }
        }        

        motor::font::code_points_t pts ;
        if( !ps->get_value( "code_points", pts ) )
        {
            for( uint32_t i=33; i<=126; ++i ) pts.emplace_back( i ) ;
            for( uint32_t i : {uint32_t(0x00003041)} ) pts.emplace_back( i ) ;
        }
        
        pts.emplace_back( motor::font::utf32_t('?') ) ;
        pts.emplace_back( motor::font::utf32_t(' ') ) ;

        size_t dpi ;
        if( !ps->get_value( "dpi", dpi ) ) dpi = 96 ;

        size_t pt ;
        if( !ps->get_value( "point_size", pt ) ) pt = 25 ;

        size_t max_width, max_height ;
        if( !ps->get_value( "atlas_width", max_width ) ) max_width = 2048 ;
        if( !ps->get_value( "atlas_height", max_height ) ) max_height = 2048 ;

        motor::font::glyph_atlas_t ga = motor::font::stb::glyph_atlas_creator_t::create_glyph_atlas( ffs, pt, dpi, pts, 
            max_width, max_height ) ;
                    

        return motor::shared( motor::format::glyph_atlas_item_t( motor::shared( std::move( ga ) ) ) ) ;
    } ) ;
}

// ***
motor::format::future_item_t stb_font_module::export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t what ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > resl( what ) ;
        return motor::shared( motor::format::status_item_t( "Export not implemented" ) ) ;
    } ) ;
}