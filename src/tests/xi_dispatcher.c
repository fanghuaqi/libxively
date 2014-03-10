// Copyright (c) 2003-2013, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.
#include "tinytest.h"
#include "tinytest_macros.h"

#include "xi_heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "xi_event_dispatcher_logic.h"
#include "xi_heap.h"

XI_EVTD_EVENTS_BEGIN()
XI_EVTD_EVENTS_4( XI_EVENT_WANT_READ, XI_EVENT_WANT_WRITE, XI_EVENT_ERROR, XI_EVENT_CLOSE )
XI_EVTD_EVENTS_END()

XI_EVTD_RET( void );
XI_EVTD_HANDLE_1( int );
XI_EVTD_HANDLE_2( void* );
XI_EVTD_HANDLE_3( char* );
XI_EVTD_HANDLE_PTRS();

typedef uint8_t xi_evtd_evt_desc_t;

#include "xi_event_dispatcher_api.h"

void continuation( void )
{
    printf( "test\n" );
}

void continuation1( int a )
{
    printf( "test %d\n", a );
}

void continuation2( int a, void* b )
{
    printf( "test %d, %p\n", a, b );
}

void continuation3( int a, void* b, char* c )
{
    printf( "test %d, %p, %s\n", a, b, c );
}

///////////////////////////////////////////////////////////////////////////////
// DISPATCHER TESTS
///////////////////////////////////////////////////////////////////////////////



int main( )
{
    xi_evtd_instance_t* evtd_i  = xi_evtd_create_instance();

    //xi_evtd_continue( evtd_i, &evtd_handle );
    {
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_0_ID, { &continuation } };
        xi_evtd_execute_handle( &evtd_handle );
    }
    {
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_1_ID, .handlers.h1 = { &continuation1, 4 } };
        xi_evtd_execute_handle( &evtd_handle );
    }
    {
        void* pv = ( void* ) evtd_i;
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_2_ID, .handlers.h2 = { &continuation2, 16, pv } };
        xi_evtd_execute_handle( &evtd_handle );
    }
    {
        void* pv    = ( void* ) evtd_i;
        char pv1[]  = "test of the string\n";
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_3_ID, .handlers.h3 = { &continuation3, 32, pv, ( char* ) pv1 } };
        xi_evtd_execute_handle( &evtd_handle );
    }


    xi_evtd_destroy_instance( evtd_i );

    return 0;
}