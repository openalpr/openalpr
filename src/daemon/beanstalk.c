#include "beanstalk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <inttypes.h>
#include <poll.h>

#define BS_STATUS_IS(message, code) strncmp(message, code, strlen(code)) == 0

#define BS_MESSAGE_NO_BODY  0
#define BS_MESSAGE_HAS_BODY 1

#ifndef BS_READ_CHUNK_SIZE
#define BS_READ_CHUNK_SIZE  4096
#endif

#define DATA_PENDING (errno == EAGAIN || errno == EWOULDBLOCK)

const char *bs_status_verbose[] = {
    "Success",
    "Operation failed",
    "Expected CRLF",
    "Job too big",
    "Queue draining",
    "Timed out",
    "Not found",
    "Deadline soon",
    "Buried",
    "Not ignored"
};

const char bs_resp_using[]          = "USING";
const char bs_resp_watching[]       = "WATCHING";
const char bs_resp_inserted[]       = "INSERTED";
const char bs_resp_buried[]         = "BURIED";
const char bs_resp_expected_crlf[]  = "EXPECTED_CRLF";
const char bs_resp_job_too_big[]    = "JOB_TOO_BIG";
const char bs_resp_draining[]       = "DRAINING";
const char bs_resp_reserved[]       = "RESERVED";
const char bs_resp_deadline_soon[]  = "DEADLINE_SOON";
const char bs_resp_timed_out[]      = "TIMED_OUT";
const char bs_resp_deleted[]        = "DELETED";
const char bs_resp_not_found[]      = "NOT_FOUND";
const char bs_resp_released[]       = "RELEASED";
const char bs_resp_touched[]        = "TOUCHED";
const char bs_resp_not_ignored[]    = "NOT_IGNORED";
const char bs_resp_found[]          = "FOUND";
const char bs_resp_kicked[]         = "KICKED";
const char bs_resp_ok[]             = "OK";

const char* bs_status_text(int code) {
    unsigned int cindex = (unsigned int) abs(code);
    return (cindex > sizeof(bs_status_verbose) / sizeof(char*)) ? 0 : bs_status_verbose[cindex];
}

int bs_resolve_address(char *host, int port, struct sockaddr_in *server) {
    char service[64];
    struct addrinfo *addr, *rec;

    snprintf(service, 64, "%d", port);
    if (getaddrinfo(host, service, 0, &addr) != 0)
        return BS_STATUS_FAIL;

    for (rec = addr; rec != 0; rec = rec->ai_next) {
        if (rec->ai_family == AF_INET) {
            memcpy(server, rec->ai_addr, sizeof(*server));
            break;
        }
    }

    freeaddrinfo(addr);
    return BS_STATUS_OK;
}

int bs_connect(char *host, int port) {
    int fd, state = 1;
    struct sockaddr_in server;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0 || bs_resolve_address(host, port, &server) < 0)
        return BS_STATUS_FAIL;

    if (connect(fd, (struct sockaddr*)&server, sizeof(server)) != 0) {
        close(fd);
        return BS_STATUS_FAIL;
    }

    /* disable nagle - we buffer in the application layer */
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &state, sizeof(state));
    return fd;
}

int bs_connect_with_timeout(char *host, int port, float secs) {
    struct sockaddr_in server;
    int fd, res, option, state = 1;
    socklen_t option_length;
    struct pollfd pfd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0 || bs_resolve_address(host, port, &server) < 0)
        return BS_STATUS_FAIL;

    // Set non-blocking
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, NULL) | O_NONBLOCK);

    res = connect(fd, (struct sockaddr*)&server, sizeof(server));
    if (res < 0) {
        if (errno == EINPROGRESS) {
            // Init poll structure
            pfd.fd = fd;
            pfd.events = POLLOUT;

            if (poll(&pfd, 1, (int)(secs*1000)) > 0) {
                option_length = sizeof(int);
                getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)(&option), &option_length);
                if (option) {
                    close(fd);
                    return BS_STATUS_FAIL;
                }
            } else {
                close(fd);
                return BS_STATUS_FAIL;
            }
        } else {
            close(fd);
            return BS_STATUS_FAIL;
        }
    }

    // Set to blocking mode
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, NULL) & ~(O_NONBLOCK));

    /* disable nagle - we buffer in the application layer */
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &state, sizeof(state));
    return fd;
}

