#ifndef XI_MQTT_ENABLED
#define XI_MQTT_ENABLED // for my llvm auto build system
#endif

#ifndef XI_NOB_ENABLED
#define XI_NOB_ENABLED  // for my llvm auto build system
#endif

#include <xively.h>
#include <xi_helpers.h>
#include <xi_coroutine.h>
#include <xi_connection_data.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>

static char* test_topic = NULL;
static uint8_t connected  = 0;

// logic loop for the mqtt processing in python like pseudocode
//
// while( true ):
//      if not connected:
//          yield connect()
//
//      yield process_user_idle_function()
//
//      if something to publish or subscribe:
//          yield publish_or_subscribe()
//      else:
//          yield wait_for_transmission()
//

// states: connecting, reading subscription, publishing, idle
// possible transitions:
// S->connecting
// connecting->idle
// idle->reading subscription
// idle->publishing
// reading subscription->idle
// publishing->idle

layer_state_t on_test_message(
      void* context
    , void* message
    , layer_state_t state )
{
    // xi_mqtt_nob_publish( "/topic/B", "got B!" );
    mqtt_message_t* msg = ( mqtt_message_t* ) message;
    xi_debug_logger( "received message: " );
    //mqtt_message_dump( msg );
    xi_debug_logger( "\n" );

    for( int i = 0; i < msg->publish.content.length; ++i )
    {
        printf( "%c", msg->publish.content.data[ i ] );
    }
    printf( "\n" );

    fflush( stdout );

    //printf( "%s\n", msg->publish.content.data );
    //
    XI_SAFE_FREE( msg );

    return LAYER_STATE_OK;
}

layer_state_t on_test_message2(
      void* context
    , void* message
    , layer_state_t state )
{
    // xi_mqtt_nob_publish( "/topic/B", "got B!" );
    mqtt_message_t* msg = ( mqtt_message_t* ) message;
    xi_debug_logger( "received message2: " );
    mqtt_message_dump( msg );
    xi_debug_logger( "\n" );
    //

    return LAYER_STATE_OK;
}

uint8_t xi_mqtt_logic_loop( layer_t* layer )
{
    static uint16_t state = 0;

    BEGIN_CORO( state );

    // connect

    while( 1 )
    {
        //  YIELD_AND_CONTINUE( call_users_idle_loop );
        //  YIELD_AND_CONTINUE( receive_publishes );
    }

    END_CORO();

    return 0;
}

// that how we can handle the system events
// if you need more sophisticated handling you can decouple that
// function onto multiple smaller ones
void main_loop()
{
    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    FD_ZERO( &rfds );
    FD_ZERO( &wfds );
    FD_ZERO( &efds );

    struct timeval tv;

    while( xi_evtd_dispatcher_continue( xi_evtd_instance ) )
    {
        xi_static_vector_index_type_t i = 0;
        int max_fd                      = 0;

        tv.tv_sec   = 1;
        tv.tv_usec  = 0;

        FD_ZERO( &rfds );
        FD_ZERO( &wfds );
        FD_ZERO( &efds );

        for( ; i < xi_evtd_instance->handles_and_fd->elem_no; ++i  )
        {
            xi_evtd_triplet_t* triplet =
                ( xi_evtd_triplet_t* ) xi_evtd_instance->handles_and_fd->array[ i ].value;

            if( ( triplet->event_type & XI_EVENT_WANT_READ ) > 0 )
            {
                FD_SET( triplet->fd, &rfds );
                max_fd = triplet->fd > max_fd ? triplet->fd : max_fd;
            }

            if( ( triplet->event_type & XI_EVENT_WANT_WRITE ) > 0 )
            {
                FD_SET( triplet->fd, &wfds );
                max_fd = triplet->fd > max_fd ? triplet->fd : max_fd;
            }

            if( ( triplet->event_type & XI_EVENT_ERROR ) > 0 )
            {
                FD_SET( triplet->fd, &efds );
                max_fd = triplet->fd > max_fd ? triplet->fd : max_fd;
            }
        }

        int result = select( max_fd + 1, &rfds, &wfds, &efds, &tv );

        if( result > 0 )
        {
            for( i = 0 ; i < xi_evtd_instance->handles_and_fd->elem_no; ++i  )
            {
                xi_evtd_triplet_t* triplet =
                    ( xi_evtd_triplet_t* ) xi_evtd_instance->handles_and_fd->array[ i ].value;

                if( FD_ISSET( triplet->fd, &rfds ) )
                {
                    xi_evtd_update_event( xi_evtd_instance, triplet->fd );
                }

                if( FD_ISSET( triplet->fd, &wfds ) )
                {
                    xi_evtd_update_event( xi_evtd_instance, triplet->fd );
                }

                if( FD_ISSET( triplet->fd, &efds ) )
                {
                    xi_evtd_update_event( xi_evtd_instance, triplet->fd );
                }
            }
        }

        xi_evtd_step( xi_evtd_instance, time( 0 ) );
    }
}

