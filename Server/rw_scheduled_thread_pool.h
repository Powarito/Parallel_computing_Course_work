#pragma once

#include <vector>
#include <functional>

#include "concurrent_queue.h"

class rw_scheduled_thread_pool {
public:
    inline rw_scheduled_thread_pool() = default;
    inline ~rw_scheduled_thread_pool() { terminate(); }

    inline rw_scheduled_thread_pool(const rw_scheduled_thread_pool& other) = delete;
    inline rw_scheduled_thread_pool(rw_scheduled_thread_pool&& other) = delete;
    inline rw_scheduled_thread_pool& operator=(const rw_scheduled_thread_pool& rhs) = delete;
    inline rw_scheduled_thread_pool& operator=(rw_scheduled_thread_pool&& rhs) = delete;

public:
    inline void initialize(std::size_t worker_count, float writer_duration = 0.5f, float reader_duration = 5.0f, bool can_rw_interlap = false, bool start_with_writers = false);
    inline void terminate(bool immediately = false);

    inline void set_paused(const bool paused);
    inline bool is_paused() const;

    inline bool working() const;
    inline bool working_unsafe() const;

    inline bool set_reader_duration(float new_reader_duration);
    inline float get_reader_duration() const;

    inline bool set_writer_duration(float new_writer_duration);
    inline float get_writer_duration() const;

    template <typename task_t, typename... arguments>
    inline void add_reader_task(task_t&& task, arguments&&... parameters);

    template <typename task_t, typename... arguments>
    inline void add_writer_task(task_t&& task, arguments&&... parameters);

private:
    inline bool do_set_duration(float& target_duration, float new_duration);

    template <typename task_t, typename... arguments>
    inline void do_add_task(concurrent_queue<std::function<void()>>& target_queue, task_t&& task, arguments&&... parameters);

    inline void routine();

private:
    mutable read_write_lock                 rw_lock;
    mutable std::condition_variable_any     cv_task_waiter;
    std::vector<std::thread>                workers;

    concurrent_queue<std::function<void()>>	reader_tasks;
    concurrent_queue<std::function<void()>>	writer_tasks;

    bool initialized = false;
    bool terminated = false;
    bool paused = false;

private:
    std::thread timer_thread;
    std::condition_variable_any cv_timer_waiter;

    bool can_interlap;
    bool writer_flag;
    std::size_t readers_counter = 0;
    std::size_t writers_counter = 0;

    float reader_duration;
    float writer_duration;

    inline void timer_function();
};


inline void rw_scheduled_thread_pool::initialize(std::size_t worker_count, float writer_duration, float reader_duration, bool can_rw_interlap, bool start_with_writers) {
    write_lock w_lock(rw_lock);

    if (initialized || terminated) {
        return;
    }

    workers.reserve(worker_count);
    for (std::size_t id = 0; id < worker_count; ++id) {
        workers.emplace_back(&rw_scheduled_thread_pool::routine, this);
    }

    bool workers_not_empty = !workers.empty();
    initialized = workers_not_empty;

    if (workers_not_empty) {
        timer_thread = std::thread(&rw_scheduled_thread_pool::timer_function, this);
        this->reader_duration = reader_duration;
        this->writer_duration = writer_duration;
        can_interlap = can_rw_interlap;
        writer_flag = start_with_writers;
    }
}

inline void rw_scheduled_thread_pool::terminate(bool immediately) {
    {
        write_lock w_lock(rw_lock);

        if (working_unsafe()) {
            terminated = true;
            paused = false;

            if (immediately) {
                reader_tasks.clear();
                writer_tasks.clear();
            }
        }
        else {
            return;
        }
    }

    cv_task_waiter.notify_all();

    for (std::thread& worker : workers) {
        worker.join();
    }
    timer_thread.join();

    write_lock w_lock(rw_lock);

    workers.clear();

    terminated = false;
    initialized = false;
    paused = false;
}

inline void rw_scheduled_thread_pool::routine() {
    while (true) {
        bool task_accquiered = false;
        bool is_writer = false;
        std::function<void()> task;

        {
            write_lock w_lock(rw_lock);

            auto wait_condition = [this, &task_accquiered, &is_writer, &task] {
                if (paused) {
                    return false;
                }

                auto& target_queue = writer_flag ? writer_tasks : reader_tasks;
                std::size_t& other_counter = !writer_flag ? writers_counter : readers_counter;
                is_writer = writer_flag;

                if (can_interlap || other_counter == 0) {
                    task_accquiered = target_queue.pop(task);
                }

                if (terminated && can_interlap && !task_accquiered) {
                    auto& last_queue = !writer_flag ? writer_tasks : reader_tasks;
                    task_accquiered = last_queue.pop(task);
                }
                return terminated || task_accquiered;
            };

            cv_task_waiter.wait(w_lock, wait_condition);

            if (terminated && !task_accquiered) {
                return;
            }

            std::size_t& counter = is_writer ? writers_counter : readers_counter;
            ++counter;
        }

        task();

        {
            write_lock w_lock(rw_lock);
            std::size_t& counter = is_writer ? writers_counter : readers_counter;
            --counter;
        }

        cv_timer_waiter.notify_one();
    }
}

