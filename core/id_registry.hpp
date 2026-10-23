#pragma once

#include <motor/base/types.hpp>

#include <vector>
#include <mutex>
#include <iostream>

namespace motor
{
namespace core
{
using namespace motor::core::types;

template < uint64_t offset, uint64_t max_ids = uint64_t( -1 ), bool assert_on_match = true >
class id_registry
{
    motor_this_typedefs( id_registry );

  private:

    struct type_info
    {
        char const * type_name = nullptr;
    };

    std::mutex _mtx;
    std::vector< type_info > _infos;

    // this is basically the index into _infos
    uint64_t _id = uint64_t( -1 );

  public:

    uint64_t register_type( char const * const name ) noexcept
    {
        std::lock_guard< std::mutex > lk( _mtx );

        // #1 check if name was already registered.
        {
            uint64_t i = uint64_t( -1 );
            while( ++i < ( _id + 1 ) && std::strcmp( name, _infos[ i ].type_name ) != 0 );
            if( i != ( _id + 1 ) )
            {
                if constexpr( assert_on_match )
                {
                    std::cerr << "registring multiple time is considered an error." << std::endl;
                    assert( false );
                    std::terminate();
                }

                return i + offset;
            }
        }

        if( _infos.size() <= ++_id )
        {
            _infos.resize( _infos.size() + 50 );
        }

        if( _id >= max_ids )
        {
            std::cerr << "the maximum is set" << std::endl;
            assert( false );
            std::terminate();
        }

        _infos[ _id ] = this_t::type_info{ name };
        return _id + offset;
    }
};
} // namespace core
} // namespace motor