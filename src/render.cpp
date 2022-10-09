#include "core.h"

Render Render::Instance;

GLFWwindow *Render::Window = nullptr;
const char *Render::WindowTitle = "PetProject";
vec2 Render::WindowSize = vec2(1920, 1080);

void Render::Init()
{
    auto glfwResult = glfwInit() == 0 ? true : false;

    if (glfwResult)
        std::cout << "True" << std::endl;
    else
        std::cout << "False" << std::endl;

    glfwResult = glfwVulkanSupported() == 0 ? true : false;

    if (glfwResult)
        std::cout << "V True" << std::endl;
    else
        std::cout << "V False" << std::endl;
}

void Render::OnWindowResize(GLFWwindow *window, int width, int height)
{
    WindowSize = vec2(width, height);
}

vec2 Render::GetWindowSize()
{
    return WindowSize;
}
