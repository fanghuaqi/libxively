#include "xi_event_dispatcher_logic.h"
#include "xi_heap.h"

XI_EVTD_EVENTS_BEGIN()
XI_EVTD_EVENTS_4( XI_EVENT_WANT_READ, XI_EVENT_WANT_WRITE, XI_EVENT_ERROR, XI_EVENT_CLOSE )
XI_EVTD_EVENTS_END()

XI_EVTD_RET( void );
XI_EVTD_HANDLE_1( int );
XI_EVTD_HANDLE_2( void* );
XI_EVTD_HANDLE_3( void* );
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

int main( )
{
    xi_evtd_instance_t* evtd_i  = xi_evtd_create_instance();

    //xi_evtd_continue( evtd_i, &evtd_handle );
    {
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_0_ID, { &continuation } };
        xi_evtd_execute_handle( &evtd_handle );
    }
    {
        xi_evtd_handle_t evtd_handle = { XI_EVTD_HANDLE_1_ID, .handlers.h1 = { &continuation1, 256 } };
        xi_evtd_execute_handle( &evtd_handle );
    }

    return 0;
}