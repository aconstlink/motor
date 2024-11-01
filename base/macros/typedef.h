
#pragma once

#include "../mtr_ownership.hpp"

#define motor_comma , 

#define motor_typedefs_extend( from_, to_ )                                \
    typedef to_ ## _t * to_ ## _ptr_t ;                                 \
    typedef to_ ## _t * to_ ## _mtr_t ;                                 \
    typedef to_ ## _t * to_ ## _vtr_t ;                                 \
    typedef to_ ## _t & to_ ## _ref_t ;                                 \
    typedef to_ ## _t && to_ ## _rref_t ;                               \
    typedef to_ ## _t const * to_ ## _cptr_t ;                          \
    typedef to_ ## _t * const to_ ## _ptrc_t ;                          \
    typedef to_ ## _t const * const to_ ## _cptrc_t ;                   \
    typedef to_ ## _t const * to_ ## _cvtr_t ;                          \
    typedef to_ ## _t const & to_ ## _cref_t ;                          \
    typedef to_ ## _cref_t to_ ## _in_t ;                               \
    typedef to_ ## _ref_t to_ ## _out_t ;                               \
    typedef to_ ## _ref_t to_ ## _inout_t ;                             \
    typedef to_ ## _t * & to_ ## _ptr_ref_t ;                           \
    typedef to_ ## _t * && to_ ## _ptr_rref_t ;                         \
    typedef to_ ## _t * & to_ ## _mtr_ref_t ;                           \
    typedef to_ ## _t * && to_ ## _mtr_rref_t ;                         \
    typedef motor::core::mtr_safe< to_ ## _t > to_ ## _mtr_safe_t  ;    \
    typedef motor::core::mtr_safe< to_ ## _t > to_ ## _safe_t ;         \
    typedef motor::core::mtr_borrow< to_ ## _t > to_ ## _borrow_t 
    


#define motor_typedef_extend( what_ ) motor_typedefs_extend( what_, what_ )

#define motor_typedefs( from_, to_ )                                       \
    typedef from_ to_ ## _t ;                                           \
    motor_typedefs_extend( from_, to_ )

#define motor_typedef( what_ ) motor_typedefs( what_, what_ )

#define motor_this_typedefs(x)                                             \
    typedef x this_t ;                                                  \
    typedef this_t & this_ref_t ;                                       \
    typedef this_t const & this_cref_t ;                                \
    typedef this_t * this_ptr_t ;                                       \
    typedef this_t * this_mtr_t ;                                       \
    typedef this_t * & this_mtr_ref_t ;                                       \
    typedef this_t * && this_mtr_rref_t ;                                       \
    typedef this_t const * this_cptr_t ;                                \
    typedef this_t && this_rref_t ;                                     \
    typedef this_t & this_inout_t ;                                     \
    typedef motor::core::mtr_safe< this_t > this_mtr_safe_t ;           \
    typedef motor::core::mtr_safe< this_t > this_safe_t ;               \
    typedef motor::core::mtr_borrow< this_t > this_borrow_t 

#define motor_class_proto_typedefs(x)                                      \
    class x ;                                                           \
    typedef x * x ## _ptr_t ;                                           \
    typedef x * x ## _mtr_t ;                                           \
    typedef x * x ## _vtr_t ;                                           \
    typedef x const * x ## _cptr_t ;                                    \
    typedef x * const x ## _ptrc_t ;                                    \
    typedef x const * const x ## _cptrc_t ;                             \
    typedef x && x ## _rref_t ;                                         \
    typedef x & x ## _inout_t ;                                         \
    typedef x & x ## _ref_t ;                                           \
    typedef x * & x ## _mtr_ref_t ;                                           \
    typedef x * && x ## _mtr_rref_t ;                                     \
    typedef motor::core::mtr_safe< x > x ## _mtr_safe_t ;     \
    typedef motor::core::mtr_safe< x > x ## _safe_t ;         \
    typedef motor::core::mtr_borrow< x > x ## _borrow_t 

#define motor_struct_proto_typedefs(x)                                     \
    struct x ;                                                          \
    typedef x * x ## _ptr_t ;                                           \
    typedef x * x ## _vtr_t ;                                           \
    typedef x const * x ## _cptr_t ;                                    \
    typedef x * const x ## _ptrc_t ;                                    \
    typedef x const * const x ## _cptrc_t ;                             \
    typedef x const * x ## _cvtr_t                                         
