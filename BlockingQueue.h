#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>

namespace SimpleIndexer {

    //This Queue sits between the directory traverser and the indexing threads.
    //
    //To signal end of stream, the producer enqueues empty work items into the queue. Once a consumer retrieves an
    // empty item, it will not dequeue anything further from the queue. Due to the batch reading and 
    //multiple consumers, the producer should take care to put batchSize() * numIndexingThreads sentinel items
    //to make sure that all consumers see the end of stream
    
    template<class T>
    class BlockingQueue {
        std::deque<T> q_;
        std::mutex qLock_;
        std::condition_variable notFull_;
        std::condition_variable notEmpty_;

        const size_t capacity_;

    public:

        explicit BlockingQueue(size_t capacity = 16*1024) : capacity_(capacity){
        }

        bool isFull() const {
            return q_.size() == capacity_;
        }

        bool isEmpty() const {
            return q_.empty();
        }

        void put(T&& val) {
            {
                std::unique_lock<std::mutex> ul(qLock_);
                if (isFull())
                    notFull_.wait(ul, [this] {
                        return !isFull();
                    });

                q_.push_back(std::move(val));
            }
            notEmpty_.notify_one();
        }

        template<class OutputIterator>
        void take(OutputIterator out, size_t n = 1) {
            {
                std::unique_lock<std::mutex> ul(qLock_);
                if (isEmpty()) {
                    notEmpty_.wait(ul, [this] {
                        return !isEmpty();
                    });
                }

                for(size_t i = 0; i < n && !isEmpty(); i++) {
                        *out++ = std::move(q_.front());
                        q_.pop_front();
                }
            }
            notFull_.notify_one();
        }
    };
}
