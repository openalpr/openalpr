#ifndef SAFE_QUEUE_H_
#define SAFE_QUEUE_H_

#include <queue>
#include "support/tinythread.h"

template <typename T>
class SafeQueue
{
    public:
        T pop() 
        {
            tthread::lock_guard<tthread::mutex> mlock(_mutex);
            while (_queue.empty()) {
                _cond.wait(_mutex);
            }
            T val = _queue.front();
            _queue.pop();
            return val;
        }

        void push(const T& item)
        {
            tthread::lock_guard<tthread::mutex> mlock(_mutex);
            _queue.push(item);
            _cond.notify_one();
        }

        bool empty()
        {
            return _queue.empty();
        }

    private:
        std::queue<T> _queue;
        tthread::mutex _mutex;
        tthread::condition_variable _cond;
};

#endif
