// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_EVENT_DISPATCHER_LOGIC_H__
#define __XI_EVENT_DISPATCHER_LOGIC_H__

#include <stdint.h>

// |
// |------------- E()
// |
// |------------- CALL()
// |
// ED

#define XI_EVTD_EVENTS_BEGIN() typedef enum evts{ \
    XI_EVTD_COUNT = 0
#define XI_EVTD_EVENTS_END() }xi_event_type_t;

#define XI_EVTD_EVENTS_1( e1 ) \
    + 1 ,e1 = 1 << 0
#define XI_EVTD_EVENTS_2( e1, e2 ) \
    + 1 XI_EVTD_EVENTS_1( e1 ), e2 = 1 << 1
#define XI_EVTD_EVENTS_3( e1, e2, e3 ) \
    + 1 XI_EVTD_EVENTS_2( e1, e2 ), e3 = 1 << 2
#define XI_EVTD_EVENTS_4( e1, e2, e3, e4 ) \
    + 1 XI_EVTD_EVENTS_3( e1, e2, e3 ), e4 = 1 << 3
#define XI_EVTD_EVENTS_5( e1, e2, e3, e4, e5 ) \
    + 1 XI_EVTD_EVENTS_4( e1, e2, e3, e4 ), e5 = 1 << 4

#define XI_EVTD_RET( ret )       typedef ret RET;
#define XI_EVTD_HANDLE_1( T1 )   typedef T1 xi_evtd_handle_1_t;
#define XI_EVTD_HANDLE_2( T2 )   typedef T2 xi_evtd_handle_2_t;
#define XI_EVTD_HANDLE_3( T3 )   typedef T3 xi_evtd_handle_3_t;

#define XI_EVTD_HANDLE_PTRS() \
typedef RET (handle_0)(void); typedef handle_0* handle_0_ptr; \
typedef RET (handle_1)(xi_evtd_handle_1_t); typedef handle_1* handle_1_ptr; \
typedef RET (handle_2)(xi_evtd_handle_1_t, xi_evtd_handle_2_t); typedef handle_2* handle_2_ptr; \
typedef RET (handle_3)(xi_evtd_handle_1_t, xi_evtd_handle_2_t,xi_evtd_handle_3_t); typedef handle_3* handle_3_ptr;

typedef enum
{
    XI_EVTD_HANDLE_0_ID = 0,
    XI_EVTD_HANDLE_1_ID,
    XI_EVTD_HANDLE_2_ID,
    XI_EVTD_HANDLE_3_ID
} xi_evtd_handle_id_t;


#endif // __XI_EVENT_DISPATCHER_LOGIC_H__