#ifndef BOIDS_THEAD_POOL_LOCK_FREE_H
#define BOIDS_THEAD_POOL_LOCK_FREE_H

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <semaphore>
#include <thread>

#include "CoreAffinity.h"

// Debug
#include <iostream>

template <std::size_t threadCount>
class ThreadPoolLockFree {
  public:
    ThreadPoolLockFree() {
        void* semaphoreRaw = operator new[](threadCount * sizeof(std::binary_semaphore));
        m_semaphores = static_cast<std::binary_semaphore*>(semaphoreRaw);
        for (std::size_t i = 0; i < threadCount; ++i) {
            new (&m_semaphores[i]) std::binary_semaphore(0);
        }

        for (std::size_t i = 0; i < threadCount; ++i) {
            m_taskList[i] = NULL;
        }

        isRunning = true;

        m_threads.reserve(threadCount);
        for (std::size_t i = 0; i < threadCount; ++i) {
            m_threads.emplace_back([this, i] { this->worker(i); });
        }
    }

    template <class F, class... Args>
    void enque(int thread, F&& f, Args&&... args) {
        std::cout << "[" << thread << "] Adding task:\n";

        if (m_taskList[thread] == NULL) {
            m_taskList[thread] = [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable {
                f(std::forward<Args>(args)...);
            };
            std::cout << "\tTask added\n";
            m_tasksInProgress[thread] = true;
            m_semaphores[thread].release();
        } else {
            std::cout << "\tTask Not added\n";
        }
    }

    void await() {
        int tasksInProgress = 0;
        while (true) {
            tasksInProgress = 0;
            for (std::size_t i = 0; i < threadCount; ++i) {
                if (m_tasksInProgress[i]) {
                    ++tasksInProgress;
                }
            }
            if (tasksInProgress == 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void close() {
        await();

        isRunning = false;
        for (int i = 0; i < threadCount; ++i) {
            m_tasksInProgress[i] = true;
            m_semaphores[i].release();
            m_threads[i].join();
        }

        for (std::size_t i = 0; i < threadCount; ++i) {
            m_semaphores[i].~counting_semaphore<1>();
        }

        operator delete[](m_semaphores);
    }

  private:
    std::atomic<bool> isRunning;
    std::vector<std::thread> m_threads;
    std::binary_semaphore* m_semaphores;
    std::array<std::function<void()>, threadCount> m_taskList;
    std::array<std::atomic<bool>, threadCount> m_tasksInProgress = {};

    void worker(const int thread) {
        // Main thread runs on core 1
        setAffinity(thread + 2);

        std::function<void()> task;
        while (isRunning) {
            m_semaphores[thread].acquire();

            if (m_taskList[thread]) {
                task = std::move(m_taskList[thread]);
                m_taskList[thread] = {};

                task();
            }

            m_tasksInProgress[thread] = false;
        }
    }
};

#endif
