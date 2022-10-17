#include "core.h"

Render Render::Instance;

GLFWwindow *Render::Window = nullptr;
const char *Render::WindowTitle = "PetProject";
vec2 Render::WindowSize = vec2(1920, 1080);

VkInstance Render::Vulkan = nullptr;

void Render::Init()
{
    // Initialize GLFW

    std::cout << (glfwInit() != 0 ? "GLFW Initialized" : "GLFW Panicked") << std::endl;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    Window = glfwCreateWindow(WindowSize.x, WindowSize.y, WindowTitle, nullptr, nullptr);

    std::cout << (glfwVulkanSupported() != 0 ? "Vulkan Supported" : "Vulkan Not Supported") << std::endl;

    // Initialize Vulkan

    VkApplicationInfo appInfo{};                           // Optional arguments about our app.
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;    //
    appInfo.pApplicationName = "Hello Triangle";           // May provide useful info to the driver
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // in order to optimize the app.
    appInfo.pEngineName = "No Engine";                     //
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);      //
    appInfo.apiVersion = VK_API_VERSION_1_0;               //

    VkInstanceCreateInfo createInfo{};                                       // Non optional arguments.
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;               //
    createInfo.pApplicationInfo = &appInfo;                                  // Tells the vulkan driver about
                                                                             // required extensions and the
    u32 glfwExtensionCount = 0;                                              // validations layers we want
    const char **glfwExtensions;                                             // to use.
                                                                             //
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // Required extensions comes from
                                                                             // glfw and are passed to vulkan.
    createInfo.enabledExtensionCount = glfwExtensionCount;                   //
    createInfo.ppEnabledExtensionNames = glfwExtensions;                     //
    createInfo.enabledLayerCount = 0;                                        //

    if (vkCreateInstance(&createInfo, nullptr, &Vulkan) != VK_SUCCESS)
        throw std::runtime_error("Failed to create instance!");
    else
        std::cout << "Vulkan Instance Created" << std::endl;

    // Extension support

    u32 vkExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
    list<VkExtensionProperties> vkExtensions(vkExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data());

    // Validate extensions

    std::cout << "Vulkan found extensions:" << std::endl;
    for (const auto &extension : vkExtensions)
        std::cout << "    " << extension.extensionName << std::endl;

    std::cout << "GLFW required extensions:" << std::endl;
    for (size_t i = 0; i < glfwExtensionCount; i++)
        std::cout << "    " << glfwExtensions[i] << std::endl;

    i32 validatedExtensions = 0;
    for (size_t i = 0; i < glfwExtensionCount; i++)
        for (const auto &extension : vkExtensions)
            if (strcmp(glfwExtensions[i], extension.extensionName) == 0)
                validatedExtensions++;

    std::cout << "Required extensions validated: " << validatedExtensions << "/" << glfwExtensionCount << std::endl;
}

void Render::Run()
{
}

void Render::Exit()
{
    vkDestroyInstance(Vulkan, nullptr);
    glfwDestroyWindow(Render::GetWindow());
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

GLFWwindow *Render::GetWindow()
{
    return Window;
}
