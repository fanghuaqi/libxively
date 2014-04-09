// c
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>

// local
#include "posix_asynch_io_layer.h"
#include "posix_asynch_data.h"
#include "xi_allocator.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_common.h"
#include "xi_connection_data.h"
#include "xi_coroutine.h"
#include "xi_event_dispatcher_api.h"
#include "xi_event_dispatcher_global_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

layer_state_t posix_asynch_io_layer_data_ready(
      void* context
    , void* data
    , layer_state_t state )
{
    XI_UNUSED( state );

    posix_asynch_data_t* posix_asynch_data  = CON_SELF( context )->user_data;
    const_data_descriptor_t* buffer         = ( const_data_descriptor_t* ) data;

    if( posix_asynch_data == 0 )
    {
        xi_debug_logger( "layer_data == 0\n" );
        return CALL_ON_NEXT_DATA_READY( context, data, LAYER_STATE_ERROR );
    }

    posix_asynch_data_t* layer_data
        = ( posix_asynch_data_t* ) CON_SELF( context )->user_data;

    BEGIN_CORO( layer_data->cs );

    if( buffer != 0 && buffer->data_size > 0 )
    {
        size_t left = buffer->data_size - buffer->curr_pos;

        if( left == 0 )
        {
            xi_debug_logger( "nothing left to send!\n" );
        }

        do {
            int len = write(
                  posix_asynch_data->socket_fd
                , buffer->data_ptr + buffer->curr_pos
                , buffer->data_size - buffer->curr_pos );

            xi_debug_format( "written: %d\n", len );

            if( len < 0 )
            {
                int errval = errno;
                if( errval == EAGAIN ) // that can happen
                {
                    xi_debug_logger( "eagain....\n" );
                    MAKE_HANDLE_H3( &posix_asynch_io_layer_data_ready, context, data, LAYER_STATE_OK );
                    EXIT( layer_data->cs, xi_evtd_continue_when_evt( xi_evtd_instance
                        , XI_EVENT_WANT_WRITE, handle, posix_asynch_data->socket_fd ) );
                }

                xi_debug_printf( "error writing: errno = %d \n", errval );
                EXIT( layer_data->cs
                    , CALL_ON_NEXT_DATA_READY( context, data, LAYER_STATE_ERROR ) );
            }

            if( len == 0 )
            {
                xi_debug_logger( "disconnection....\n" );
                return EXIT( layer_data->cs, CALL_ON_SELF_CLOSE( context, 0, LAYER_STATE_OK ) );
            }

            buffer->curr_pos += len;
            left = buffer->data_size - buffer->curr_pos;
        } while( left > 0 );
    }

    xi_debug_logger( "exit....\n" );
    EXIT( layer_data->cs, CALL_ON_NEXT_DATA_READY( context, data, LAYER_STATE_OK ) );

    END_CORO();

    return LAYER_STATE_OK;
}

layer_state_t posix_asynch_io_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( in_state );

    //xi_debug_logger( "[posix_asynch_io_layer_on_data_ready]" );

    posix_asynch_data_t* posix_asynch_data = CON_SELF( context )->user_data;

    if( posix_asynch_data == 0 )
    {
        return CALL_ON_NEXT_ON_DATA_READY( context, data, LAYER_STATE_ERROR );
    }

    data_descriptor_t* buffer = 0;

    if( data )
    {
        buffer = ( data_descriptor_t* ) data;
    }
    else
    {
        static char data_buffer[ 32 ];
        memset( data_buffer, 0, sizeof( data_buffer ) );
        static data_descriptor_t buffer_descriptor = { data_buffer, sizeof( data_buffer ), 0, 0 };
        buffer = &buffer_descriptor;
    }

    memset( buffer->data_ptr, 0, buffer->data_size );
    int len = read( posix_asynch_data->socket_fd, buffer->data_ptr, buffer->data_size - 1 );

    xi_debug_format( "read: %d\n", len );

    if( len < 0 )
    {
        int errval = errno;
        if( errval == EAGAIN ) // register for an event on that socket
        {
            {
                MAKE_HANDLE_H3( &posix_asynch_io_layer_on_data_ready
                    , context, data, in_state );
                return xi_evtd_continue_when_evt( xi_evtd_instance
                    , XI_EVENT_WANT_READ, handle, posix_asynch_data->socket_fd );
            }
        }

        xi_debug_printf( "error reading: errno = %d \n", errval );
        return CALL_ON_NEXT_ON_DATA_READY( context, ( void*) 0, LAYER_STATE_ERROR );
    }

    if( len == 0 ) // we've been disconnected, so let's roll down
    {
        return CALL_ON_SELF_CLOSE( context, 0, LAYER_STATE_OK );
    }

    buffer->real_size = len;

    buffer->data_ptr[ buffer->real_size ] = '\0'; // put guard
    buffer->curr_pos = 0;

    return CALL_ON_NEXT_ON_DATA_READY( context, ( void* ) buffer, LAYER_STATE_OK );
}

