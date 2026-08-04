#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/frontend/command_status.hpp>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/io/monitor.hpp>
#include <motor/io/database.h>

#include <motor/concurrent/mrsw.hpp>
#include <motor/std/hash_map>
#include <motor/std/vector>

#include <thread>

namespace motor
{
namespace gfx
{
class MOTOR_GFX_API msl_manager
{
    motor_this_typedefs( msl_manager );

  private:

    struct shared_data
    {
        shared_data( void_t ) noexcept {}
        shared_data( shared_data const & ) = delete;
        shared_data( shared_data && rhv ) noexcept
            : owner( motor::move( rhv.owner ) ), running( rhv.running ), has_work( rhv.has_work )
        {
        }

        ~shared_data( void_t ) noexcept {}

        msl_manager * owner;
        bool_t running;

        bool_t has_work;
        std::mutex mtx;
        std::condition_variable cv;
    };
    motor_typedef( shared_data );

    std::thread _th;
    shared_data_ptr_t _sd = nullptr;

  private:

    struct msl_data
    {
        motor::graphics::msl_object_mtr_t msl = nullptr;
        motor::io::monitor_mtr_t mon = nullptr;
    };

    motor::io::database_mtr_t _db = nullptr;

    motor::concurrent::mrsw_t _mutex;

    motor::hash_map< motor::string_t, size_t > _location_to_id;
    motor::hash_map< motor::string_t, size_t > _name_to_id;

    motor::vector< msl_data > _msls;

    motor::vector< size_t > _msls_config;
    motor::vector< size_t > _msls_release;

    motor::vector< size_t > _configures_in_process;

    // running load operations
    struct load_operation
    {
        motor::io::location loc;
        motor::string_t name;
        motor::io::database::cache_access_t ca;
    };
    motor::vector< load_operation > _loads;

  public:

    msl_manager( motor::io::database_mtr_safe_t db ) noexcept;
    msl_manager( this_cref_t ) = delete;
    msl_manager( this_rref_t ) noexcept;
    ~msl_manager( void_t ) noexcept;

  public:

    bool_t add( motor::string_cref_t name, motor::io::location_cref_t loc ) noexcept;

    void_t on_update( void_t ) noexcept;

    using on_configure_funk_t = std::function< void_t( motor::string_in_t,
        motor::graphics::msl_object_mtr_t ) >;
    void_t for_each_configure_done( on_configure_funk_t ) noexcept;

  public: // graphics interface

    void_t on_render_init( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    void_t on_render_release( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    void_t on_render( motor::graphics::gen4::frontend_ptr_t ) noexcept;

    // this needs to be called if all windows are rendered, so
    // nothing is missed.
    void_t on_frame_done( void_t ) noexcept;

  private:

    void_t start_thread( void_t ) noexcept;
    void_t stop_thread( void_t ) noexcept;
};
motor_typedef( msl_manager );
} // namespace gfx
} // namespace motor