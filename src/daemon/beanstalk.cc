#include "beanstalk.hpp"
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>

using namespace std;

namespace Beanstalk {

    Job::Job() {
        _id = 0;
    }

    Job::Job(int64_t id, char *data, size_t size) {
        _body.assign(data, size);
        _id = id;
    }

    Job::Job(BSJ *job) {
        if (job) {
            _body.assign(job->data, job->size);
            _id = job->id;
            bs_free_job(job);
        }
        else {
            _id = 0;
        }
    }

    string& Job::body() {
        return _body;
    }

    int64_t Job::id() {
        return _id;
    }

    /* start helpers */

    void parsedict(stringstream &stream, info_hash_t &dict) {
        string key, value;
        while(true) {
            stream >> key;
            if (stream.eof()) break;
            if (key[0] == '-') continue;
            stream >> value;
            key.erase(--key.end());
            dict[key] = value;
        }
    }

    void parselist(stringstream &stream, info_list_t &list) {
        string value;
        while(true) {
            stream >> value;
            if (stream.eof()) break;
            if (value[0] == '-') continue;
            list.push_back(value);
        }
    }

    /* end helpers */

    Client::~Client() {
        if (handle > 0)
          bs_disconnect(handle);
        handle = -1;
    }

    Client::Client() {
        handle       = -1;
        host         = "";
        port         = 0;
        timeout_secs = 0;
    }

    Client::Client(string host, int port, float secs) {
        handle       = -1;
        timeout_secs = secs;
        connect(host, port, timeout_secs);
    }

    void Client::connect(string _host, int _port, float secs) {
        if (handle > 0)
            throw runtime_error("already connected to beanstalkd at " + host);

        host         = _host;
        port         = _port;
        timeout_secs = secs;

        handle = secs > 0 ? bs_connect_with_timeout((char *)host.c_str(), port, secs) : bs_connect((char*)host.c_str(), port);
        if (handle < 0)
            throw runtime_error("unable to connect to beanstalkd at " + host);
    }

    bool Client::is_connected() {
      return handle > 0;
    }

    bool Client::disconnect() {
        if (handle > 0 && bs_disconnect(handle) == BS_STATUS_OK) {
            handle = -1;
            return true;
        }
        return false;
    }

    void Client::version(int *major, int *minor, int *patch) {
        bs_version(major, minor, patch);
    }

    void Client::reconnect() {
        disconnect();
        connect(host, port, timeout_secs);
    }

    bool Client::use(string tube) {
        return bs_use(handle, (char*)tube.c_str()) == BS_STATUS_OK;
    }

    bool Client::watch(string tube) {
        return bs_watch(handle, (char*)tube.c_str()) == BS_STATUS_OK;
    }

    bool Client::ignore(string tube) {
        return bs_ignore(handle, (char*)tube.c_str()) == BS_STATUS_OK;
    }

    int64_t Client::put(string body, uint32_t priority, uint32_t delay, uint32_t ttr) {
        int64_t id = bs_put(handle, priority, delay, ttr, (char*)body.data(), body.size());
        return (id > 0 ? id : 0);
    }

    int64_t Client::put(char *body, size_t bytes, uint32_t priority, uint32_t delay, uint32_t ttr) {
        int64_t id = bs_put(handle, priority, delay, ttr, body, bytes);
        return (id > 0 ? id : 0);
    }

    bool Client::del(Job &job) {
        return bs_delete(handle, job.id()) == BS_STATUS_OK;
    }

    bool Client::del(int64_t id) {
        return bs_delete(handle, id) == BS_STATUS_OK;
    }

    bool Client::reserve(Job &job) {
        BSJ *bsj;
        if (bs_reserve(handle, &bsj) == BS_STATUS_OK) {
            job = bsj;
            return true;
        }
        return false;
    }

    bool Client::reserve(Job &job, uint32_t timeout) {
        BSJ *bsj;
        if (bs_reserve_with_timeout(handle, timeout, &bsj) == BS_STATUS_OK) {
            job = bsj;
            return true;
        }
        return false;
    }

