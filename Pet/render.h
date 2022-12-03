#pragma once

#include "core.h"

#include "shaderc/shaderc.hpp"

using namespace glm;

struct QueueFamilyIndices
{
    opt<uint32_t> _graphicsFamily;
    opt<uint32_t> _presentFamily;

    bool IsComplete();
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR _capabilities{};
    list<VkSurfaceFormatKHR> _formats;
    list<VkPresentModeKHR> _presentModes;
};

class Render
{
    // Static

  private:
    static Render *_instance;

  public:
    static Render *Instance();

    // Instance

  private:
    GLFWwindow *_window = nullptr;
    const char *_windowTitle = "PetProject";
    i32vec2 _windowSize = i32vec2(800, 600);

    const list<const char *> _vkValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const list<const char *> _vkDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance _vkInstance;
    VkDebugUtilsMessengerEXT _vkMessenger;

    VkPhysicalDevice _vkPhyDevice;
    QueueFamilyIndices _vkPhyDeviceIndices;
    VkDevice _vkLogDevice;

    VkQueue _vkGraphicsQueue;
    VkSurfaceKHR _vkSurface;

    VkSwapchainKHR _vkCurSwapChain;
    VkSwapchainKHR _vkOldSwapChain;
    list<VkImage> _vkSwapChainImages;
    VkFormat _vkSwapChainImageFormat;
    VkExtent2D _vkSwapChainExtent;
    list<VkImageView> _vkSwapChainImageViews;
    list<VkFramebuffer> _vkFramesBuffer;

    VkRenderPass _vkPasses;
    VkPipelineLayout _vkPipeLayout;
    VkPipeline _vkPipe;

    VkCommandPool _vkCmdPool;

    list<VkCommandBuffer> _vkCmdBuffers;
    list<VkSemaphore> _imageAvailableSemaphores;
    list<VkSemaphore> _renderFinishedSemaphores;
    list<VkFence> _inFlightFences;
    u32 _curFrame;

    GLFWwindow *InitializeGLFW();

    VkApplicationInfo PopulateVkAppInfo();
    VkInstanceCreateInfo PopulateVkInstanceInfo(const VkApplicationInfo &vkAppInfo,
                                                const list<const char *> &requiredExtensions,
                                                const VkDebugUtilsMessengerCreateInfoEXT &messengerInfo);
    VkDebugUtilsMessengerCreateInfoEXT PopulateVkMessengerInfo();
    VkWin32SurfaceCreateInfoKHR PopulateVkSurface();
    void PopulateSyncObjects();

    // Extension validation

    list<const char *> GetRequiredExtensions();
    list<VkExtensionProperties> GetAvailableExtensions();
    bool ValidateExtensions(const list<const char *> &required, const list<VkExtensionProperties> &available);

    // Layer validation

    list<VkLayerProperties> GetAvailableLayers();
    bool ValidateLayers(const list<const char *> &required, const list<VkLayerProperties> &available);

    // Physical Device validation

    VkPhysicalDevice GetMostSuitableDevice();
    u32 RateDevice(const VkPhysicalDevice &device);
    bool RateAvailableQueueFamilies(const VkPhysicalDevice &device);
    bool RateExtensionSupport(const VkPhysicalDevice &device);
    bool RateSwapChainDetails(const VkPhysicalDevice &device);

    // Queue family validation

    QueueFamilyIndices GetAvailableQueuesFamilies(const VkPhysicalDevice &device);

    // Logic Device validation

    VkDevice GetLogicalDevice(const VkPhysicalDevice &vkPhyDevice, const QueueFamilyIndices &vkPhyDeviceIndices);

    // Graphics queue

    VkQueue GetGraphicsQueue(const VkDevice &vkLogDevice, const QueueFamilyIndices &indices);

    // SwapChain

    SwapChainSupportDetails GetSwapChainSupportDetails(const VkPhysicalDevice &device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const list<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const list<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    VkSwapchainKHR GetSwapChain(VkSwapchainKHR *oldSwapChain = nullptr);

    // Image views

    list<VkImageView> GetImageViews();

    // Pipeline

    VkRenderPass GetRenderPass();
    void GetPipeline(VkPipeline &pipe, VkPipelineLayout &layout);

    list<char> GenerateShader(str path, shaderc_shader_kind kind);
    VkShaderModule GetShaderModule(const list<char> &shader);

    // Framebuffer

    list<VkFramebuffer> GetFramesBuffer();

    // Commands

    VkCommandPool GetCommandPool();
    list<VkCommandBuffer> GetCommandBuffers();

    void RecordCommandBuffer(const VkCommandBuffer &buffer, u32 idx);

    //

    void OnWindowResize(GLFWwindow *window, int width, int height);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

  public:
    Render();
    ~Render();

    void Init();
    void Run();
    void Exit();

    i32vec2 GetWindowSize();
    GLFWwindow *GetWindow();
};
