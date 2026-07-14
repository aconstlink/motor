#pragma once

#include "../result.h"
#include "../typedefs.h"

#include <functional>

namespace motor
{
namespace graphics
{
class command_status
{
    motor_this_typedefs( command_status );

  public:

    enum class status
    {
        invalid,
        raw,
        in_transit,
        configured,
        released
    };

  private:

    struct data
    {
        size_t bid = size_t( -1 );
        this_t::status status;
        motor::graphics::result res;
    };
    motor_typedef( data );

    // per backend status.
    motor::vector< data_t > _datas;

  public:

    command_status( void_t ) noexcept {}

    command_status( this_cref_t ) = delete;
    command_status( this_rref_t rhv ) noexcept : _datas( std::move( rhv._datas ) ) {}

    ~command_status( void_t ) noexcept {}

  public: // status

    bool_t set_in_transit( size_t const bid ) noexcept
    {
        return this_t::set( bid, motor::graphics::command_status::status::in_transit,
            [ = ]( motor::graphics::command_status::status const s )
        { return s != motor::graphics::command_status::status::in_transit; } );
    }

    // only set s if cond is true.
    // @return true if status replaced, otherwise false
    bool_t set( size_t const bid, this_t::status const s,
        std::function< bool_t( this_t::status const ) > cond )
    {
        auto & d = this_t::ensure_entry( bid );

        if( cond( d.status ) )
        {
            d.status = s;
            return true;
        }
        return false;
    }

    void_t set( size_t const bid, this_t::status const s ) noexcept
    {
        auto & d = this_t::ensure_entry( bid );
        d.status = s;
    }

    bool_t get_status( size_t const bid, this_t::status & ret ) const noexcept
    {
        size_t i = size_t( -1 );
        while( ++i < _datas.size() && _datas[ i ].bid != bid )
            continue;

        if( i < _datas.size() )
        {
            ret = _datas[ i ].status;
            return true;
        }
        return false;
    }

  public: // graphics result

    void_t set( size_t const bid, motor::graphics::result const r ) noexcept
    {
        auto & d = this_t::ensure_entry( bid ) ;
        d.res = r ; 
    }

    bool_t get_result( size_t const bid, motor::graphics::result & ret ) const noexcept
    {
        size_t i = size_t( -1 );
        while( ++i < _datas.size() && _datas[ i ].bid != bid )
            continue;

        if( i < _datas.size() )
        {
            ret = _datas[ i ].res;
            return true;
        }
        return false;
    }

  private:

    data_ref_t ensure_entry( size_t const bid ) noexcept
    {
        size_t i = size_t( -1 );
        while( ++i < _datas.size() && _datas[ i ].bid != bid )
            continue;

        if( i >= _datas.size() )
        {
            _datas.emplace_back(
                this_t::data_t{ bid, this_t::status::raw, motor::graphics::result::invalid } );
        }

        return _datas[ i ];
    }
};
motor_typedef( command_status );
} // namespace graphics
} // namespace motor