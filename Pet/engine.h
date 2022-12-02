#pragma once

#include "input.h"
#include "logic.h"
#include "render.h"
#include "threads.h"

class App
{
    // Static

  private:
    static App *_instance;

  public:
    static App *Instance();

    // Instance

  private:
    Threads _threads;

    Input _input;
    Logic _logic;
    Render _render;

    bool _quitRequested = false;

  public:
    App();
    ~App();

    void Init();
    void Run();
    void Exit();

    void Quit();
};