int bs_disconnect(int fd) {
    close(fd);
    return BS_STATUS_OK;
}

void bs_free_message(BSM* m) {
    if (m->status)
        free(m->status);
    if (m->data)
        free(m->data);
    free(m);
}

void bs_free_job(BSJ *job) {
    if (job->data)
        free(job->data);
    free(job);
}

// optional polling
bs_poll_function bs_poll = 0;

void bs_start_polling(bs_poll_function f) {
    bs_poll = f;
}

void bs_reset_polling() {
    bs_poll = 0;
}

BSM* bs_recv_message(int fd, int expect_data) {
    char *token, *data;
    size_t bytes, data_size, status_size, status_max = 512, expect_data_bytes = 0;
    ssize_t ret;
    BSM *message = (BSM*)calloc(1, sizeof(BSM));
    if (!message) return 0;

    message->status = (char*)calloc(1, status_max);
    if (!message->status) {
        bs_free_message(message);
        return 0;
    }

    // poll until ready to read
    if (bs_poll) bs_poll(1, fd);
    ret = recv(fd, message->status, status_max - 1, 0);
    if (ret < 0) {
        bs_free_message(message);
        return 0;
    } else {
      bytes = (size_t) ret;
    }

    token = strstr(message->status, "\r\n");
    if (!token) {
        bs_free_message(message);
        return 0;
    }

    *token        = 0;
    status_size   = (size_t) (token - message->status);

    if (expect_data) {
        token  = rindex(message->status, ' ');
        expect_data_bytes = token ? strtoul(token + 1, NULL, 10) : 0;
    }

    if (!expect_data || expect_data_bytes == 0)
        return message;

    message->size = bytes - status_size - 2;
    data_size     = message->size > BS_READ_CHUNK_SIZE ? message->size + BS_READ_CHUNK_SIZE : BS_READ_CHUNK_SIZE;

    message->data = (char*)malloc(data_size);
    if (!message->data) {
        bs_free_message(message);
        return 0;
    }

    memcpy(message->data, message->status + status_size + 2, message->size);
    data = message->data + message->size;

    // already read the body along with status, all good.
    if ((expect_data_bytes + 2) <= message->size) {
        message->size = expect_data_bytes;
        return message;
    }

    while (1) {
        // poll until ready to read.
        if (bs_poll) bs_poll(1, fd);
        ret = recv(fd, data, data_size - message->size, 0);
        if (ret < 0) {
            if (bs_poll && DATA_PENDING)
                continue;
            else {
                bs_free_message(message);
                return 0;
            }
        } else {
          bytes = (size_t) ret;
        }

        // doneski, we have read enough bytes + \r\n
        if (message->size + bytes >= expect_data_bytes + 2) {
            message->size = expect_data_bytes;
            break;
        }

        data_size      += BS_READ_CHUNK_SIZE;
        message->size  += bytes;
        message->data   = (char*)realloc(message->data, data_size);
        if (!message->data) {
            bs_free_message(message);
            return 0;
        }

        // move ahead pointer for reading more.
        data = message->data + message->size;
    }

    return message;
}

ssize_t bs_send_message(int fd, char *message, size_t size) {
    // poll until ready to write.
    if (bs_poll) bs_poll(2, fd);
    return send(fd, message, size, bs_poll ? MSG_DONTWAIT : 0);
}

typedef struct bs_message_packet {
    char *data;
    size_t offset;
    size_t size;
} BSMP;

BSMP* bs_message_packet_new(size_t bytes) {
    BSMP *packet = (BSMP*)malloc(sizeof(BSMP));
    assert(packet);

    packet->data = (char*)malloc(bytes);
    assert(packet->data);

    packet->offset = 0;
    packet->size   = bytes;

    return packet;
}

