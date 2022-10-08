#include "core.h"

Render Render::Instance;

GLFWwindow *Render::Window = nullptr;
const char *Render::WindowTitle = "PetProject";
V2f Render::WindowSize = V2f(1920, 1080);

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
    WindowSize = V2f(width, height);
}

V2f Render::GetWindowSize()
{
    return WindowSize;
}
