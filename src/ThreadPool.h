#ifndef BOIDS_THREAD_POOL_H
#define BOIDS_THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool {
  public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    void awaitCompletion();

    size_t numThreads;

  private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    size_t tasks_in_progress = 0;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable completion_condition;
    bool stop;

    void worker();
};

inline ThreadPool::ThreadPool(size_t threads) : stop(false), numThreads(threads) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] { this->worker(); });
    }
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace([task]() { (*task)(); });
        tasks_in_progress++;
    }
    condition.notify_one();

    return res;
}

inline void ThreadPool::awaitCompletion() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    completion_condition.wait(lock, [this] {
        return tasks.empty() && tasks_in_progress == 0;
    });
}

inline void ThreadPool::worker() {
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] {
                return stop || !tasks.empty();
            });

            if (stop && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }

        task();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks_in_progress--;
            if (tasks_in_progress == 0 && tasks.empty()) {
                completion_condition.notify_all();
            }
        }
    }
}

#endif