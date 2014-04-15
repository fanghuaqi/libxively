#include "xi_event_dispatcher_api.h"
#include "xi_heap.h"

static inline int8_t xi_evtd_cmp_fd(
      void* e0
    , void* value )
{
    xi_evtd_triplet_t* trip = ( xi_evtd_triplet_t* ) e0;

    if( trip->fd == ( xi_fd_t )( intptr_t ) value )
    {
        return 0;
    }

    return -1;
}

int8_t xi_evtd_register_fd(
      xi_evtd_instance_t* instance
    , xi_fd_t fd
    , xi_evtd_handle_t read_handle )
{
    // PRECONDITIONS
    assert( instance != 0 );
    assert( xi_static_vector_find( instance->handles_and_fd, ( void* )( intptr_t ) fd, &xi_evtd_cmp_fd ) == -1 );

    // register new file descriptor
    xi_evtd_triplet_t* triplet = ( xi_evtd_triplet_t* ) xi_alloc( sizeof( xi_evtd_triplet_t ) );
    XI_CHECK_MEMORY( triplet );

    triplet->fd            = fd;
    triplet->event_type    = XI_EVENT_WANT_READ; // this is default state
    triplet->read_handle   = read_handle;        // remember the read one
    triplet->handle        = read_handle;        // we shall start to listen

    // register within the handles
    {
        const xi_static_vector_elem_t* e =
            xi_static_vector_push(
                  instance->handles_and_fd
                , triplet );
        if( e == 0 ){ goto err_handling; }
    }

    return 1;

err_handling:
    XI_SAFE_FREE( triplet );
    return 0;
}

int8_t xi_evtd_unregister_fd(
      xi_evtd_instance_t* instance
    , xi_fd_t fd )
{
    // PRECONDITIONS
    assert( instance != 0 );

    xi_static_vector_index_type_t id
        = xi_static_vector_find(
              instance->handles_and_fd
            , ( void* )( intptr_t ) fd, &xi_evtd_cmp_fd );

    // remove from the vector
    if( id != -1 )
    {
        assert( instance->handles_and_fd->array[ id ].value != 0 );
        XI_SAFE_FREE( instance->handles_and_fd->array[ id ].value );
        xi_static_vector_del( instance->handles_and_fd, id );
        return 1;
    }

    assert( 1 == 0 );

    return -1;
}

int8_t xi_evtd_continue_when_evt(
      xi_evtd_instance_t* instance
    , xi_event_type_t event_type
    , xi_evtd_handle_t handle
    , xi_fd_t fd )
{
    // PRECONDITIONS
    assert( instance != 0 );

    xi_static_vector_index_type_t id
        = xi_static_vector_find(
              instance->handles_and_fd
            , ( void* )( intptr_t ) fd, &xi_evtd_cmp_fd );


    // set up the values of the triplet
    if( id != -1 )
    {
        xi_evtd_triplet_t* triplet
            = ( xi_evtd_triplet_t* )
                instance->handles_and_fd->array[ id ].value;
        triplet->event_type = event_type;
        triplet->handle     = handle;
        return 1;
    }

    assert( 1 == 0 );

    return -1;
}

struct xi_heap_element_s* xi_evtd_continue(
      xi_evtd_instance_t* instance
    , xi_evtd_handle_t handle
    , xi_heap_key_type_t time_diff )
{
    return xi_heap_element_add(
          instance->call_heap
        , instance->current_step + time_diff
        , handle );
}

void xi_evtd_cancel(
      xi_evtd_instance_t* instance
    , struct xi_heap_element_s* heap_element )
{
    xi_heap_element_remove( instance->call_heap, heap_element );
}

void xi_evtd_restart(
      xi_evtd_instance_t* instance
    , struct xi_heap_element_s* heap_element
    , xi_heap_key_type_t new_time )
{
    xi_heap_element_update_key(
          instance->call_heap
        , heap_element
        , new_time );
}

