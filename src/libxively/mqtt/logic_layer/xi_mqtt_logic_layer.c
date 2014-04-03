// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#include "xi_coroutine.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_event_dispatcher_global_instance.h"
#include "xively.h"

#include "message.h"
#include "serialiser.h"
#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void fill_with_connect_data( mqtt_message_t* msg )
{
    memset( msg, 0, sizeof( mqtt_message_t ) );

    msg->common.common_u.common_bits.retain     = MQTT_RETAIN_FALSE;
    msg->common.common_u.common_bits.qos        = MQTT_QOS_AT_MOST_ONCE;
    msg->common.common_u.common_bits.dup        = MQTT_DUP_FALSE;
    msg->common.common_u.common_bits.type       = MQTT_TYPE_CONNECT;
    msg->common.remaining_length                = 0; // ?????????

    memcpy( msg->connect.protocol_name.data, "MQIsdp", 6 );
    msg->connect.protocol_name.length                    = 6;
    msg->connect.protocol_version                        = 3;

    msg->connect.flags_u.flags_bits.username_follows     = 0;
    msg->connect.flags_u.flags_bits.password_follows     = 0;
    msg->connect.flags_u.flags_bits.will_retain          = 0;
    msg->connect.flags_u.flags_bits.will_qos             = 0;
    msg->connect.flags_u.flags_bits.will                 = 0;
    msg->connect.flags_u.flags_bits.clean_session        = 0;

    msg->connect.keep_alive                              = 0;

    {
        const char client_id[] = "xi_test_client";
        msg->connect.client_id.length = sizeof( client_id ) - 1;
        memcpy( msg->connect.client_id.data, client_id, sizeof( client_id ) - 1 );
    }
}

layer_state_t xi_mqtt_logic_layer_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    // sending request
    // type of the request depends on
    // the state that we are in
    // it's easy to determine if we are in the middle of
    // processing a single request or we are starting new one
    //

    static mqtt_message_t mqtt_message;
    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    xi_mqtt_logic_in_t* dr = ( xi_mqtt_logic_in_t* ) data;

    BEGIN_CORO( layer_data->data_ready_cs );

    if( dr->scenario_t == XI_MQTT_CONNECT )
    {
        fill_with_connect_data( &mqtt_message );
        YIELD( layer_data->data_ready_cs
            , CALL_ON_PREV_ON_DATA_READY(
                  context
                , &mqtt_message
                , LAYER_STATE_OK ) );
    }
    else if( dr->scenario_t == XI_MQTT_PUBLISH )
    {

    }
    else if( dr->scenario_t == XI_MQTT_SUBSCRIBE )
    {

    }

    // ERROR UNKNOWN SCENARIO
    END_CORO();

    return LAYER_STATE_OK;
}

layer_state_t xi_mqtt_logic_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    // receiving message
    // will go through the state machine
    // so that it will decide what to do next
    // this is very important part of the
    //
    return LAYER_STATE_OK;
}

layer_state_t xi_mqtt_logic_layer_init(
      void* context
    , void* data
    , layer_state_t in_state )
{
    return CALL_ON_PREV_INIT( context, data, in_state );
}

layer_state_t xi_mqtt_logic_layer_connect(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    xi_mqtt_logic_layer_data_t* layer_data = CON_SELF( context )->user_data;
    xi_evtd_continue( xi_evtd_instance, layer_data->on_connected, 0 );

    return LAYER_STATE_OK;
}

layer_state_t xi_mqtt_logic_layer_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}

layer_state_t xi_mqtt_logic_layer_on_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}


#ifdef __cplusplus
}
#endif
