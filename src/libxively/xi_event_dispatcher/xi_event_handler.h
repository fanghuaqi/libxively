#ifndef __XI_EVENT_HANDLER_H__
#define __XI_EVENT_HANDLER_H__

#include "xi_static_vector.h"
#include "xi_handle_typedef.h"

typedef struct xi_evtd_handle_s
{
    xi_evtd_handle_id_t handle_type;
    union
    {
        struct
        {
            handle_0_ptr phandle_0;
        }h0;

        struct
        {
            handle_1_ptr phandle_1;
            xi_evtd_handle_1_t a1;
        }h1;

        struct
        {
            handle_2_ptr phandle_2;
            xi_evtd_handle_1_t a1;
            xi_evtd_handle_2_t a2;
        }h2;

        struct
        {
            handle_3_ptr phandle_3;
            xi_evtd_handle_1_t a1;
            xi_evtd_handle_2_t a2;
            xi_evtd_handle_3_t a3;
        }h3;
    }handlers;
} xi_evtd_handle_t;

#endif // __XI_EVENT_HANDLER_H__