layer_state_t delayed_publish(
          void* in_context
    )
{
    xi_context_t* context = ( xi_context_t* ) in_context;

    // sending the connect request
    xi_nob_mqtt_publish( context, test_topic, "test_msg_delayed" );

    { // register delayed publish again
        MAKE_HANDLE_H1( &delayed_publish, in_context );
        xi_evtd_continue( xi_evtd_instance, handle, 11 );
    }

    return LAYER_STATE_OK;
}

layer_state_t on_connected(
      void* in_context
    , void* data
    , layer_state_t state )
{
    xi_context_t* context = ( xi_context_t* ) in_context;

    if( state == LAYER_STATE_OK )
    {
        printf( "connected!\n" );
    }
    else
    {
        printf( "error while connecting!\n" );
    }

    // sending the connect request
    xi_nob_mqtt_publish( context, test_topic, "test_msg" );
    xi_nob_mqtt_publish( context, test_topic, "test_msg2" );
    xi_nob_mqtt_publish( context, test_topic, "test_msg3" );

    { // register delayed publish
        MAKE_HANDLE_H1( &delayed_publish, in_context );
        xi_evtd_continue( xi_evtd_instance, handle, 2 );
    }

    if( connected == 0 ) // we want to call it only once
    { // register delayed publish
        MAKE_HANDLE_H1( &delayed_publish, in_context );
        xi_evtd_continue( xi_evtd_instance, handle, 3 );
        connected = 1;
    }

    {
        MAKE_HANDLE_H3( &on_test_message, context, 0, LAYER_STATE_OK );
        xi_nob_mqtt_subscribe( context, test_topic, handle );
    }

    /*{
        MAKE_HANDLE_H3( &on_test_message2, context, 0, LAYER_STATE_OK );
        xi_nob_mqtt_subscribe( context, "test_topic2", handle );
    }*/

    return LAYER_STATE_OK;
}


int main( int argc, char* argv[] )
{

#ifndef XI_NOB_ENABLED
    XI_UNUSED( argc );
    XI_UNUSED( argv );
#else

    const char* local_test_topic = "test_topic";
    // create the xi library context
    xi_context_t* xi_context
        = xi_create_context( XI_MQTT, 0, 0 );

    // check if everything works
    if( xi_context == 0 ) { return -1; }

    // @TODO replace with simple macro
    {
        MAKE_HANDLE_H3(
              on_connected
            , ( void* ) xi_context
            , 0
            , 0 );

        if(argc != 5)
        {
            test_topic = local_test_topic;
            xi_nob_mqtt_connect(
                    xi_context
                  , "localhost"
                  , 1883
                  , 10
                  , 0
                  , 0
                  , handle );
        }
        else
        {
            test_topic = argv[4];
            xi_nob_mqtt_connect(
                    xi_context
                  , argv[1]
                  , 1883
                  , 10
                  , argv[2]
                  , argv[3]
                  , handle );
        }
    }

    //xi_nob_mqtt_subscribe( xi, "/a/b/c/0", on_0 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/1", on_1 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/2", on_2 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/3", on_3 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/4", on_4 );
    //xi_call_every( 10, handler );

    //xi_nob_mqtt_publish( xi_context, argv[ 1 ], argv[ 2 ] );

    main_loop();

    xi_delete_context( xi_context );

    return 0;

err_handling:
    return 1;

#endif

    return 0;
}
