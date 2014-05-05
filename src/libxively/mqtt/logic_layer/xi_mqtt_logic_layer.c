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
    void* data );

static layer_state_t xi_mqtt_logic_layer_reconnect(
    void*, void*, layer_state_t );

static layer_state_t keepalive_logic (
      layer_connectivity_t* context // should be the context of the logic layer
    , xi_mqtt_logic_task_t* task
    , mqtt_message_t* msg_memory
    , layer_state_t* state );

static layer_state_t send_keep_alive(
    void* data )
{
    layer_connectivity_t* context = data;

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    if( layer_data == 0 )
    {
        return LAYER_STATE_ERROR;
    }

    layer_data->keep_alive_event = 0;

    xi_mqtt_logic_task_t* task
        = ( xi_mqtt_logic_task_t* ) xi_alloc( sizeof( xi_mqtt_logic_task_t ) );
    XI_CHECK_MEMORY( task );
    memset( task, 0, sizeof( xi_mqtt_logic_task_t ) );

    task->data.mqtt_settings.scenario_t = XI_MQTT_KEEPALIVE;
    task->data.mqtt_settings.qos_t      = XI_MQTT_QOS_ZERO;

    xi_debug_logger( "send_keep_alive" );

    return xi_mqtt_logic_layer_data_ready( context, task, LAYER_STATE_OK );

err_handling:
    XI_SAFE_FREE( task );

    return LAYER_STATE_ERROR;
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

static void fill_with_pingreq_data(
    mqtt_message_t* msg )
{
    memset( msg, 0, sizeof( mqtt_message_t ) );
    msg->common.common_u.common_bits.type = MQTT_TYPE_PINGREQ;
}

static void fill_with_connect_data(
      mqtt_message_t* msg
    , const char* username
    , const char* password
    , uint16_t keepalive_timeout )
{
    memset( msg, 0, sizeof( mqtt_message_t ) );

    msg->common.common_u.common_bits.retain     = MQTT_RETAIN_FALSE;
    msg->common.common_u.common_bits.qos        = MQTT_QOS_AT_MOST_ONCE;
    msg->common.common_u.common_bits.dup        = MQTT_DUP_FALSE;
    msg->common.common_u.common_bits.type       = MQTT_TYPE_CONNECT;
    msg->common.remaining_length                = 0; // this is filled during the serialization

    memcpy( msg->connect.protocol_name.data, "MQTT", 4 );
    msg->connect.protocol_name.length                    = 4;
    msg->connect.protocol_version                        = 4;

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

    msg->connect.keep_alive                              = keepalive_timeout;

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
    msg->common.common_u.common_bits.qos        = MQTT_QOS_AT_LEAST_ONCE; // forced by the protocol
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
    , xi_mqtt_logic_task_t* task
    , mqtt_message_t* msg_memory
    , layer_state_t* state )
{
    XI_UNUSED( state );

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    //
    BEGIN_CORO( task->cs );

    msg_memory = xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( msg_memory );
    memset( msg_memory, 0, sizeof( mqtt_message_t ) );

    fill_with_connect_data( msg_memory
        , layer_data->conn_data->username
        , layer_data->conn_data->password
        , layer_data->conn_data->keepalive_timeout );

    YIELD( task->cs
        , CALL_ON_PREV_DATA_READY(
              context
            , msg_memory
            , LAYER_STATE_OK ) );

    if( *state != LAYER_STATE_OK )
    {
        xi_debug_logger( "error during connection!" );
        EXIT( task->cs, run_next_task( context ) );
    }

    YIELD( task->cs
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

            XI_SAFE_FREE( msg_memory );

            if( layer_data->conn_data->keepalive_timeout > 0 )
            {
                MAKE_HANDLE_H1(
                      &send_keep_alive
                    , context );

                layer_data->keep_alive_event
                    = xi_evtd_continue(
                            xi_evtd_instance
                          , handle
                          , layer_data->conn_data->keepalive_timeout );
            }

            // now the layer is fully connected to the server
            layer_data->the_state = XI_MLLS_CONNECTED;
            EXIT( task->cs, run_next_task( context ) );
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

            XI_SAFE_FREE( msg_memory );
            EXIT( task->cs, run_next_task( context ) );
        }
    }

    XI_SAFE_FREE( msg_memory );
    EXIT( layer_data->data_ready_cs, run_next_task( context ); );
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
    BEGIN_CORO( task->cs );

    //
    if( *state == LAYER_STATE_ERROR )
    {
        EXIT( task->cs, run_next_task( context ) );
    }

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

    if( layer_data->keep_alive_event )
    {
        xi_evtd_restart(
              xi_evtd_instance
            , layer_data->keep_alive_event
            , layer_data->conn_data->keepalive_timeout );
    }

    YIELD( task->cs
        , CALL_ON_PREV_DATA_READY(
              context
            , msg_memory
            , LAYER_STATE_OK ) );

    if( *state == LAYER_STATE_ERROR )
    {
        xi_debug_logger( "publish message not sent..." );
        EXIT( task->cs, run_next_task( context ) );
    }

    xi_debug_logger( "publish message sent..." );

    EXIT( task->cs, run_next_task( context ) );

    END_CORO();

    return LAYER_STATE_ERROR;

err_handling:

    return LAYER_STATE_ERROR;
}

