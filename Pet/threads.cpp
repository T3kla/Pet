#include "threads.h"

Threads *Threads::_instance;

Threads *Threads::Instance()
{
    return _instance;
}

Threads::Threads()
{
    if (_instance)
        LOG("\nInstance already exists");

    _instance = this;
}

Threads::~Threads()
{
}

void Threads::Init()
{
    _poolSize = std::thread::hardware_concurrency();
    _pool.reserve(_poolSize);

    for (int i = 0; i < _poolSize; i++)
        _pool.push_back(std::thread(&Loop));
}

void Threads::Run()
{
}

void Threads::Exit()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _shutdown = true;
    }

    _cond.notify_all();

    for (auto &&thread : _pool)
        if (thread.joinable())
            thread.join();

    _pool.clear();
}

void Threads::Loop()
{
    while (true)
    {
        job job;

        {
            std::unique_lock<std::mutex> lock(_instance->_mutex);

            _instance->_cond.wait(lock, [&]() { return !_instance->_jobs.empty() || _instance->_shutdown; });

            if (_instance->_shutdown)
                goto end;

            job = _instance->_jobs.front();
            _instance->_jobs.pop();
        }

        job();
    }
end:;
}

void Threads::AddJob(del<void()> job)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _jobs.push(job);
    }

    _cond.notify_one();
}

int Threads::GetThreadNum()
{
    return _poolSize;
}
