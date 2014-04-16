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


static layer_state_t run_next_task(
    layer_connectivity_t* context );

static layer_state_t send_keep_alive(
    void* data )
{
    layer_connectivity_t* context = data;

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    xi_debug_logger( "send_keep_alive" );

    {
        MAKE_HANDLE_H1(
              &send_keep_alive
            , context );

        layer_data->keep_alive_event
            = xi_evtd_continue(
                    xi_evtd_instance
                  , handle
                  , 10 );
    }

    return LAYER_STATE_OK;
}

static int8_t cmp_topics( void* a, void* b )
{
    union data_t* ca = ( union data_t* ) a; // this suppose to be the one from the vector
    mqtt_buffer_t* cb = ( mqtt_buffer_t* ) b; // this suppose to be the one from the msg

    for( uint8_t i = 0; i < cb->length; ++i )
    {
        if( ca->subscribe.topic[ i ] != cb->data[ i ] )
        {
            return 1;
        }
    }

    return 0;
}

static inline void fill_with_connect_data(
      mqtt_message_t* msg
    , const char* username
    , const char* password )
{
    memset( msg, 0, sizeof( mqtt_message_t ) );

    msg->common.common_u.common_bits.retain     = MQTT_RETAIN_FALSE;
    msg->common.common_u.common_bits.qos        = MQTT_QOS_AT_MOST_ONCE;
    msg->common.common_u.common_bits.dup        = MQTT_DUP_FALSE;
    msg->common.common_u.common_bits.type       = MQTT_TYPE_CONNECT;
    msg->common.remaining_length                = 0; // this is filled during the serialization

    memcpy( msg->connect.protocol_name.data, "MQIsdp", 6 );
    msg->connect.protocol_name.length                    = 6;
    msg->connect.protocol_version                        = 3;

    if( username )
    {
        msg->connect.flags_u.flags_bits.username_follows     = 1;
        size_t len                                           = strlen( username );
        memcpy( msg->connect.username.data, username, len + 1 );
        msg->connect.username.length = len;
    }
    else
    {
        msg->connect.flags_u.flags_bits.username_follows     = 0;
    }

    if( password )
    {
        msg->connect.flags_u.flags_bits.password_follows     = 1;
        size_t len                                           = strlen( password );
        memcpy( msg->connect.password.data, password, len + 1 );
        msg->connect.password.length = len;
    }
    else
    {
        msg->connect.flags_u.flags_bits.password_follows     = 0;
    }

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

static inline void fill_with_publish_data(
      mqtt_message_t* msg
    , const char* topic
    , const char* cnt )
{
    memset( msg, 0, sizeof( mqtt_message_t ) );

    msg->common.common_u.common_bits.retain     = MQTT_RETAIN_FALSE;
    msg->common.common_u.common_bits.qos        = MQTT_QOS_AT_MOST_ONCE;
    msg->common.common_u.common_bits.dup        = MQTT_DUP_FALSE;
    msg->common.common_u.common_bits.type       = MQTT_TYPE_PUBLISH;
    msg->common.remaining_length                = 0; // this is filled during the serialization

    msg->publish.topic_name.length              = strlen( topic );
    msg->publish.content.length                 = strlen( cnt );

    memcpy(
          msg->publish.topic_name.data
        , topic
        , msg->publish.topic_name.length );

    memcpy(
          msg->publish.content.data
        , cnt
        , msg->publish.content.length );
}

static inline void fill_with_subscribe_data(
      mqtt_message_t* msg
    , const char* topic )
{
    memset( msg, 0, sizeof( mqtt_message_t ) );

    msg->common.common_u.common_bits.retain     = MQTT_RETAIN_FALSE;
    msg->common.common_u.common_bits.qos        = MQTT_QOS_EXACTLY_ONCE; // forced by the protocol
    msg->common.common_u.common_bits.dup        = MQTT_DUP_FALSE;
    msg->common.common_u.common_bits.type       = MQTT_TYPE_SUBSCRIBE;
    msg->common.remaining_length                = 0; // this is filled during the serialization

    msg->subscribe.topics.name.length           = strlen( topic );
    msg->subscribe.message_id                   = 1; // just for now
    memcpy(
          msg->subscribe.topics.name.data
        , topic
        , msg->subscribe.topics.name.length );
}

/**
 * The function works as a working
 */
static layer_state_t connect_server_logic(
      layer_connectivity_t* context // should be the context of the logic layer
    , void* data
    , mqtt_message_t* msg_memory
    , layer_state_t* state )
{
    XI_UNUSED( state );

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    //
    BEGIN_CORO( layer_data->data_ready_cs );

    msg_memory = xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( msg_memory );
    memset( msg_memory, 0, sizeof( mqtt_message_t ) );

    fill_with_connect_data( msg_memory
        , layer_data->conn_data->username
        , layer_data->conn_data->password );

    YIELD( layer_data->data_ready_cs
        , CALL_ON_PREV_DATA_READY(
              context
            , msg_memory
            , LAYER_STATE_OK ) );

    YIELD( layer_data->data_ready_cs
        , LAYER_STATE_OK );

    // parse the response
    if( msg_memory->common.common_u.common_bits.type == MQTT_TYPE_CONNACK )
    {
        if( msg_memory->connack.return_code == 0 )
        {
            layer_data->conn_data->on_connected.handlers.h3.a2 = layer_data->conn_data;
            layer_data->conn_data->on_connected.handlers.h3.a3 = LAYER_STATE_OK;
            xi_evtd_continue(
                  xi_evtd_instance
                , layer_data->conn_data->on_connected
                , 0 );

            run_next_task( context );
            XI_SAFE_FREE( msg_memory );

            {
                MAKE_HANDLE_H1(
                      &send_keep_alive
                    , context );

                layer_data->keep_alive_event
                    = xi_evtd_continue(
                            xi_evtd_instance
                          , handle
                          , 10 );
            }

            EXIT( layer_data->data_ready_cs, LAYER_STATE_OK );
        }
        else
        {
            xi_debug_format( "connack.return_code == %d\n", msg_memory->connack.return_code );

            layer_data->conn_data->on_connected.handlers.h3.a2 = layer_data->conn_data;
            layer_data->conn_data->on_connected.handlers.h3.a3 = LAYER_STATE_ERROR;

            xi_evtd_continue(
                  xi_evtd_instance
                , layer_data->conn_data->on_connected
                , 0 );

            run_next_task( context );
            XI_SAFE_FREE( msg_memory );

            EXIT( layer_data->data_ready_cs, LAYER_STATE_ERROR );
        }
    }

    XI_SAFE_FREE( msg_memory );

    run_next_task( context );

    EXIT( layer_data->data_ready_cs, LAYER_STATE_ERROR );
    END_CORO();

    return LAYER_STATE_ERROR;

err_handling:
    xi_free_connection_data( layer_data->conn_data );
    return LAYER_STATE_ERROR;
}

static layer_state_t publish_server_logic(
      layer_connectivity_t* context // should be the context of the logic layer
    , xi_mqtt_logic_task_t* task
    , mqtt_message_t* msg_memory
    , layer_state_t* state )
{
    XI_UNUSED( state );

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    //
    BEGIN_CORO( layer_data->data_ready_cs );

    xi_debug_logger( "publish preparing message..." );

    msg_memory = xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( msg_memory );
    memset( msg_memory, 0, sizeof( mqtt_message_t ) );

    fill_with_publish_data(
          msg_memory
        , task->data.data_u->publish.topic
        , task->data.data_u->publish.msg );

    XI_SAFE_FREE( task->data.data_u->publish.topic );
    XI_SAFE_FREE( task->data.data_u->publish.msg );
    XI_SAFE_FREE( task->data.data_u );

    xi_debug_logger( "publish sending message..." );

    YIELD( layer_data->data_ready_cs
        , CALL_ON_PREV_DATA_READY(
              context
            , msg_memory
            , LAYER_STATE_OK ) );

    xi_debug_logger( "publish message sent..." );

    run_next_task( context );

    EXIT( layer_data->data_ready_cs, LAYER_STATE_OK );

    END_CORO();

    return LAYER_STATE_ERROR;

err_handling:

    return LAYER_STATE_ERROR;
}

static layer_state_t send_subscribe_logic(
      layer_connectivity_t* context // should be the context of the logic layer
    , xi_mqtt_logic_task_t* task
    , mqtt_message_t* msg_memory
    , layer_state_t* state )
{
    XI_UNUSED( state );

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    //
    BEGIN_CORO( layer_data->data_ready_cs );

    msg_memory = xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( msg_memory );
    memset( msg_memory, 0, sizeof( mqtt_message_t ) );

    fill_with_subscribe_data(
          msg_memory
        , task->data.data_u->subscribe.topic );

    xi_debug_logger( "message memory filled with subscribe data" );

    YIELD( layer_data->data_ready_cs
        , CALL_ON_PREV_DATA_READY(
              context
            , msg_memory
            , LAYER_STATE_OK ) );

    xi_debug_logger( "subscribe message sent... waiting for response" );

    YIELD( layer_data->data_ready_cs
        , LAYER_STATE_OK );

    xi_debug_logger( "suback received..." );

    run_next_task( context );

    EXIT( layer_data->data_ready_cs, LAYER_STATE_OK );

    END_CORO();

    return LAYER_STATE_OK;

err_handling:

    return LAYER_STATE_ERROR;
}

static inline layer_state_t recv_publish (
      layer_connectivity_t* context // should be the context of the logic layer
    , xi_mqtt_logic_task_t* task
    , mqtt_message_t* msg_memory
    , layer_state_t* state )
{
    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    //
    BEGIN_CORO( layer_data->data_ready_cs );

    xi_static_vector_index_type_t index = 0;

    xi_debug_logger( "received publish message" );

    index = xi_static_vector_find( layer_data->handlers_for_topics,
          &msg_memory->publish.topic_name
        , cmp_topics );

    if( index != -1 )
    {
        //
        union data_t* subscribe_data
            = layer_data->handlers_for_topics->array[ index ].value;

        subscribe_data->subscribe.handler.handlers.h3.a2 = msg_memory;

        xi_evtd_continue(
               xi_evtd_instance
             , subscribe_data->subscribe.handler
             , 0 );
    }
    else
    {
        xi_debug_logger( "received publish message for unregistered topic..." );
        mqtt_message_dump( msg_memory );
        XI_SAFE_FREE( msg_memory );
    }

    END_CORO()

    return LAYER_STATE_OK;
}

static layer_state_t main_logic(
        void* in
      , void* data
      , layer_state_t in_state )
{
    if( in_state == LAYER_STATE_ERROR )
    {
        xi_debug_logger( "There has been error" );
    }

    layer_connectivity_t* context = ( layer_connectivity_t* ) in;

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    xi_mqtt_logic_task_t* task
        = layer_data->curr_task;

    if( task )
    {
        switch( task->data.mqtt_settings.scenario_t )
        {
            case XI_MQTT_CONNECT:
            {
                return connect_server_logic(
                      context
                    , task
                    , data
                    , &in_state );
            }
            case XI_MQTT_PUBLISH:
            {
                return publish_server_logic(
                      context
                    , task
                    , data
                    , &in_state );
            }
            case XI_MQTT_SUBSCRIBE:
            {
                return send_subscribe_logic(
                      context
                    , task
                    , data
                    , &in_state );
            }
            case XI_MQTT_NONE:
            {
                xi_debug_logger( "XI_MQTT_NONE reached!" );
            }
            default:
            {
                xi_debug_logger( "!default! reached!" );
            }
        }
    }
    else
    {
        xi_debug_logger( "no task when received message... " );
    }

    return LAYER_STATE_ERROR;
}

static layer_state_t run_next_task(
    layer_connectivity_t* context )
{
    xi_debug_logger( "run_next_task" );

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    if( layer_data == 0 )
    {
        xi_debug_logger( "no layer_data" );
        return LAYER_STATE_ERROR;
    }

    xi_mqtt_logic_queue_t* out = 0;

    XI_SAFE_FREE( layer_data->curr_task );

    if( layer_data->tasks_queue )
    {
        POP( xi_mqtt_logic_queue_t
           , layer_data->tasks_queue
           , out );

        layer_data->curr_task = out->task;

        layer_connectivity_t* context   = out->context;
        layer_state_t state             = out->state;

        XI_SAFE_FREE( out );

        switch( layer_data->curr_task->data.mqtt_settings.scenario_t )
        {
            case XI_MQTT_NONE:
                xi_debug_logger( "new task: XI_MQTT_NONE" );
                break;
            case XI_MQTT_CONNECT:
                xi_debug_logger( "new task: XI_MQTT_CONNECT" );
                break;
            case XI_MQTT_PUBLISH:
                xi_debug_logger( "new task: XI_MQTT_PUBLISH" );
                break;
            case XI_MQTT_SUBSCRIBE:
                xi_debug_logger( "new task: XI_MQTT_SUBSCRIBE" );
                break;
        }

        {
            MAKE_HANDLE_H3( &main_logic, context, 0, state );
            xi_evtd_continue( xi_evtd_instance, handle, 0 );
        }
    }
    else
    {
        xi_debug_logger( "task queue empty!" );
    }

    return LAYER_STATE_OK;
}

layer_state_t xi_mqtt_logic_layer_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    if( layer_data == 0 )
    {
        xi_debug_logger( "no layer_data" );
        return LAYER_STATE_ERROR;
    }

    xi_mqtt_logic_queue_t* tmp = 0;

    if( layer_data->curr_task != 0 )
    {
        tmp = ( xi_mqtt_logic_queue_t* )
                xi_alloc( sizeof( xi_mqtt_logic_queue_t ) );

        XI_CHECK_MEMORY( tmp );

        tmp->context        = context;
        tmp->task           = ( xi_mqtt_logic_task_t* ) data;
        tmp->state          = in_state;
        tmp->__next         = 0;

        PUSH_BACK(
              xi_mqtt_logic_queue_t
            , layer_data->tasks_queue
            , tmp );

        xi_debug_logger( "adding task to the queue..." );

        return LAYER_STATE_OK;
    }

    layer_data->curr_task = ( xi_mqtt_logic_task_t* ) data;

    // sending request
    // type of the request depends on
    // the state that we are in
    // it's easy to determine if we are in the middle of
    // processing a single request or we are starting new one

    xi_debug_logger( "calling task directly..." );

    return main_logic( context, data, in_state );

err_handling:
    XI_SAFE_FREE( tmp );
    return LAYER_STATE_ERROR;
}

