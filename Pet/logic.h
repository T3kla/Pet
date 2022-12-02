#pragma once

class Logic
{
    // Static

  private:
    static Logic *_instance;

  public:
    static Logic *Instance();

    // Instance

  public:
    Logic();
    ~Logic();

    static void Init();
    static void Run();
    static void Exit();
};
