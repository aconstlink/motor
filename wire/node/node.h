
#pragma once

#include "slot_sheet_policies.hpp"

#include "../slot/input_slot.h"
#include "../slot/output_slot.h"
#include "../slot/sheet.hpp"

#include <motor/concurrent/task/task.hpp>
#include <motor/concurrent/mrsw.hpp>

#include <motor/std/string>

namespace motor
{
namespace wire
{
class MOTOR_WIRE_API inode
{
    motor_this_typedefs( inode );

  public:

    motor_typedefs( motor::vector< this_mtr_t >, nodes );

  private: // task and nodes

    motor::concurrent::task_mtr_t _task;

    motor::concurrent::mrsw_t _mtx_in;
    motor::concurrent::mrsw_t _mtx_out;
    nodes_t _incoming;
    nodes_t _outgoing;

    motor::string_t _name;

#if 0
  private: // slots

    motor::wire::inputs_t _inputs;
    motor::wire::outputs_t _outputs;
#endif

  public:

    virtual ~inode( void_t ) noexcept;

  protected:

    inode( motor::string_in_t, motor::concurrent::task_mtr_t t ) noexcept;
    inode( this_rref_t ) noexcept;
    inode( this_cref_t ) = delete;
    inode( motor::concurrent::task_mtr_t t ) noexcept;

    motor::concurrent::task_mtr_t borrow_task( void_t ) noexcept
    {
        return _task;
    }

  public:

    this_mtr_t then( this_mtr_safe_t ) noexcept;

    motor::concurrent::task_mtr_safe_t get_task( void_t ) noexcept;

    void_t disconnect( void_t ) noexcept;

    motor::string_cref_t name( void_t ) const noexcept;

  public:

    virtual void_t execute( void_t ) noexcept = 0;

  private:

    // other connects before this
    // other -> this
    bool_t add_outgoing( this_ptr_t other ) noexcept;

    void_t remove_outgoing( this_ptr_t other ) noexcept;

    // other connects after this
    // this -> other
    void_t add_incoming( this_ptr_t other ) noexcept;

    void_t remove_incoming( this_ptr_t other ) noexcept;

    motor::concurrent::task_ptr_t task( void_t ) noexcept;

  public:

    struct cleaner_accessor
    {
        static void_t move_out_all( this_mtr_t t, motor::vector< this_mtr_t > & nodes ) noexcept
        {
            t->move_out_all( nodes );
        }
    };
    friend struct cleaner_accessor;

    struct tier_builder
    {
        struct tier
        {
            nodes_t nodes;

            bool_t has_nodes( void_t ) const noexcept
            {
                return nodes.size() != 0;
            }
        };
        motor_typedefs( motor::vector< tier >, tiers );
        tiers_t tiers;

        struct build_result
        {
            tiers_t tiers;
            size_t num_nodes = 0;
            bool_t has_cylce = false;
        };
        motor_typedef( build_result );

        static void_t build( this_mtr_t start, build_result_out_t res ) noexcept
        {
            size_t id = 0;
            motor::hash_map< this_mtr_t, size_t > ids;

            size_t cur_tier = 0;

            tiers_ref_t tiers = res.tiers;
            tiers.clear();
            res.num_nodes = 1;

            tiers.push_back( tier{ { start } } );
            ids[ start ] = id++;

            while( tiers[ cur_tier ].has_nodes() )
            {
                tiers.resize( tiers.size() + 1 );

                for( auto * n : tiers[ cur_tier ].nodes )
                {
                    for( auto * n2 : n->_outgoing )
                    {
                        // self check
                        {
                            auto iter = ids.find( n2 );
                            if( iter != ids.end() ) continue;
                        }

                        // check if candidate for tier
                        {
                            bool_t complete = true;
                            for( auto * incoming_node : n2->_incoming )
                            {
                                auto iter = ids.find( incoming_node );
                                if( iter == ids.end() )
                                {
                                    complete = false;
                                    break;
                                }
                            }

                            if( !complete ) continue;
                        }

                        // if candidate, check cycles
                        {
                            bool_t has_cycle = false;
                            for( auto * outgoing_node : n2->_outgoing )
                            {
                                auto iter = ids.find( outgoing_node );
                                if( iter != ids.end() )
                                {
                                    has_cycle = true;
                                    break;
                                }
                            }

                            if( has_cycle )
                            {
                                // motor::log::global_t::status("graph has cycle.") ;
                                res.has_cylce = true;
                                continue;
                            }
                        }

                        tiers[ cur_tier + 1 ].nodes.push_back( n2 );
                        ids[ n2 ] = id++;
                        ++res.num_nodes;
                    }
                }

                ++cur_tier;
            }
        }

