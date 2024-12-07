#ifndef TEST_THREAD_VECTOR
#define TEST_THREAD_VECTOR

#include <iostream>
#include <string>

#include "ThreadPool.h"
#include "ThreadVector.h"

void test_threadVector() {

    ThreadPool threadPool;

    int numData = 100;
    ThreadVector<std::string> logVec;
    logVec.initialize(numData * 2);

    for (int i = 0; i < ThreadSettings::THREAD_COUNT; ++i) {
        threadPool.enqueue(i, [i, numData, &logVec] {
            for (int j = 0; j < numData; ++j) {
                logVec.push_back("Thread[" + std::to_string(i) + "]: " + std::to_string(j));
            }
        });
    }

    threadPool.await();

    for (int i = 0; i < logVec.size(); ++i) {
        std::cout << logVec[i] << "\n";
    }

    logVec.reset();
}

#endif
