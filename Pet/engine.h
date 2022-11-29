#pragma once

#include "core.h"

class Engine
{
    static Engine Instance;

  private:
    bool quitRequested = false;

  public:
  private:
    Engine() = default;
    Engine(const Engine &) = delete;
    ~Engine() = default;

  public:
    static void Init();
    static void Run();
    static void Exit();

    static void Quit();
};
