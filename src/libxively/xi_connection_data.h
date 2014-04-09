// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_CONNECTION_DATA_H__
#define __XI_CONNECTION_DATA_H__

#include "xi_event_dispatcher_api.h"

typedef struct
{
    const char*         address;
    int                 port;
    xi_evtd_handle_t    on_connected;
} xi_connection_data_t;

#endif // __XI_CONNECTION_DATA_H__
