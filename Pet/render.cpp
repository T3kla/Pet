#include "render.h"

constexpr u32 MAX_FRAMES_IN_FLIGHT = 2;

Render *Render::_instance;

Render *Render::Instance()
{
    return _instance;
}

Render::Render()
{
    if (_instance)
        LOG("\nInstance already exists");

    _instance = this;
}

Render::~Render()
{
}

bool QueueFamilyIndices::IsComplete()
{
    return _graphicsFamily.has_value() && _presentFamily.has_value();
}

void Render::Init()
{
    _window = InitializeGLFW();

    PopulateVkAppInfo(_vkAppInfo);

    // Validate extensions

    auto requiredExtensions = GetRequiredExtensions();
    auto availableExtensions = GetAvailableExtensions();

    if (!ValidateExtensions(requiredExtensions, availableExtensions))
        throw std::runtime_error("\nExtension validation failed!");

    std::cout << std::endl;

    // Validate layers

    auto &requiredLayers = _vkValidationLayers;
    auto availableLayers = GetAvailableLayers();

    if (!ValidateLayers(requiredLayers, availableLayers))
        throw std::runtime_error("\nLayer validation failed!");

    std::cout << std::endl;

    //

    PopulateVkMessengerInfo(_vkMessengerInfo);

    // Vulkan initialization

    PopulateVkInstanceInfo(_vkInstanceInfo, requiredExtensions);
    GetVkInstance(_vkInstance);

    AttatchDebugMessenger();
    GetSurface(_vkSurface);

    GetMostSuitableDevice(_vkPhyDevice);
    GetAvailableQueuesFamilies(_vkPhyDeviceIndices, _vkPhyDevice);
    GetLogicalDevice(_vkLogDevice);
    GetGraphicsQueue(_vkGraphicsQueue);
    GetSwapChain(&_vkCurSwapChain, nullptr);
    GetImageViews(_vkImageViews);
    GetRenderPass(_vkRenderPass);
    GetPipeline(_vkPipe, _vkPipeLayout);
    GetFramesBuffer(_vkFramesBuffer);
    GetCommandPool(_vkCmdPool);
    PopulateFrames(_frames);
}

void Render::Run()
{
    vkWaitForFences(_vkLogDevice, 1, &_frames.fences[_frames.current], VK_TRUE, UINT64_MAX);
    vkResetFences(_vkLogDevice, 1, &_frames.fences[_frames.current]);

    u32 idx;
    vkAcquireNextImageKHR(_vkLogDevice, _vkCurSwapChain, UINT64_MAX, _frames.imgSemaphores[_frames.current],
                          VK_NULL_HANDLE, &idx);

    vkResetCommandBuffer(_frames.cmdBuffers[_frames.current], 0);
    RecordCommandBuffer(_frames.cmdBuffers[_frames.current], idx);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_frames.imgSemaphores[_frames.current]};

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_frames.cmdBuffers[_frames.current];

    VkSemaphore signalSemaphores[] = {_frames.rndSemaphores[_frames.current]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_vkGraphicsQueue, 1, &submitInfo, _frames.fences[_frames.current]) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_vkCurSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &idx;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(_vkGraphicsQueue, &presentInfo);

    _frames.current = (_frames.current + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Render::Exit()
{
    VkCleanup();
}

void Render::VkCleanup()
{
    if (IsDebug)
    {
        auto fn =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vkInstance, "vkDestroyDebugUtilsMessengerEXT");

        if (fn)
            fn(_vkInstance, _vkMessenger, nullptr);
    }

    vkDeviceWaitIdle(_vkLogDevice);

    VkCleanupSwapChain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(_vkLogDevice, _frames.rndSemaphores[i], nullptr);
        vkDestroySemaphore(_vkLogDevice, _frames.imgSemaphores[i], nullptr);
        vkDestroyFence(_vkLogDevice, _frames.fences[i], nullptr);
    }

    vkDestroyCommandPool(_vkLogDevice, _vkCmdPool, nullptr);
    vkDestroyPipeline(_vkLogDevice, _vkPipe, nullptr);
    vkDestroyPipelineLayout(_vkLogDevice, _vkPipeLayout, nullptr);
    vkDestroyRenderPass(_vkLogDevice, _vkRenderPass, nullptr);
    vkDestroyDevice(_vkLogDevice, nullptr);
    vkDestroySurfaceKHR(_vkInstance, _vkSurface, nullptr);
    vkDestroyInstance(_vkInstance, nullptr);

    glfwDestroyWindow(_window);
    glfwTerminate();
}

