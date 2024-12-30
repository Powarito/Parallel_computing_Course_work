#pragma once

#include <shared_mutex>
#include <atomic>

class w_prioritized_shared_mutex {
private:
    std::shared_mutex mutex;
    std::atomic<int> pending_writers = 0;

public:
    w_prioritized_shared_mutex() = default;
    w_prioritized_shared_mutex(const w_prioritized_shared_mutex& other) = delete;
    w_prioritized_shared_mutex(w_prioritized_shared_mutex&& other) = default;
    w_prioritized_shared_mutex& operator=(const w_prioritized_shared_mutex& other) = delete;
    w_prioritized_shared_mutex& operator=(w_prioritized_shared_mutex&& other) = default;

public:
    inline void lock_shared() {
        while (pending_writers.load(std::memory_order_acquire) > 0) {
            std::this_thread::yield();
        }
        mutex.lock_shared();
    }

    inline void unlock_shared() {
        mutex.unlock_shared();
    }

    inline void lock() {
        pending_writers.fetch_add(1, std::memory_order_acq_rel);
        mutex.lock();
    }

    inline void unlock() {
        mutex.unlock();
        pending_writers.fetch_sub(1, std::memory_order_acq_rel);
    }
};


using read_write_lock = std::shared_mutex;
using read_lock = std::shared_lock<read_write_lock>;
using write_lock = std::unique_lock<read_write_lock>;
