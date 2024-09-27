
#pragma once

#include "protos.h"
#include "result.h"
#include "visitor/ivisitor.h"

#include <motor/core/double_dispatch.hpp>

#include <functional>

namespace motor
{
    namespace scene
    {
        struct double_dispatch_callbacks
        {
            using funk_visit_t = std::function< motor::scene::result ( ivisitor_ptr_t, ivisitable_ptr_t ) > ;
            using funk_post_visit_t = std::function< motor::scene::result ( ivisitor_ptr_t, ivisitable_ptr_t, motor::scene::result const ) > ;

            funk_visit_t visit = [=] ( ivisitor_ptr_t a, ivisitable_ptr_t b ) { return a->visit( b ) ; }  ;
            funk_post_visit_t post_visit = [=] ( ivisitor_ptr_t a, ivisitable_ptr_t b, motor::scene::result const r ) { return a->post_visit( b, r ) ; } ;

            template< class caller_t, class callee_t, bool_t with_post >
            static double_dispatch_callbacks make_default_( void_t ) noexcept ;

        private: // detail - choosing the right functions

            template< typename A, typename B, bool with_post >
            struct make_default_tpl {};

            template< typename A, typename B >
            struct make_default_tpl< A, B, true >
            {
                static double_dispatch_callbacks make_default( void_t ) noexcept
                {
                    return {
                        [=] ( ivisitor_ptr_t a, ivisitable_ptr_t b ) { return dynamic_cast<A *>( a )->visit( dynamic_cast<B *>( b ) ) ; },
                        [=] ( ivisitor_ptr_t a, ivisitable_ptr_t b, motor::scene::result const r ) 
                            { return dynamic_cast<A *>( a )->post_visit( dynamic_cast<B *>( b ), r ) ; }
                    } ;
                }
            };

            template< typename A, typename B >
            struct make_default_tpl< A, B, false >
            {
                static double_dispatch_callbacks make_default( void_t ) noexcept
                {
                    return {
                        [=] ( ivisitor_ptr_t a, ivisitable_ptr_t b ) { return dynamic_cast<A *>( a )->visit( dynamic_cast<B *>( b ) ) ; },
                        [=] ( ivisitor_ptr_t a, ivisitable_ptr_t b, motor::scene::result const r ) { return motor::scene::result::not_implemented ; }
                    } ;
                }
            };

        public:

            template< class caller_t, class callee_t >
            static double_dispatch_callbacks make_default( void_t ) noexcept 
            {
                constexpr bool_t b1 = std::is_base_of< motor::scene::group, callee_t >::value ;
                constexpr bool_t b2 = std::is_base_of< motor::scene::decorator, callee_t >::value ;

                return make_default_tpl< caller_t, callee_t, b1 || b2 >::make_default() ;
            }
        };
        using double_dispatcher_t_ = motor::core::double_dispatcher< motor::scene::ivisitor, motor::scene::ivisitable, double_dispatch_callbacks > ;
        motor_typedefs( double_dispatcher_t_, double_dispatcher ) ;
    }
}