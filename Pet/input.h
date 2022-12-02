#pragma once

class Input
{
    // Static

  private:
    static Input *_instance;

  public:
    static Input *Instance();

    // Instance

  public:
    Input();
    ~Input();

    void Init();
    void Run();
    void Exit();
};