void bs_message_packet_append(BSMP *packet, char *data, size_t bytes) {
    if (packet->offset + bytes > packet->size) {
        packet->data = (char*)realloc(packet->data, packet->size + bytes);
        assert(packet->data);
        packet->size += bytes;
    }

    memcpy(packet->data + packet->offset, data, bytes);
    packet->offset += bytes;
}

void bs_message_packet_free(BSMP *packet) {
    free(packet->data);
    free(packet);
}

#define BS_SEND(fd, command, size) {                        \
    if (bs_send_message(fd, command, size) < 0)             \
        return BS_STATUS_FAIL;                              \
}

#define BS_CHECK_MESSAGE(message) {                         \
    if (!message)                                           \
        return BS_STATUS_FAIL;                              \
}

#define BS_RETURN_OK_WHEN(message, okstatus) {              \
    if (BS_STATUS_IS(message->status, okstatus)) {          \
        bs_free_message(message);                           \
        return BS_STATUS_OK;                                \
    }                                                       \
}

#define BS_RETURN_FAIL_WHEN(message, nokstatus, nokcode) {  \
    if (BS_STATUS_IS(message->status, nokstatus)) {         \
        bs_free_message(message);                           \
        return nokcode;                                     \
    }                                                       \
}

#define BS_RETURN_INVALID(message) {                        \
    bs_free_message(message);                               \
    return BS_STATUS_FAIL;                                  \
}

int bs_use(int fd, char *tube) {
    BSM *message;
    char command[1024];

    snprintf(command, 1024, "use %s\r\n", tube);
    BS_SEND(fd, command, strlen(command));

    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_using);
    BS_RETURN_INVALID(message);
}

int bs_watch(int fd, char *tube) {
    BSM *message;
    char command[1024];

    snprintf(command, 1024, "watch %s\r\n", tube);
    BS_SEND(fd, command, strlen(command));

    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_watching);
    BS_RETURN_INVALID(message);
}

int bs_ignore(int fd, char *tube) {
    BSM *message;
    char command[1024];

    snprintf(command, 1024, "ignore %s\r\n", tube);
    BS_SEND(fd, command, strlen(command));

    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_watching);
    BS_RETURN_INVALID(message);
}

int64_t bs_put(int fd, uint32_t priority, uint32_t delay, uint32_t ttr, char *data, size_t bytes) {
    int64_t id;
    BSMP *packet;
    BSM *message;
    char command[1024];
    size_t command_bytes;

    snprintf(command, 1024, "put %"PRIu32" %"PRIu32" %"PRIu32" %lu\r\n", priority, delay, ttr, bytes);

    command_bytes = strlen(command);
    packet = bs_message_packet_new(command_bytes + bytes + 3);
    bs_message_packet_append(packet, command, command_bytes);
    bs_message_packet_append(packet, data, bytes);
    bs_message_packet_append(packet, "\r\n", 2);

    // Can't use BS_SEND here, allocated memory needs to 
    // be cleared on error
    int ret_code = bs_send_message(fd, packet->data, packet->offset);
    bs_message_packet_free(packet);
    if (ret_code <0) {
        return BS_STATUS_FAIL;
    }

    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);

    if (BS_STATUS_IS(message->status, bs_resp_inserted)) {
      id = strtoll(message->status + strlen(bs_resp_inserted) + 1, NULL, 10);
        bs_free_message(message);
        return id;
    }

    if (BS_STATUS_IS(message->status, bs_resp_buried)) {
      id = strtoll(message->status + strlen(bs_resp_buried) + 1, NULL, 10);
        bs_free_message(message);
        return id;
    }

    BS_RETURN_FAIL_WHEN(message, bs_resp_expected_crlf, BS_STATUS_EXPECTED_CRLF);
    BS_RETURN_FAIL_WHEN(message, bs_resp_job_too_big,   BS_STATUS_JOB_TOO_BIG);
    BS_RETURN_FAIL_WHEN(message, bs_resp_draining,      BS_STATUS_DRAINING);
    BS_RETURN_INVALID(message);
}

