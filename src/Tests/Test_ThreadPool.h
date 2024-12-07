#ifndef TEST_THREAD_POOL_H
#define TEST_THREAD_POOL_H

#include <iostream>

#include "ThreadPool.h"

constexpr const int threads = 6;
using namespace std::chrono_literals;
void ThreadPoolTest() {
    std::cout << "Starting ThreadPool test\n";
    ThreadPool pool;

    for (int i = 0; i < threads; ++i) {
        pool.enqueue(i, [i] {
            std::cout << "Task " << i << "\n";
        });
    }

    pool.await();

    for (int i = 0; i < threads; ++i) {
        pool.enqueue(i, [i] {
            std::cout << "Task " << i << "\n";
        });
    }

    pool.await();
}

#endif
