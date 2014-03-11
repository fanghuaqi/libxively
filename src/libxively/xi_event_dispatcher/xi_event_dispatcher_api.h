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
    xi_evtd_handle_t*   handle;
    void*               io_data;
} xi_evtd_io_pair_t;

typedef struct
{
    XI_HEAP_KEY_TYPE    current_step;
    xi_heap_t*          call_heap;
    xi_evtd_io_pair_t   events_queue[ XI_EVTD_COUNT ][ 1 ];
    uint8_t             events_queue_sizes[ XI_EVTD_COUNT ];
} xi_evtd_instance_t;

static inline void xi_evtd_continue_when_evt(
      xi_evtd_instance_t* instance
    , xi_event_type_t event_type
    , xi_evtd_handle_t* handle
    , void* io_data )
{
    //events_queue[ event_type ]
}

static inline void xi_evtd_continue(
      xi_evtd_instance_t* instance
    , xi_evtd_handle_t* handle )
{
    xi_heap_element_add(
          instance->call_heap
        , instance->current_step + 1
        , handle );
}

static inline xi_evtd_instance_t* xi_evtd_create_instance()
{
    xi_evtd_instance_t* evtd_instance = ( xi_evtd_instance_t* ) xi_alloc( sizeof( xi_evtd_instance_t ) );

    XI_CHECK_MEMORY( evtd_instance );

    memset( evtd_instance, 0, sizeof( xi_evtd_instance_t ) );

    evtd_instance->call_heap = xi_heap_create( 16 );

    XI_CHECK_MEMORY( evtd_instance->call_heap );

    return evtd_instance;

err_handling:
    XI_SAFE_FREE( evtd_instance );
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
static inline void xi_evtd_update_events(
      xi_evtd_instance_t* evtd_instance
    , uint32_t events_mask )
{

}

static inline void xi_evtd_execute_handle( xi_evtd_handle_t* handle )
{
    switch( handle->handle_type )
    {
        case XI_EVTD_HANDLE_0_ID:
            ( *handle->handlers.h0.phandle_0 )();
        break;
        case XI_EVTD_HANDLE_1_ID:
            ( *handle->handlers.h1.phandle_1 )( handle->handlers.h1.a1 );
        break;
        case XI_EVTD_HANDLE_2_ID:
            ( *handle->handlers.h2.phandle_2 )( handle->handlers.h2.a1, handle->handlers.h2.a2 );
        break;
        case XI_EVTD_HANDLE_3_ID:
            ( *handle->handlers.h3.phandle_3 )( handle->handlers.h3.a1, handle->handlers.h3.a2, handle->handlers.h3.a3 );
        break;
    }
}

static inline void xi_evtd_step( xi_evtd_instance_t* evtd_instance )
{
    evtd_instance->current_step  += 1;
    const xi_heap_element_t* tmp = 0;

    while( !xi_heap_is_empty( evtd_instance->call_heap ) )
    {
        tmp = xi_heap_peek_top( evtd_instance->call_heap );
        if( tmp->key <= evtd_instance->current_step )
        {
            tmp = xi_heap_get_top( evtd_instance->call_heap );
            xi_evtd_handle_t* handle = ( xi_evtd_handle_t* ) tmp->value;
            xi_evtd_execute_handle( handle );
        }
        else
        {
            break;
        }
    }
}

#endif // __XI_EVENT_DISPATCHER_API_H__