int bs_delete(int fd, int64_t job) {
    BSM *message;
    char command[512];

    snprintf(command, 512, "delete %"PRId64"\r\n", job);
    BS_SEND(fd, command, strlen(command));

    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_deleted);
    BS_RETURN_FAIL_WHEN(message, bs_resp_not_found, BS_STATUS_NOT_FOUND);
    BS_RETURN_INVALID(message);
}

int bs_reserve_job(int fd, char *command, BSJ **result) {
    BSJ *job;
    BSM *message;

//  XXX: debug
//  struct timeval start, end;
//  gettimeofday(&start, 0);

    BS_SEND(fd, command, strlen(command));
    message = bs_recv_message(fd, BS_MESSAGE_HAS_BODY);
    BS_CHECK_MESSAGE(message);

    if (BS_STATUS_IS(message->status, bs_resp_reserved)) {
        *result = job = (BSJ*)malloc(sizeof(BSJ));
        if (!job) {
            bs_free_message(message);
            return BS_STATUS_FAIL;
        }

        sscanf(message->status + strlen(bs_resp_reserved) + 1, "%"PRId64" %lu", &job->id, &job->size);
        job->data      = message->data;
        message->data  = 0;
        bs_free_message(message);

    //  XXX: debug
    //  gettimeofday(&end, 0);
    //  printf("elapsed: %lu\n", (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
        return BS_STATUS_OK;
    }

    // i don't think we'll ever hit this status code here.
    BS_RETURN_FAIL_WHEN(message, bs_resp_timed_out,     BS_STATUS_TIMED_OUT);
    BS_RETURN_FAIL_WHEN(message, bs_resp_deadline_soon, BS_STATUS_DEADLINE_SOON);
    BS_RETURN_INVALID(message);
}

int bs_reserve(int fd, BSJ **result) {
    char *command = "reserve\r\n";
    return bs_reserve_job(fd, command, result);
}

int bs_reserve_with_timeout(int fd, uint32_t ttl, BSJ **result) {
    char command[512];
    snprintf(command, 512, "reserve-with-timeout %"PRIu32"\r\n", ttl);
    return bs_reserve_job(fd, command, result);
}

int bs_release(int fd, int64_t id, uint32_t priority, uint32_t delay) {
    BSM *message;
    char command[512];

    snprintf(command, 512, "release %"PRId64" %"PRIu32" %"PRIu32"\r\n", id, priority, delay);
    BS_SEND(fd, command, strlen(command));

    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_released);
    BS_RETURN_FAIL_WHEN(message, bs_resp_buried,    BS_STATUS_BURIED);
    BS_RETURN_FAIL_WHEN(message, bs_resp_not_found, BS_STATUS_NOT_FOUND);
    BS_RETURN_INVALID(message);
}

int bs_bury(int fd, int64_t id, uint32_t priority) {
    BSM *message;
    char command[512];

    snprintf(command, 512, "bury %"PRId64" %"PRIu32"\r\n", id, priority);
    BS_SEND(fd, command, strlen(command));
    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_buried);
    BS_RETURN_FAIL_WHEN(message, bs_resp_not_found, BS_STATUS_NOT_FOUND);
    BS_RETURN_INVALID(message);
}

int bs_touch(int fd, int64_t id) {
    BSM *message;
    char command[512];

    snprintf(command, 512, "touch %"PRId64"\r\n", id);
    BS_SEND(fd, command, strlen(command));
    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_touched);
    BS_RETURN_FAIL_WHEN(message, bs_resp_not_found, BS_STATUS_NOT_FOUND);
    BS_RETURN_INVALID(message);
}

