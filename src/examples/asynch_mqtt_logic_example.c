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

void print_usage()
{
    static const char usage[] = "This is example_mqtt_publish of xi library\n"
    "to get publish value at certain topic: \n"
    "mqtt_connect_and_publish topic msg\n";

    printf( "%s", usage );
}

#define REQUIRED_ARGS 3

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

uint8_t on_a( layer_t* layer, mqtt_message_t* message )
{
    // xi_mqtt_nob_publish( "/topic/B", "got B!" );
    return 0;
}

uint8_t on_b( layer_t* layer, mqtt_message_t* message )
{
    // CONTINUE_ON( xi_mqtt_nob_publish( "/topic/A", "got A!" ) );
    return 0;
}

uint8_t publish_tempreture( layer_t* layer )
{
    // CONTINUE_ON( xi_mqtt_nob_publish( "/topic/tempreture", "current tempreture is %s", tempreture_value ) );
    return 0;
}

uint8_t publish_cpu( layer_t* layer )
{
    // CONTINUE_ON( xi_mqtt_nob_publish( "/topic/cpu", "current cpu is %s%%", cpu_value ) );
    return 0;
}

uint8_t users_idle_loop( layer_t* layer )
{
    static uint16_t coro_state = 0;

    BEGIN_CORO( coro_state );

    // YIELD_AND_CONTINUE_ON( coro_state, xi_mqtt_nob_subscribe( "/topic/A", on_a ) );
    // YIELD_AND_CONTINUE_ON( coro_state, xi_mqtt_nob_subscribe( "/topic/B", on_b ) );
    // YIELD_AND_CONTINUE_ON( coro_state, xi_mqtt_nob_publish( "/topic/A", "Hi friends from A!" ) );
    // YIELD_AND_CONTINUE_ON( coro_state, xi_mqtt_nob_publish( "/topic/B", "Hi friends from B!" ) );

    while( 1 )
    {
        // YIELD_AND_CONTINUE_ON_IN( coro_state, 10, publish_tempreture );
        // YIELD_AND_CONTINUE_ON_IN( coro_state, 20, publish_cpu );
    }

    END_CORO();

    return 0;
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

        tv.tv_sec   = 0;
        tv.tv_usec  = 250000;

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

layer_state_t on_connected(
      void* in_context
    , void* data )
{
    xi_context_t* context = ( xi_context_t* ) in_context;
    printf( "connected\n!" );
    return LAYER_STATE_OK;
}

int main( int argc, char* argv[] )
{

#ifndef XI_NOB_ENABLED
    XI_UNUSED( argc );
    XI_UNUSED( argv );
#else
    if( argc < REQUIRED_ARGS )
    {
        print_usage();
        exit( 0 );
    }

    // create the xi library context
    xi_context_t* xi_context
        = xi_create_context( XI_MQTT, 0, 0 );

    // check if everything works
    if( xi_context == 0 ) { return -1; }

    xi_connection_data_t connection_data = { XI_HOST, XI_PORT };

    // @TODO replace with simple macro
    {
        MAKE_HANDLE_H2( on_connected, ( void* ) xi_context, 0 );

        xi_nob_mqtt_connect(
                xi_context
              , &connection_data
              , handle );
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

#endif

    return 0;
}
