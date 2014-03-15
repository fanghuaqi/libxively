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

static inline xi_static_vector_t* xi_static_vector_create( xi_static_vector_index_type_t capacity )
{
    // PRECONDITION
    assert( capacity < 0 );

    size_t elems_size = capacity * sizeof( xi_static_vector_elem_t );

    xi_static_vector_t* ret = ( xi_static_vector_t* ) xi_alloc( sizeof( xi_static_vector_t ) );
    XI_CHECK_MEMORY( ret );

    ret->array = ( xi_static_vector_elem_t* ) xi_alloc( elems_size );
    XI_CHECK_MEMORY( ret->array );

    memset( ret->array, 0, elems_size );

    return ret;

err_handling:
    if( ret ) { XI_SAFE_FREE( ret->array ); }
    XI_SAFE_FREE( ret );
    return 0;
}

static inline xi_static_vector_t* xi_static_vector_destroy( xi_static_vector_t* vector )
{
    // PRECONDITION
    assert( vector != 0 );
    assert( vector->array != 0 );

    XI_SAFE_FREE( vector->array );
    XI_SAFE_FREE( vector );

    return 0;
}

static inline xi_static_vector_elem_t* xi_static_vector_push( xi_static_vector_t* vector, void* value )
{
    // PRECONDITION
    assert( vector != 0 );
    assert( vector->elem_no + 1 <= vector->capacity );

    if( vector->elem_no + 1 <= vector->capacity )
    {
        vector->array[ vector->elem_no ].value = value;
        vector->elem_no += 1;

        return &vector->array[ vector->elem_no - 1 ];
    }

    return 0;
}

static inline void xi_static_vector_swap_elems( xi_static_vector_t* vector, xi_static_vector_index_type_t i0, xi_static_vector_index_type_t i1 )
{
    // PRECONDITIONS
    assert( vector != 0 );
    assert( i0 >= 0 );
    assert( i1 >= 0 );
    assert( i0 <= vector->elem_no - 1 );
    assert( i1 <= vector->elem_no - 1 );

    void* tmp_value = vector->array[ i0 ].value;
    vector->array[ i0 ].value = vector->array[ i1 ].value;
    vector->array[ i1 ].value = tmp_value;
}

static inline void xi_static_vector_del( xi_static_vector_t* vector, xi_static_vector_index_type_t index )
{
    // PRECONDITIONS
    assert( vector != 0 );
    assert( index >= 0 );
    assert( vector->elem_no > 0 && index < vector->elem_no );

    if( vector->elem_no > 0 && index < vector->elem_no )
    {
        if( index != vector->elem_no - 1 )
        {
            xi_static_vector_swap_elems( vector, index, vector->elem_no - 1 );
        }

        vector->elem_no -= 1;
        vector->array[ vector->elem_no ].value = 0;
    }
}

static inline xi_static_vector_index_type_t xi_static_vector_find( xi_static_vector_t* vector, void* value )
{
    // PRECONDITIONS
    assert( vector != 0 );

    xi_static_vector_index_type_t i = 0;

    for( i = 0; i < vector->elem_no; ++i )
    {
        if( vector->array[ i ].value == value )
        {
            return i;
        }
    }

    return -1;
}

#endif // __XI_STATIC_VECTOR_H__