layer_state_t xi_mqtt_logic_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    mqtt_message_t* recvd_msg
        = ( mqtt_message_t* ) data;

    xi_mqtt_logic_layer_data_t* layer_data  = CON_SELF( context )->user_data;
    xi_mqtt_logic_task_t* task              = layer_data->curr_task;

    // special case doesn't change the current logic settings
    // this way the state machine is always at the proper state
    if( recvd_msg != 0 )
    {
        switch( recvd_msg->common.common_u.common_bits.type )
        {
            case MQTT_TYPE_PUBLISH:
                return recv_publish( context, data, recvd_msg, &in_state );
            case MQTT_TYPE_CONNACK:
                return connect_server_logic( context, data, recvd_msg, &in_state );
            case MQTT_TYPE_SUBACK:
                return send_subscribe_logic(
                              context
                            , task
                            , data
                            , &in_state );
            default:
                xi_debug_logger( "unhandled message received: " );
                mqtt_message_dump( recvd_msg );
        };
    }

    return main_logic( context, data, in_state );
}

layer_state_t xi_mqtt_logic_layer_init(
      void* context
    , void* data
    , layer_state_t in_state )
{
    assert( CON_SELF( context )->user_data == 0 );

    CON_SELF( context )->user_data = xi_alloc( sizeof( xi_mqtt_logic_layer_data_t ) );
    XI_CHECK_MEMORY( CON_SELF( context )->user_data );
    memset( CON_SELF( context )->user_data, 0, sizeof( xi_mqtt_logic_layer_data_t ) );

    xi_mqtt_logic_layer_data_t* layer_data = CON_SELF( context )->user_data;

    xi_connection_data_t* conn_data = data;
    layer_data->conn_data = conn_data;

    layer_data->handlers_for_topics = xi_static_vector_create( 4 );

    XI_CHECK_MEMORY( layer_data->handlers_for_topics );

    return CALL_ON_PREV_INIT( context, data, in_state );

err_handling:
    XI_SAFE_FREE( CON_SELF( context )->user_data );
    return CALL_ON_PREV_INIT( context, data, LAYER_STATE_ERROR );
}

