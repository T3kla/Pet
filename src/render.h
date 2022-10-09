#pragma once

#include "core.h"
using namespace glm;

class Render
{
  private:
    static Render Instance;

    static GLFWwindow *Window;
    static const char *WindowTitle;
    static vec2 WindowSize;

  public:
  private:
    Render() = default;
    Render(const Render &) = delete;
    ~Render() = default;

    static void OnWindowResize(GLFWwindow *window, int width, int height);

  public:
    static void Init();
    static vec2 GetWindowSize();
};
