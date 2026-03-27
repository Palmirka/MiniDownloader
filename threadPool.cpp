#include "threadPool.hpp"

ThreadPool::ThreadPool (size_t numOfThreads) :
    numOfThreads(numOfThreads)
{
    threads.reserve(numOfThreads);
    for (size_t i = 0; i < numOfThreads; i++)
    {
        threads.emplace_back([this, i] {
            this->workerLoop(i);
        });
    }
}

void ThreadPool::emplace (std::function<bool()> function)
{
    {
        std::unique_lock<std::mutex> lock(m);
        cvQueue.wait(lock, [this] {
            return taskQueue.size() < numOfThreads * 2 || end;
        });
        if (end) return;
        taskQueue.emplace(function);
    }
    cvTask.notify_one();
}

void ThreadPool::join ()
{
    end = true;
    cvQueue.notify_all();
    cvTask.notify_all();
    for (auto &thread : threads)
    {
        thread.join();
    }
}

void ThreadPool::workerLoop (int i)
{
    while(true)
    {
        std::function<bool()> task;
        {
            std::unique_lock<std::mutex> lock(m);
            cvTask.wait(lock, [this] {
                return !taskQueue.empty() || end;
            });

            if (end) return;
            
            task = taskQueue.front();
            taskQueue.pop();
        }
        bool finished = task();
        if (finished) {
            end = true;
        };
        cvQueue.notify_one();
    }
}

size_t ThreadPool::queueSize()
{
    std::unique_lock<std::mutex> lock(m);
    return taskQueue.size();
}

ThreadPool::~ThreadPool ()
{
    if (!end){
        join();
    }
}
