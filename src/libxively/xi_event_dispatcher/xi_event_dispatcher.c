#include "xi_event_dispatcher_api.h"
#include "xi_heap.h"

static inline int8_t xi_evtd_cmp_fd( void* e0, void* value )
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
    , xi_fd_t fd )
{
    // PRECONDITIONS
    assert( instance != 0 );
    assert( xi_static_vector_find( instance->handles_and_fd, ( void* )( intptr_t ) fd, &xi_evtd_cmp_fd ) == -1 );

    // register new file descriptor
    xi_evtd_triplet_t* triplet = ( xi_evtd_triplet_t* ) xi_alloc( sizeof( xi_evtd_triplet_t ) );
    XI_CHECK_MEMORY( triplet );

    triplet->fd            = fd;
    triplet->event_type    = XI_EVTD_NO_EVENT;

    // register within the handles
    {
        const xi_static_vector_elem_t* e = xi_static_vector_push( instance->handles_and_fd, triplet );
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

void xi_evtd_continue(
      xi_evtd_instance_t* instance
    , xi_evtd_handle_t* handle
    , xi_heap_key_type_t time_diff )
{
    xi_heap_element_add(
          instance->call_heap
        , instance->current_step + time_diff
        , handle );
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

void xi_evtd_destroy_instance( xi_evtd_instance_t* instance )
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
    }
}

void xi_evtd_step(
      xi_evtd_instance_t* evtd_instance
    , xi_heap_key_type_t new_step )
{
    evtd_instance->current_step  = new_step;
    const xi_heap_element_t* tmp = 0;

    while( !xi_heap_is_empty( evtd_instance->call_heap ) )
    {
        tmp = xi_heap_peek_top( evtd_instance->call_heap );
        if( tmp->key <= evtd_instance->current_step )
        {
            tmp = xi_heap_get_top( evtd_instance->call_heap );
            xi_evtd_handle_t* handle = ( xi_evtd_handle_t* ) tmp->value;
            xi_evtd_execute_handle( handle );
        }
        else
        {
            break;
        }
    }
}

/**
 * \brief update events triggers registration of continuations assigned to the given event on given device
 * \note events_mask is the mask that's created using | operator
 */
void xi_evtd_update_events(
      xi_evtd_instance_t* instance
    , xi_static_vector_t* fds )
{
    for( int32_t i = 0; i < fds->elem_no; ++i )
    {
        xi_fd_t fd = ( xi_fd_t )( intptr_t ) fds->array[ i ].value;

        xi_static_vector_index_type_t id
            = xi_static_vector_find(
                  instance->handles_and_fd
                , ( void* )( intptr_t ) fd
                , &xi_evtd_cmp_fd );

        if( id != -1 )
        {
            xi_evtd_triplet_t* triplet
                = ( xi_evtd_triplet_t* ) instance->handles_and_fd->array[ id ].value;

            xi_evtd_execute_handle( &triplet->handle );
        }
        else
        {
            assert( 1 == 0 );
        }
    }
}
