// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#include "xi_coroutine.h"
#include "xi_mqtt_layer.h"
#include "xively.h"

#include "message.h"
#include "serialiser.h"
#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

layer_state_t xi_mqtt_layer_data_ready(
      void* context
    , void* data
    , layer_state_t state )
{
    XI_UNUSED( state );

    mqtt_message_t* msg = ( mqtt_message_t* ) data;

    mqtt_serialiser_t serializer;
    mqtt_serialiser_init( & serializer );

    // @TODO change to use dynamic allocation
    static uint8_t buffer[ 256 ];
    static data_descriptor_t data_descriptor = { ( char* ) buffer, 0, 0, 0 };

    xi_mqtt_layer_data_t* layer_data
        = ( xi_mqtt_layer_data_t* ) CON_SELF( context )->user_data;

    BEGIN_CORO( layer_data->cs );

    int len = mqtt_serialiser_size( &serializer, msg );

    data_descriptor.data_size = len;
    data_descriptor.real_size = len;

    mqtt_serialiser_rc_t rc = mqtt_serialiser_write( &serializer, msg, buffer, len );

    XI_SAFE_FREE( msg ); // msg no longer needed

    if( rc == MQTT_SERIALISER_RC_ERROR )
    {
        // return LAYER_STATE_ERROR;
    }

    YIELD( layer_data->cs
        , CALL_ON_PREV_DATA_READY( context, ( void* ) &data_descriptor, state ) );

    EXIT( layer_data->cs
        , CALL_ON_NEXT_ON_DATA_READY( context, 0, state ) );

    END_CORO();

    return LAYER_STATE_ERROR;
}

layer_state_t xi_mqtt_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    // must survive the yield
    static uint16_t cs                  = 0;
    static layer_state_t local_state    = LAYER_STATE_OK; //

    // tmp variables for
    const const_data_descriptor_t* data_descriptor = ( const const_data_descriptor_t* ) data;
    xi_mqtt_layer_data_t* layer_data = ( xi_mqtt_layer_data_t* ) CON_SELF( context )->user_data;

    BEGIN_CORO( cs )

    layer_data->msg = ( mqtt_message_t* ) xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( layer_data->msg );
    memset( layer_data->msg, 0, sizeof( mqtt_message_t ) );

    mqtt_parser_init( &layer_data->parser );

    do
    {
        local_state = mqtt_parser_execute(
              &layer_data->parser
            , layer_data->msg
            , ( const uint8_t* ) data_descriptor->data_ptr
            , data_descriptor->real_size, 0 );

        YIELD_UNTIL( cs, ( local_state == LAYER_STATE_WANT_READ ), LAYER_STATE_WANT_READ );
    } while( local_state == LAYER_STATE_WANT_READ );

    EXIT( cs, CALL_ON_NEXT_ON_DATA_READY( context, layer_data->msg, local_state ) );

    END_CORO();

err_handling:
    return LAYER_STATE_ERROR;
}

layer_state_t xi_mqtt_layer_init(
      void* context
    , void* data
    , layer_state_t in_state )
{
    return CALL_ON_PREV_INIT( context, data, in_state );
}

layer_state_t xi_mqtt_layer_connect(
      void* context
    , void* data
    , layer_state_t in_state )
{
    return CALL_ON_NEXT_CONNECT( context, data, in_state );
}

layer_state_t xi_mqtt_layer_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( data );

    mqtt_message_t message;
    memset( &message, 0, sizeof( mqtt_message_t ) );
    message.common.common_u.common_bits.type = MQTT_TYPE_DISCONNECT;

    CALL_ON_SELF_DATA_READY( context, &message, in_state );
    return CALL_ON_PREV_CLOSE( context, 0, in_state );
}

layer_state_t xi_mqtt_layer_on_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    // reaction on closed event
    return LAYER_STATE_OK;
}

#ifdef __cplusplus
}
#endif
