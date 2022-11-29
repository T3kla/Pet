#include "render.h"

Render Render::Instance;

GLFWwindow *Render::Window = nullptr;
const char *Render::WindowTitle = "PetProject";
i32vec2 Render::WindowSize = i32vec2(1280, 720);

VkInstance Render::VkInstance;
VkDebugUtilsMessengerEXT Render::VkMessenger;

VkPhysicalDevice Render::VkPhyDevice;
QueueFamilyIndices Render::VkPhyDeviceIndices;
VkDevice Render::VkLogDevice;

VkQueue Render::VkGraphicsQueue;

const list<const char *> VkValidationLayers = {"VK_LAYER_KHRONOS_validation"};

bool QueueFamilyIndices::IsComplete()
{
    return graphicsFamily.has_value();
}

void Render::Init()
{
    Window = InitializeGLFW();

    auto vkAppInfo = PopulateVkAppInfo();

    // Validate extensions

    auto requiredExtensions = GetRequiredExtensions();
    auto availableExtensions = GetAvailableExtensions();

    if (!ValidateExtensions(requiredExtensions, availableExtensions))
        throw std::runtime_error("\nExtension validation failed!");

    std::cout << std::endl;

    // Validate layers

    auto &requiredLayers = VkValidationLayers;
    auto availableLayers = GetAvailableLayers();

    if (!ValidateLayers(requiredLayers, availableLayers))
        throw std::runtime_error("\nLayer validation failed!");

    std::cout << std::endl;

    // Vulkan debuger callback

    auto vkMessengerInfo = PopulateVkMessengerInfo();

    // Vulkan other non-optional info

    auto vkInstanceInfo = PopulateVkInstanceInfo(vkAppInfo, requiredExtensions, vkMessengerInfo);

    // Vulkan initialization

    if (vkCreateInstance(&vkInstanceInfo, nullptr, &VkInstance) == VK_SUCCESS)
        std::cout << std::endl << "Vulkan instance created!" << std::endl;
    else
        throw std::runtime_error("\nVulkan failed to create instance!");

    // Vulkan attatch debug messenger

    if (Debug)
    {
        auto fn =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VkInstance, "vkCreateDebugUtilsMessengerEXT");

        if (fn)
            fn(VkInstance, &vkMessengerInfo, nullptr, &VkMessenger);
        else
            throw std::runtime_error("\nFailed to create debug messenger!");
    }

    // Physical device

    VkPhyDevice = GetMostSuitableDevice();
    VkPhyDeviceIndices = GetAvailableQueuesFamilies(VkPhyDevice);

    // Logical device

    u32 extensionNum = requiredExtensions.size();
    u32 layerNum = requiredLayers.size();

    VkLogDevice =
        GetLogicalDevice(VkPhyDevice, VkPhyDeviceIndices, extensionNum, requiredExtensions, layerNum, requiredLayers);

    // Graphics queue

    VkGraphicsQueue = GetGraphicsQueue(VkLogDevice, VkPhyDeviceIndices);
}

void Render::Run()
{
}

void Render::Exit()
{
    if (Debug)
    {
        auto fn =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VkInstance, "vkDestroyDebugUtilsMessengerEXT");

        if (fn)
            fn(VkInstance, VkMessenger, nullptr);
    }

    vkDestroyInstance(VkInstance, nullptr);
    vkDestroyDevice(VkLogDevice, nullptr);
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

#pragma region Populate

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

VkInstanceCreateInfo Render::PopulateVkInstanceInfo(const VkApplicationInfo &vkAppInfo,
                                                    const list<const char *> &requiredExtensions,
                                                    const VkDebugUtilsMessengerCreateInfoEXT &messengerInfo)
{
    VkInstanceCreateInfo instanceInfo{};

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &vkAppInfo;
    instanceInfo.enabledExtensionCount = requiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = nullptr;
    instanceInfo.pNext = nullptr;

    if (Debug)
    {
        instanceInfo.enabledLayerCount = (u32)VkValidationLayers.size();
        instanceInfo.ppEnabledLayerNames = VkValidationLayers.data();
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&messengerInfo;
    }

    return instanceInfo;
}

