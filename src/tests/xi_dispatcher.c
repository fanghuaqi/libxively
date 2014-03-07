#include "xi_event_dispatcher_logic.h"

XI_EVTD_EVENTS_BEGIN()
XI_EVTD_EVENTS_3( XI_EVENT_WANT_READ, XI_EVENT_WANT_WRITE, XI_EVENT_ERROR )
XI_EVTD_EVENTS_END()

XI_EVTD_RET( void );
XI_EVTD_HANDLE_1( void* );
XI_EVTD_HANDLE_2( void* );
XI_EVTD_HANDLE_3( void* );
XI_EVTD_HANDLE_PTRS();

typedef uint8_t xi_evtd_evt_desc_t;

#include "xi_event_dispatcher_api.h"

void continuation( void )
{

}

int main( )
{
    xi_evtd_handle_t dis_handle = { XI_EVTD_HANDLE_0_ID, { &continuation } };
    xi_evtd_continue( dis_handle );

    return 0;
}