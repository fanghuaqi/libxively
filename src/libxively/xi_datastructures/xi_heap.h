// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_HEAP_H__
#define __XI_HEAP_H__

// C
#include <stdint.h>

// local
#include "xi_debug.h"
#include "xi_allocator.h"
#include "xi_macros.h"

//
#ifndef XI_HEAP_KEY_TYPE
#define XI_HEAP_KEY_TYPE uint8_t
#endif // XI_HEAP_KEY_TYPE

//
#ifndef XI_HEAP_INDEX_TYPE
#define XI_HEAP_INDEX_TYPE uint8_t
#endif // XI_HEAP_INDEX_TYPE

//
typedef struct
{
    void*               value;
    XI_HEAP_KEY_TYPE    key;
    XI_HEAP_INDEX_TYPE  index;
} xi_heap_element_t;

//
typedef struct
{
    xi_heap_element_t*  elements;
    XI_HEAP_INDEX_TYPE  first_free;
    XI_HEAP_INDEX_TYPE  capacity;
} xi_heap_t;

// api
inline xi_heap_t* xi_heap_create( XI_HEAP_INDEX_TYPE capacity )
{
    // PRECONDITIONS
    assert( capacity != 0 );

    size_t elements_size = sizeof( xi_heap_element_t ) * capacity;

    xi_heap_t* ret = ( xi_heap_t* ) xi_alloc( sizeof( xi_heap_t ) );

    XI_CHECK_MEMORY( ret );

    ret->elements = ( xi_heap_element_t* ) xi_alloc( elements_size );

    XI_CHECK_MEMORY( ret->elements );

    memset( ret->elements, 0, elements_size );
    ret->capacity       = capacity;
    ret->first_free     = 0;

    // POSTCONDITIONS
    assert( ret->elements != 0 );
    assert( ret != 0 );

    return ret;

err_handling:
    XI_SAFE_FREE( ret->elements );
    XI_SAFE_FREE( ret );
    return 0;
}

inline void xi_heap_destroy( xi_heap_t* xi_heap )
{
    // PRECONDITIONS
    assert( xi_heap != 0 );
    assert( xi_heap->elements != 0 );
    assert( xi_heap->capacity != 0 );

    XI_SAFE_FREE( xi_heap->elements );
    XI_SAFE_FREE( xi_heap );

    // POSTONDITIONS
    assert( xi_heap == 0 );
}

// the the indexes are calculated
// note: the indexes are being increased and decreased
// in order to maintain the 0 - based indexing of elements
#define LEFT( i )   ( (   ( i + 1 ) >> 1 ) - 1 )
#define RIGHT( i )  ( ( ( ( i + 1 ) >> 1 ) + 1 ) - 1 )
#define PARENT( i ) (   ( ( i + 1 ) << 1 ) - 1 )

//
inline void xi_fix_order_up( xi_heap_t* xi_heap, XI_HEAP_INDEX_TYPE index )
{
    while( index != 0 )
    {
        xi_heap_element_t* e = &xi_heap->elements[ index ];
        xi_heap_element_t* p = &xi_heap->elements[ PARENT( index ) ];

        if( e->key > p->key )
        {

        }
    }
}

inline uint8_t xi_heap_element_add( xi_heap_t* xi_heap, XI_HEAP_KEY_TYPE key, void* value )
{
    // PRECONDITIONS
    assert( xi_heap != 0 );
    assert( xi_heap->elements != 0 );
    assert( xi_heap->capacity != 0 );

    // check the capacity
    if( xi_heap->first_free > xi_heap->capacity ) { return 0; }

    // derefence
    xi_heap_element_t* element  = &xi_heap->elements[ xi_heap->first_free ];

    // add the element
    element->index              = xi_heap->first_free;
    element->key                = key;
    element->value              = value;

    // increase the next free counter
    xi_heap->first_free        += 1;

    // fix the order up
    xi_fix_order_up( xi_heap, element->index );

    return 1;
}

inline const xi_heap_element_t* xi_heap_get_top( xi_heap_t* xi_heap )
{
    return 0;
}

inline const xi_heap_element_t* xi_heap_peek_top( xi_heap_t* xi_heap )
{
    return 0;
}

#endif // __XI_HEAP_H__