#include "render.h"

#include "engine.h"

bool QueueFamilyIndices::IsComplete() const
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

void Render::Init()
{
    window = InitializeGLFW();

    PopulateVkAppInfo(vkAppInfo);

    // Validate extensions

    auto requiredExtensions = GetRequiredExtensions();
    auto availableExtensions = GetAvailableExtensions();

    if (!ValidateExtensions(requiredExtensions, availableExtensions))
        throw std::runtime_error("\nExtension validation failed!");

    std::cout << std::endl;

    // Validate layers

    auto &requiredLayers = vkValidationLayers;
    auto availableLayers = GetAvailableLayers();

    if (!ValidateLayers(requiredLayers, availableLayers))
        throw std::runtime_error("\nLayer validation failed!");

    std::cout << std::endl;

    //

    PopulateVkMessengerInfo(vkMessengerInfo);

    // Vulkan initialization

    PopulateVkInstanceInfo(vkInstanceInfo, requiredExtensions);
    GetVkInstance(vkInstance);

    AttachDebugMessenger();
    GetSurface(vkSurface);

    GetMostSuitableDevice(vkPhyDevice);
    GetAvailableQueuesFamilies(vkPhyDeviceIndices, vkPhyDevice);
    GetLogicalDevice(vkLogDevice);
    GetGraphicsQueue(vkGraphicsQueue);
    GetSwapChain(&vkCurSwapChain, nullptr);
    GetImageViews(vkImageViews);
    GetRenderPass(vkRenderPass);
    GetPipeline(vkPipe, vkPipeLayout);
    GetFramesBuffer(vkFramesBuffer);
    GetCommandPool(vkCmdPool);
    GetVertexBuffer(vkVertexBuffer, vkVertexMemory);
    PopulateFrames(frames);
}