layer_state_t posix_asynch_io_layer_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    return CALL_ON_SELF_ON_CLOSE( context, data, LAYER_STATE_OK );
}

layer_state_t posix_asynch_io_layer_on_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( in_state );
    XI_UNUSED( data );

    //
    posix_asynch_data_t* posix_asynch_data = CON_SELF( context )->user_data;
    layer_state_t ret = LAYER_STATE_OK;

    if( shutdown( posix_asynch_data->socket_fd, SHUT_RDWR ) == -1 )
    {
        xi_set_err( XI_SOCKET_SHUTDOWN_ERROR );
        close( posix_asynch_data->socket_fd ); // just in case
        ret = LAYER_STATE_ERROR;
        goto err_handling;
    }

    // close the connection & the socket
    if( close( posix_asynch_data->socket_fd ) == -1 )
    {
        xi_set_err( XI_SOCKET_CLOSE_ERROR );
        ret = LAYER_STATE_ERROR;
        goto err_handling;
    }

err_handling:
    // unregister the fd
    xi_evtd_unregister_fd( xi_evtd_instance, posix_asynch_data->socket_fd );
    // cleanup the memory
    XI_SAFE_FREE( posix_asynch_data );

    return CALL_ON_NEXT_ON_CLOSE( context, data, ret );
}

layer_state_t posix_asynch_io_layer_init(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    // PRECONDITIONS
    assert( context != 0 );
    assert( CON_SELF( context )->user_data == 0 );

    xi_debug_logger( "[posix_io_layer_init]" );

    layer_t* layer                              = ( layer_t* ) CON_SELF( context );
    posix_asynch_data_t* posix_asynch_data      = xi_alloc( sizeof( posix_asynch_data_t ) );

    XI_CHECK_MEMORY( posix_asynch_data );

    layer->user_data                            = ( void* ) posix_asynch_data;

    xi_debug_logger( "Creating socket..." );

    posix_asynch_data->socket_fd                = socket( AF_INET, SOCK_STREAM, 0 );

    if( posix_asynch_data->socket_fd == -1 )
    {
        xi_debug_logger( "Socket creation [failed]" );
        xi_set_err( XI_SOCKET_INITIALIZATION_ERROR );
        // return 0;
    }

    xi_debug_logger( "Setting socket non blocking behaviour..." );

    int flags = fcntl( posix_asynch_data->socket_fd, F_GETFL, 0 );

    if( flags == -1 )
    {
        xi_debug_logger( "Socket non blocking behaviour [failed]" );
        xi_set_err( XI_SOCKET_INITIALIZATION_ERROR );
        goto err_handling;
    }

    if( fcntl( posix_asynch_data->socket_fd, F_SETFL, flags | O_NONBLOCK ) == -1 )
    {
        xi_debug_logger( "Socket non blocking behaviour [failed]" );
        xi_set_err( XI_SOCKET_INITIALIZATION_ERROR );
        goto err_handling;
    }

    xi_debug_logger( "Socket creation [ok]" );

    // POSTCONDITIONS
    assert( layer->user_data != 0 );
    assert( posix_asynch_data->socket_fd != -1 );

    return CALL_ON_SELF_CONNECT( context, data, LAYER_STATE_OK );

err_handling:
    // cleanup the memory
    if( posix_asynch_data )     { close( posix_asynch_data->socket_fd ); }
    if( layer->user_data )      { XI_SAFE_FREE( layer->user_data ); }

    return CALL_ON_SELF_CONNECT( context, data, LAYER_STATE_ERROR );
}

