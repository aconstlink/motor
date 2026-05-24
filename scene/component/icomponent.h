
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../protos.h"

#include "../ivisitable.hpp"

#include <motor/wire/slot/sheet.hpp>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API icomponent
{
    motor_this_typedefs( icomponent );

  private:

    motor::vector< motor::wire::iinput_slot_mtr_t > _inputs;
    motor::vector< motor::wire::ioutput_slot_mtr_t > _outputs;

  public:

    icomponent( void_t ) noexcept {}
    icomponent( this_rref_t rhv ) noexcept : _inputs( std::move( rhv._inputs ) ), _outputs( std::move( rhv._outputs ) )
    {
    }

    virtual ~icomponent( void_t ) noexcept
    {
        for( auto * in : _inputs )
        {
            in->disconnect();
            motor::release( motor::move( in ) );
        }

        for( auto * out : _outputs )
        {
            out->disconnect();
            motor::release( motor::move( out ) );
        }
    }

  public: // scene node interface

    // if a component is attached to a node, this function is called.
    // @param nptr a borrowed pointer to the node. The node will exist as
    // long as this component is attached, so it is safe to store the pointer.
    virtual void_t attached_to( motor::scene::node_ptr_t nptr ) noexcept {}

    // the component is detached from the node.
    virtual void_t detach( void_t ) noexcept {}

    // if a scene nodes moves, all attached componentes will be moved to,
    // so those will be given a new owner. this function allows to recognize
    // the new node owner.
    virtual void_t moved_to( motor::scene::node_ptr_t nptr ) noexcept {}

  public: // slot interface for external

    virtual bool_t inputs( motor::wire::inputs_out_t ) noexcept
    {
        return false;
    }
    virtual bool_t outputs( motor::wire::outputs_out_t ) noexcept
    {
        return false;
    }

  protected: // slot interface for derived

    template < typename T >
    motor::wire::input_slot< T > * create_input_slot( char const * purpose ) noexcept
    {
        return this_t::create_input_slot< T >( T(), purpose );
    }

    template < typename T >
    motor::wire::input_slot< T > * create_input_slot( void_t ) noexcept
    {
        return this_t::create_input_slot< T >( T(), "[icomponent] : create_input_slot" );
    }

    template < typename T >
    motor::wire::input_slot< T > * create_input_slot( T const & initial, char const * purpose = "" ) noexcept
    {
        // here we create a managed pointer
        auto ret = motor::shared( motor::wire::input_slot< T >( initial ), purpose );
        // here we simply copy the managed pointer unsafe
        _inputs.emplace_back( ret );
        // we know, a managed copy is present, so just return a borrowed pointer
        return motor::move( ret );
    }

    template < typename T >
    motor::wire::output_slot< T > * create_output_slot( void_t ) noexcept
    {
        return this_t::create_output_slot< T >( T(), "[icomponent] : create_output_slot" );
    }

    template < typename T >
    motor::wire::output_slot< T > * create_output_slot( char const * purpose ) noexcept
    {
        return this_t::create_output_slot< T >( T(), purpose );
    }

    template < typename T >
    motor::wire::output_slot< T > * create_output_slot( T const & initial, char const * purpose = "" ) noexcept
    {
        auto ret = motor::shared( motor::wire::output_slot< T >( initial ), purpose );
        _outputs.emplace_back( ret );
        return motor::move( ret );
    }

  public:

    // this is a temporary class allowing a visitor
    // to sync the slots to the domain data in
    // a derived class.
    class visitor_sync_accessor
    {
      public:

        static void_t sync_inputs( motor::scene::icomponent_ptr_t cptr ) noexcept
        {
            cptr->sync_inputs();
        }

        static void_t sync_outputs( motor::scene::icomponent_ptr_t cptr ) noexcept
        {
            cptr->sync_outputs();
        }
    };
    friend class visitor_sync_accessor;

  protected:

    virtual void_t sync_inputs( void_t ) noexcept {}
    virtual void_t sync_outputs( void_t ) noexcept {}
};
} // namespace scene
} // namespace motor