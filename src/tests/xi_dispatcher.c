#include "xi_event_dispatcher_logic.h"

XI_DIS_EVENTS_BEGIN
XI_DIS_EVENTS_FIRST_EVENT(XI_EVENT_WANT_READ)
XI_DIS_EVENTS_EVENT(XI_EVENT_WANT_WRITE)
XI_DIS_EVENTS_EVENT(XI_EVENT_ERROR)
XI_DIS_EVENTS_LAST_EVENT
XI_DIS_EVENTS_END

XI_DIS_RET(void);
XI_DIS_HANDLE_1( void* );
XI_DIS_HANDLE_2( void* );
XI_DIS_HANDLE_3( void* );
XI_DIS_HANDLE_PTRS();

typedef uint8_t xi_dis_evt_desc_t;

#include "xi_event_dispatcher_api.h"

void continuation( void )
{

}

int main( )
{
    xi_dis_handle_t dis_handle = { XI_DIS_HANDLE_0_ID, { &continuation } };
    xi_dis_continue( dis_handle );

    return 0;
}