layer_state_t posix_asynch_io_layer_connect(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    static uint16_t cs = 0; // local coroutine prereserved state

    xi_connection_data_t* connection_data   = ( xi_connection_data_t* ) data;
    layer_t* layer                          = ( layer_t* ) CON_SELF( context );
    posix_asynch_data_t* posix_asynch_data  = ( posix_asynch_data_t* ) layer->user_data;

    BEGIN_CORO( cs )

    xi_debug_format( "Connecting layer [%d] to the endpoint", layer->layer_type_id );

    // socket specific data
    struct sockaddr_in name;
    struct hostent* hostinfo;

    // get the hostaddress
    hostinfo = gethostbyname( connection_data->address );

    // if null it means that the address has not been founded
    if( hostinfo == NULL )
    {
        xi_debug_logger( "Getting Host by name [failed]" );
        xi_set_err( XI_SOCKET_GETHOSTBYNAME_ERROR );
        goto err_handling;
    }

    xi_debug_logger( "Getting Host by name [ok]" );

    // set the address and the port for further connection attempt
    memset( &name, 0, sizeof( struct sockaddr_in ) );
    name.sin_family     = AF_INET;
    name.sin_addr       = *( ( struct in_addr* ) hostinfo->h_addr_list[0] );
    name.sin_port       = htons( connection_data->port );

    xi_debug_logger( "Connecting to the endpoint..." );

    {
        MAKE_HANDLE_H3( &posix_asynch_io_layer_on_data_ready
            , context
            , 0
            , LAYER_STATE_OK );

        xi_evtd_register_fd(
              xi_evtd_instance
            , posix_asynch_data->socket_fd
            , handle );
    }

    if( connect( posix_asynch_data->socket_fd, ( struct sockaddr* ) &name, sizeof( struct sockaddr ) ) == -1 )
    {
        if( errno != EINPROGRESS )
        {
            xi_debug_printf( "errno: %d", errno );
            xi_debug_logger( "Connecting to the endpoint [failed]" );
            xi_set_err( XI_SOCKET_CONNECTION_ERROR );
            goto err_handling;
        }
        else
        {
            // @TODO
            // think about that... maybe some new macro
            // to wrap it with some nice call
            // cause that won't work in asynch word
            // ....
            MAKE_HANDLE_H3(
                  &posix_asynch_io_layer_connect
                , ( void* ) context
                , data
                , LAYER_STATE_OK );

            xi_evtd_continue_when_evt(
                  xi_evtd_instance
                , XI_EVENT_WANT_WRITE
                , handle
                , posix_asynch_data->socket_fd );

            YIELD( cs, LAYER_STATE_OK ); // return here whenever we can write
        }
    }

    EXIT( cs, CALL_ON_NEXT_CONNECT( context, data, LAYER_STATE_OK ););

err_handling:
    // cleanup the memory
    xi_evtd_unregister_fd( xi_evtd_instance, posix_asynch_data->socket_fd );

    if( posix_asynch_data )     { close( posix_asynch_data->socket_fd ); }
    if( layer->user_data )      { XI_SAFE_FREE( layer->user_data ); }

    EXIT( cs, CALL_ON_NEXT_CONNECT( CON_SELF( context ), data, LAYER_STATE_ERROR ) );

    END_CORO()
}

#ifdef __cplusplus
}
#endif
