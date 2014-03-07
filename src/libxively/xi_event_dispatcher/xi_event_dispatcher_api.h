// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_EVENT_DISPATCHER_API_H__
#define __XI_EVENT_DISPATCHER_API_H__

#include <stdint.h>

#include "xi_allocator.h"
#include "xi_macros.h"
#include "xi_heap.h"

typedef struct
{
    xi_evtd_handle_id_t handle_type;
    union
    {
        struct
        {
            handle_0_ptr phandle_0;
        }h0;

        struct
        {
            handle_1_ptr phandle_1;
            xi_evtd_handle_1_t a1;
        }h1;

        struct
        {
            handle_2_ptr phandle_2;
            xi_evtd_handle_1_t a1;
            xi_evtd_handle_2_t a2;
        }h2;

        struct
        {
            handle_3_ptr phandle_3;
            xi_evtd_handle_1_t a1;
            xi_evtd_handle_2_t a2;
            xi_evtd_handle_3_t a3;
        }h3;
    }handlers;
} xi_evtd_handle_t;

typedef struct
{
    XI_HEAP_KEY_TYPE    current_step;
    xi_heap_t*          call_heap;

} xi_evtd_instance_t;

static inline void xi_evtd_continue_when_evt( xi_event_type_t event_type, xi_evtd_handle_t handle )
{

}

static inline void xi_evtd_continue( xi_evtd_handle_t handle )
{

}

static inline xi_evtd_instance_t* xi_evtd_create_instance()
{
    xi_evtd_instance_t* dis_instance = ( xi_evtd_instance_t* ) xi_alloc( sizeof( xi_evtd_instance_t ) );

    XI_CHECK_MEMORY( dis_instance );

    memset( dis_instance, 0, sizeof( xi_evtd_instance_t ) );

    dis_instance->call_heap = xi_heap_create( 16 );

    XI_CHECK_MEMORY( dis_instance->call_heap );

err_handling:
    XI_SAFE_FREE( dis_instance );
    return 0;
}

static inline void xi_evtd_destroy_instance( xi_evtd_instance_t* instance )
{
    xi_heap_destroy( instance->call_heap );
    XI_SAFE_FREE( instance );
}

/**
 * \brief update events triggers registration of continuations assigned to the given event on given device
 * \note events_mask is the mask that's created using | operator
 */
static inline void xi_evtd_update_events( xi_evtd_instance_t* dis_instance, uint32_t events_mask )
{

}

static inline void xi_evtd_step( xi_evtd_instance_t* dis_instance )
{
    dis_instance->current_step  += 1;
    const xi_heap_element_t* tmp = 0;

    while( !xi_heap_is_empty( dis_instance->call_heap ) )
    {
        tmp = xi_heap_peek_top( dis_instance->call_heap );
        if( tmp->key <= dis_instance->current_step )
        {

        }
        else
        {
            break;
        }
    }
}

#endif // __XI_EVENT_DISPATCHER_API_H__