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
VkSurfaceKHR Render::VkSurface;

VkSwapchainKHR Render::VkCurSwapChain;
VkSwapchainKHR Render::VkOldSwapChain;
list<VkImage> Render::VkSwapChainImages;
VkFormat Render::VkSwapChainImageFormat;
VkExtent2D Render::VkSwapChainExtent;
list<VkImageView> Render::VkSwapChainImageViews;

const list<const char *> VkValidationLayers = {"VK_LAYER_KHRONOS_validation"};
const list<const char *> VkDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

bool QueueFamilyIndices::IsComplete()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
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

    if (PetDebug)
    {
        auto fn =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VkInstance, "vkCreateDebugUtilsMessengerEXT");

        if (fn)
            fn(VkInstance, &vkMessengerInfo, nullptr, &VkMessenger);
        else
            throw std::runtime_error("\nFailed to create debug messenger!");
    }

    // Surface

    auto vkSurfaceInfo = PopulateVkSurface();

    if (glfwCreateWindowSurface(VkInstance, Window, nullptr, &VkSurface) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create window surface!");

    // if (vkCreateWin32SurfaceKHR(VkInstance, &vkSurfaceInfo, nullptr, &VkSurface) != VK_SUCCESS)
    //     throw std::runtime_error("\nFailed to create window surface!");

    // Physical device

    VkPhyDevice = GetMostSuitableDevice();
    VkPhyDeviceIndices = GetAvailableQueuesFamilies(VkPhyDevice);

    // Logical device

    u32 extensionNum = (u32)requiredExtensions.size();
    u32 layerNum = (u32)requiredLayers.size();

    VkLogDevice = GetLogicalDevice(VkPhyDevice, VkPhyDeviceIndices);

    // Graphics queue

    VkGraphicsQueue = GetGraphicsQueue(VkLogDevice, VkPhyDeviceIndices);

    // SwapChain

    VkCurSwapChain = GetSwapChain();

    // Image views

    VkSwapChainImageViews = GetImageViews();

    // Compile shaders

    const char *fragPath = "shaders/shader.frag";
    const char *vertPath = "shaders/shader.vert";

    auto fragCodeCompiled = GenerateShader(fragPath, shaderc_glsl_fragment_shader);
    auto vertCodeCompiled = GenerateShader(vertPath, shaderc_glsl_vertex_shader);

    // vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {};
    // fragmentShaderModuleCreateInfo.codeSize = fragmentCodeCompiled.size() * sizeof(glm::uint);
    // fragmentShaderModuleCreateInfo.pCode = fragmentCodeCompiled.data();
}

void Render::Run()
{
}

void Render::Exit()
{
    if (PetDebug)
    {
        auto fn =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VkInstance, "vkDestroyDebugUtilsMessengerEXT");

        if (fn)
            fn(VkInstance, VkMessenger, nullptr);
    }

    for (auto &&imageView : VkSwapChainImageViews)
    {
        vkDestroyImageView(VkLogDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(VkLogDevice, VkCurSwapChain, nullptr);
    vkDestroyDevice(VkLogDevice, nullptr);
    vkDestroySurfaceKHR(VkInstance, VkSurface, nullptr);
    vkDestroyInstance(VkInstance, nullptr);

    glfwDestroyWindow(Window);
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
    VkApplicationInfo info{};

    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "Pet";
    info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    info.pEngineName = "PetEngine";
    info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    info.apiVersion = VK_API_VERSION_1_0;

    return info;
}

VkInstanceCreateInfo Render::PopulateVkInstanceInfo(const VkApplicationInfo &vkAppInfo,
                                                    const list<const char *> &requiredExtensions,
                                                    const VkDebugUtilsMessengerCreateInfoEXT &messengerInfo)
{
    VkInstanceCreateInfo info{};

    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &vkAppInfo;
    info.enabledExtensionCount = (u32)requiredExtensions.size();
    info.ppEnabledExtensionNames = requiredExtensions.data();
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = nullptr;
    info.pNext = nullptr;

    if (PetDebug)
    {
        info.enabledLayerCount = (u32)VkValidationLayers.size();
        info.ppEnabledLayerNames = VkValidationLayers.data();
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&messengerInfo;
    }

    return info;
}

VkDebugUtilsMessengerCreateInfoEXT Render::PopulateVkMessengerInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT info{};

    if (PetDebug)
    {
        info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | //
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;    //
        info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |         //
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |      //
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;      //
        info.pfnUserCallback = DebugCallback;
        info.pUserData = nullptr;
    }

    return info;
}

VkWin32SurfaceCreateInfoKHR Render::PopulateVkSurface()
{
    VkWin32SurfaceCreateInfoKHR info{};

    info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.hwnd = glfwGetWin32Window(Window);
    info.hinstance = GetModuleHandle(nullptr);

    return info;
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

    if (PetDebug)
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

#pragma region Physical Device Validation

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

    if (!RateAvailableQueueFamilies(device))
        score = 0;
    else if (!RateExtensionSupport(device))
        score = 0;
    else if (!RateSwapChainDetails(device))
        score = 0;

    //

    return score;
}

bool Render::RateAvailableQueueFamilies(const VkPhysicalDevice &device)
{
    auto idx = GetAvailableQueuesFamilies(device);
    return idx.IsComplete();
}

bool Render::RateExtensionSupport(const VkPhysicalDevice &device)
{
    u32 count;

    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    list<VkExtensionProperties> availableExtensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());

    oset<str> requiredExtensions(VkDeviceExtensions.begin(), VkDeviceExtensions.end());

    for (const auto &extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

bool Render::RateSwapChainDetails(const VkPhysicalDevice &device)
{
    auto details = GetSwapChainSupportDetails(device);
    return !details.formats.empty() && !details.presentModes.empty();
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

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VkSurface, &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        // TODO: It is possible to check for queues who have both
        // graphics and present support for added performance
    }

    return indices;
}

