#ifndef __XI_HANDLE_TYPEDEF_H__
#define __XI_HANDLE_TYPEDEF_H__

#include "xi_event_dispatcher_macros.h"

// this flag will prevent xi types from being declared
#ifndef XI_DISPATCHER_CUSTOM_TYPES
#include "xi_layer_interface.h"

// forward declaration of the context structure
// originally declared in xively.h
struct layer_connectivity_s;

// the mqtt event dispatcher configuration
XI_EVTD_EVENTS_BEGIN()
XI_EVTD_EVENTS_3( XI_EVENT_WANT_READ, XI_EVENT_WANT_WRITE, XI_EVENT_ERROR )
XI_EVTD_EVENTS_END()

#define RET layer_state_t
#define xi_evtd_handle_1_t void*
#define xi_evtd_handle_2_t void*
#define xi_evtd_handle_3_t layer_state_t

XI_EVTD_HANDLE_PTRS()

typedef uint8_t xi_evtd_evt_desc_t;

#else

#include "xi_dispatcher_custom_types.i"

#endif

#endif // __XI_HANDLE_TYPEDEF_H__
