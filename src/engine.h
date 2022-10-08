#pragma once

class Engine
{
    static Engine Instance;

  private:
    Engine() = default;
    Engine(const Engine &) = delete;
    ~Engine() = default;

  public:
    static void Init();
};