layer_state_t xi_mqtt_logic_layer_connect(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    xi_mqtt_logic_layer_data_t* layer_data
        = CON_SELF( context )->user_data;

    xi_mqtt_logic_task_t* task = xi_alloc(
        sizeof( xi_mqtt_logic_task_t ) );
    XI_CHECK_MEMORY( task );
    memset( task, 0, sizeof( xi_mqtt_logic_task_t ) );

    task->data.mqtt_settings.scenario_t = XI_MQTT_CONNECT;
    task->data.mqtt_settings.qos_t      = XI_MQTT_QOS_ZERO;

    MAKE_HANDLE_H3(
          &xi_mqtt_logic_layer_data_ready
        , context
        , task
        , LAYER_STATE_OK );

    xi_evtd_continue( xi_evtd_instance, handle, 0 );

    return LAYER_STATE_OK;

err_handling:
    return LAYER_STATE_ERROR;
}

layer_state_t xi_mqtt_logic_layer_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return CALL_ON_PREV_CLOSE( context, data, in_state );
}

layer_state_t xi_mqtt_logic_layer_on_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    xi_mqtt_logic_layer_data_t* layer_data = CON_SELF( context )->user_data;

    while( layer_data->handlers_for_topics->elem_no )
    {
        union data_t* sub
            = layer_data->handlers_for_topics->array[ 0 ].value;

        XI_SAFE_FREE( sub->subscribe.topic );
        XI_SAFE_FREE( sub );

        xi_static_vector_del( layer_data->handlers_for_topics, 0 );
    }

    xi_static_vector_destroy( layer_data->handlers_for_topics );

    while( layer_data->tasks_queue )
    {
        xi_mqtt_logic_queue_t* out = 0;

        POP( xi_mqtt_logic_queue_t
           , layer_data->tasks_queue
           , out );

        if( layer_data->curr_task != out->task )
        {
            XI_SAFE_FREE( out );
        }
    }

    XI_SAFE_FREE( CON_SELF( context )->user_data );

    run_next_task( context );

    return LAYER_STATE_OK;
}


#ifdef __cplusplus
}
#endif
