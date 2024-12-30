#pragma once

#include <queue>
#include <thread>
#include <shared_mutex>
#include "concurrent_utility.h"

template <typename T>
class concurrent_queue {
public:
    inline concurrent_queue() = default;
    inline ~concurrent_queue() { clear(); }

    inline concurrent_queue(const concurrent_queue& other) = delete;
    inline concurrent_queue(concurrent_queue&& other) = delete;
    inline concurrent_queue& operator=(const concurrent_queue& rhs) = delete;
    inline concurrent_queue& operator=(concurrent_queue&& rhs) = delete;

public:
    inline bool empty() const;
    inline std::size_t size() const;

    inline void clear();
    inline bool pop(T& value);
    inline bool pop();

    template <typename... arguments>
    inline void emplace(arguments&&... parameters);

private:
    using concurrent_queue_implementation = std::queue<T>;

    mutable read_write_lock rw_lock;
    concurrent_queue_implementation queue_impl;
};


template <typename T>
inline bool concurrent_queue<T>::empty() const {
    read_lock r_lock(rw_lock);
    return queue_impl.empty();
}

template <typename T>
inline std::size_t concurrent_queue<T>::size() const {
    read_lock r_lock(rw_lock);
    return queue_impl.size();
}

template <typename T>
inline void concurrent_queue<T>::clear() {
    write_lock w_lock(rw_lock);
    queue_impl = concurrent_queue_implementation();
    //while (!queue_impl.empty()) {
    //	queue_impl.pop();
    //}
}

template <typename T>
inline bool concurrent_queue<T>::pop(T& value) {
    write_lock w_lock(rw_lock);

    if (queue_impl.empty()) {
        return false;
    }
    else {
        value = std::move(queue_impl.front());
        queue_impl.pop();
        return true;
    }
}

template <typename T>
inline bool concurrent_queue<T>::pop() {
    write_lock w_lock(rw_lock);

    if (queue_impl.empty()) {
        return false;
    }
    else {
        queue_impl.pop();
        return true;
    }
}

template <typename T>
template <typename... arguments>
inline void concurrent_queue<T>::emplace(arguments&&... parameters) {
    write_lock w_lock(rw_lock);
    queue_impl.emplace(std::forward<arguments>(parameters)...);
}
