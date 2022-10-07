#pragma once

class Engine
{
    static Engine Instance;

  private:
    Engine() = default;
    ~Engine() = default;

  public:
    static void Init();
};
