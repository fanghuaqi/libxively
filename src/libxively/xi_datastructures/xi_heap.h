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
#define XI_HEAP_KEY_TYPE int8_t
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
    xi_heap_element_t** elements;
    XI_HEAP_INDEX_TYPE  first_free;
    XI_HEAP_INDEX_TYPE  capacity;
} xi_heap_t;

// api
static inline xi_heap_t* xi_heap_create( XI_HEAP_INDEX_TYPE capacity )
{
    // PRECONDITIONS
    assert( capacity != 0 );

    size_t elements_size = sizeof( xi_heap_element_t* ) * capacity;

    xi_heap_t* ret = ( xi_heap_t* ) xi_alloc( sizeof( xi_heap_t ) );

    XI_CHECK_MEMORY( ret );

    ret->elements = ( xi_heap_element_t** ) xi_alloc( elements_size );

    XI_CHECK_MEMORY( ret->elements );

    memset( ret->elements, 0, elements_size );

    for( size_t i = 0; i < capacity; ++i )
    {
        ret->elements[ i ] = ( xi_heap_element_t* ) xi_alloc( sizeof( xi_heap_element_t ) );
        XI_CHECK_MEMORY( ret->elements );
        memset( ret->elements[ i ], 0, sizeof( xi_heap_element_t ) );
    }

    ret->capacity       = capacity;
    ret->first_free     = 0;

    // POSTCONDITIONS
    assert( ret->elements != 0 );
    assert( ret != 0 );

    return ret;

err_handling:
    if( ret->elements )
    {
        for( size_t i = 0; i < capacity; ++i )
        {
            XI_SAFE_FREE( ret->elements[ i ] );
        }
    }
    XI_SAFE_FREE( ret->elements );
    XI_SAFE_FREE( ret );
    return 0;
}

static inline void xi_heap_destroy( xi_heap_t* xi_heap )
{
    // PRECONDITIONS
    assert( xi_heap != 0 );
    assert( xi_heap->elements != 0 );
    assert( xi_heap->capacity != 0 );

    if( xi_heap->elements )
    {
        for( size_t i = 0; i < xi_heap->capacity; ++i )
        {
            XI_SAFE_FREE( xi_heap->elements[ i ] );
        }
    }

    XI_SAFE_FREE( xi_heap->elements );
    XI_SAFE_FREE( xi_heap );

    // POSTONDITIONS
    assert( xi_heap == 0 );
}

// inxex calculus
// note: the indexes are increased and decreased
// in order to maintain the 0 - based indexing of elements
#define LEFT( i )   ( (   ( i + 1 ) << 1 ) - 1 )
#define RIGHT( i )  ( ( ( ( i + 1 ) << 1 ) + 1 ) - 1 )
#define PARENT( i ) (   ( ( i + 1 ) >> 1 ) - 1 )

static inline void xi_heap_elements_swap( xi_heap_element_t** a, xi_heap_element_t** b )
{
    // PRECONDITIONS
    assert( a != 0 );
    assert( b != 0 );

    // swap elements
    xi_heap_element_t* tmp_elem = *a;
    *a = *b;
    *b = tmp_elem;

    // swap indexes
    XI_HEAP_INDEX_TYPE tmp_index = ( **a ).index;
    ( **a ).index = ( **b ).index;
    ( **b ).index = tmp_index;
}

static inline void xi_heap_fix_order_up( xi_heap_t* xi_heap, XI_HEAP_INDEX_TYPE index )
{
    // PRECONDITIONS
    assert( xi_heap != 0 );
    assert( xi_heap->elements != 0 );
    assert( xi_heap->capacity != 0 );

    while( index != 0 )
    {
        xi_heap_element_t** e = &xi_heap->elements[ index ];
        xi_heap_element_t** p = &xi_heap->elements[ PARENT( index ) ];

        if( ( *e )->key < ( *p )->key )
        {
            xi_heap_elements_swap( e, p );
        }

        index = PARENT( index );
    }
}

static inline void xi_heap_fix_order_down( xi_heap_t* xi_heap, XI_HEAP_INDEX_TYPE index )
{
    // PRECONDITIONS
    assert( xi_heap != 0 );
    assert( xi_heap->elements != 0 );
    assert( xi_heap->capacity != 0 );

    XI_HEAP_INDEX_TYPE li       = LEFT( index );
    XI_HEAP_INDEX_TYPE ri       = RIGHT( index );

    // guard the array bounds
    li = li >= xi_heap->first_free ? index : li;
    ri = ri >= xi_heap->first_free ? index : ri;

    do
    {
        xi_heap_element_t** e   = &xi_heap->elements[ index ];

        xi_heap_element_t** lc  = &xi_heap->elements[ li ];
        xi_heap_element_t** rc  = &xi_heap->elements[ ri ];

        if( ( *lc )->key < ( *rc )->key )
        {
            if( ( *lc )->key < ( *e )->key )
            {
                xi_heap_elements_swap( e, lc );
                index = li;
            }
            else { return; }
        }
        else
        {
            if( ( *rc )->key < ( *e )->key )
            {
                xi_heap_elements_swap( e, rc );
                index = ri;
            }
            else { return; }
        }

        // update the indexes
        li                      = LEFT( index );
        ri                      = RIGHT( index );

        // guard the array bounds
        li = li >= xi_heap->first_free ? index : li;
        ri = ri >= xi_heap->first_free ? index : ri;

    } while( index != li && index != ri );
}

static inline const xi_heap_element_t* xi_heap_element_add( xi_heap_t* xi_heap, XI_HEAP_KEY_TYPE key, void* value )
{
    // PRECONDITIONS
    assert( xi_heap != 0 );
    assert( xi_heap->elements != 0 );
    assert( xi_heap->capacity != 0 );

    // check the capacity
    if( xi_heap->first_free > xi_heap->capacity ) { return 0; }

    // derefence
    xi_heap_element_t* element  = xi_heap->elements[ xi_heap->first_free ];

    // add the element
    element->index              = xi_heap->first_free;
    element->key                = key;
    element->value              = value;

    // increase the next free counter
    xi_heap->first_free        += 1;

    // fix the order up
    xi_heap_fix_order_up( xi_heap, element->index );

    return element;
}

static inline const xi_heap_element_t* xi_heap_get_top( xi_heap_t* xi_heap )
{
    // PRECONDITIONS
    assert( xi_heap != 0 );
    assert( xi_heap->elements != 0 );
    assert( xi_heap->capacity != 0 );

    if( xi_heap->first_free == 0 )
    {
        return 0;
    }

    xi_heap_element_t** last_e  = 0;
    xi_heap_element_t** e       = &xi_heap->elements[ 0 ];
    XI_HEAP_INDEX_TYPE last_i   = xi_heap->first_free - 1;
    xi_heap->first_free        -= 1;

    if( last_i != 0 )
    {
        last_e = &xi_heap->elements[ last_i ];
        xi_heap_elements_swap( e, last_e );
        xi_heap_fix_order_down( xi_heap, 0 );
        return *last_e;
    }

    return *e;
}

static inline const xi_heap_element_t* xi_heap_peek_top( xi_heap_t* xi_heap )
{
    return xi_heap->elements[ 0 ];
}

#endif // __XI_HEAP_H__
