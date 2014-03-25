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
#define XI_HEAP_KEY_TYPE uint32_t
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

// inxex calculus
// note: the indexes are increased and decreased
// in order to maintain the 0 - based indexing of elements
#define LEFT( i )   ( (   ( i + 1 ) << 1 ) - 1 )
#define RIGHT( i )  ( ( ( ( i + 1 ) << 1 ) + 1 ) - 1 )
#define PARENT( i ) (   ( ( i + 1 ) >> 1 ) - 1 )

// api
extern xi_heap_t* xi_heap_create( XI_HEAP_INDEX_TYPE capacity );

extern void xi_heap_destroy( xi_heap_t* xi_heap );

extern void xi_heap_elements_swap( xi_heap_element_t** a, xi_heap_element_t** b );

extern void xi_heap_fix_order_up( xi_heap_t* xi_heap, XI_HEAP_INDEX_TYPE index );

extern void xi_heap_fix_order_down( xi_heap_t* xi_heap, XI_HEAP_INDEX_TYPE index );

extern const xi_heap_element_t* xi_heap_element_add( xi_heap_t* xi_heap, XI_HEAP_KEY_TYPE key, void* value );

extern const xi_heap_element_t* xi_heap_get_top( xi_heap_t* xi_heap );

extern const xi_heap_element_t* xi_heap_peek_top( xi_heap_t* xi_heap );

extern uint8_t xi_heap_is_empty( xi_heap_t* xi_heap );

#endif // __XI_HEAP_H__
