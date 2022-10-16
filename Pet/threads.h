#pragma once

#include <functional>

typedef std::function<void()> Job;

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

    static void AddJob(Job job);
    static int GetThreadNum();
};
