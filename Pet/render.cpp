#include "core.h"

Render Render::Instance;

GLFWwindow *Render::Window = nullptr;
const char *Render::WindowTitle = "PetProject";
i32vec2 Render::WindowSize = i32vec2(1280, 720);

VkInstance Render::Vulkan = nullptr;
const list<const char *> Render::VkValidationLayers = {"VK_LAYER_KHRONOS_validation"};

void Render::Init()
{
    // Initialize GLFW

    std::cout << (glfwInit() != 0 ? "GLFW Initialized" : "GLFW Panicked") << std::endl;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    Window = glfwCreateWindow(WindowSize.x, WindowSize.y, WindowTitle, nullptr, nullptr);

    std::cout << (glfwVulkanSupported() != 0 ? "Vulkan Supported" : "Vulkan Not Supported") << std::endl;

    // Vulkan optinal app info

    VkApplicationInfo appInfo{};                           // Optional arguments about our app.
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;    //
    appInfo.pApplicationName = "Hello Triangle";           // May provide useful info to the driver
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // in order to optimize the app.
    appInfo.pEngineName = "No Engine";                     //
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);      //
    appInfo.apiVersion = VK_API_VERSION_1_0;               //

    // Validate extensions

    u32 glfwExtensionCount, vkExtensionCount;

    const char **glfwExtensions;                                             // Get glfw required extensions.
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); //

    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);             // Get vulkan driver
    list<VkExtensionProperties> vkExtensions(vkExtensionCount);                              // suported extensions.
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensions.data()); //

    std::cout << std::endl << "Vulkan found extensions:" << std::endl;
    for (const auto &extension : vkExtensions)
        std::cout << "    " << extension.extensionName << std::endl;

    std::cout << "GLFW required extensions:" << std::endl;
    for (size_t i = 0; i < glfwExtensionCount; i++)
        std::cout << "    " << glfwExtensions[i] << std::endl;

    u32 validatedExtensions = 0;
    for (auto i = 0; i < glfwExtensionCount; i++)
        for (auto &extension : vkExtensions)
            if (strcmp(glfwExtensions[i], extension.extensionName) == 0)
                validatedExtensions++;

    std::cout << "Required extensions validated: " << validatedExtensions << "/" << glfwExtensionCount << std::endl;

    if (validatedExtensions < glfwExtensionCount)
        throw std::runtime_error("\nRequired extensions not available!");

    // Validate layers

    u32 requiredLayersCount, availableLayersCount;

    requiredLayersCount = (u32)VkValidationLayers.size();

    vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);                // Get vulkan driver
    list<VkLayerProperties> availableLayers(availableLayersCount);                     // available layers.
    vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data()); //

    std::cout << std::endl << "Vulkan required layers:" << std::endl;
    for (auto &layer : VkValidationLayers)
        std::cout << "    " << layer << std::endl;

    std::cout << "Vulkan available layers:" << std::endl;
    for (auto &layer : availableLayers)
        std::cout << "    " << layer.layerName << std::endl;

    u32 validatedLayers = 0u;
    for (auto &req : VkValidationLayers)
        for (auto &avl : availableLayers)
            if (strcmp(req, avl.layerName) == 0)
                validatedLayers++;

    std::cout << "Required layers validated: " << validatedLayers << "/" << requiredLayersCount << std::endl;

    if (Debug && validatedLayers < requiredLayersCount)
        throw std::runtime_error("\nRequired extensions not available!");

    // Vulkan non optinal app info

    VkInstanceCreateInfo createInfo{};                         // Tells the vulkan driver about required extensions
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // and the validations layers we want to use.
    createInfo.enabledExtensionCount = glfwExtensionCount;     //
    createInfo.ppEnabledExtensionNames = glfwExtensions;       // Required extensions comes from glfw and are
    createInfo.enabledLayerCount = 0;                          // passed to vulkan.

    if (Debug)
    {
        createInfo.enabledLayerCount = (u32)VkValidationLayers.size();
        createInfo.ppEnabledLayerNames = VkValidationLayers.data();
    }

    // Vulkan initialization

    if (vkCreateInstance(&createInfo, nullptr, &Vulkan) != VK_SUCCESS)
        throw std::runtime_error("\nVulkan failed to create instance!");
    else
        std::cout << std::endl << "Vulkan instance created!" << std::endl;
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

i32vec2 Render::GetWindowSize()
{
    return WindowSize;
}

GLFWwindow *Render::GetWindow()
{
    return Window;
}
