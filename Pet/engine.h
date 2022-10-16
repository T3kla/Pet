#pragma once

class Engine
{
  private:
    static Engine Instance;

    bool closeRequested = false;

  public:
  private:
    Engine() = default;
    Engine(const Engine &) = delete;
    ~Engine() = default;

  public:
    static void Init();
    static void Run();
    static void Exit();

    static void CloseRequest();
};
