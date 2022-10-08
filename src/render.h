#pragma once

#include "core.h"

struct V2f;

class Render
{
  private:
    static Render Instance;

    static GLFWwindow *Window;
    static const char *WindowTitle;
    static V2f WindowSize;

  public:
  private:
    Render() = default;
    Render(const Render &) = delete;
    ~Render() = default;

    static void OnWindowResize(GLFWwindow *window, int width, int height);

  public:
    static void Init();
    static V2f GetWindowSize();
};