static layer_state_t send_subscribe_logic(
      void* ctx // asd a should be the context of the logic layer
    , void* data
    , layer_state_t state
    , void* msg )
{
    layer_connectivity_t* context = ctx;
    xi_mqtt_logic_task_t* task = data;
    mqtt_message_t* msg_memory = msg;

    XI_UNUSED( state );

    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    //
    BEGIN_CORO( task->cs );

    msg_memory = xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( msg_memory );
    memset( msg_memory, 0, sizeof( mqtt_message_t ) );

    fill_with_subscribe_data(
          msg_memory
        , task->data.data_u->subscribe.topic );

    xi_debug_logger( "message memory filled with subscribe data" );

    YIELD( task->cs
        , CALL_ON_PREV_DATA_READY(
              context
            , msg_memory
            , LAYER_STATE_OK ) );

    if( state == LAYER_STATE_ERROR )
    {
        xi_debug_logger( "subscribe message not sent... properly" );
        EXIT( task->cs, run_next_task( context ) );
    }

    xi_debug_logger( "subscribe message sent... waiting for response" );

    YIELD( task->cs
        , LAYER_STATE_OK );

    if( state == LAYER_STATE_OK )
    {
        xi_debug_logger( "msg received..." );
        mqtt_message_dump( msg_memory );

        XI_SAFE_FREE( msg_memory );
    }
    else
    {
        xi_debug_logger( "suback errored" );
    }

    EXIT( task->cs, run_next_task( context ) );

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
    BEGIN_CORO( task->cs );

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

static layer_state_t keepalive_timeout(
      void* context
    , void* task
    , layer_state_t state
    , void* msg_memory )
{
    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    state = LAYER_STATE_TIMEOUT;
    keepalive_logic( context, task, msg_memory, &state );

    return LAYER_STATE_OK;
}

static layer_state_t keepalive_logic (
      layer_connectivity_t* context // should be the context of the logic layer
    , xi_mqtt_logic_task_t* task
    , mqtt_message_t* msg_memory
    , layer_state_t* state )
{
    xi_mqtt_logic_layer_data_t* layer_data
        = ( xi_mqtt_logic_layer_data_t* ) CON_SELF( context )->user_data;

    //
    assert( task != 0 );

    //
    BEGIN_CORO( task->cs );

    msg_memory = xi_alloc( sizeof( mqtt_message_t ) );
    XI_CHECK_MEMORY( msg_memory );
    memset( msg_memory, 0, sizeof( mqtt_message_t ) );

    fill_with_pingreq_data( msg_memory );

    xi_debug_logger( "message memory filled with pingreq data" );

    YIELD( task->cs
        , CALL_ON_PREV_DATA_READY(
              context
            , msg_memory
            , LAYER_STATE_OK ) );

    if( *state == LAYER_STATE_ERROR )
    {
        xi_debug_logger( "pingreq message not sent..." );
        EXIT( task->cs, run_next_task( context ) );
    }

    xi_debug_logger( "pingreq message sent... waiting for response" );

    // wait for a interval of keepalive
    {
        MAKE_HANDLE_H4(
              &keepalive_timeout
            , context
            , task
            , *state
            , msg_memory );

        assert( layer_data->keep_alive_timeout == 0 );

        layer_data->keep_alive_timeout =
            xi_evtd_continue(
                  xi_evtd_instance
                , handle
                , layer_data->conn_data->keepalive_timeout );
    }

    YIELD( task->cs
        , LAYER_STATE_OK );

    if( *state == LAYER_STATE_OK )
    {
        if( layer_data->keep_alive_timeout != 0 )
        {
            layer_data->keep_alive_timeout = xi_evtd_cancel( xi_evtd_instance
                , layer_data->keep_alive_timeout );
        }
    }
    else if( *state == LAYER_STATE_TIMEOUT )
    {
        xi_debug_logger( "keepalive timeout passed!" );
        layer_data->keep_alive_timeout = 0;
        EXIT( layer_data->data_ready_cs, xi_mqtt_logic_layer_reconnect( context, 0, LAYER_STATE_TIMEOUT ) );
    }
    else if( *state == LAYER_STATE_ERROR )
    {
        xi_debug_logger( "error while waiting for pingresp!" );

        if( layer_data->keep_alive_timeout != 0 )
        {
            layer_data->keep_alive_timeout = xi_evtd_cancel( xi_evtd_instance
                , layer_data->keep_alive_timeout );
        }

        EXIT( task->cs, run_next_task( context ) );
    }
    else
    {
        xi_debug_logger( "unexpected error" );
        if( layer_data->keep_alive_timeout != 0 )
        {
            layer_data->keep_alive_timeout = xi_evtd_cancel( xi_evtd_instance
                , layer_data->keep_alive_timeout );
        }

        EXIT( task->cs, run_next_task( context ) );
    }

    xi_debug_logger( "pingresp received..." );

    {
        MAKE_HANDLE_H1(
              &send_keep_alive
            , context );

        layer_data->keep_alive_event
            = xi_evtd_continue(
                    xi_evtd_instance
                  , handle
                  , layer_data->conn_data->keepalive_timeout );
    }

    XI_SAFE_FREE( msg_memory );

    EXIT( task->cs, run_next_task( context ) );

    END_CORO();

    return LAYER_STATE_OK;

err_handling:
    return LAYER_STATE_ERROR;
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
                    , in_state
                    , data );
            }
            case XI_MQTT_KEEPALIVE:
            {
                return keepalive_logic(
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
    void* data )
{
    layer_connectivity_t* context = data;
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

    if( layer_data->tasks_queue != 0 )
    {
        assert( layer_data->tasks_queue != 0 );

        POP( xi_mqtt_logic_queue_t
           , layer_data->tasks_queue
           , out );

        // prevent execution of other tasks while connecting
        if( layer_data->the_state == XI_MLLS_CONNECTING )
        {
            // we only allows connect to happen
            // while client is connecting
            if( out->task->data.mqtt_settings.scenario_t != XI_MQTT_CONNECT )
            {
                PUSH_BACK( xi_mqtt_logic_queue_t
                    , layer_data->tasks_queue
                    , out );

                assert( layer_data->tasks_queue != 0 );

                return LAYER_STATE_OK;
            }
        }

        layer_data->curr_task           = out->task;
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
            case XI_MQTT_KEEPALIVE:
                xi_debug_logger( "new task: XI_MQTT_KEEPALIVE" );
                break;
            default:
                assert( 1 == 0 );
                break;
        }

        {
            // @TODO
            // For Qos 1 and 2 here we are going to assign the proper
            // id, we shall have few options how to generate them
            // my initial idea is to create a list of ranges, so that
            // we always be able to reuse the id's that will return to the pool
            // the other solution is just a simple int that we will increment
            // less memory, but no reusage the choice must be disscussed
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

    tmp = ( xi_mqtt_logic_queue_t* )
            xi_alloc( sizeof( xi_mqtt_logic_queue_t ) );
    XI_CHECK_MEMORY( tmp );
    memset( tmp, 0, sizeof( xi_mqtt_logic_queue_t ) );

    tmp->context        = context;
    tmp->task           = ( xi_mqtt_logic_task_t* ) data;
    tmp->state          = in_state;
    tmp->__next         = 0;

    assert( tmp->task->data.mqtt_settings.scenario_t <= XI_MQTT_KEEPALIVE );

    if( tmp->task->priority == XI_MQTT_LOGIC_TASK_NORMAL )
    {
        PUSH_BACK(
              xi_mqtt_logic_queue_t
            , layer_data->tasks_queue
            , tmp );
    }
    else if( tmp->task->priority == XI_MQTT_LOGIC_TASK_IMMEDIATE )
    {
        PUSH_FRONT(
              xi_mqtt_logic_queue_t
            , layer_data->tasks_queue
            , tmp );
    }
    else
    {
        xi_debug_logger( "Unknown task priority!" );
        return LAYER_STATE_ERROR;
    }

    xi_debug_logger( "added task to the queue..." );

    if( layer_data->curr_task == 0 )
    {
        return run_next_task( context );
    }

    return LAYER_STATE_OK;

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
                return recv_publish( context, task, recvd_msg, &in_state );
            case MQTT_TYPE_CONNACK:
                return connect_server_logic( context, task, recvd_msg, &in_state );
            case MQTT_TYPE_SUBACK:
                return send_subscribe_logic( context, task, in_state, data );
            case MQTT_TYPE_PINGRESP:
                return keepalive_logic( context, task, recvd_msg, &in_state );
            default:
                xi_debug_logger( "unhandled message received: " );
                mqtt_message_dump( recvd_msg );
                XI_SAFE_FREE( recvd_msg );
                return LAYER_STATE_ERROR;
        };
    }

    return main_logic( context, data, in_state );
}

