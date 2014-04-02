// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __MQTT_LOGIC_LAYER_DATA_H__
#define __MQTT_LOGIC_LAYER_DATA_H__

//#include "xi_static_vector.h"
#include "xi_event_dispatcher_api.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef int8_t ( xi_user_idle_t )( void* );

typedef struct
{
    // here we are going to store the mapping of the
    // handle functions versus the subscribed topics
    // so it's easy for the user to register his callback
    // for each of the subscribed topics

    // handle to the user idle function that suppose to
    //xi_user_idle_t*     user_idle_ptr;
    //xi_static_vector_t* handlers_for_topics;
    xi_evtd_handle_t    on_connected;
} xi_mqtt_logic_layer_data_t;

#ifdef __cplusplus
}
#endif

#endif // __MQTT_LOGIC_LAYER_DATA_H__
