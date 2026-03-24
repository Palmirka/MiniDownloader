#pragma once

#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <atomic>
#include <iostream>
#include <functional>
#include <condition_variable>

class ThreadPool {
    size_t numOfThreads;
    std::vector<std::thread> threads;
    std::queue<std::function<bool()>> taskQueue;
    std::mutex m;
    std::condition_variable cvTask;
    std::condition_variable cvQueue;

    void workerLoop(int i);

public:
    std::atomic<bool> end;

    ThreadPool(size_t numOfThreads);
    void emplace(std::function<bool()> function);
    void join();
    size_t queueSize();
    ~ThreadPool();
};
