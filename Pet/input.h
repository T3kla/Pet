#pragma once

class Input
{
    static Input Instance;

  private:
    Input() = default;
    Input(const Input &) = delete;
    ~Input() = default;

  public:
    static void Init();
    static void Run();
    static void Exit();
};
