#pragma once

#include "core.h"

class Threads
{
    // Static

  private:
    static Threads *_instance;

    static void Loop();

  public:
    static Threads *Instance();

    // Instance

  private:
    std::vector<std::thread> _pool;
    int _poolSize = 0;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::queue<del<void()>> _jobs;
    bool _shutdown = false;

  public:
    Threads();
    ~Threads();

    void Init();
    void Run();
    void Exit();

    void AddJob(del<void()> job);
    int GetThreadNum();
};
