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

void xi_mqtt_layer_data_ready(
      void* context
    , void* data
    , layer_state_t state )
{
    XI_UNUSED( state );

    const mqtt_message_t* msg = ( const mqtt_message_t* ) data;

    mqtt_serialiser_t serializer;
    mqtt_serialiser_init( & serializer );

    int len = mqtt_serialiser_size( &serializer, msg );

    uint8_t buffer[ 256 ];
    data_descriptor_t data_descriptor = { ( char* ) buffer, len, len, 0 };

    mqtt_serialiser_rc_t rc = mqtt_serialiser_write( &serializer, msg, buffer, len );

    if( rc == MQTT_SERIALISER_RC_ERROR )
    {
        // return LAYER_STATE_ERROR;
    }

    CALL_ON_PREV_DATA_READY( context, ( void* ) &data_descriptor, LAYER_STATE_OK );

    //return LAYER_STATE_OK;
}

void xi_mqtt_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t state )
{
    // must survive the yield
    static uint16_t cs                  = 0;
    static layer_state_t local_state    = LAYER_STATE_OK; //

    // tmp variables for
    const const_data_descriptor_t* data_descriptor = ( const const_data_descriptor_t* ) data;
    xi_mqtt_layer_data_t* layer_data = ( xi_mqtt_layer_data_t* ) CON_SELF( context )->user_data;

    BEGIN_CORO( cs )

    mqtt_parser_init( &layer_data->parser );

    do
    {
        local_state = mqtt_parser_execute(
              &layer_data->parser
            , &layer_data->msg
            , ( const uint8_t* ) data_descriptor->data_ptr
            , data_descriptor->real_size, 0 );

        //YIELD_UNTIL( cs, ( local_state == LAYER_STATE_WANT_READ ), LAYER_STATE_WANT_READ );
    } while( local_state == LAYER_STATE_WANT_READ );

    //EXIT( cs, LAYER_STATE_OK );

    END_CORO()
}

void xi_mqtt_layer_init(
      void* context
    , void* data
    , layer_state_t state )
{
    CALL_ON_PREV_INIT( context, data, 0 );
}

void xi_mqtt_layer_connect(
      void* context
    , void* data
    , layer_state_t state )
{
    CALL_ON_NEXT_CONNECT( context, data, 0 );
}

void xi_mqtt_layer_close(
      void* context
    , void* data
    , layer_state_t state )
{
    mqtt_message_t message;
    memset( &message, 0, sizeof( mqtt_message_t ) );
    message.common.common_u.common_bits.type = MQTT_TYPE_DISCONNECT;

    CALL_ON_SELF_DATA_READY( context, &message, LAYER_STATE_OK );
    CALL_ON_PREV_CLOSE( context, 0, LAYER_STATE_OK );
}

void xi_mqtt_layer_on_close(
      void* context
    , void* data
    , layer_state_t state )
{
    // reaction on closed event
}

#ifdef __cplusplus
}
#endif
