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
