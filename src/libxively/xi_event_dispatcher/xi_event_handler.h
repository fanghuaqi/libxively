#ifndef __XI_EVENT_HANDLER_H__
#define __XI_EVENT_HANDLER_H__

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

        struct
        {
            handle_4_ptr phandle_4;
            xi_evtd_handle_1_t a1;
            xi_evtd_handle_2_t a2;
            xi_evtd_handle_3_t a3;
            xi_evtd_handle_4_t a4;
        }h4;
    }handlers;
} xi_evtd_handle_t;

#define MAKE_HANDLE_H0( f ) \
    xi_evtd_handle_t handle = { \
          XI_EVTD_HANDLE_0_ID \
       , .handlers.h0 = { \
            f } };

#define MAKE_HANDLE_H1( f, a0 ) \
    xi_evtd_handle_t handle = { \
          XI_EVTD_HANDLE_1_ID \
       , .handlers.h1 = { \
            f, a0 } };

#define MAKE_HANDLE_H2( f, a0, a1 ) \
    xi_evtd_handle_t handle = { \
          XI_EVTD_HANDLE_2_ID \
       , .handlers.h2 = { \
            f, a0, a1 } };

#define MAKE_HANDLE_H3( f, a0, a1, a2 ) \
    xi_evtd_handle_t handle = { \
          XI_EVTD_HANDLE_3_ID \
       , .handlers.h3 = { \
            f, a0, a1, a2 } };

#define MAKE_HANDLE_H4( f, a0, a1, a2, a3 ) \
    xi_evtd_handle_t handle = { \
          XI_EVTD_HANDLE_4_ID \
       , .handlers.h4 = { \
            f, a0, a1, a2, a3 } };

#endif // __XI_EVENT_HANDLER_H__
