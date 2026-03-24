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
            return taskQueue.size() < numOfThreads * 2;
        });
        taskQueue.emplace(function);
        std::cout << "Queue size: " << taskQueue.size() << std::endl;
    }
    cvTask.notify_one();
}

void ThreadPool::join ()
{
    end = true;
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
            std::cout << i << " ";
            std::unique_lock<std::mutex> lock(m);
            std::cout << "IM LOCKED " << i << std::endl;
            cvTask.wait(lock, [this] {
                return !taskQueue.empty() || end;
            });

            if (end) return;
            
            task = taskQueue.front();
            taskQueue.pop();
            end = task();
            //if (end) return;
            if (end) {
                cvTask.notify_all();
            };
            cvQueue.notify_one();
        }
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
