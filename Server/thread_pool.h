#pragma once

#include <vector>
#include <functional>

#include "concurrent_queue.h"

class thread_pool {
public:
	inline thread_pool() = default;
	inline ~thread_pool() { terminate(); }

public:
	inline void initialize(const std::size_t worker_count);
	inline void terminate(const bool immediately = false);

	inline void set_paused(const bool paused);
	inline bool is_paused() const;

	inline bool working() const;
	inline bool working_unsafe() const;

	template <typename task_t, typename... arguments>
	inline void add_task(task_t&& task, arguments&&... parameters);

public:
	inline thread_pool(const thread_pool& other) = delete;
	inline thread_pool(thread_pool&& other) = delete;
	inline thread_pool& operator=(const thread_pool& rhs) = delete;
	inline thread_pool& operator=(thread_pool&& rhs) = delete;

private:
	inline void routine();

	mutable read_write_lock					rw_lock;
	mutable std::condition_variable_any		cv_task_waiter;
	std::vector<std::thread>				workers;

	concurrent_queue<std::function<void()>>	tasks;

	bool initialized = false;
	bool terminated = false;
	bool paused = false;
};


inline void thread_pool::initialize(const std::size_t worker_count) {
	write_lock w_lock(rw_lock);

	if (initialized || terminated) {
		return;
	}

	workers.reserve(worker_count);
	for (size_t id = 0; id < worker_count; ++id) {
		workers.emplace_back(&thread_pool::routine, this);
	}

	initialized = !workers.empty();
}

inline void thread_pool::terminate(const bool immediately) {
	{
		write_lock w_lock(rw_lock);

		if (working_unsafe()) {
			terminated = true;
			paused = false;

			if (immediately) {
				tasks.clear();
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

	write_lock w_lock(rw_lock);

	workers.clear();
	terminated = false;
	initialized = false;
	paused = false;
}

inline void thread_pool::routine() {
	while (true) {
		bool task_accquiered = false;
		std::function<void()> task;

		{
			write_lock w_lock(rw_lock);

			auto wait_condition = [this, &task_accquiered, &task] {
				if (paused) {
					return false;
				}

				task_accquiered = tasks.pop(task);
				return terminated || task_accquiered;
			};

			cv_task_waiter.wait(w_lock, wait_condition);

			if (terminated && !task_accquiered) {
				return;
			}
		}

		task();
	}
}

inline void thread_pool::set_paused(const bool paused) {
	write_lock w_lock(rw_lock);

	if (working_unsafe()) {
		this->paused = paused;

		if (!paused) {
			cv_task_waiter.notify_all();
		}
	}
}

inline bool thread_pool::is_paused() const {
	read_lock r_lock(rw_lock);
	return paused;
}

inline bool thread_pool::working() const {
	read_lock r_lock(rw_lock);
	return working_unsafe();
}

inline bool thread_pool::working_unsafe() const {
	return initialized && !terminated;
}

template <typename task_t, typename... arguments>
inline void thread_pool::add_task(task_t&& task, arguments&&... parameters) {
	{
		read_lock r_lock(rw_lock);
		if (!working_unsafe()) {
			return;
		}
	}

	auto bind = std::bind(std::forward<task_t>(task), std::forward<arguments>(parameters)...);

	tasks.emplace(bind);
	cv_task_waiter.notify_one();
}