VkDebugUtilsMessengerCreateInfoEXT Render::PopulateVkMessengerInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};

    if (Debug)
    {
        messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | //
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;    //
        messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |         //
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |      //
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;      //
        messengerInfo.pfnUserCallback = DebugCallback;
        messengerInfo.pUserData = nullptr;
    }

    return messengerInfo;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Render::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                     void *pUserData)
{
    std::cerr << "[VK]: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

#pragma endregion

#pragma region Extension Validation

list<const char *> Render::GetRequiredExtensions()
{
    u32 count;

    auto **raw = glfwGetRequiredInstanceExtensions(&count);
    auto extensions = list<const char *>(raw, raw + count);

    if (Debug)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

list<VkExtensionProperties> Render::GetAvailableExtensions()
{
    u32 count;

    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    auto extensions = list<VkExtensionProperties>(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

    return extensions;
}

bool Render::ValidateExtensions(const list<const char *> &required, const list<VkExtensionProperties> &available)
{
    u32 reqCount = (u32)required.size();
    u32 avlCount = (u32)available.size();
    u32 validated = 0;

    std::cout << std::endl;

    std::cout << "Required extensions [" << reqCount << "] :" << std::endl;
    for (const auto &ext : required)
        std::cout << "    " << ext << std::endl;

    std::cout << "Available extensions [" << avlCount << "] :" << std::endl;
    for (const auto &ext : available)
        std::cout << "    " << ext.extensionName << std::endl;

    for (const auto &req : required)
        for (const auto &avl : available)
            if (strcmp(req, avl.extensionName) == 0)
                validated++;

    std::cout << "Required extensions validated: " << validated << "/" << reqCount << std::endl;

    return validated == reqCount;
}

#pragma endregion

#pragma region Layer Validation

list<VkLayerProperties> Render::GetAvailableLayers()
{
    u32 count;

    vkEnumerateInstanceLayerProperties(&count, nullptr);
    auto layers = list<VkLayerProperties>(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());

    return layers;
}

bool Render::ValidateLayers(const list<const char *> &required, const list<VkLayerProperties> &available)
{
    u32 reqCount = (u32)required.size();
    u32 avlCount = (u32)available.size();
    u32 validated = 0;

    std::cout << "Required layers [" << reqCount << "] :" << std::endl;
    for (const auto &layer : required)
        std::cout << "    " << layer << std::endl;

    std::cout << "Available layers [" << avlCount << "] :" << std::endl;
    for (const auto &layer : available)
        std::cout << "    " << layer.layerName << std::endl;

    for (const auto &req : VkValidationLayers)
        for (const auto &avl : available)
            if (strcmp(req, avl.layerName) == 0)
                validated++;

    std::cout << "Required layers validated: " << validated << "/" << reqCount << std::endl;

    return validated == reqCount;
}

#pragma endregion

#pragma region Phisical Device Validation

VkPhysicalDevice Render::GetMostSuitableDevice()
{
    VkPhysicalDevice device;

    u32 deviceCount = 0;

    vkEnumeratePhysicalDevices(VkInstance, &deviceCount, nullptr);
    list<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(VkInstance, &deviceCount, devices.data());

    if (deviceCount == 0)
        throw std::runtime_error("\nFailed to find GPUs with Vulkan support!");

    odic<u32, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
        candidates.insert({RateDevice(device), device});

    if (candidates.rbegin()->first > 0)
        device = candidates.rbegin()->second;
    else
        throw std::runtime_error("\nFailed to find a suitable GPU!");

    return device;
}

u32 Render::RateDevice(const VkPhysicalDevice &device)
{
    u32 score = 0;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Rating

    score += deviceProperties.limits.maxImageDimension2D;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    if (!deviceFeatures.geometryShader)
        score = 0;

    auto idx = GetAvailableQueuesFamilies(device);
    if (!idx.IsComplete())
        score = 0;

    //

    return score;
}

#pragma endregion

#pragma region Queue Family Validation

QueueFamilyIndices Render::GetAvailableQueuesFamilies(const VkPhysicalDevice &device)
{
    QueueFamilyIndices indices;

    u32 queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    list<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (u32 i = 0; i < queueFamilies.size(); i++)
    {
        if (indices.IsComplete())
            break;
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;
    }

    return indices;
}

#pragma endregion

#pragma region Logical Device Validation

VkDevice Render::GetLogicalDevice(const VkPhysicalDevice &vkPhyDevice, const QueueFamilyIndices &vkPhyDeviceIndices,
                                  u32 extensionNum, const list<const char *> &extensions, u32 layerNum,
                                  const list<const char *> &layers)
{
    VkDevice device;

    float queuePriority = 1.0f;

    // Queue info

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = vkPhyDeviceIndices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Features info

    VkPhysicalDeviceFeatures deviceFeatures{};

    // Create logical device

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    // System extensions are not equal to physical device extensions
    // Asking for system extensions to an individual device can fail
    // createInfo.enabledExtensionCount = extensionNum;
    // createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount = layerNum;
    createInfo.ppEnabledLayerNames = layers.data();

    if (vkCreateDevice(vkPhyDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");

    return device;
}

#pragma endregion

#pragma region Graphics queue

VkQueue Render::GetGraphicsQueue(const VkDevice &vkLogDevice, const QueueFamilyIndices &indices)
{
    VkQueue queue{};

    vkGetDeviceQueue(vkLogDevice, indices.graphicsFamily.value(), 0, &queue);

    return queue;
}

#pragma endregion

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