void Render::Run()
{
    vkWaitForFences(vkLogDevice, 1, &frames.fences[frames.current], VK_TRUE, UINT64_MAX);

    u32 idx;
    auto result = vkAcquireNextImageKHR(vkLogDevice, vkCurSwapChain, UINT64_MAX, frames.imgSemaphores[frames.current],
                                        VK_NULL_HANDLE, &idx);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("\nFailed to acquire swap chain image!");

    vkResetFences(vkLogDevice, 1, &frames.fences[frames.current]);

    vkResetCommandBuffer(frames.cmdBuffers[frames.current], 0);
    RecordCommandBuffer(frames.cmdBuffers[frames.current], idx);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {frames.imgSemaphores[frames.current]};

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frames.cmdBuffers[frames.current];

    VkSemaphore signalSemaphores[] = {frames.rndSemaphores[frames.current]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, frames.fences[frames.current]) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vkCurSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &idx;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(vkGraphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized)
    {
        frameBufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    frames.current = (frames.current + 1) % frames.size;
}

void Render::Exit()
{
    VkCleanup();
}

void Render::VkCleanup()
{
    if (IsDebug)
        if (auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT")))
            fn(vkInstance, vkMessenger, nullptr);

    vkDeviceWaitIdle(vkLogDevice);

    for (const auto &imageView : vkImageViews)
        vkDestroyImageView(vkLogDevice, imageView, nullptr);
    for (const auto &frameBuffer : vkFramesBuffer)
        vkDestroyFramebuffer(vkLogDevice, frameBuffer, nullptr);
    vkDestroySwapchainKHR(vkLogDevice, vkCurSwapChain, nullptr);

    vkDestroyBuffer(vkLogDevice, vkVertexBuffer, nullptr);
    vkFreeMemory(vkLogDevice, vkVertexMemory, nullptr);

    for (size_t i = 0; i < frames.size; i++)
    {
        vkDestroySemaphore(vkLogDevice, frames.rndSemaphores[i], nullptr);
        vkDestroySemaphore(vkLogDevice, frames.imgSemaphores[i], nullptr);
        vkDestroyFence(vkLogDevice, frames.fences[i], nullptr);
    }

    vkDestroyCommandPool(vkLogDevice, vkCmdPool, nullptr);
    vkDestroyPipeline(vkLogDevice, vkPipe, nullptr);
    vkDestroyPipelineLayout(vkLogDevice, vkPipeLayout, nullptr);
    vkDestroyRenderPass(vkLogDevice, vkRenderPass, nullptr);
    vkDestroyDevice(vkLogDevice, nullptr);
    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
    vkDestroyInstance(vkInstance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Render::VkCleanupSwapChain()
{
}

GLFWwindow *Render::InitializeGLFW()
{
    if (glfwInit() == 0)
        throw std::runtime_error("\nGLFW Panicked!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto *ptr = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle, nullptr, nullptr);

    glfwSetFramebufferSizeCallback(ptr, &Render::OnWindowResize);

    if (glfwVulkanSupported() == 0)
        throw std::runtime_error("\nGLFW Vulkan Not Supported!");

    return ptr;
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
    info.pApplicationInfo = &vkAppInfo;
    info.enabledExtensionCount = static_cast<u32>(requiredExtensions.size());
    info.ppEnabledExtensionNames = requiredExtensions.data();
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = nullptr;
    info.pNext = nullptr;

    if (IsDebug)
    {
        info.enabledLayerCount = static_cast<u32>(vkValidationLayers.size());
        info.ppEnabledLayerNames = vkValidationLayers.data();
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&vkMessengerInfo;
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

void Render::AttachDebugMessenger()
{
    if (!IsDebug)
        return;

    if (auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT")))
        fn(vkInstance, &vkMessengerInfo, nullptr, &vkMessenger);
    else
        throw std::runtime_error("\nFailed to create debug messenger!");
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
    u32 reqCount = static_cast<u32>(required.size());
    u32 avlCount = static_cast<u32>(available.size());
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
    u32 reqCount = static_cast<u32>(required.size());
    u32 avlCount = static_cast<u32>(available.size());
    u32 validated = 0;

    std::cout << "Required layers [" << reqCount << "] :" << std::endl;
    for (const auto &layer : required)
        std::cout << "    " << layer << std::endl;

    std::cout << "Available layers [" << avlCount << "] :" << std::endl;
    for (const auto &layer : available)
        std::cout << "    " << layer.layerName << std::endl;

    for (const auto &req : vkValidationLayers)
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
    if (vkCreateInstance(&vkInstanceInfo, nullptr, &vkInstance) != VK_SUCCESS)
        throw std::runtime_error("\nVulkan failed to create instance!");

    std::cout << std::endl << "Vulkan instance created!" << std::endl;
}

#pragma endregion

#pragma region Physical Device Validation

void Render::GetMostSuitableDevice(VkPhysicalDevice &device)
{
    u32 deviceCount = 0;

    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
    list<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    if (deviceCount == 0)
        throw std::runtime_error("\nFailed to find GPUs with Vulkan support!");

    odic<u32, VkPhysicalDevice> candidates;

    for (const auto &dev : devices)
        candidates.insert({RateDevice(dev), dev});

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

    if (!RateAvailableQueueFamilies(device) || !RateExtensionSupport(device) || !RateSwapChainDetails(device))
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
    if (glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) != VK_SUCCESS)
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

        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);

        if (presentSupport)
            flag = flag | 0b10;

        candidates.insert({flag, i});
    }

    for (const auto &[flag, index] : candidates)
    {
        if (flag & 0b01 && flag & 0b10)
        {
            indices.graphicsFamily = index;
            indices.presentFamily = index;
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

    oset<str> requiredExtensions(vkDeviceExtensions.begin(), vkDeviceExtensions.end());

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

#pragma region Logical Device Validation

void Render::GetLogicalDevice(VkDevice &device)
{
    float queuePriority = 1.0f;

    // Queue info

    auto queueInfos = list<VkDeviceQueueCreateInfo>();
    auto uniqueQueueFamilies = oset<uint32_t>();

    if(vkPhyDeviceIndices.graphicsFamily.has_value())
		uniqueQueueFamilies.insert(vkPhyDeviceIndices.graphicsFamily.value());
	if (vkPhyDeviceIndices.presentFamily.has_value())
		uniqueQueueFamilies.insert(vkPhyDeviceIndices.presentFamily.value());

    for (auto const &queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueInfos.push_back(queueInfo);
    }

    // Create logical device

    VkDeviceCreateInfo deviceInfo{};

    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<u32>(queueInfos.size());
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.enabledExtensionCount = static_cast<u32>(vkDeviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = vkDeviceExtensions.data();

    if (IsDebug)
    {
        deviceInfo.enabledLayerCount = static_cast<u32>(vkValidationLayers.size());
        deviceInfo.ppEnabledLayerNames = vkValidationLayers.data();
    }

    if (vkCreateDevice(vkPhyDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("failed to create logical device!");
}

#pragma endregion

#pragma region SwapChain

void Render::GetGraphicsQueue(VkQueue &queue)
{
    auto presentFamily = vkPhyDeviceIndices.presentFamily.has_value() ? vkPhyDeviceIndices.presentFamily.value() : 0;
    vkGetDeviceQueue(vkLogDevice, presentFamily, 0, &queue);
}

SwapChainSupportDetails Render::GetSwapChainSupportDetails(const VkPhysicalDevice &device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &details.capabilities);

    u32 count;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &count, nullptr);
    details.formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &count, details.formats.data());

    count = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &count, nullptr);
    details.presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &count, details.presentModes.data());

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
    if (capabilities.currentExtent.width != limits<uint32_t>::max())
        return capabilities.currentExtent;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent = {static_cast<u32>(width), static_cast<u32>(height)};

    extent.width = glm::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = glm::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return extent;
}

void Render::GetSwapChain(VkSwapchainKHR *cur, VkSwapchainKHR *old)
{
    auto swapChainSupport = GetSwapChainSupportDetails(vkPhyDevice);
    auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = ChooseSwapExtent(swapChainSupport.capabilities);

    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    vkSwapChainImageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    vkSwapChainExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    u32 queueFamilyIndices[2] = {vkPhyDeviceIndices.graphicsFamily.value(), vkPhyDeviceIndices.presentFamily.value()};

    if (vkPhyDeviceIndices.graphicsFamily != vkPhyDeviceIndices.presentFamily)
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
    createInfo.oldSwapchain = old ? *old : VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vkLogDevice, &createInfo, nullptr, &*cur) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create swap chain!");

    vkGetSwapchainImagesKHR(vkLogDevice, *cur, &imageCount, nullptr);
    vkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vkLogDevice, *cur, &imageCount, vkSwapChainImages.data());
}

void Render::RecreateSwapChain()
{
    while (windowSize.x == 0 || windowSize.y == 0)
    {
        glfwGetFramebufferSize(window, &windowSize.x, &windowSize.y);
        glfwWaitEvents();
    }

    vkOldSwapChain = vkCurSwapChain;

    GetSwapChain(&vkCurSwapChain, &vkOldSwapChain);
    vkDeviceWaitIdle(vkLogDevice);

    for (const auto &imageView : vkImageViews)
        vkDestroyImageView(vkLogDevice, imageView, nullptr);
    for (const auto &frameBuffer : vkFramesBuffer)
        vkDestroyFramebuffer(vkLogDevice, frameBuffer, nullptr);
    vkDestroySwapchainKHR(vkLogDevice, vkOldSwapChain, nullptr);

    GetImageViews(vkImageViews);
    GetFramesBuffer(vkFramesBuffer);
}

#pragma endregion

#pragma region Image Views

void Render::GetImageViews(list<VkImageView> &views)
{
    views.resize(vkSwapChainImages.size());

    for (size_t i = 0; i < views.size(); i++)
    {
        VkImageViewCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vkSwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vkSwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkLogDevice, &createInfo, nullptr, &views[i]) != VK_SUCCESS)
            throw std::runtime_error("\nFailed to create image views!");
    }
}

#pragma endregion

#pragma region Pipeline

void Render::GetRenderPass(VkRenderPass &pass)
{
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = vkSwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vkLogDevice, &renderPassInfo, nullptr, &pass) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create render pass!");
}

void Render::GetPipeline(VkPipeline &pipe, VkPipelineLayout &layout)
{
    auto *fragPath = "shaders/shader.frag";
    auto *vertPath = "shaders/shader.vert";

    // Compile or read shaders

    auto fragShader = GenerateShader(fragPath, shaderc_glsl_fragment_shader);
    auto vertShader = GenerateShader(vertPath, shaderc_glsl_vertex_shader);

    // Pack shaders in shader modules

    auto fragShaderModule = GetShaderModule(fragShader);
    auto vertShaderModule = GetShaderModule(vertShader);

    // Pack shader modules in shader stages

    VkPipelineShaderStageCreateInfo vertShaderStageInfo;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Create pipeline layout

    list<VkDynamicState> dynStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynStates.size());
    dynamicState.pDynamicStates = dynStates.data();

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(vkSwapChainExtent.width);
    viewport.height = static_cast<float>(vkSwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = vkSwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer;
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

    VkPipelineMultisampleStateCreateInfo multisampling;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;            // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(vkLogDevice, &pipelineLayoutInfo, nullptr, &layout) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create pipeline layout!");

    // Create pipeline

    VkGraphicsPipelineCreateInfo pipelineInfo;
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
    pipelineInfo.renderPass = vkRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1;              // Optional

    if (vkCreateGraphicsPipelines(vkLogDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipe) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create graphics pipeline!");

    vkDestroyShaderModule(vkLogDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(vkLogDevice, vertShaderModule, nullptr);
}

list<char> Render::GenerateShader(const str &path, shaderc_shader_kind kind)
{
#ifdef _DEBUG

    // Get the file text

    auto file = std::ifstream(path + ".spv", std::ios::ate | std::ios::binary);
    auto fileSize = static_cast<i64>(file.tellg());
    auto fileBuffer = list<char>(fileSize);
    file.seekg(0);
    file.read(fileBuffer.data(), fileSize);
    file.close();

    // Return the compiled shader

    return fileBuffer;

#else

    // FIXME: doesn't compile correctly, must fix

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
    VkShaderModuleCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = shader.size();
    info.pCode = reinterpret_cast<const u32 *>(shader.data());

    VkShaderModule module;

    if (vkCreateShaderModule(vkLogDevice, &info, nullptr, &module) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create shader module!");

    return module;
}

#pragma endregion

#pragma region Buffer

void Render::GetFramesBuffer(list<VkFramebuffer> &buffer)
{
    buffer.resize(vkImageViews.size());

    for (size_t i = 0; i < vkImageViews.size(); i++)
    {
        VkImageView attachments[] = {vkImageViews[i]};

        VkFramebufferCreateInfo frameBufferInfo;
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = vkRenderPass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = vkSwapChainExtent.width;
        frameBufferInfo.height = vkSwapChainExtent.height;
        frameBufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkLogDevice, &frameBufferInfo, nullptr, &buffer[i]) != VK_SUCCESS)
            throw std::runtime_error("\nFailed to create frameBuffer!");
    }
}

void Render::GetVertexBuffer(VkBuffer &buffer, VkDeviceMemory &memory)
{
    VkBufferCreateInfo bufferInfo;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vkLogDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create vertex buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkLogDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(vkLogDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to allocate vertex buffer memory!");

    vkBindBufferMemory(vkLogDevice, buffer, memory, 0);

    void *data;
    vkMapMemory(vkLogDevice, memory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferInfo.size);
    vkUnmapMemory(vkLogDevice, memory);
}

#pragma endregion

#pragma region Commands

void Render::GetCommandPool(VkCommandPool &pool)
{
    VkCommandPoolCreateInfo poolInfo;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex =
        vkPhyDeviceIndices.graphicsFamily.has_value() ? vkPhyDeviceIndices.graphicsFamily.value() : 0;

    if (vkCreateCommandPool(vkLogDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to create command pool!");
}

void Render::PopulateFrames(FramesInFlight &framesInFlight)
{
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkCmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = framesInFlight.size;

    if (vkAllocateCommandBuffers(vkLogDevice, &allocInfo, framesInFlight.cmdBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to allocate command buffers!");

    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < framesInFlight.size; i++)
        if (vkCreateSemaphore(vkLogDevice, &semaphoreInfo, nullptr, &framesInFlight.imgSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vkLogDevice, &semaphoreInfo, nullptr, &framesInFlight.rndSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vkLogDevice, &fenceInfo, nullptr, &framesInFlight.fences[i]) != VK_SUCCESS)
            throw std::runtime_error("\nFailed to create synchronization objects for a frame!");
}

void Render::RecordCommandBuffer(const VkCommandBuffer &buffer, u32 idx)
{
    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = vkFramesBuffer[idx];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = vkSwapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipe);

    VkBuffer vertexBuffers[] = {vkVertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(vkSwapChainExtent.width);
    viewport.height = static_cast<float>(vkSwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = vkSwapChainExtent;
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    vkCmdDraw(buffer, static_cast<u32>(vertices.size()), 1, 0, 0);
    vkCmdEndRenderPass(buffer);

    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
        throw std::runtime_error("\nFailed to record command buffer!");
}

#pragma endregion

#pragma region Memory

u32 Render::FindMemoryType(const u32 &typeFilter, const VkMemoryPropertyFlags &flags)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhyDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & flags) == flags)
            return i;

    throw std::runtime_error("\nFailed to find suitable memory type!");
}

#pragma endregion

void Render::OnWindowResize(GLFWwindow *window, int width, int height)
{
    auto &render = App::Instance().render;
    render.frameBufferResized = true;
    render.windowSize = glm::vec2(width, height);
}

glm::i32vec2 Render::GetWindowSize()
{
    return windowSize;
}

GLFWwindow *Render::GetWindow()
{
    return window;
}