inline void rw_scheduled_thread_pool::set_paused(const bool paused) {
    write_lock w_lock(rw_lock);

    if (working_unsafe()) {
        this->paused = paused;

        if (!paused) {
            cv_task_waiter.notify_all();
        }
    }
}

inline bool rw_scheduled_thread_pool::is_paused() const {
    read_lock r_lock(rw_lock);
    return paused;
}

inline bool rw_scheduled_thread_pool::working() const {
    read_lock r_lock(rw_lock);
    return working_unsafe();
}

inline bool rw_scheduled_thread_pool::working_unsafe() const {
    return initialized && !terminated;
}

inline bool rw_scheduled_thread_pool::set_reader_duration(float new_reader_duration) {
    return do_set_duration(reader_duration, new_reader_duration);
}

inline bool rw_scheduled_thread_pool::set_writer_duration(float new_writer_duration) {
    return do_set_duration(writer_duration, new_writer_duration);
}

inline bool rw_scheduled_thread_pool::do_set_duration(float& target_duration, float new_duration) {
    if (new_duration < 0.5 - 0.001) {
        return false;
    }
    write_lock w_lock(rw_lock);
    target_duration = new_duration;
    return true;
}

inline float rw_scheduled_thread_pool::get_reader_duration() const {
    read_lock r_lock(rw_lock);
    return reader_duration;
}

inline float rw_scheduled_thread_pool::get_writer_duration() const {
    read_lock r_lock(rw_lock);
    return writer_duration;
}

template <typename task_t, typename... arguments>
inline void rw_scheduled_thread_pool::add_reader_task(task_t&& task, arguments&& ...parameters) {
    do_add_task(reader_tasks, std::forward<task_t>(task), std::forward<arguments>(parameters)...);
}

template<typename task_t, typename ...arguments>
inline void rw_scheduled_thread_pool::add_writer_task(task_t&& task, arguments&& ...parameters) {
    do_add_task(writer_tasks, std::forward<task_t>(task), std::forward<arguments>(parameters)...);
}

template<typename task_t, typename ...arguments>
inline void rw_scheduled_thread_pool::do_add_task(concurrent_queue<std::function<void()>>& target_queue, task_t&& task, arguments&& ...parameters) {
    {
        read_lock r_lock(rw_lock);
        if (!working_unsafe()) {
            return;
        }
    }

    auto bind = std::bind(std::forward<task_t>(task), std::forward<arguments>(parameters)...);

    target_queue.emplace(bind);
    cv_task_waiter.notify_one();
}

inline void rw_scheduled_thread_pool::timer_function() {
    while (true) {
        if (can_interlap) {
            float sleep_time = 1.0f;
            {
                read_lock r_lock(rw_lock);
                sleep_time = writer_flag ? writer_duration : reader_duration; // read_lock because one of the durations might get changed from the outside
            }
            int sleep_time_ms = static_cast<int>(sleep_time * 1000.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));

            {
                write_lock w_lock(rw_lock);
                if (terminated && writer_tasks.empty() && reader_tasks.empty()) {
                    return;
                }
                // If there's no tasks in the other queue - continue executing tasks from the current queue
                std::size_t other_tasks = !writer_flag ? writer_tasks.size() : reader_tasks.size();
                if (other_tasks > 0) {
                    writer_flag = !writer_flag;
                }
            }

            cv_task_waiter.notify_all();

            {
                write_lock w_lock(rw_lock);

                // Wait until previous group finish all their tasks (counter == 0) to start the new cooldown:
                // each group has time of AT LEAST their duration specially only for them
                cv_timer_waiter.wait(w_lock, [this] {
                    std::size_t& other_counter = !writer_flag ? writers_counter : readers_counter;
                    return other_counter == 0;
                });
            }
        }
        else {
            float sleep_time = 1.0f;
            {
                read_lock r_lock(rw_lock);
                sleep_time = writer_flag ? writer_duration : reader_duration; // read_lock because one of the durations might get changed from the outside
            }
            int sleep_time_ms = static_cast<int>(sleep_time * 1000.0f);
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));

            {
                write_lock w_lock(rw_lock);
                // If there's no tasks in the other queue - continue executing tasks from the current queue
                std::size_t other_tasks = !writer_flag ? writer_tasks.size() : reader_tasks.size();
                if (other_tasks > 0) {
                    writer_flag = !writer_flag;
                }
                else if (terminated) {
                    return;
                }

                // Wait until previous group finish all their tasks (counter == 0) to start the new cooldown:
                // Each group has time of AT LEAST their duration specially only for them
                cv_timer_waiter.wait(w_lock, [this] {
                    std::size_t& counter = !writer_flag ? writers_counter : readers_counter;
                    return counter == 0;
                });
            }

            cv_task_waiter.notify_all();
        }
    }
}
