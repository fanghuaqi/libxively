#include "xi_connection_data.h"
#include "xi_helpers.h"

xi_connection_data_t* xi_alloc_connection_data(
      const char* host
    , int port )
{
    xi_connection_data_t* ret = xi_alloc( sizeof( xi_connection_data_t ) );
    XI_CHECK_MEMORY( ret );
    memset( ret, 0, sizeof( xi_connection_data_t ) );

    ret->address    = xi_str_dup( host );
    XI_CHECK_MEMORY( ret->address );

    ret->port       = port;

    return ret;

err_handling:
    if( ret )
    {
        XI_SAFE_FREE( ret->address );
    }
    XI_SAFE_FREE( ret );
    return 0;
}

extern void xi_free_connection_data(
    xi_connection_data_t* data )
{
    if( data )
    {
        XI_SAFE_FREE( data->address );
    }

    XI_SAFE_FREE( data );
}