void Render::VkCleanupSwapChain()
{
    for (auto const &imageView : _vkImageViews)
        vkDestroyImageView(_vkLogDevice, imageView, nullptr);

    for (auto const &framebuffer : _vkFramesBuffer)
        vkDestroyFramebuffer(_vkLogDevice, framebuffer, nullptr);

    vkDestroySwapchainKHR(_vkLogDevice, _vkCurSwapChain, nullptr);
}

GLFWwindow *Render::InitializeGLFW()
{
    if (glfwInit() == 0)
        throw std::runtime_error("\nGLFW Panicked!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto window = glfwCreateWindow(_windowSize.x, _windowSize.y, _windowTitle, nullptr, nullptr);

    if (glfwVulkanSupported() == 0)
        throw std::runtime_error("\nGLFW Vulkan Not Supported!");

    return window;
}

#pragma region Populate

void Render::PopulateVkAppInfo(VkApplicationInfo &info)
{
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "Pet";
    info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    info.pEngineName = "PetEngine";
    info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    info.apiVersion = VK_API_VERSION_1_0;
}

void Render::PopulateVkInstanceInfo(VkInstanceCreateInfo &info, const list<const char *> &requiredExtensions)
{
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &_vkAppInfo;
    info.enabledExtensionCount = (u32)requiredExtensions.size();
    info.ppEnabledExtensionNames = requiredExtensions.data();
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = nullptr;
    info.pNext = nullptr;

    if (IsDebug)
    {
        info.enabledLayerCount = (u32)_vkValidationLayers.size();
        info.ppEnabledLayerNames = _vkValidationLayers.data();
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&_vkMessengerInfo;
    }
}

void Render::PopulateVkMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &info)
{
    if (!IsDebug)
        return;

    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | //
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;    //
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |         //
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |      //
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;      //
    info.pfnUserCallback = DebugCallback;
    info.pUserData = nullptr;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Render::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                     void *pUserData)
{
    std::cerr << "[VK]: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Render::AttatchDebugMessenger()
{
    if (!IsDebug)
        return;

    auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_vkInstance, "vkCreateDebugUtilsMessengerEXT");

    if (!fn)
        throw std::runtime_error("\nFailed to create debug messenger!");

    fn(_vkInstance, &_vkMessengerInfo, nullptr, &_vkMessenger);
}

#pragma endregion

#pragma region Extension Validation

list<const char *> Render::GetRequiredExtensions()
{
    u32 count;

    auto **raw = glfwGetRequiredInstanceExtensions(&count);
    auto extensions = list<const char *>(raw, raw + count);

    if (IsDebug)
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

    for (const auto &req : _vkValidationLayers)
        for (const auto &avl : available)
            if (strcmp(req, avl.layerName) == 0)
                validated++;

    std::cout << "Required layers validated: " << validated << "/" << reqCount << std::endl;

    return validated == reqCount;
}

#pragma endregion

#pragma region Vulkan Instance

void Render::GetVkInstance(VkInstance &instance)
{
    if (vkCreateInstance(&_vkInstanceInfo, nullptr, &_vkInstance) != VK_SUCCESS)
        throw std::runtime_error("\nVulkan failed to create instance!");

    std::cout << std::endl << "Vulkan instance created!" << std::endl;
}

#pragma endregion

#pragma region Physical Device Validation

void Render::GetMostSuitableDevice(VkPhysicalDevice &device)
{
    u32 deviceCount = 0;

    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, nullptr);
    list<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, devices.data());

    if (deviceCount == 0)
        throw std::runtime_error("\nFailed to find GPUs with Vulkan support!");

    odic<u32, VkPhysicalDevice> candidates;

    for (const auto &device : devices)
        candidates.insert({RateDevice(device), device});

    if (candidates.rbegin()->first > 0)
        device = candidates.rbegin()->second;
    else
        throw std::runtime_error("\nFailed to find a suitable GPU!");
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

    return score;
}

bool Render::RateAvailableQueueFamilies(const VkPhysicalDevice &device)
{
    auto indices = QueueFamilyIndices();
    GetAvailableQueuesFamilies(indices, device);
    return indices.IsComplete();
}

