
#pragma once

#include "../../render_engine.h"
#include "../ifrontend.h"
#include "../../backend/gen4/backend.h"

#include "../command_status.hpp"

namespace motor
{
namespace graphics
{
namespace gen4
{
class frontend : public motor::graphics::ifrontend
{
    motor_this_typedefs( frontend );

  private:

    motor::graphics::render_engine_ptr_t _re;
    motor::graphics::gen4::backend_ptr_t _be;

  public:

    frontend(
        motor::graphics::render_engine_ptr_t re, motor::graphics::gen4::backend_ptr_t be ) noexcept
        : _re( re ), _be( be )
    {
    }

    frontend( this_cref_t ) = delete;
    frontend( this_rref_t rhv ) noexcept
    {
        _re = motor::move( rhv._re );
        _be = motor::move( rhv._be );
    }
    virtual ~frontend( void_t ) noexcept {}

    bool_t has_commands( void_t ) const noexcept
    {
        return _re->can_execute();
    }

    motor::graphics::render_engine_ptr_t get_render_engine( void_t ) noexcept
    {
        return _re;
    }

    virtual bool_t can_enter_frame( void_t ) const noexcept
    {
        return _re->can_enter_frame();
    }

    virtual motor::graphics::render_engine_ptr_t borrow_render_engine( void_t ) noexcept
    {
        return _re;
    }

  public:

    template < typename T >
    this_ref_t configure( typename motor::core::mtr_borrow< T >::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->configure( o ); } );

        return *this;
    }

    template < typename T >
    this_ref_t configure( typename motor::core::mtr_borrow< T >::mtr_t o,
        motor::graphics::command_status_mtr_t cs ) noexcept
    {
        assert( cs != nullptr );

        if( !cs->set_in_transit( _be->get_bid() ) ) return *this;
        _re->send_execute( [ = ]( void_t )
        {
            auto const res = _be->configure( o );
            cs->set( _be->get_bid(), res );
            cs->set( _be->get_bid(), motor::graphics::command_status::status::configured );
        } );

        return *this;
    }

    template < typename T >
    this_ref_t release( typename motor::core::mtr_borrow< T >::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->release( o ); } );

        return *this;
    }

    template < typename T >
    this_ref_t release( typename motor::core::mtr_borrow< T >::mtr_t o,
        motor::graphics::command_status_mtr_t cs ) noexcept
    {
        assert( cs != nullptr );

        if( !cs->set_in_transit( _be->get_bid() ) ) return *this;
        _re->send_execute( [ = ]( void_t )
        {
            auto const res = _be->release( o );
            cs->set( _be->get_bid(), res );
            cs->set( _be->get_bid(), motor::graphics::command_status::status::released );
        } );

        return *this;
    }

    // takes the managed pointer and releases it after
    // the backend function is called. This function can be
    // used to let the fontend release the last instance
    // of an object.
    // e.g. if the owner calls, call it like so:
    // fe->release( motor::move( some_obj ) ) ;
    // => the application layer does not need to wait for
    // the release!
    template < typename T >
    this_ref_t release( motor::core::mtr_safe< T > o ) noexcept
    {
        _re->send_execute( [ =, mtr = o.mtr() ]( void_t )
        {
            _be->release( mtr );
            motor::memory::release_ptr( mtr );
        } );

        return *this;
    }

    this_ref_t update( motor::graphics::geometry_object_borrow_t::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->update( o ); } );

        return *this;
    }

    this_ref_t update( motor::graphics::array_object_borrow_t::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->update( o ); } );

        return *this;
    }

    this_ref_t update( motor::graphics::streamout_object_borrow_t::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->update( o ); } );

        return *this;
    }

    this_ref_t update( motor::graphics::image_object_borrow_t::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->update( o ); } );

        return *this;
    }

    this_ref_t update(
        motor::graphics::render_object_borrow_t::mtr_t o, size_t const varset ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->update( o, varset ); } );

        return *this;
    }

    this_ref_t use( motor::graphics::framebuffer_object_borrow_t::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->use( o ); } );

        return *this;
    }

    this_ref_t use( motor::graphics::streamout_object_borrow_t::mtr_t o ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->use( o ); } );

        return *this;
    }

    this_ref_t unuse( motor::graphics::gen4::backend::unuse_type const t ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->unuse( t ); } );

        return *this;
    }

    this_ref_t push( motor::graphics::state_object_borrow_t::mtr_t so, size_t const sid = 0,
        bool_t const push = true ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->push( so, sid, push ); } );

        return *this;
    }

    this_ref_t pop( motor::graphics::gen4::backend::pop_type const pt ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->pop( pt ); } );

        return *this;
    }

    this_ref_t render( motor::graphics::render_object_borrow_t::mtr_t o,
        motor::graphics::gen4::backend::render_detail_cref_t rd ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->render( o, rd ); } );

        return *this;
    }

    this_ref_t render( motor::graphics::msl_object_borrow_t::mtr_t o,
        motor::graphics::gen4::backend::render_detail_cref_t rd ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { _be->render( o, rd ); } );

        return *this;
    }

    void_t force_context_swap( void_t ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) {} );
    }

    using fence_funk_t = std::function< void_t( void_t ) >;
    void_t fence( fence_funk_t funk ) noexcept
    {
        _re->send_execute( [ = ]( void_t ) { funk(); } );
    }

  public:

    bool_t decode( motor::graphics::command_status_cref_t cs,
        motor::graphics::command_status::status & ret ) const noexcept
    {
        return cs.get_status( _be->get_bid(), ret );
    }

    motor::graphics::command_status::status decode( motor::graphics::command_status_cref_t cs ) const noexcept
    {
        motor::graphics::command_status::status ret ;
        auto res = cs.get_status( _be->get_bid(), ret );
        return res ? ret : motor::graphics::command_status::status::invalid ;
    }
};
motor_typedef( frontend );
} // namespace gen4
} // namespace graphics
} // namespace motor