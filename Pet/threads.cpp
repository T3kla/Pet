#include "core.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace std;

static vector<thread> ThreadPool;
static int ThreadPoolSize = 0;
static mutex ThreadMutex;
static condition_variable ThreadCondition;
static queue<function<void()>> ThreadJobs;
static bool ThreadShutdown = false;

Threads Threads::Instance;

void Threads::Init()
{
    ThreadPoolSize = thread::hardware_concurrency();
    ThreadPool.reserve(ThreadPoolSize);

    for (int i = 0; i < ThreadPoolSize; i++)
        ThreadPool.push_back(thread(&Run));
}

void Threads::Run()
{
    while (true)
    {
        job job;

        {
            unique_lock<mutex> lock(ThreadMutex);

            // Threads *a = &Instance;
            ThreadCondition.wait(lock, []() { return !ThreadJobs.empty() || ThreadShutdown; });

            if (ThreadShutdown)
                goto end;

            job = ThreadJobs.front();
            ThreadJobs.pop();
        }

        job();
    }
end:;
}

void Threads::Exit()
{
    {
        unique_lock<mutex> lock(ThreadMutex);
        ThreadShutdown = true;
    }

    ThreadCondition.notify_all();

    for (auto &&t : ThreadPool)
        if (t.joinable())
            t.join();

    ThreadPool.clear();
}

void Threads::AddJob(job job)
{
    {
        unique_lock<mutex> lock(ThreadMutex);
        ThreadJobs.push(job);
    }

    ThreadCondition.notify_one();
}

int Threads::GetThreadNum()
{
    return ThreadPoolSize;
}