void Render::GetSurface(VkSurfaceKHR &surface)
{
    if (glfwCreateWindowSurface(_vkInstance, _window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create window surface!");
}

void Render::GetAvailableQueuesFamilies(QueueFamilyIndices &indices, const VkPhysicalDevice &device)
{
    u32 queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    list<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    odic<u8, u32> candidates;
    VkBool32 presentSupport = false;

    for (u32 i = 0; i < queueFamilies.size(); i++)
    {
        u8 flag = 0b00;

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            flag = flag | 0b01;

        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _vkSurface, &presentSupport);

        if (presentSupport)
            flag = flag | 0b10;

        candidates.insert({flag, i});
    }

    for (auto const &[flag, index] : candidates)
    {
        if (flag & 0b01 && flag & 0b10)
        {
            indices._graphicsFamily = index;
            indices._presentFamily = index;
            break;
        }
    }

    if (!indices.IsComplete())
        throw std::runtime_error("\nFailed to find a suitable queue!");
}

bool Render::RateExtensionSupport(const VkPhysicalDevice &device)
{
    u32 count;

    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    list<VkExtensionProperties> availableExtensions(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, availableExtensions.data());

    oset<str> requiredExtensions(_vkDeviceExtensions.begin(), _vkDeviceExtensions.end());

    for (const auto &extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

bool Render::RateSwapChainDetails(const VkPhysicalDevice &device)
{
    auto details = GetSwapChainSupportDetails(device);
    return !details._formats.empty() && !details._presentModes.empty();
}

#pragma endregion

#pragma region Logical Device Validation

void Render::GetLogicalDevice(VkDevice &device)
{
    float queuePriority = 1.0f;

    // Queue info

    list<VkDeviceQueueCreateInfo> queueInfos;

    oset<uint32_t> uniqueQueueFamilies = {_vkPhyDeviceIndices._graphicsFamily.value(),
                                          _vkPhyDeviceIndices._presentFamily.value()};

    for (auto const &queueFamily : uniqueQueueFamilies)
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

    deviceInfo.enabledExtensionCount = (u32)_vkDeviceExtensions.size();
    deviceInfo.ppEnabledExtensionNames = _vkDeviceExtensions.data();

    if (IsDebug)
    {
        deviceInfo.enabledLayerCount = (u32)_vkValidationLayers.size();
        deviceInfo.ppEnabledLayerNames = _vkValidationLayers.data();
    }

    if (vkCreateDevice(_vkPhyDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");
}

#pragma endregion

#pragma region SwapChain

void Render::GetGraphicsQueue(VkQueue &queue)
{
    vkGetDeviceQueue(_vkLogDevice, _vkPhyDeviceIndices._presentFamily.value(), 0, &queue);
}

SwapChainSupportDetails Render::GetSwapChainSupportDetails(const VkPhysicalDevice &device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _vkSurface, &details._capabilities);

    u32 count;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &count, nullptr);
    details._formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vkSurface, &count, details._formats.data());

    count = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &count, nullptr);
    details._presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vkSurface, &count, details._presentModes.data());

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
    glfwGetFramebufferSize(_window, &width, &height);

    VkExtent2D extent = {(u32)width, (u32)height};

    extent.width = clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}

void Render::GetSwapChain(VkSwapchainKHR *cur, VkSwapchainKHR *old)
{
    auto swapChainSupport = GetSwapChainSupportDetails(_vkPhyDevice);
    auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport._formats);
    auto presentMode = ChooseSwapPresentMode(swapChainSupport._presentModes);
    auto extent = ChooseSwapExtent(swapChainSupport._capabilities);

    u32 imageCount = swapChainSupport._capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    _vkSwapChainImageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    _vkSwapChainExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 queueFamilyIndices[] = {_vkPhyDeviceIndices._graphicsFamily.value(),
                                _vkPhyDeviceIndices._presentFamily.value()};

    if (_vkPhyDeviceIndices._graphicsFamily != _vkPhyDeviceIndices._presentFamily)
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

    createInfo.preTransform = swapChainSupport._capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = old ? *old : VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_vkLogDevice, &createInfo, nullptr, &*cur) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create swap chain!");

    vkGetSwapchainImagesKHR(_vkLogDevice, *cur, &imageCount, nullptr);
    _vkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_vkLogDevice, *cur, &imageCount, _vkSwapChainImages.data());
}

void Render::RecreateSwapChain()
{
    _vkOldSwapChain = _vkCurSwapChain;
    GetSwapChain(&_vkCurSwapChain, &_vkOldSwapChain);

    vkDeviceWaitIdle(_vkLogDevice);

    VkCleanupSwapChain();

    GetImageViews(_vkImageViews);
    GetFramesBuffer(_vkFramesBuffer);
}

#pragma endregion

#pragma region Image Views

void Render::GetImageViews(list<VkImageView> &views)
{
    views.resize(_vkSwapChainImages.size());

    for (size_t i = 0; i < views.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _vkSwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = _vkSwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(_vkLogDevice, &createInfo, nullptr, &views[i]) != VK_SUCCESS)
            throw std::runtime_error("\nFailed to create image views!");
    }
}

