#include "xi_connection_data.h"
#include "xi_helpers.h"

xi_connection_data_t* xi_alloc_connection_data(
      const char* host
    , int port
    , const char* username
    , const char* password )
{
    xi_connection_data_t* ret = xi_alloc( sizeof( xi_connection_data_t ) );
    XI_CHECK_MEMORY( ret );
    memset( ret, 0, sizeof( xi_connection_data_t ) );

    ret->host = xi_str_dup( host );
    XI_CHECK_MEMORY( ret->host );

    if( username )
    {
        ret->username   = xi_str_dup( username );
        XI_CHECK_MEMORY( ret->username );
    }

    if( password )
    {
        ret->password   = xi_str_dup( password );
        XI_CHECK_MEMORY( ret->password );
    }

    ret->port = port;

    return ret;

err_handling:
    if( ret )
    {
        XI_SAFE_FREE( ret->host );
        XI_SAFE_FREE( ret->username );
        XI_SAFE_FREE( ret->password );
    }
    XI_SAFE_FREE( ret );
    return 0;
}

extern void xi_free_connection_data(
    xi_connection_data_t* data )
{
    if( data )
    {
        XI_SAFE_FREE( data->host );
        XI_SAFE_FREE( data->username );
        XI_SAFE_FREE( data->password );
    }

    XI_SAFE_FREE( data );
}
