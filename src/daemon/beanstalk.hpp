#pragma once

#include "beanstalk.h"
#include <string>
#include <vector>
#include <map>

namespace Beanstalk {
    typedef std::vector<std::string> info_list_t;
    typedef std::map<std::string, std::string> info_hash_t;

    class Job {
        public:
            int64_t id();
            std::string& body();
            Job(int64_t, char*, size_t);
            Job(BSJ*);
            Job();
            operator bool() { return _id > 0; }
        protected:
            int64_t _id;
            std::string _body;
    };

    class Client {
        public:
            ~Client();
            Client();
            Client(std::string host, int port, float timeout_secs = 0);
            bool ping();
            bool use(std::string);
            bool watch(std::string);
            bool ignore(std::string);
            int64_t put(std::string, uint32_t priority = 0, uint32_t delay = 0, uint32_t ttr = 60);
            int64_t put(char *data, size_t bytes, uint32_t priority, uint32_t delay, uint32_t ttr);
            bool del(int64_t id);
            bool del(Job&);
            bool reserve(Job &);
            bool reserve(Job &, uint32_t timeout);
            bool release(Job &, uint32_t priority = 1, uint32_t delay = 0);
            bool release(int64_t id, uint32_t priority = 1, uint32_t delay = 0);
            bool bury(Job &, uint32_t priority = 1);
            bool bury(int64_t id, uint32_t priority = 1);
            bool touch(Job &);
            bool touch(int64_t id);
            bool peek(Job &, int64_t id);
            bool peek_ready(Job &);
            bool peek_delayed(Job &);
            bool peek_buried(Job &);
            bool kick(int bound);
            void connect(std::string host, int port, float timeout_secs = 0);
            void reconnect();
            bool disconnect();
            void version(int *major, int *minor, int *patch);
            bool is_connected();
            std::string list_tube_used();
            info_list_t list_tubes();
            info_list_t list_tubes_watched();
            info_hash_t stats();
            info_hash_t stats_job(int64_t);
            info_hash_t stats_tube(std::string);
        protected:
            float timeout_secs;
            int handle, port;
            std::string host;
    };
}