xi_evtd_instance_t* xi_evtd_create_instance()
{
    xi_evtd_instance_t* evtd_instance = ( xi_evtd_instance_t* ) xi_alloc( sizeof( xi_evtd_instance_t ) );

    XI_CHECK_MEMORY( evtd_instance );

    memset( evtd_instance, 0, sizeof( xi_evtd_instance_t ) );

    evtd_instance->call_heap = xi_heap_create( 16 );

    XI_CHECK_MEMORY( evtd_instance->call_heap );

    evtd_instance->handles_and_fd = xi_static_vector_create( 16 );

    XI_CHECK_MEMORY( evtd_instance->handles_and_fd );

    return evtd_instance;

err_handling:
    XI_SAFE_FREE( evtd_instance );
    return 0;
}

void xi_evtd_destroy_instance(
    xi_evtd_instance_t* instance )
{
    xi_static_vector_destroy( instance->handles_and_fd );
    xi_heap_destroy( instance->call_heap );
    XI_SAFE_FREE( instance );
}

void xi_evtd_execute_handle( xi_evtd_handle_t* handle )
{
    switch( handle->handle_type )
    {
        case XI_EVTD_HANDLE_0_ID:
            ( *handle->handlers.h0.phandle_0 )();
        break;
        case XI_EVTD_HANDLE_1_ID:
            ( *handle->handlers.h1.phandle_1 )( handle->handlers.h1.a1 );
        break;
        case XI_EVTD_HANDLE_2_ID:
            ( *handle->handlers.h2.phandle_2 )( handle->handlers.h2.a1, handle->handlers.h2.a2 );
        break;
        case XI_EVTD_HANDLE_3_ID:
            ( *handle->handlers.h3.phandle_3 )( handle->handlers.h3.a1, handle->handlers.h3.a2, handle->handlers.h3.a3 );
        break;
        case XI_EVTD_HANDLE_4_ID:
            ( *handle->handlers.h4.phandle_4 )( handle->handlers.h4.a1, handle->handlers.h4.a2, handle->handlers.h4.a3, handle->handlers.h4.a4 );
        break;
    }
}

void xi_evtd_step(
      xi_evtd_instance_t* evtd_instance
    , xi_heap_key_type_t new_step )
{
    evtd_instance->current_step  = new_step;
    const xi_heap_element_t* tmp = 0;

    xi_debug_format( "size of event queue: %d", evtd_instance->call_heap->first_free );

    while( !xi_heap_is_empty( evtd_instance->call_heap ) )
    {
        tmp = xi_heap_peek_top( evtd_instance->call_heap );
        if( tmp->key <= evtd_instance->current_step )
        {
            tmp = xi_heap_get_top( evtd_instance->call_heap );
            xi_evtd_handle_t* handle = ( xi_evtd_handle_t* ) &tmp->heap_value.type_value;
            xi_evtd_execute_handle( handle );
        }
        else
        {
            xi_debug_format( "next key execution time: %d\tcurrent time: %d"
                , tmp->key
                , evtd_instance->current_step );
            break;
        }
    }
}

uint8_t xi_evtd_dispatcher_continue(
    xi_evtd_instance_t* instance )
{
    return instance->stop != 1;
}

/**
 * \brief update events triggers registration of continuations assigned to the given event on given device
 * \note events_mask is the mask that's created using | operator
 */
void xi_evtd_update_event(
      xi_evtd_instance_t* instance
    , xi_fd_t fd )
{
    assert( instance != 0 );

    xi_static_vector_index_type_t id
        = xi_static_vector_find(
              instance->handles_and_fd
            , ( void* )( intptr_t ) fd
            , &xi_evtd_cmp_fd );

    if( id != -1 )
    {
        xi_evtd_triplet_t* triplet
            = ( xi_evtd_triplet_t* ) instance->handles_and_fd->array[ id ].value;

        // save the handle to execute
        xi_evtd_handle_t to_exec    = triplet->handle;

        // set the default one
        triplet->event_type         = XI_EVENT_WANT_READ;       // default
        triplet->handle             = triplet->read_handle;

        // execute previously saved handle
        // we save the handle because the triplet->handle
        // may be overrided within the handle execution
        // so we don't won't to override that again
        xi_evtd_execute_handle( &to_exec );
    }
    else
    {
        assert( 1 == 0 );
    }
}

void xi_evtd_stop(
    xi_evtd_instance_t* instance )
{
    assert( instance != 0 );
    instance->stop = 1;
}
