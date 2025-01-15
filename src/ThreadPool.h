#ifndef BOIDS_THREAD_H
#define BOIDS_THREAD_H

#include <array>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>
#include <vector>

#include "CoreAffinity.h"
#include "Settings.h"

class ThreadPool {
  public:
    ThreadPool() : isRunning(true) {
        m_threads.reserve(ThreadSettings::THREAD_COUNT);
        for (std::size_t i = 0; i < ThreadSettings::THREAD_COUNT; ++i) {
            m_taskQueues[i] = std::make_unique<std::queue<std::function<void()>>>();
            m_queueMutexes[i] = std::make_unique<std::mutex>();
            m_queueConditions[i] = std::make_unique<std::condition_variable>();
            m_threads.emplace_back([this, i] { this->worker(i); });
        }
    }

    ~ThreadPool() {
        close();
    }

    template <class F, class... Args>
    void enqueue(int thread, F&& f, Args&&... args) {
        {
            std::lock_guard<std::mutex> lock(*m_queueMutexes[thread]);
            m_taskQueues[thread]->emplace([func = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable {
                func(std::forward<Args>(args)...);
            });
        }
        m_tasksInProgress[thread].fetch_add(1, std::memory_order_relaxed);
        m_queueConditions[thread]->notify_one();
    }

    void await() {
        for (std::size_t i = 0; i < ThreadSettings::THREAD_COUNT; ++i) {
            std::unique_lock<std::mutex> lock(m_taskCompletionMutexes[i]);
            m_taskCompletionConditions[i].wait(lock, [this, i] {
                return m_tasksInProgress[i].load(std::memory_order_relaxed) == 0;
            });
        }
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(m_runningMutex);
            isRunning = false;
        }
        for (std::size_t i = 0; i < ThreadSettings::THREAD_COUNT; ++i) {
            m_queueConditions[i]->notify_all();
        }
        for (auto& thread : m_threads) {
            thread.join();
        }
    }

  private:
    std::atomic<bool> isRunning;
    std::mutex m_runningMutex;

    std::vector<std::thread> m_threads;
    std::array<std::unique_ptr<std::queue<std::function<void()>>>, ThreadSettings::THREAD_COUNT> m_taskQueues;
    std::array<std::unique_ptr<std::mutex>, ThreadSettings::THREAD_COUNT> m_queueMutexes;
    std::array<std::unique_ptr<std::condition_variable>, ThreadSettings::THREAD_COUNT> m_queueConditions;

    std::array<std::atomic<int>, ThreadSettings::THREAD_COUNT> m_tasksInProgress = {};

    std::array<std::mutex, ThreadSettings::THREAD_COUNT> m_taskCompletionMutexes;
    std::array<std::condition_variable, ThreadSettings::THREAD_COUNT> m_taskCompletionConditions;

    void worker(const int thread) {
        // Optional: Set thread affinity if needed
        setAffinity(thread + 1);

        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(*m_queueMutexes[thread]);
                m_queueConditions[thread]->wait(lock, [this, thread] {
                    return !m_taskQueues[thread]->empty() || !isRunning.load(std::memory_order_acquire);
                });

                if (!isRunning.load(std::memory_order_acquire) && m_taskQueues[thread]->empty()) {
                    break;
                }

                task = std::move(m_taskQueues[thread]->front());
                m_taskQueues[thread]->pop();
            }

            try {
                task();
            } catch (const std::exception& e) {
                // Handle exceptions from tasks if necessary
            }

            if (m_tasksInProgress[thread].fetch_sub(1, std::memory_order_relaxed) == 1) {
                std::lock_guard<std::mutex> lock(m_taskCompletionMutexes[thread]);
                m_taskCompletionConditions[thread].notify_all();
            }
        }
    }
};

#endif