#pragma endregion

#pragma region Pipeline

void Render::GetRenderPass(VkRenderPass &pass)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _vkSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_vkLogDevice, &renderPassInfo, nullptr, &pass) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create render pass!");
}

void Render::GetPipeline(VkPipeline &pipe, VkPipelineLayout &layout)
{
    const char *fragPath = "shaders/shader.frag";
    const char *vertPath = "shaders/shader.vert";

    // Compile or read shaders

    auto fragShader = GenerateShader(fragPath, shaderc_glsl_fragment_shader);
    auto vertShader = GenerateShader(vertPath, shaderc_glsl_vertex_shader);

    // Pack shaders in shader modules

    auto fragShaderModule = GetShaderModule(fragShader);
    auto vertShaderModule = GetShaderModule(vertShader);

    // Pack shader modules in shader stages

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Create pipeline layout

    list<VkDynamicState> dynStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (u32)dynStates.size();
    dynamicState.pDynamicStates = dynStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_vkSwapChainExtent.width;
    viewport.height = (float)_vkSwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _vkSwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;            // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(_vkLogDevice, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create pipeline layout!");

    // Create pipeline

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = _vkRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    if (vkCreateGraphicsPipelines(_vkLogDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipe) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create graphics pipeline!");

    vkDestroyShaderModule(_vkLogDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(_vkLogDevice, vertShaderModule, nullptr);
}

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

    // FIXME: doesn't compile correclty, must fix

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

    auto result = compiler.CompileGlslToSpv(fileBuffer.data(), kind, path.data(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        throw std::runtime_error("\nFailed to compile shader!");

    auto compiledShader = list<char>(result.cbegin(), result.cend());

    // Write compiled shader to a file

    auto save = std::ofstream(path + ".spv", std::ios::out | std::ios::trunc | std::ios::binary);
    save.write(compiledShader.data(), compiledShader.size());
    save.close();

    // Return the compiled shader

    return compiledShader;

#endif
}

VkShaderModule Render::GetShaderModule(const list<char> &shader)
{
    VkShaderModuleCreateInfo info{};

    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = shader.size();
    info.pCode = reinterpret_cast<const u32 *>(shader.data());

    VkShaderModule module{};

    if (vkCreateShaderModule(_vkLogDevice, &info, nullptr, &module) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create shader module!");

    return module;
}

#pragma endregion

#pragma region Frames buffer

void Render::GetFramesBuffer(list<VkFramebuffer> &buffer)
{
    buffer.resize(_vkImageViews.size());

    for (size_t i = 0; i < _vkImageViews.size(); i++)
    {
        VkImageView attachments[] = {_vkImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _vkRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _vkSwapChainExtent.width;
        framebufferInfo.height = _vkSwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_vkLogDevice, &framebufferInfo, nullptr, &buffer[i]) != VK_SUCCESS)
            throw std::runtime_error("\nFailed to create framebuffer!");
    }
}

#pragma endregion

#pragma region Commands

void Render::GetCommandPool(VkCommandPool &pool)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = _vkPhyDeviceIndices._graphicsFamily.value();

    if (vkCreateCommandPool(_vkLogDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create command pool!");
}

void Render::PopulateFrames(FramesInFlight &frames)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _vkCmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = frames.size;

    if (vkAllocateCommandBuffers(_vkLogDevice, &allocInfo, frames.cmdBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to allocate command buffers!");

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < frames.size; i++)
        if (vkCreateSemaphore(_vkLogDevice, &semaphoreInfo, nullptr, &frames.imgSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_vkLogDevice, &semaphoreInfo, nullptr, &frames.rndSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_vkLogDevice, &fenceInfo, nullptr, &frames.fences[i]) != VK_SUCCESS)
            throw std::runtime_error("\nFailed to create synchronization objects for a frame!");
}

void Render::RecordCommandBuffer(const VkCommandBuffer &buffer, u32 idx)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _vkRenderPass;
    renderPassInfo.framebuffer = _vkFramesBuffer[idx];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _vkSwapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vkPipe);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_vkSwapChainExtent.width;
    viewport.height = (float)_vkSwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _vkSwapChainExtent;
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    vkCmdDraw(buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(buffer);

    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to record command buffer!");
}

#pragma endregion

void Render::OnWindowResize(GLFWwindow *window, int width, int height)
{
    _windowSize = vec2(width, height);
}

i32vec2 Render::GetWindowSize()
{
    return _windowSize;
}

GLFWwindow *Render::GetWindow()
{
    return _window;
}
