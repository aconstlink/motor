
#pragma once

#include "api.h"
#include "protos.h"
#include "typedefs.h"
#include "result.h"

#include <motor/concurrent/mrsw.hpp>
#include <motor/std/vector>

namespace motor
{
namespace graphics
{

enum class process_state
{
    invalid,
    idle,
    in_transit
};

// state changes:
// raw -> in_transit -> ready -> in_transit -> raw
enum class object_state
{
    invalid,
    raw,
    in_transit,
    ready
};

class MOTOR_GRAPHICS_API object
{
    motor_this_typedefs( object );

  private:

    // backends can store an id for their own.
    typedef struct
    {
        size_t bid;
        size_t oid;

        // this is a very convenience flag but not used
        // much in the engine. This enables lazy initialization
        // which might hurt user experience.
        bool_t changed;

        // can be used in the backend to signal that the
        // object is being used in multiple threads.
        // use for configure and release only at the moment.
        bool_t is_in_transit;

        // only purpose is to know if it is
        // configured or not.
        motor::graphics::object_state os;

        // this is the backend result of the
        // last configuration/release operation
        motor::graphics::result res;
    } backend_data_t;

    mutable motor::concurrent::mrsw_t _mutex;
    motor::vector< backend_data_t > _datas;
    bool_t _managed = false;

  public: // manager data

    class MOTOR_GRAPHICS_API manager_interface
    {

        object * _ptr;

      public:

        manager_interface( object * ptr ) noexcept : _ptr( ptr ) {}
        manager_interface( manager_interface const & ) = delete;
        manager_interface( manager_interface && ) = delete;

        void_t set_managed( bool_t const b ) noexcept
        {
            _ptr->_managed = b;
        }
    };
    friend class manager_interface;

    bool_t is_managed( void_t ) const noexcept
    {
        return _managed;
    }

  public:

    class MOTOR_GRAPHICS_API data_manipulator
    {
      public:

        using state_pair_t = std::pair< motor::graphics::object_state, motor::graphics::result >;

      private:

        size_t _bid;
        object * _obj;

      public:

        data_manipulator( object * ptr, size_t const bid ) noexcept;
        data_manipulator( data_manipulator const & ) = delete;
        data_manipulator( data_manipulator && ) = delete;
        ~data_manipulator( void_t ) noexcept;

      public:

        // object state
        bool_t change_to_in_transit( void_t ) noexcept;

        bool_t change_if( motor::graphics::object_state const os,
            std::function< bool_t( motor::graphics::object_state const ) > funk ) noexcept;

        bool_t change( motor::graphics::result const res ) noexcept;

        // only change the internal result if the last one was not ok.
        // if the last one was ok but the current is not ok, the object
        // still is considered ok.
        bool_t change_to_ready( motor::graphics::result const res ) noexcept;

        bool_t change_to_raw( motor::graphics::result const res ) noexcept;

      public: //

        state_pair_t get_status( void_t ) const noexcept;

      public: // in transit flag

        bool_t set_in_transit( void_t ) noexcept;
        bool_t reset_in_transit( void_t ) noexcept;
    };
    friend class data_manipulator;

    bool_t is_ready( size_t const bid ) const noexcept;

    // are all backend objects ready.
    // ready means successfully configured.
    bool_t is_ready( void_t ) const noexcept;

    // is any backend object in transit?
    bool_t is_any_in_transit( void_t ) const noexcept;

  public:

    object( void_t ) noexcept;
    object( bool_t const managed ) noexcept;
    object( this_cref_t rhv ) noexcept;
    object( this_rref_t rhv ) noexcept;
    virtual ~object( void_t ) noexcept;

    this_ref_t operator=( this_cref_t rhv ) noexcept;
    this_ref_t operator=( this_rref_t rhv ) noexcept;

  public:

    size_t set_oid( size_t const bid, size_t const oid ) noexcept;

    size_t get_oid( size_t const bid ) const noexcept;

    void_t remove_bid( size_t const bid ) noexcept;

  public: // change interface

    void_t set_changed( void_t ) noexcept;

    bool_t has_changed( size_t const bid ) const noexcept;

    void_t reset_changed( size_t const bid ) noexcept;

    bool_t check_and_reset_changed( size_t const bid ) noexcept;
};
motor_typedef( object );

} // namespace graphics
} // namespace motor
