#ifndef __XI_HANDLE_TYPEDEF_H__
#define __XI_HANDLE_TYPEDEF_H__

#include "xi_event_dispatcher_macros.h"

// this flag will prevent xi types from being declared
#ifndef XI_DISPATCHER_CUSTOM_TYPES

// the mqtt event dispatcher defininition
XI_EVTD_EVENTS_BEGIN()
XI_EVTD_EVENTS_3( XI_EVENT_WANT_READ, XI_EVENT_WANT_WRITE, XI_EVENT_ERROR )
XI_EVTD_EVENTS_END()

XI_EVTD_RET( void );
XI_EVTD_HANDLE_1( uint32_t* );
XI_EVTD_HANDLE_2( void* );
XI_EVTD_HANDLE_3( char* );
XI_EVTD_HANDLE_PTRS();

typedef uint8_t xi_evtd_evt_desc_t;

#else

#include "xi_dispatcher_custom_types.i"

#endif


#endif // __XI_HANDLE_TYPEDEF_H__
