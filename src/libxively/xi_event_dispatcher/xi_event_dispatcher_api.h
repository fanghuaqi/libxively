// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_EVENT_DISPATCHER_API_H__
#define __XI_EVENT_DISPATCHER_API_H__

#include <stdint.h>
#include <stdio.h>

#include "xi_allocator.h"
#include "xi_macros.h"
#include "xi_event_handler.h"
#include "xi_heap_typedefs.h"
#include "xi_static_vector.h"

//
typedef int xi_fd_t;
struct xi_heap_s;
struct xi_heap_element_s;

typedef struct
{
    xi_fd_t             fd;
    xi_evtd_handle_t    handle;
    xi_evtd_handle_t    read_handle;
    xi_event_type_t     event_type;
} xi_evtd_triplet_t;

typedef struct
{
    xi_heap_key_type_t      current_step;
    struct xi_heap_s*       call_heap;
    xi_static_vector_t*     handles_and_fd;
    uint8_t                 stop;
} xi_evtd_instance_t;

extern int8_t xi_evtd_register_fd(
      xi_evtd_instance_t* instance
    , xi_fd_t fd
    , xi_evtd_handle_t read_handle );

extern int8_t xi_evtd_unregister_fd(
      xi_evtd_instance_t* instance
    , xi_fd_t fd );

extern int8_t xi_evtd_continue_when_evt(
      xi_evtd_instance_t* instance
    , xi_event_type_t event_type
    , xi_evtd_handle_t handle
    , xi_fd_t fd );

extern struct xi_heap_element_s* xi_evtd_continue(
      xi_evtd_instance_t* instance
    , xi_evtd_handle_t handle
    , xi_heap_key_type_t time_diff );

extern struct xi_heap_element_s* xi_evtd_cancel(
      xi_evtd_instance_t* instance
    , struct xi_heap_element_s* heap_element );

extern void xi_evtd_restart(
      xi_evtd_instance_t* instance
    , struct xi_heap_element_s* heap_element
    , xi_heap_key_type_t new_time );

extern xi_evtd_instance_t* xi_evtd_create_instance();

extern void xi_evtd_destroy_instance(
    xi_evtd_instance_t* instance );

extern void xi_evtd_execute_handle(
    xi_evtd_handle_t* handle );

extern void xi_evtd_step(
      xi_evtd_instance_t* evtd_instance
    , xi_heap_key_type_t new_step );

extern uint8_t xi_evtd_dispatcher_continue(
    xi_evtd_instance_t* instance );

/**
 * \brief update events triggers registration of continuations assigned to the given event on given device
 * \note events_mask is the mask that's created using | operator
 */
extern void xi_evtd_update_event(
      xi_evtd_instance_t* instance
    , xi_fd_t fds );

extern void xi_evtd_stop( xi_evtd_instance_t* instance );

#endif // __XI_EVENT_DISPATCHER_API_H__
