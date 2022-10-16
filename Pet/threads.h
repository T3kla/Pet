#pragma once

#include <functional>

typedef std::function<void()> job;

class Threads
{
  private:
    static Threads Instance;

  public:
  private:
    Threads() = default;
    Threads(const Threads &) = delete;
    ~Threads() = default;

  public:
    static void Init();
    static void Run();
    static void Exit();

    static void AddJob(job job);
    static int GetThreadNum();
};
