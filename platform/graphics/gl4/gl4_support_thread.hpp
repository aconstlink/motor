
#pragma once

#include "gl4.h"
#include "gl4_convert.h"
#include "gl4_independent.h"

#include <motor/msl/symbol.hpp>
#include <motor/msl/database.hpp>
#include <motor/msl/parser.h>
#include <motor/msl/dependency_resolver.hpp>
#include <motor/msl/generators/generator.h>
#include <motor/msl/generators/glsl4_generator.h>

#include <motor/ogl/gl/gl.h>
#include <motor/ogl/gl/convert.hpp>
#include <motor/ogl/gl/error.hpp>
#include <motor/memory/global.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/std/vector>
#include <motor/std/stack>
#include <motor/std/string_split.hpp>
#include <motor/std/hash_map>

#include <motor/concurrent/typedefs.h>

namespace motor
{
namespace graphics
{
namespace local_gl4
{
template < typename T >
struct work_item_deduction;

struct work_item
{
    enum class work_type
    {
        compile,
        compiled,
        release
    };

    enum class obj_type
    {
        unknown,
        msl
    };

    work_type wt;
    obj_type ot;
    void_ptr_t ptr;

    template < typename T >
    static work_item make_item( work_type const t, motor::core::mtr_safe< T > obj ) noexcept
    {
        return work_item{ t, work_item_deduction< T >::deduce( obj.mtr() ),
            static_cast< void_ptr_t >( obj.mtr() ) };
    }
};
motor_typedef( work_item );

template < typename T >
struct work_item_deduction
{
    static work_item::obj_type deduce( T * ptr ) noexcept;
};

template <>
struct work_item_deduction< motor::graphics::msl_object >
{
    static work_item::obj_type deduce( motor::graphics::msl_object * ptr ) noexcept
    {
        return work_item::obj_type::msl;
    }
};
} // namespace local_gl4

namespace gl4
{
using namespace motor::platform::gen4;
using namespace motor::ogl;

template < typename O >
class support_thread
{
    motor_this_typedefs( support_thread );
    motor_typedefs( O, owner );

  public:

    // for the support thread
    struct shared_data
    {
        bool_t running = false;
        bool_t has_work = false;
        owner_ptr_t owner = nullptr;
        motor::platform::opengl::rendering_context_ptr_t ctx;

        motor::msl::database_t mdb;

        std::mutex mtx;
        std::condition_variable cv;

        std::mutex mtx_backward ;

        motor::vector< local_gl4::work_item > forward_items;
        motor::vector< gl4_independent::backward_message > backward_items;

        void_t add_forward_item( local_gl4::work_item_rref_t item ) noexcept
        {
            std::unique_lock< std::mutex > lk( mtx );

            if( forward_items.size() == forward_items.capacity() )
                forward_items.reserve( forward_items.size() + 50 );

            forward_items.emplace_back( std::move( item ) );

            has_work = true;
        }

        void_t add_backward_item( gl4_independent::backward_message && item ) noexcept
        {
            std::unique_lock< std::mutex > lk( mtx_backward );

            if( backward_items.size() == backward_items.capacity() )
                backward_items.reserve( backward_items.size() + 50 );

            backward_items.emplace_back( std::move( item ) );
        }

        using for_each_backward_item_funk_t =
            std::function< void_t( gl4_independent::backward_message & ) >;

        // must be called in-sync
        void_t for_each( for_each_backward_item_funk_t f ) noexcept
        {
            std::unique_lock< std::mutex > lk( mtx_backward );

            for( auto & item : backward_items )
            {
                f( item );
            }
            backward_items.clear();
        }

      public:

        void_t notify_thread( void_t ) noexcept
        {
            if( has_work ) cv.notify_one();
        }
    };
    motor_typedef( shared_data );

  private:

    motor::concurrent::thread_t _thread;
    this_t::shared_data_ptr_t _ctsd = nullptr;

  public:

    support_thread( void_t ) noexcept {}

    support_thread( this_cref_t ) = delete;
    support_thread( this_rref_t rhv ) noexcept
    {
        _thread = std::move( rhv._thread );
        _ctsd = motor::move( rhv._ctsd );
    }
    ~support_thread( void_t ) noexcept
    {
        assert( _ctsd == nullptr );
    }

    this_t::shared_data_ptr_t start_support_thread(
        owner_ptr_t owner, motor::platform::opengl::rendering_context_mtr_t ctx ) noexcept
    {
        assert( _ctsd == nullptr && "Just start/stop the thread." );

        if( _ctsd == nullptr )
        {
            _ctsd = motor::memory::global::alloc< shared_data >( "support thread shared data" );
            _ctsd->running = true;
            _ctsd->owner = owner;
            _ctsd->ctx = ctx;
        }

        auto * ctsd = _ctsd;

        _thread = std::thread( [ ctsd ]( void_t )
        {
            motor::log::global_t::status( "[gl4] : support thread started" );

            motor::vector< local_gl4::work_item_t > items;
            items.reserve( 50 );

            // activate shared context
            {
                auto const res = ctsd->ctx->activate();
                if( res != motor::platform::result::ok )
                {
                    motor::log::global::error( "[gl4] : unable to activate shared context" );
                    ctsd->running = false;
                }
            }

            // just testing the shared context
            {
                glClearColor( 1.0f, 1.0f, 0.0f, 1.0f );
                auto err = glGetError();
                assert( err == GL_NO_ERROR );
                glClear( GL_COLOR_BUFFER_BIT );
                err = glGetError();
                assert( err == GL_NO_ERROR );
            }

            while( ctsd->running )
            {
                // wait for some work and
                // swap the items buffer
                {
                    std::unique_lock< std::mutex > lk( ctsd->mtx );
                    while( !ctsd->has_work && ctsd->running ) ctsd->cv.wait( lk );

                    auto tmp = std::move( items );
                    items = std::move( ctsd->forward_items );
                    ctsd->forward_items = std::move( tmp );

                    ctsd->has_work = false;
                }

                for( auto & item : items )
                {
                    switch( item.ot )
                    {
                    case local_gl4::work_item::obj_type::msl: {
                        auto * msl = static_cast< motor::graphics::msl_object_ptr_t >( item.ptr );
                        switch( item.wt )
                        {
                        case local_gl4::work_item::work_type::compile: {
                            auto [ res, so, sd ] =
                                gl4_independent::construct_shader_from_msl_data_st(
                                    *msl, ctsd->mdb );
                            ctsd->add_backward_item( gl4_independent::compilation_result{
                                res, motor::move( msl ), std::move( so ), std::move( sd ) } );
                                item.ptr = nullptr ;
                        }

                        break;
                        case local_gl4::work_item::work_type::release:
                            break;
                        default:
                            break;
                        }
                    }
                    break;
                    default:
                        break;
                    }

                    if( !ctsd->running ) break;
                }

                for( auto & item : items )
                {
                    auto obj_ptr = static_cast< motor::graphics::object * >( item.ptr );
                    motor::release( motor::move( obj_ptr ) );
                }
                items.clear();
            }
            ctsd->ctx->deactivate();

            motor_status( "[gl4] : support thread shut down" );
        } );

        return _ctsd;
    }

    void_t stop_support_thread( void_t ) noexcept
    {
        if( _ctsd == nullptr ) return;

        {
            std::unique_lock< std::mutex > lk( _ctsd->mtx );
            _ctsd->running = false;
        }
        _ctsd->cv.notify_one();
        if( _thread.joinable() ) _thread.join();

        motor::memory::global::dealloc< shared_data >( motor::move( _ctsd ) );
    }
};
} // namespace gl4
} // namespace graphics
} // namespace motor