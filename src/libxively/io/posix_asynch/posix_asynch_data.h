#ifndef __POSIX_ASYNCH_DATA_H__
#define __POSIX_ASYNCH_DATA_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int         socket_fd;
    uint16_t    cs;
} posix_asynch_data_t;

#ifdef __cplusplus
}
#endif

#endif // __POSIX_ASYNCH_DATA_H__
