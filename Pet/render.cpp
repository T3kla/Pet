#include "core.h"

Render Render::Instance;

GLFWwindow *Render::Window = nullptr;
const char *Render::WindowTitle = "PetProject";
vec2 Render::WindowSize = vec2(1920, 1080);

void Render::Init()
{
    bool result = false;

    result = glfwInit() != 0 ? true : false;
    std::cout << (result ? "GLFW Initialized" : "GLFW Panicked") << std::endl;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    Window = glfwCreateWindow(WindowSize.x, WindowSize.y, WindowTitle, nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    result = glfwVulkanSupported() != 0 ? true : false;
    std::cout << (result ? "Vulkan Supported" : "Vulkan Not Supported") << std::endl;

    while (!glfwWindowShouldClose(Window))
        glfwPollEvents();

    glfwDestroyWindow(Window);
    glfwTerminate();
}

void Render::OnWindowResize(GLFWwindow *window, int width, int height)
{
    WindowSize = vec2(width, height);
}

vec2 Render::GetWindowSize()
{
    return WindowSize;
}