layer_state_t xi_mqtt_logic_layer_init(
      void* context
    , void* data
    , layer_state_t in_state )
{
    xi_mqtt_logic_layer_data_t* layer_data
        = CON_SELF( context )->user_data;

    // only if the layer_data has been wiped out
    // that means that we are connecting for the
    // first time
    if( layer_data == 0 )
    {
        CON_SELF( context )->user_data = xi_alloc( sizeof( xi_mqtt_logic_layer_data_t ) );
        XI_CHECK_MEMORY( CON_SELF( context )->user_data );
        memset( CON_SELF( context )->user_data, 0, sizeof( xi_mqtt_logic_layer_data_t ) );

        layer_data = CON_SELF( context )->user_data;

        xi_connection_data_t* conn_data = data;
        layer_data->conn_data = conn_data;
    }

    assert( layer_data != 0 );

    layer_data->the_state = XI_MLLS_CONNECTING;

    layer_data->handlers_for_topics = xi_static_vector_create( 4 );
    XI_CHECK_MEMORY( layer_data->handlers_for_topics );

    return CALL_ON_PREV_INIT( context, data, in_state );

err_handling:
    XI_SAFE_FREE( CON_SELF( context )->user_data );
    return LAYER_STATE_ERROR;
}

layer_state_t xi_mqtt_logic_layer_reconnect(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    xi_mqtt_logic_layer_data_t* layer_data
        = CON_SELF( context )->user_data;

    // PRECONDITIONS
    assert( layer_data != 0 );

    layer_data->the_state = XI_MLLS_CONNECTING;

    return CALL_ON_PREV_CLOSE( context, data, in_state );
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

    if( in_state == LAYER_STATE_ERROR )
    {
        xi_debug_logger( "error while io connection" );
        return xi_mqtt_logic_layer_reconnect( context, 0, LAYER_STATE_OK );
    }

    xi_mqtt_logic_task_t* task = xi_alloc(
        sizeof( xi_mqtt_logic_task_t ) );
    XI_CHECK_MEMORY( task );
    memset( task, 0, sizeof( xi_mqtt_logic_task_t ) );

    task->data.mqtt_settings.scenario_t = XI_MQTT_CONNECT;
    task->data.mqtt_settings.qos_t      = XI_MQTT_QOS_ZERO;
    task->priority                      = XI_MQTT_LOGIC_TASK_IMMEDIATE;

    // push the task directly
    return xi_mqtt_logic_layer_data_ready( context, task, LAYER_STATE_OK );

err_handling:
    return LAYER_STATE_ERROR;
}

