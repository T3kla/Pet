#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

using namespace glm;

class Render
{
  private:
    static Render Instance;

    static GLFWwindow *Window;
    static const char *WindowTitle;
    static i32vec2 WindowSize;

    static VkInstance Vulkan;
    static const list<const char *> VkValidationLayers;

  public:
  private:
    Render() = default;
    Render(const Render &) = delete;
    ~Render() = default;

    static void OnWindowResize(GLFWwindow *window, int width, int height);

  public:
    static void Init();
    static void Run();
    static void Exit();

    static i32vec2 GetWindowSize();
    static GLFWwindow *GetWindow();
    static bool CheckValidationLayers();
};
