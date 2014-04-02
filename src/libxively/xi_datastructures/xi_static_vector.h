#ifndef __XI_STATIC_VECTOR_H__
#define __XI_STATIC_VECTOR_H__

#include <stdint.h>

// local
#include "xi_debug.h"
#include "xi_allocator.h"
#include "xi_macros.h"

typedef int8_t xi_static_vector_index_type_t;

typedef struct
{
    void*                         value;
} xi_static_vector_elem_t;

typedef struct
{
    xi_static_vector_elem_t*        array;
    xi_static_vector_index_type_t   elem_no;
    xi_static_vector_index_type_t   capacity;
} xi_static_vector_t;

// declaration of the comparition function type
// suppose to return -1 if e0 < e1, 0 if e0 == e1 and 1 if e0 > e1
typedef int8_t ( xi_static_vector_cmp_t )( void* e0, void* e1 );

extern xi_static_vector_t* xi_static_vector_create(
    xi_static_vector_index_type_t capacity );

extern xi_static_vector_t* xi_static_vector_destroy(
    xi_static_vector_t* vector );

extern const xi_static_vector_elem_t* xi_static_vector_push(
    xi_static_vector_t* vector, void* value );

extern void xi_static_vector_swap_elems(
      xi_static_vector_t* vector
    , xi_static_vector_index_type_t i0
    , xi_static_vector_index_type_t i1 );

extern void xi_static_vector_del(
      xi_static_vector_t* vector
    , xi_static_vector_index_type_t index );

extern xi_static_vector_index_type_t xi_static_vector_find(
      xi_static_vector_t* vector
    , void* value
    , xi_static_vector_cmp_t* fun_cmp );

#endif // __XI_STATIC_VECTOR_H__
