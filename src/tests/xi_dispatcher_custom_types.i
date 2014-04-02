XI_EVTD_EVENTS_BEGIN()
XI_EVTD_EVENTS_4( XI_EVENT_WANT_READ, XI_EVENT_WANT_WRITE, XI_EVENT_ERROR, XI_EVENT_CLOSE )
XI_EVTD_EVENTS_END()


#define RET void
#define xi_evtd_handle_1_t uint32_t*
#define xi_evtd_handle_2_t void*
#define xi_evtd_handle_3_t char*

XI_EVTD_HANDLE_PTRS();

typedef uint8_t xi_evtd_evt_desc_t;