    bool Client::release(Job &job, uint32_t priority, uint32_t delay) {
        return bs_release(handle, job.id(), priority, delay) == BS_STATUS_OK;
    }

    bool Client::release(int64_t id, uint32_t priority, uint32_t delay) {
        return bs_release(handle, id, priority, delay) == BS_STATUS_OK;
    }

    bool Client::bury(Job &job, uint32_t priority) {
        return bs_bury(handle, job.id(), priority) == BS_STATUS_OK;
    }

    bool Client::bury(int64_t id, uint32_t priority) {
        return bs_bury(handle, id, priority) == BS_STATUS_OK;
    }

    bool Client::touch(Job &job) {
        return bs_touch(handle, job.id()) == BS_STATUS_OK;
    }

    bool Client::touch(int64_t id) {
        return bs_touch(handle, id) == BS_STATUS_OK;
    }

    bool Client::peek(Job &job, int64_t id) {
        BSJ *bsj;
        if (bs_peek(handle, id, &bsj) == BS_STATUS_OK) {
            job = bsj;
            return true;
        }
        return false;
    }

    bool Client::peek_ready(Job &job) {
        BSJ *bsj;
        if (bs_peek_ready(handle, &bsj) == BS_STATUS_OK) {
            job = bsj;
            return true;
        }
        return false;
    }

    bool Client::peek_delayed(Job &job) {
        BSJ *bsj;
        if (bs_peek_delayed(handle, &bsj) == BS_STATUS_OK) {
            job = bsj;
            return true;
        }
        return false;
    }

    bool Client::peek_buried(Job &job) {
        BSJ *bsj;
        if (bs_peek_buried(handle, &bsj) == BS_STATUS_OK) {
            job = bsj;
            return true;
        }
        return false;
    }

    bool Client::kick(int bound) {
        return bs_kick(handle, bound) == BS_STATUS_OK;
    }

    string Client::list_tube_used() {
        char *name;
        string tube;
        if (bs_list_tube_used(handle, &name) == BS_STATUS_OK) {
            tube.assign(name);
            free(name);
        }

        return tube;
    }

    info_list_t Client::list_tubes() {
        char *yaml, *data;
        info_list_t tubes;
        if (bs_list_tubes(handle, &yaml) == BS_STATUS_OK) {
            if ((data = strstr(yaml, "---"))) {
                stringstream stream(data);
                parselist(stream, tubes);
            }
            free(yaml);
        }
        return tubes;
    }

    info_list_t Client::list_tubes_watched() {
        char *yaml, *data;
        info_list_t tubes;
        if (bs_list_tubes_watched(handle, &yaml) == BS_STATUS_OK) {
            if ((data = strstr(yaml, "---"))) {
                stringstream stream(data);
                parselist(stream, tubes);
            }
            free(yaml);
        }
        return tubes;
    }

    info_hash_t Client::stats() {
        char *yaml, *data;
        info_hash_t stats;
        string key, value;
        if (bs_stats(handle, &yaml) == BS_STATUS_OK) {
            if ((data = strstr(yaml, "---"))) {
                stringstream stream(data);
                parsedict(stream, stats);
            }
            free(yaml);
        }
        return stats;
    }

    info_hash_t Client::stats_job(int64_t id) {
        char *yaml, *data;
        info_hash_t stats;
        string key, value;
        if (bs_stats_job(handle, id, &yaml) == BS_STATUS_OK) {
            if ((data = strstr(yaml, "---"))) {
                stringstream stream(data);
                parsedict(stream, stats);
            }
            free(yaml);
        }
        return stats;
    }

    info_hash_t Client::stats_tube(string name) {
        char *yaml, *data;
        info_hash_t stats;
        string key, value;
        if (bs_stats_tube(handle, (char*)name.c_str(), &yaml) == BS_STATUS_OK) {
            if ((data = strstr(yaml, "---"))) {
                stringstream stream(data);
                parsedict(stream, stats);
            }
            free(yaml);
        }
        return stats;
    }

    bool Client::ping() {
        char *yaml;
        if (bs_list_tubes(handle, &yaml) == BS_STATUS_OK) {
            free(yaml);
            return true;
        }

        return false;
    }
}
