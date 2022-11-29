#pragma once

#include "core.h"

class Logic
{
    static Logic Instance;

  private:
    Logic() = default;
    Logic(const Logic &) = delete;
    ~Logic() = default;

  public:
    static void Init();
    static void Run();
    static void Exit();
};
