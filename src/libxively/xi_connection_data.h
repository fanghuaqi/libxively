// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_CONNECTION_DATA_H__
#define __XI_CONNECTION_DATA_H__

#include "xi_event_dispatcher_api.h"

typedef struct
{
    char*               host;
    int                 port;
    char*               username;
    char*               password;
    xi_evtd_handle_t    on_connected;
} xi_connection_data_t;

extern xi_connection_data_t* xi_alloc_connection_data(
      const char* host
    , int port
    , const char* username
    , const char* password );

extern void xi_free_connection_data(
    xi_connection_data_t* data );

#endif // __XI_CONNECTION_DATA_H__
