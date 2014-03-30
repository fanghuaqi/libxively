#ifndef XI_MQTT_ENABLED
#define XI_MQTT_ENABLED // for my llvm auto build system
#endif

#ifndef XI_NOB_ENABLED
#define XI_NOB_ENABLED  // for my llvm auto build system
#endif

#include <xively.h>
#include <xi_helpers.h>
#include <xi_coroutine.h>

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

struct user_data
{

};

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

// that how
void main_loop()
{
    //while( evt_dispatcher_have_events_to_process() )
    //{
        // get_dispatcher_evts( read_fds, write_fds, error_fds );
        // int result = select( read_fds, write_fds, error_fds, timeout );
        // update_dispatcher_evts( read_fds, write_fds );
        // update_dispatcher_handles( get_current_time() );
        // }
    //}
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
    if( xi_context == 0 )
    {
        return -1;
    }

    //xi_nob_mqtt_subscribe( xi, "/a/b/c/0", on_0 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/1", on_1 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/2", on_2 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/3", on_3 );
    //xi_nob_mqtt_subscribe( xi, "/a/b/c/4", on_4 );
    //xi_call_every( 10, handler );


    main_loop();

    // const xi_response_t* response = xi_nob_mqtt_publish( xi_context, argv[ 1 ], argv[ 2 ] );

    xi_delete_context( xi_context );

#endif

    return 0;
}