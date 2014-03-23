#ifndef __XI_MQTT_EVENT_DISPATCHER_H__
#define __XI_MQTT_EVENT_DISPATCHER_H__

#include "xi_event_dispatcher_logic.h"
#include "xi_heap.h"

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


// pure mqtt api


#endif // __XI_MQTT_EVENT_DISPATCHER_H__