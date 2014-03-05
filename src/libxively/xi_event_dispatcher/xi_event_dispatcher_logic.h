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

#define XI_DIS_EVENTS_BEGIN typedef enum evts{
#define XI_DIS_EVENTS_FIRST_EVENT(x) x = 0
#define XI_DIS_EVENTS_EVENT(x) ,x
#define XI_DIS_EVENTS_LAST_EVENT ,XI_DIS_EVENTS_COUNT
#define XI_DIS_EVENTS_END }xi_event_type_t;

/*#define XI_DIS_EVENT_1( e1 ) e1 = 1
#define XI_DIS_EVENT_2( e2, )
#devine XI_DIS_EVENT_5( e5,  )
#define XI_DIS_EVENT_6( e6, XI_DIS_EVENT_5( e5, XI_DIS_EVENT_4( e4, XI_DIS_EVENT_3( e3, XI_DIS_EVENT_2( e2, XI_DIS_EVENT_1( e1 ) ) ) ) ) )*/

#define XI_DIS_RET( ret )       typedef ret RET;
#define XI_DIS_HANDLE_1( T1 )   typedef T1 xi_dis_handle_1_t;
#define XI_DIS_HANDLE_2( T2 )   typedef T2 xi_dis_handle_2_t;
#define XI_DIS_HANDLE_3( T3 )   typedef T3 xi_dis_handle_3_t;

#define XI_DIS_HANDLE_PTRS() \
typedef RET (handle_0)(void); typedef handle_0* handle_0_ptr; \
typedef RET (handle_1)(xi_dis_handle_1_t); typedef handle_1* handle_1_ptr; \
typedef RET (handle_2)(xi_dis_handle_1_t, xi_dis_handle_2_t); typedef handle_2* handle_2_ptr; \
typedef RET (handle_3)(xi_dis_handle_1_t, xi_dis_handle_2_t,xi_dis_handle_3_t); typedef handle_3* handle_3_ptr;

typedef enum
{
    XI_DIS_HANDLE_0_ID = 0,
    XI_DIS_HANDLE_1_ID,
    XI_DIS_HANDLE_2_ID,
    XI_DIS_HANDLE_3_ID
} xi_dis_handle_id_t;

#endif // __XI_EVENT_DISPATCHER_LOGIC_H__