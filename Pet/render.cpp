#include "render.h"

Render Render::Instance;

GLFWwindow *Render::Window = nullptr;
const char *Render::WindowTitle = "PetProject";
i32vec2 Render::WindowSize = i32vec2(1280, 720);

VkInstance Render::VulkanInstance;
VkDebugUtilsMessengerEXT Render::VulkanMessenger;
VkPhysicalDevice Render::VulkanDevice;

const list<const char *> VkValidationLayers = {"VK_LAYER_KHRONOS_validation"};

void Render::Init()
{
    auto window = InitializeGLFW();
    auto vkAppInfo = PopulateVkAppInfo();

    Window = window;

    // Validate extensions

    // auto requiredExtensions = GetRequiredExtensions();
    // auto availableExtensions = GetAvailableExtensions();

    // if (!ValidateExtensions(requiredExtensions, availableExtensions))
    //     throw std::runtime_error("\nExtension validation failed!");

    // Validate layers

    u32 requiredLayersCount, availableLayersCount;

    requiredLayersCount = (u32)VkValidationLayers.size();

    vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);
    list<VkLayerProperties> availableLayers(availableLayersCount);
    vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());

    std::cout << std::endl;

    std::cout << "Required layers:" << std::endl;
    for (const auto &layer : VkValidationLayers)
        std::cout << "    " << layer << std::endl;

    std::cout << "Available layers:" << std::endl;
    for (const auto &layer : availableLayers)
        std::cout << "    " << layer.layerName << std::endl;

    u32 validatedLayers = 0u;
    for (const auto &req : VkValidationLayers)
        for (const auto &avl : availableLayers)
            if (strcmp(req, avl.layerName) == 0)
                validatedLayers++;

    std::cout << "Required layers validated: " << validatedLayers << "/" << requiredLayersCount << std::endl;

    if (Debug && validatedLayers < requiredLayersCount)
        throw std::runtime_error("\nRequired extensions not available!");

    // Vulkan non optinal app info

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &vkAppInfo;
    // instanceInfo.enabledExtensionCount = requiredExtensions.size();
    // instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
    instanceInfo.pNext = nullptr;

    // Vulkan setup debug messenger

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};

    if (Debug)
    {
        messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerInfo.pfnUserCallback = DebugCallback;
        messengerInfo.pUserData = nullptr;

        instanceInfo.enabledLayerCount = (u32)VkValidationLayers.size();
        instanceInfo.ppEnabledLayerNames = VkValidationLayers.data();
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&messengerInfo;
    }

    // Vulkan initialization

    if (vkCreateInstance(&instanceInfo, nullptr, &VulkanInstance) == VK_SUCCESS)
        std::cout << std::endl << "Vulkan instance created!" << std::endl;
    else
        throw std::runtime_error("\nVulkan failed to create instance!");

    // Vulkan attatch debug messenger

    if (Debug)
    {
        auto fn =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VulkanInstance, "vkCreateDebugUtilsMessengerEXT");

        if (fn)
            fn(VulkanInstance, &messengerInfo, nullptr, &VulkanMessenger);
        else
            throw std::runtime_error("\nFailed to create debug messenger!");
    }

    // Physical device

    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(VulkanInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("\nFailed to find GPUs with Vulkan support!");

    list<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(VulkanInstance, &deviceCount, devices.data());

    odic<u32, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        u32 score = 0;

        score += deviceProperties.limits.maxImageDimension2D;
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;
        if (!deviceFeatures.geometryShader)
            score = 0;

        candidates.insert({score, device});
    }

    if (candidates.rbegin()->first > 0)
        VulkanDevice = candidates.rbegin()->second;
    else
        throw std::runtime_error("\nFailed to find a suitable GPU!");
}

void Render::Run()
{
}

void Render::Exit()
{
    if (Debug)
    {
        auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VulkanInstance,
                                                                             "vkDestroyDebugUtilsMessengerEXT");

        if (fn)
            fn(VulkanInstance, VulkanMessenger, nullptr);
    }

    vkDestroyInstance(VulkanInstance, nullptr);
    glfwDestroyWindow(Render::GetWindow());
    glfwTerminate();
}

GLFWwindow *Render::InitializeGLFW()
{
    if (glfwInit() == 0)
        throw std::runtime_error("\nGLFW Panicked!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto window = glfwCreateWindow(WindowSize.x, WindowSize.y, WindowTitle, nullptr, nullptr);

    if (glfwVulkanSupported() == 0)
        throw std::runtime_error("\nGLFW Vulkan Not Supported!");

    return window;
}

VkApplicationInfo Render::PopulateVkAppInfo()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Pet";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "PetEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    return appInfo;
}

list<const char *> GetRequiredExtensions()
{
    u32 count;

    auto **raw = glfwGetRequiredInstanceExtensions(&count);
    auto extensions = list<const char *>(raw, raw + count);

    if (Debug)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

list<VkExtensionProperties> GetAvailableExtensions()
{
    u32 count;

    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    auto extensions = list<VkExtensionProperties>(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

    return extensions;
}

bool ValidateExtensions(const list<const char *> &required, const list<VkExtensionProperties> &available)
{
    u32 reqCount = (u32)required.size();
    u32 validated = 0;

    std::cout << std::endl;

    std::cout << "Required extensions [" << reqCount << "] : " << std::endl;
    for (const auto &ext : required)
        std::cout << "    " << ext << std::endl;

    std::cout << "Available extensions [" << reqCount << "] : " << std::endl;
    for (const auto &ext : available)
        std::cout << "    " << ext.extensionName << std::endl;

    for (const auto &req : required)
        for (const auto &avl : available)
            if (strcmp(req, avl.extensionName) == 0)
                validated++;

    std::cout << "Required extensions validated: " << validated << "/" << reqCount << std::endl;

    return validated == reqCount;
}

void Render::OnWindowResize(GLFWwindow *window, int width, int height)
{
    WindowSize = vec2(width, height);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Render::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                     void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

i32vec2 Render::GetWindowSize()
{
    return WindowSize;
}

GLFWwindow *Render::GetWindow()
{
    return Window;
}
