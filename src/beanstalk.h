#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <netinet/in.h>
#include <sys/fcntl.h>

#define BS_MAJOR_VERSION  1
#define BS_MINOR_VERSION  2
#define BS_PATCH_VERSION  0

#define BS_STATUS_OK             0
#define BS_STATUS_FAIL          -1
#define BS_STATUS_EXPECTED_CRLF -2
#define BS_STATUS_JOB_TOO_BIG   -3
#define BS_STATUS_DRAINING      -4
#define BS_STATUS_TIMED_OUT     -5
#define BS_STATUS_NOT_FOUND     -6
#define BS_STATUS_DEADLINE_SOON -7
#define BS_STATUS_BURIED        -8
#define BS_STATUS_NOT_IGNORED   -9

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct bs_message {
    char *data;
    char *status;
    size_t size;
} BSM;

typedef struct bs_job {
    int64_t id;
    char *data;
    size_t size;
} BSJ;


// optional polling call, returns 1 if the socket is ready of the rw operation specified.
// rw: 1 => read, 2 => write, 3 => read/write
// fd: file descriptor of the socket
typedef int (*bs_poll_function)(int rw, int fd);

/*  Handle DSO symbol visibility - Stolen from zmq.h */
#if defined _WIN32
#   if defined DLL_EXPORT
#       define BSC_EXPORT __declspec(dllexport)
#   else
#       define BSC_EXPORT __declspec(dllimport)
#   endif
#else
#   if defined __SUNPRO_C  || defined __SUNPRO_CC
#       define BSC_EXPORT __global
#   elif (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#       define BSC_EXPORT __attribute__ ((visibility("default")))
#   else
#       define BSC_EXPORT
#   endif
#endif

// export version
BSC_EXPORT void bs_version(int *major, int *minor, int *patch);

// polling setup
BSC_EXPORT void bs_start_polling(bs_poll_function f);
BSC_EXPORT void bs_reset_polling(void);

// returns a descriptive text of the error code.
BSC_EXPORT const char* bs_status_text(int code);

BSC_EXPORT void bs_free_message(BSM* m);
BSC_EXPORT void bs_free_job(BSJ *job);

// returns socket descriptor or BS_STATUS_FAIL
BSC_EXPORT int bs_connect(char *host, int port);
BSC_EXPORT int bs_connect_with_timeout(char *host, int port, float secs);

// returns job id or one of the negative failure codes.
BSC_EXPORT int64_t bs_put(int fd, uint32_t priority, uint32_t delay, uint32_t ttr, char *data, size_t bytes);

// rest return BS_STATUS_OK or one of the failure codes.
BSC_EXPORT int bs_disconnect(int fd);
BSC_EXPORT int bs_use(int fd, char *tube);
BSC_EXPORT int bs_watch(int fd, char *tube);
BSC_EXPORT int bs_ignore(int fd, char *tube);
BSC_EXPORT int bs_delete(int fd, int64_t job);
BSC_EXPORT int bs_reserve(int fd, BSJ **job);
BSC_EXPORT int bs_reserve_with_timeout(int fd, uint32_t ttl, BSJ **job);
BSC_EXPORT int bs_release(int fd, int64_t id, uint32_t priority, uint32_t delay);
BSC_EXPORT int bs_bury(int fd, int64_t id, uint32_t priority);
BSC_EXPORT int bs_touch(int fd, int64_t id);
BSC_EXPORT int bs_peek(int fd, int64_t id, BSJ **job);
BSC_EXPORT int bs_peek_ready(int fd, BSJ **job);
BSC_EXPORT int bs_peek_delayed(int fd, BSJ **job);
BSC_EXPORT int bs_peek_buried(int fd, BSJ **job);
BSC_EXPORT int bs_kick(int fd, int bound);
BSC_EXPORT int bs_list_tube_used(int fd, char **tube);
BSC_EXPORT int bs_list_tubes(int fd, char **yaml);
BSC_EXPORT int bs_list_tubes_watched(int fd, char **yaml);
BSC_EXPORT int bs_stats(int fd, char **yaml);
BSC_EXPORT int bs_stats_job(int fd, int64_t id, char **yaml);
BSC_EXPORT int bs_stats_tube(int fd, char *tube, char **yaml);

#ifdef __cplusplus
    }
#endif
