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

    xi_mqtt_layer_data_t* layer_data
        = ( xi_mqtt_layer_data_t* ) CON_SELF( context )->user_data;

    mqtt_message_t* msg = ( mqtt_message_t* ) data;

    if( layer_data == 0 )
    {
        if( msg ) { XI_SAFE_FREE( msg ); }
        return CALL_ON_NEXT_DATA_READY( context, data, LAYER_STATE_ERROR );
    }

    mqtt_serialiser_t serializer;
    mqtt_serialiser_init( & serializer );

    // @TODO change to use dynamic allocation
    static uint8_t buffer[ 256 ];
    static data_descriptor_t data_descriptor = { ( char* ) buffer, 0, 0, 0 };

    BEGIN_CORO( layer_data->cs );

    int len = mqtt_serialiser_size( &serializer, msg );

    data_descriptor.data_size = len;
    data_descriptor.real_size = len;
    data_descriptor.curr_pos  = 0;

    mqtt_serialiser_rc_t rc = mqtt_serialiser_write( &serializer, msg, buffer, len );

    XI_SAFE_FREE( msg ); // msg no longer needed

    if( rc == MQTT_SERIALISER_RC_ERROR )
    {
        EXIT( layer_data->cs
            , CALL_ON_NEXT_DATA_READY( context, 0, LAYER_STATE_ERROR ) );
    }

    xi_debug_logger( "sending message... " );

    YIELD( layer_data->cs
        , CALL_ON_PREV_DATA_READY( context, ( void* ) &data_descriptor, state ) );

    if( state == LAYER_STATE_ERROR )
    {
        xi_debug_logger( "message not sent... " );
    }
    else
    {
        xi_debug_logger( "message sent... " );
    }

    EXIT( layer_data->cs
        , CALL_ON_NEXT_ON_DATA_READY( context, 0, state ) );

    xi_debug_logger( "I'm not suppose to be here!" );

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

    xi_mqtt_layer_data_t* layer_data
        = ( xi_mqtt_layer_data_t* ) CON_SELF( context )->user_data;

    if( layer_data == 0 )
    {
        return CALL_ON_NEXT_ON_DATA_READY( context, data, LAYER_STATE_ERROR );
    }

    // must survive the yield
    static uint16_t cs                  = 0;
    static layer_state_t local_state    = LAYER_STATE_OK; //

    // tmp variables for
    data_descriptor_t* data_descriptor = ( data_descriptor_t* ) data;

    BEGIN_CORO( cs )

    if( in_state == LAYER_STATE_ERROR )
    {
        goto err_handling;
    }

    layer_data->msg = ( mqtt_message_t* ) xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( layer_data->msg );
    memset( layer_data->msg, 0, sizeof( mqtt_message_t ) );

    mqtt_parser_init( &layer_data->parser );

    do
    {
        local_state = mqtt_parser_execute(
              &layer_data->parser
            , layer_data->msg
            , data_descriptor );

        YIELD_UNTIL( cs
            , ( local_state == LAYER_STATE_WANT_READ )
            , CALL_ON_PREV_ON_DATA_READY( context, data_descriptor, local_state ) );
    } while( in_state != LAYER_STATE_ERROR && local_state == LAYER_STATE_WANT_READ );

    if( in_state == LAYER_STATE_ERROR )
    {
        goto err_handling;
    }

    // delete the buffer if limit reached or previous call might have failed reading the message
    if( data_descriptor->curr_pos == data_descriptor->real_size || local_state != LAYER_STATE_OK )
    {
        // delete buffer no longer needed
        if( data_descriptor ) { XI_SAFE_FREE( data_descriptor->data_ptr ); }
        XI_SAFE_FREE( data_descriptor );
    }
    else
    {
        // register another call to this function cause there might be next
        // message
        CALL_ON_SELF_ON_DATA_READY( context, data_descriptor, local_state );
    }

    EXIT( cs, CALL_ON_NEXT_ON_DATA_READY( context, layer_data->msg, local_state ) );

    END_CORO();

err_handling:
    return CALL_ON_NEXT_ON_DATA_READY( context, layer_data->msg, LAYER_STATE_ERROR );
}

layer_state_t xi_mqtt_layer_init(
      void* context
    , void* data
    , layer_state_t in_state )
{
    assert( CON_SELF( context )->user_data == 0 );

    CON_SELF( context )->user_data = xi_alloc( sizeof( xi_mqtt_layer_data_t ) );
    XI_CHECK_MEMORY( CON_SELF( context )->user_data );
    memset( CON_SELF( context )->user_data, 0, sizeof( xi_mqtt_layer_data_t ) );

    return CALL_ON_PREV_INIT( context, data, in_state );

err_handling:
    return CALL_ON_PREV_INIT( context, data, LAYER_STATE_ERROR );
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
    return CALL_ON_PREV_CLOSE( context, data, in_state );
}

layer_state_t xi_mqtt_layer_on_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_SAFE_FREE( CON_SELF( context )->user_data );
    return CALL_ON_NEXT_ON_CLOSE( context, data, in_state );
}

#ifdef __cplusplus
}
#endif