#pragma endregion

#pragma region Logical Device Validation

VkDevice Render::GetLogicalDevice(const VkPhysicalDevice &vkPhyDevice, const QueueFamilyIndices &vkPhyDeviceIndices)
{
    VkDevice device;

    float queuePriority = 1.0f;

    // Queue info

    list<VkDeviceQueueCreateInfo> queueInfos;

    oset<uint32_t> uniqueQueueFamilies = {VkPhyDeviceIndices.graphicsFamily.value(),
                                          VkPhyDeviceIndices.presentFamily.value()};

    for (auto queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueInfo{};

        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;

        queueInfos.push_back(queueInfo);
    }

    // Features info

    VkPhysicalDeviceFeatures deviceFeatures{};

    // Create logical device

    VkDeviceCreateInfo deviceInfo{};

    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = (u32)queueInfos.size();
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;

    deviceInfo.enabledExtensionCount = (u32)VkDeviceExtensions.size();
    deviceInfo.ppEnabledExtensionNames = VkDeviceExtensions.data();

    if (PetDebug)
    {
        deviceInfo.enabledLayerCount = (u32)VkValidationLayers.size();
        deviceInfo.ppEnabledLayerNames = VkValidationLayers.data();
    }

    if (vkCreateDevice(vkPhyDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");

    return device;
}

#pragma endregion

#pragma region Graphics queue

VkQueue Render::GetGraphicsQueue(const VkDevice &vkLogDevice, const QueueFamilyIndices &indices)
{
    VkQueue queue{};

    vkGetDeviceQueue(vkLogDevice, indices.presentFamily.value(), 0, &queue);

    return queue;
}

#pragma endregion

#pragma region SwapChain

SwapChainSupportDetails Render::GetSwapChainSupportDetails(const VkPhysicalDevice &device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VkSurface, &details.capabilities);

    u32 count;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, VkSurface, &count, nullptr);
    details.formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, VkSurface, &count, details.formats.data());

    count = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, VkSurface, &count, nullptr);
    details.presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, VkSurface, &count, details.presentModes.data());

    return details;
}

VkSurfaceFormatKHR Render::ChooseSwapSurfaceFormat(const list<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;

    return availableFormats[0];
}

VkPresentModeKHR Render::ChooseSwapPresentMode(const list<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Render::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != lim<uint32_t>::max())
        return capabilities.currentExtent;

    int width, height;
    glfwGetFramebufferSize(Window, &width, &height);

    VkExtent2D extent = {(u32)width, (u32)height};

    extent.width = clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}

VkSwapchainKHR Render::GetSwapChain(VkSwapchainKHR *oldSwapChain)
{
    VkSwapchainKHR swapChain;

    auto swapChainSupport = GetSwapChainSupportDetails(VkPhyDevice);
    auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = ChooseSwapExtent(swapChainSupport.capabilities);

    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = VkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    VkSwapChainImageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    VkSwapChainExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 queueFamilyIndices[] = {VkPhyDeviceIndices.graphicsFamily.value(), VkPhyDeviceIndices.presentFamily.value()};

    if (VkPhyDeviceIndices.graphicsFamily != VkPhyDeviceIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapChain ? *oldSwapChain : VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(VkLogDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create swap chain!");

    vkGetSwapchainImagesKHR(VkLogDevice, swapChain, &imageCount, nullptr);
    VkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(VkLogDevice, swapChain, &imageCount, VkSwapChainImages.data());

    return swapChain;
}

#pragma endregion

#pragma region Image Views

list<VkImageView> Render::GetImageViews()
{
    list<VkImageView> imageViews;
    imageViews.resize(VkSwapChainImages.size());

    for (size_t i = 0; i < imageViews.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = VkSwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VkSwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(VkLogDevice, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("\nFailed to create image views!");
    }

    return imageViews;
}

#pragma endregion

#pragma region Pipeline

list<char> Render::GenerateShader(str path, shaderc_shader_kind kind)
{
#ifdef _DEBUG

    // Get the file text

    auto file = std::ifstream(path + ".spv", std::ios::ate | std::ios::binary);
    auto fileSize = (u64)file.tellg();
    auto fileBuffer = list<char>(fileSize);
    file.seekg(0);
    file.read(fileBuffer.data(), fileSize);
    file.close();

    // Return the compiled shader

    return fileBuffer;

#else

    // Get the file text

    auto file = std::ifstream(path, std::ios::ate);
    auto fileSize = (u64)file.tellg();
    auto fileBuffer = str(fileSize, '\0');
    file.seekg(0);
    file.read(fileBuffer.data(), fileSize);
    file.close();

    // Compile shader

    auto compiler = shaderc::Compiler();
    auto options = shaderc::CompileOptions();
    options.SetOptimizationLevel(shaderc_optimization_level_size);

    const char *fragmentCode = "#version 450 \n layout(location = 0) out vec4 outColor; \n void main(){ } \n";

    auto svCompilationResult = compiler.CompileGlslToSpv(fileBuffer.data(), kind, "shaderc_s", options);

    if (svCompilationResult.GetCompilationStatus() != shaderc_compilation_status_success)
        throw std::runtime_error("\nFailed to compile shader!");

    auto compiledShader = list<char>(svCompilationResult.cbegin(), svCompilationResult.cend());

    // Write compiled shader to a file

    auto save = std::ofstream(path + ".spv", std::ios::binary);
    save.write(compiledShader.data(), compiledShader.size());

    // Return the compiled shader

    return compiledShader;

#endif
}

VkShaderModule Render::GetShaderModule(const list<char> &shader)
{
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
