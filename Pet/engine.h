#pragma once

#include "input.h"
#include "logic.h"
#include "render.h"
#include "threads.h"

class App
{
    // Static

  public:
    static App &Instance();

  private:
    static App *instance;

    // Instance

  public:
    App();

    void Init();
    void Run();
    void Exit();

    void Quit();

    Input input;
    Logic logic;
    Render render;
    Threads threads;

  private:
    bool quitRequested = false;
};
