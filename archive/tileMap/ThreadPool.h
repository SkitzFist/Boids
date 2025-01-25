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
#include "ThreadSettings.h"

class ThreadPool {
  public:
    ThreadPool(const ThreadSettings& settings) : settings(settings), isRunning(true) {

        m_taskQueues.reserve(settings.workerCount + 1);
        m_queueMutexes.reserve(settings.workerCount + 1);
        m_queueConditions.reserve(settings.workerCount + 1);
        m_tasksInProgress.reserve(settings.workerCount + 1);
        m_taskCompletionMutexes.reserve(settings.workerCount + 1);
        m_taskCompletionConditions.reserve(settings.workerCount + 1);
        m_threads.reserve(settings.workerCount + 1);

        for (std::size_t i = 0; i < settings.workerCount + 1; ++i) {
            m_taskQueues.emplace_back(std::make_unique<std::queue<std::function<void()>>>());
            m_queueMutexes.emplace_back(std::make_unique<std::mutex>());
            m_queueConditions.emplace_back(std::make_unique<std::condition_variable>());
            m_tasksInProgress.emplace_back(std::make_unique<std::atomic<int>>(0));
            m_taskCompletionMutexes.emplace_back(std::make_unique<std::mutex>());
            m_taskCompletionConditions.emplace_back(std::make_unique<std::condition_variable>());
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
        m_tasksInProgress[thread]->fetch_add(1, std::memory_order_relaxed);
        m_queueConditions[thread]->notify_one();
    }

    void await() {
        for (std::size_t i = 0; i < m_threads.size(); ++i) {
            std::unique_lock<std::mutex> lock(*m_taskCompletionMutexes[i]);
            m_taskCompletionConditions[i]->wait(lock, [this, i] {
                return m_tasksInProgress[i]->load(std::memory_order_relaxed) == 0;
            });
        }
    }

    void awaitWorkers() {
        for (std::size_t i = settings.workerStart; i < m_threads.size(); ++i) {
            std::unique_lock<std::mutex> lock(*m_taskCompletionMutexes[i]);
            m_taskCompletionConditions[i]->wait(lock, [this, i] {
                return m_tasksInProgress[i]->load(std::memory_order_relaxed) == 0;
            });
        }
    }

    void awaitTileMap() {
        std::unique_lock<std::mutex> lock(*m_taskCompletionMutexes[settings.tileMapThread]);
        m_taskCompletionConditions[settings.tileMapThread]->wait(lock, [this, tileMapThread = this->settings.tileMapThread] {
            return m_tasksInProgress[tileMapThread]->load(std::memory_order_relaxed) == 0;
        });
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(m_runningMutex);
            isRunning = false;
        }
        for (std::size_t i = 0; i < settings.workerCount + 1; ++i) {
            m_queueConditions[i]->notify_all();
        }
        for (auto& thread : m_threads) {
            thread.join();
        }
    }

  private:
    const ThreadSettings& settings;
    std::atomic<bool> isRunning;
    std::mutex m_runningMutex;

    std::vector<std::thread> m_threads;
    std::vector<std::unique_ptr<std::queue<std::function<void()>>>> m_taskQueues;
    std::vector<std::unique_ptr<std::mutex>> m_queueMutexes;
    std::vector<std::unique_ptr<std::condition_variable>> m_queueConditions;
    std::vector<std::unique_ptr<std::atomic<int>>> m_tasksInProgress;
    std::vector<std::unique_ptr<std::mutex>> m_taskCompletionMutexes;
    std::vector<std::unique_ptr<std::condition_variable>> m_taskCompletionConditions;

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

            if (m_tasksInProgress[thread]->fetch_sub(1, std::memory_order_relaxed) == 1) {
                std::lock_guard<std::mutex> lock(*m_taskCompletionMutexes[thread]);
                m_taskCompletionConditions[thread]->notify_all();
            }
        }
    }
};

#endif