int bs_peek_job(int fd, char *command, BSJ **result) {
    BSJ *job;
    BSM *message;

    BS_SEND(fd, command, strlen(command));
    message = bs_recv_message(fd, BS_MESSAGE_HAS_BODY);
    BS_CHECK_MESSAGE(message);

    if (BS_STATUS_IS(message->status, bs_resp_found)) {
        *result = job = (BSJ*)malloc(sizeof(BSJ));
        if (!job) {
            bs_free_message(message);
            return BS_STATUS_FAIL;
        }

        sscanf(message->status + strlen(bs_resp_found) + 1, "%"PRId64" %lu", &job->id, &job->size);
        job->data      = message->data;
        message->data  = 0;
        bs_free_message(message);

        return BS_STATUS_OK;
    }

    BS_RETURN_FAIL_WHEN(message, bs_resp_not_found, BS_STATUS_NOT_FOUND);
    BS_RETURN_INVALID(message);
}

int bs_peek(int fd, int64_t id, BSJ **job) {
    char command[512];
    snprintf(command, 512, "peek %"PRId64"\r\n", id);
    return bs_peek_job(fd, command, job);
}

int bs_peek_ready(int fd, BSJ **job) {
    return bs_peek_job(fd, "peek-ready\r\n", job);
}

int bs_peek_delayed(int fd, BSJ **job) {
    return bs_peek_job(fd, "peek-delayed\r\n", job);
}

int bs_peek_buried(int fd, BSJ **job) {
    return bs_peek_job(fd, "peek-buried\r\n", job);
}

int bs_kick(int fd, int bound) {
    BSM *message;
    char command[512];

    snprintf(command, 512, "kick %d\r\n", bound);
    BS_SEND(fd, command, strlen(command));
    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    BS_CHECK_MESSAGE(message);
    BS_RETURN_OK_WHEN(message, bs_resp_kicked);
    BS_RETURN_INVALID(message);
}

int bs_list_tube_used(int fd, char **tube) {
    BSM *message;
    char command[64];
    snprintf(command, 64, "list-tube-used\r\n");
    BS_SEND(fd, command, strlen(command));
    message = bs_recv_message(fd, BS_MESSAGE_NO_BODY);
    if (BS_STATUS_IS(message->status, bs_resp_using)) {
        *tube = (char*)calloc(1, strlen(message->status) - strlen(bs_resp_using) + 1);
        strcpy(*tube, message->status + strlen(bs_resp_using) + 1);
        bs_free_message(message);
        return BS_STATUS_OK;
    }
    BS_RETURN_INVALID(message);
}

int bs_get_info(int fd, char *command, char **yaml) {
    BSM *message;
    size_t size;

    BS_SEND(fd, command, strlen(command));
    message = bs_recv_message(fd, BS_MESSAGE_HAS_BODY);
    BS_CHECK_MESSAGE(message);
    if (BS_STATUS_IS(message->status, bs_resp_ok)) {
        sscanf(message->status + strlen(bs_resp_ok) + 1, "%lu", &size);
        *yaml = message->data;
        (*yaml)[size] = 0;
        message->data = 0;
        bs_free_message(message);
        return BS_STATUS_OK;
    }

    BS_RETURN_INVALID(message);
}

int bs_list_tubes(int fd, char **yaml) {
    char command[64];
    snprintf(command, 64, "list-tubes\r\n");
    return bs_get_info(fd, command, yaml);
}

int bs_list_tubes_watched(int fd, char **yaml) {
    char command[64];
    snprintf(command, 64, "list-tubes-watched\r\n");
    return bs_get_info(fd, command, yaml);
}

int bs_stats(int fd, char **yaml) {
    char command[64];
    snprintf(command, 64, "stats\r\n");
    return bs_get_info(fd, command, yaml);
}

int bs_stats_job(int fd, int64_t id, char **yaml) {
    char command[128];
    snprintf(command, 128, "stats-job %"PRId64"\r\n", id);
    return bs_get_info(fd, command, yaml);
}

int bs_stats_tube(int fd, char *tube, char **yaml) {
    char command[512];
    snprintf(command, 512, "stats-tube %s\r\n", tube);
    return bs_get_info(fd, command, yaml);
}

void bs_version(int *major, int *minor, int *patch)
{
    *major = BS_MAJOR_VERSION;
    *minor = BS_MINOR_VERSION;
    *patch = BS_PATCH_VERSION;
}