        using output_slots_funk_t = std::function< void_t( this_mtr_t, nodes_cref_t outs ) >;
        static void_t output_slot_walk( build_result_in_t res, output_slots_funk_t f ) noexcept
        {
            for( auto const & tiEr : res.tiers )
            {
                for( auto * n : tiEr.nodes )
                {
                    f( n, n->_outgoing );
                }
            }
        }
    };
    motor_typedef( tier_builder );

    friend struct tier_builder;

  private:

    void_t move_out_all( motor::vector< this_mtr_t > & nodes ) noexcept
    {
        nodes.reserve( nodes.capacity() + _incoming.size() + _outgoing.size() );
        for( auto & t : _incoming )
            nodes.emplace_back( t );
        for( auto & t : _outgoing )
            nodes.emplace_back( t );
        _outgoing.clear();
        _incoming.clear();
    }
};
motor_typedef( inode );

//*********************************************************************
template < typename slot_policy >
class node : public inode, private slot_policy
{
    using base_t = inode;
    motor_this_typedefs( node< slot_policy > );

  public:

    using slot_policy_t = slot_policy ;

  public:

    node( void_t ) noexcept
        : base_t( motor::shared( motor::concurrent::task_t( this_t::make_task_funk() ),
                                 "wire node task" ) )
    {
    }
    node( motor::string_cref_t name ) noexcept
        : base_t( name, motor::shared( motor::concurrent::task_t( this_t::make_task_funk() ),
                                       "wire node task" ) )
    {
    }
    node( this_cref_t ) = delete;
    node( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) ), slot_policy( std::move( rhv ) )
    {
        this_t::borrow_task()->set_funk( this_t::make_task_funk() );
    }
    virtual ~node( void_t ) noexcept {}

    typename slot_policy::inputs_ref_t inputs( void_t ) noexcept
    {
        return slot_policy::inputs();
    }

    typename slot_policy::outputs_ref_t outputs( void_t ) noexcept
    {
        return slot_policy::outputs();
    }

  private:

    virtual motor::concurrent::task_t::task_funk_t make_task_funk( void_t ) noexcept
    {
        return [ = ]( motor::concurrent::task_t::task_funk_param_in_t )
        {
            // exchange all inputs... (pull)
            // this->inputs().exchange() ;
            this->execute();
            // or exchange all outputs (push)
            this->outputs().exchange();
        };
    }
};

motor_typedefs( node< motor::wire::named_slot_sheet_policy >, named_slots_node );
motor_typedefs( node< motor::wire::unnamed_slot_sheet_policy >, unnamed_slots_node );

//*********************************************************************
template < typename slot_policy >
class funk_node : public node< slot_policy >
{
    using base_t = node< slot_policy >;
    motor_this_typedefs( funk_node< slot_policy > );

  public:

    using funk_t = std::function< void_t( this_ptr_t ) >;

  private:

    funk_t _funk;

  public:

    funk_node( funk_t f ) noexcept : _funk( f ) {}
    funk_node( motor::string_in_t n, funk_t f ) noexcept : base_t( n ), _funk( f ) {}
    funk_node( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _funk( std::move( rhv._funk ) )
    {
    }
    virtual ~funk_node( void_t ) noexcept {}

    void_t set_funk( funk_t f ) noexcept
    {
        _funk = f;
    }

  public:

    virtual void_t execute( void_t ) noexcept
    {
        _funk( this );
    }
};
motor_typedefs( funk_node< named_slot_sheet_policy >, funk_node_named );
motor_typedefs( funk_node< unnamed_slot_sheet_policy >, funk_node_unnamed );
motor_typedefs( funk_node< unnamed_slot_sheet_policy >, funk_node );

} // namespace wire
} // namespace motor