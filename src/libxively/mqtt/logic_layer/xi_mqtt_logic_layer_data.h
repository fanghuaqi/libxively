// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __MQTT_LOGIC_LAYER_DATA_H__
#define __MQTT_LOGIC_LAYER_DATA_H__

//#include "xi_static_vector.h"
#include "xi_event_dispatcher_api.h"
#include "message.h"
#include "xi_connection_data.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef int8_t ( xi_user_idle_t )( void* );

typedef struct xi_mqtt_logic_task_data_s
{
    struct xi_mqtt_logic_in_s
    {
        enum
        {
              XI_MQTT_NONE = 0
            , XI_MQTT_CONNECT
            , XI_MQTT_PUBLISH
            , XI_MQTT_SUBSCRIBE
            , XI_MQTT_KEEPALIVE
        } scenario_t;

        enum
        {
              XI_MQTT_QOS_ZERO = 0
            , XI_MQTT_QOS_ONE
            , XI_MQTT_QOS_TWO
        } qos_t;
    } mqtt_settings;

    union data_t
    {
        struct data_t_publish_t
        {
            char* topic;
            char* msg;
        } publish;

        struct data_t_subscribe_t
        {
            char*               topic;
            xi_evtd_handle_t    handler;
        } subscribe;
    } *data_u;
} xi_mqtt_logic_task_data_t;

typedef struct xi_mqtt_logic_task_s
{
    xi_mqtt_logic_task_data_t   data;
    uint16_t                    cs;
    struct xi_heap_element_s*   task_timeout;
} xi_mqtt_logic_task_t;

#define PUSH_BACK( type, list, elem ) { \
    type* prev = list; \
    type* curr = prev; \
    while( curr ) { \
       prev = curr; \
       curr = curr->__next; \
    } \
    if( prev == curr ) { \
        list = elem; \
    } else { \
        prev->__next = elem; \
    } \
}

#define POP( type, list, out ) { \
    out = list;\
    if( out->__next == 0 ) { \
        list = 0; \
    } else { \
        list = out->__next; \
        out->__next = 0; \
    } \
}

typedef struct xi_mqtt_logic_queue_s
{
    layer_connectivity_t*           context;
    xi_mqtt_logic_task_t*           task;
    layer_state_t                   state;
    uint16_t                        message_id;
    xi_evtd_handle_t                logic;
    struct xi_mqtt_logic_queue_s* __next;
} xi_mqtt_logic_queue_t;


typedef enum {
    XI_MLLS_NONE = 0,
    XI_MLLS_CONNECTING,
    XI_MLLS_CONNECTED,
    XI_MLLS_SHUTTING_DOWN
} xi_mqtt_logic_layer_state_t;

typedef struct
{
    // here we are going to store the mapping of the
    // handle functions versus the subscribed topics
    // so it's easy for the user to register his callback
    // for each of the subscribed topics

    // handle to the user idle function that suppose to
    //xi_user_idle_t*     user_idle_ptr;
    xi_mqtt_logic_queue_t*      msg_demux;
    xi_mqtt_logic_queue_t*      tasks_queue;
    xi_static_vector_t*         handlers_for_topics;
    xi_connection_data_t*       conn_data;
    xi_mqtt_logic_task_t*       curr_task;
    struct xi_heap_element_s*   keep_alive_event;
    struct xi_heap_element_s*   keep_alive_timeout;
    uint16_t                    data_ready_cs;
    xi_mqtt_logic_layer_state_t the_state;
} xi_mqtt_logic_layer_data_t;

#ifdef __cplusplus
}
#endif

#endif // __MQTT_LOGIC_LAYER_DATA_H__
