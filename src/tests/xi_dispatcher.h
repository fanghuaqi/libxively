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
XI_EVTD_HANDLE_1( uint32_t* );
XI_EVTD_HANDLE_2( void* );
XI_EVTD_HANDLE_3( char* );
XI_EVTD_HANDLE_PTRS();

typedef uint8_t xi_evtd_evt_desc_t;

#include "xi_event_dispatcher_api.h"

static uint32_t g_cont0_test = 0;

void continuation( void )
{
    g_cont0_test = 127;
}

void continuation1( uint32_t* a )
{
    *a = 127;
}

void continuation2( int a, void* b )
{
    printf( "test %d, %p\n", a, b );
}

void continuation3( int a, void* b, char* c )
{
    printf( "test %d, %p, %s\n", a, b, c );
}

static xi_evtd_instance_t* evtd_g_i = 0;
static xi_evtd_handle_t evtd_handle_g;

void proc_loop( uint32_t* a )
{
    *a -= 1;

    if( *a > 0 )
    {
        xi_evtd_continue( evtd_g_i, &evtd_handle_g, 1 );
    }
}

///////////////////////////////////////////////////////////////////////////////
// DISPATCHER TESTS
///////////////////////////////////////////////////////////////////////////////
void test_continuation0( void* data )
{
    (void) data;

    xi_evtd_instance_t* evtd_i  = xi_evtd_create_instance();

    {
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_0_ID, .handlers.h0 = { &continuation } };
        xi_evtd_execute_handle( &evtd_handle );
    }

    tt_assert( g_cont0_test == 127 );

end:
    xi_evtd_destroy_instance( evtd_i );
}

void test_continuation1( void* data )
{
    (void) data;

    xi_evtd_instance_t* evtd_i  = xi_evtd_create_instance();

    uint32_t counter = 0;

    {
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_1_ID, .handlers.h1 = { &continuation1, &counter } };
        xi_evtd_execute_handle( &evtd_handle );
    }

    tt_assert( counter == 127 );

end:
    xi_evtd_destroy_instance( evtd_i );
}

void test_handler_processing_loop( void* data )
{
    (void) data;

    evtd_g_i  = xi_evtd_create_instance();

    uint32_t counter        = 10;
    XI_HEAP_KEY_TYPE step   = 0;


    evtd_handle_g.handle_type           = XI_EVTD_HANDLE_1_ID;
    evtd_handle_g.handlers.h1.phandle_1 = &proc_loop;
    evtd_handle_g.handlers.h1.a1        = &counter;

    xi_evtd_continue( evtd_g_i, &evtd_handle_g, 0 );

    while( evtd_g_i->call_heap->first_free > 0 )
    {
        xi_evtd_step( evtd_g_i, step );
        step += 1;
        tt_assert( counter == 10 - step );
    }

    tt_assert( counter == 0 );
    tt_assert( step == 10 );

end:
    xi_evtd_destroy_instance( evtd_g_i );
}


struct testcase_t dispatcher_tests[] = {
    /* Here's a really simple test: it has a name you can refer to it
       with, and a function to invoke it. */
    { "test_dispatcher_continuation0", test_continuation0, TT_ENABLED_, 0, 0 },
    { "test_dispatcher_continuation1", test_continuation1, TT_ENABLED_, 0, 0 },
    { "test_dispatcher_processing_loop", test_handler_processing_loop, TT_ENABLED_, 0, 0 },
    /* The array has to end with END_OF_TESTCASES. */
    END_OF_TESTCASES
};