layer_state_t xi_mqtt_logic_layer_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    xi_mqtt_logic_layer_data_t* layer_data
        = CON_SELF( context )->user_data;

    // PRECONDITIONS
    assert( layer_data != 0 );

    // if close has been called from here we know that we want to perform shutdown
    layer_data->the_state = XI_MLLS_SHUTTING_DOWN;

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

    // PRECONDITIONS
    assert( layer_data != 0 );

    while( layer_data->handlers_for_topics->elem_no )
    {
        union data_t* sub
            = layer_data->handlers_for_topics->array[ 0 ].value;

        assert( sub != 0 );

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

        assert( layer_data->curr_task != out->task );

        XI_SAFE_FREE( out->task );
        XI_SAFE_FREE( out );
    }

    if( layer_data->curr_task != 0 )
    {
        main_logic( context, 0, LAYER_STATE_ERROR );
    }

    // unregister keepalives
    {
        if( layer_data->keep_alive_event )
        {
            layer_data->keep_alive_event = xi_evtd_cancel( xi_evtd_instance
                , layer_data->keep_alive_event );
        }

        if( layer_data->keep_alive_timeout )
        {
            layer_data->keep_alive_timeout = xi_evtd_cancel( xi_evtd_instance
                , layer_data->keep_alive_timeout );
        }
    }

    // if shutdown perform full memory cleaning on all qos levels
    if( layer_data->the_state == XI_MLLS_SHUTTING_DOWN )
    {
        XI_SAFE_FREE( CON_SELF( context )->user_data );
    }
    else
    {
        // mark the layer as disconnected
        layer_data->the_state = XI_MLLS_CONNECTING;

        // this will add a reconnect task to the task queue
        {
            MAKE_HANDLE_H3( &xi_mqtt_logic_layer_init
                , context
                , layer_data->conn_data
                , LAYER_STATE_OK );

            xi_evtd_continue( xi_evtd_instance, handle, 1 ); // try in 1 sec
        }

        if( layer_data->curr_task == 0 )
        {
            run_next_task( context );
        }
    }

    return LAYER_STATE_OK;
}


#ifdef __cplusplus
}
#endif
