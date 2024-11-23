#ifndef BOIDS_THREAD_POOL_H
#define BOIDS_THREAD_POOL_H

#include <atomic>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

#include "LockFreeQueue.h"

class ThreadPool {
  public:
    ThreadPool(size_t numThreads)
        : stopFlag(false) {
        threads.reserve(numThreads);
        for (size_t i = 0; i < numThreads; ++i)
            threads.emplace_back(&ThreadPool::workerThread, this);
    }

    ~ThreadPool() {
        stopFlag.store(true, std::memory_order_relaxed);
        for (auto& t : threads)
            t.join();
    }

    template <typename Func, typename... Args>
    void enqueue(Func&& func, Args&&... args) {
        auto task = [f = std::forward<Func>(func), ... args = std::forward<Args>(args)]() mutable {
            f(std::forward<Args>(args)...);
        };

        tasks.enqueue(std::move(task));
    }

  private:
    std::vector<std::thread> threads;
    LockFreeQueue<std::function<void()>> tasks;
    std::atomic<bool> stopFlag;

    void workerThread() {
        while (!stopFlag.load(std::memory_order_relaxed)) {
            std::function<void()> task;
            if (tasks.dequeue(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }
};

#endif
