

#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/wire/node/node.h>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API icontroller
{
    motor_this_typedefs( icontroller );

  public:
    
    virtual ~icontroller( void_t ) noexcept {}
  
};
motor_typedef( icontroller );

// holds a single controller entity, i.e. a wire node
class MOTOR_SCENE_API single_controller : public icontroller
{
    motor_this_typedefs( single_controller );

  private:

    motor::wire::inode_mtr_t _node = nullptr ;

  public:

    single_controller( void_t ) noexcept {}
    single_controller( this_cref_t ) = delete;
    single_controller( this_rref_t rhv ) noexcept : _node( motor::move( rhv._node ) ) {}
    virtual ~single_controller( void_t ) noexcept
    {
        motor::release( motor::move( _node ) );
    }

    void_t set_node( motor::wire::inode_mtr_safe_t mtr ) noexcept
    {
        motor::release( motor::move( _node ) ) ;
        _node = motor::move( mtr ) ;
    }

    motor::wire::inode_mtr_safe_t get_node( void_t ) noexcept
    {
        return motor::share( _node );
    }

    motor::wire::inode_ptr_t borrow_node( void_t ) noexcept
    {
        return _node;
    }
};
motor_typedef( single_controller );

// can hold multiple controllers. not tested yet.
class MOTOR_SCENE_API multi_controller : public icontroller
{
    motor_this_typedefs( multi_controller );

  private:
  #if 0
    motor::vector< motor::wire::node_ptr_t > _nodes;

  public:

    multi_controller( void_t ) noexcept {}
    multi_controller( this_cref_t ) = delete;
    multi_controller( this_rref_t rhv ) noexcept : _nodes( std::move( rhv._nodes ) ) {}
    virtual ~multi_controller( void_t ) noexcept
    {
        for( auto * ptr : _nodes )
            motor::release( motor::move( ptr ) );
    }

  public:

    void_t attach( motor::wire::node_mtr_safe_t ctrl ) noexcept
    {
        _nodes.emplace_back( motor::move( ctrl ) );
    }

    using for_each_funk_t = std::function< void_t( motor::wire::node_ptr_t ) >;

    void_t for_each( for_each_funk_t f ) noexcept
    {
        for( auto * ptr : _nodes )
        {
            f( ptr );
        }
    }
    #endif
};
motor_typedef( multi_controller );

} // namespace scene
} // namespace motor
