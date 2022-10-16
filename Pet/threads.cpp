#include "core.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

static int threadNum = 0;
static std::vector<std::thread> threads;
static std::mutex mutex;
static std::condition_variable condition;
static std::queue<std::function<void()>> jobs;
static bool shutdown = false;

Threads Threads::Instance;

void Threads::Init()
{
    threadNum = std::thread::hardware_concurrency();
    threads.reserve(threadNum);

    for (int i = 0; i < threadNum; i++)
        threads.push_back(std::thread(&Run));
}

void Threads::Run()
{
    while (true)
    {
        Job job;

        {
            std::unique_lock<std::mutex> lock(mutex);

            // Threads *a = &Instance;
            condition.wait(lock, []() { return !jobs.empty() || shutdown; });

            if (shutdown)
                goto end;

            job = jobs.front();
            jobs.pop();
        }

        job();
    }
end:;
}

void Threads::Exit()
{
    {
        std::unique_lock<std::mutex> lock(mutex);
        shutdown = true;
    }

    condition.notify_all();

    for (auto &&t : threads)
        if (t.joinable())
            t.join();

    threads.clear();
}

void Threads::AddJob(Job job)
{
    {
        std::unique_lock<std::mutex> lock(mutex);
        jobs.push(job);
    }

    condition.notify_one();
}

int Threads::GetThreadNum()
{
    return threadNum;
}
