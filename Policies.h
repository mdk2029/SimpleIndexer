#pragma once

#include <mutex>

namespace SimpleIndexer {
    
    struct NopLockingPolicy {
        
        void lock()   {}
        void unlock() {}
        bool try_lock() { return true; }
        
    };
    
    class MutexLockingPolicy {
        std::mutex mutex_;
    public:
        void lock()     { mutex_.lock(); }
        void unlock()   { mutex_.unlock(); }
        bool try_lock() { return mutex_.try_lock(); }
    };
    
};