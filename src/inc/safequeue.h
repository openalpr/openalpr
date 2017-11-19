#ifndef SAFE_QUEUE_H_
#define SAFE_QUEUE_H_

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class SafeQueue
{
    public:
        T pop() 
        {
            std::unique_lock<std::mutex> mlock(_mutex);
            while (_queue.empty()) {
                _cond.wait(mlock);
            }
            auto val = _queue.front();
            _queue.pop();
            return val;
        }

        void push(const T& item)
        {
            std::unique_lock<std::mutex> mlock(_mutex);
            _queue.push(item);
            mlock.unlock();
            _cond.notify_one();
        }

        bool empty()
        {
            return _queue.empty();
        }

        SafeQueue() = default;
        // Disable copying and assignments
        SafeQueue(const SafeQueue&) = delete;
        SafeQueue& operator=(const SafeQueue&) = delete;
  
    private:
        std::queue<T> _queue;
        std::mutex _mutex;
        std::condition_variable _cond;
};

#endif
