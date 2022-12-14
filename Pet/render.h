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

struct FramesInFlight
{
    list<VkCommandBuffer> cmdBuffers;
    list<VkSemaphore> imgSemaphores; // image available
    list<VkSemaphore> rndSemaphores; // render finished
    list<VkFence> fences;            // sync with cpu
    u32 current = 0;
    u32 size = 0;

    FramesInFlight() = default;
    FramesInFlight(const u32 &maxFramesInFlight = 2)
    {
        cmdBuffers.resize(maxFramesInFlight);
        imgSemaphores.resize(maxFramesInFlight);
        rndSemaphores.resize(maxFramesInFlight);
        fences.resize(maxFramesInFlight);
        size = maxFramesInFlight;
    }
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
    VkInstanceCreateInfo _vkInstanceInfo;
    VkApplicationInfo _vkAppInfo;
    VkDebugUtilsMessengerEXT _vkMessenger;
    VkDebugUtilsMessengerCreateInfoEXT _vkMessengerInfo;
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
    list<VkImageView> _vkImageViews;
    list<VkFramebuffer> _vkFramesBuffer;
    bool _framebufferResized = false;
    VkRenderPass _vkRenderPass;
    VkPipelineLayout _vkPipeLayout;
    VkPipeline _vkPipe;
    VkCommandPool _vkCmdPool;
    FramesInFlight _frames = FramesInFlight(2);

    GLFWwindow *InitializeGLFW();

    void PopulateVkAppInfo(VkApplicationInfo &info);
    void PopulateVkMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &info);
    void AttatchDebugMessenger();

    // Extension validation

    list<const char *> GetRequiredExtensions();
    list<VkExtensionProperties> GetAvailableExtensions();
    bool ValidateExtensions(const list<const char *> &required, const list<VkExtensionProperties> &available);

    // Layer validation

    list<VkLayerProperties> GetAvailableLayers();
    bool ValidateLayers(const list<const char *> &required, const list<VkLayerProperties> &available);

    // Instance

    void PopulateVkInstanceInfo(VkInstanceCreateInfo &info, const list<const char *> &requiredExtensions);
    void GetVkInstance(VkInstance &instance);

    void GetSurface(VkSurfaceKHR &surface);

    void GetMostSuitableDevice(VkPhysicalDevice &device);
    u32 RateDevice(const VkPhysicalDevice &device);
    bool RateAvailableQueueFamilies(const VkPhysicalDevice &device);
    bool RateExtensionSupport(const VkPhysicalDevice &device);
    bool RateSwapChainDetails(const VkPhysicalDevice &device);

    void GetAvailableQueuesFamilies(QueueFamilyIndices &indices, const VkPhysicalDevice &device);

    //

    void GetLogicalDevice(VkDevice &device);
    void GetGraphicsQueue(VkQueue &queue);

    void GetSwapChain(VkSwapchainKHR *cur, VkSwapchainKHR *old = nullptr);
    void RecreateSwapChain();

    SwapChainSupportDetails GetSwapChainSupportDetails(const VkPhysicalDevice &device);
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const list<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const list<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    void GetImageViews(list<VkImageView> &views);

    void GetRenderPass(VkRenderPass &pass);
    void GetPipeline(VkPipeline &pipe, VkPipelineLayout &layout);

    list<char> GenerateShader(str path, shaderc_shader_kind kind);
    VkShaderModule GetShaderModule(const list<char> &shader);

    void GetFramesBuffer(list<VkFramebuffer> &buffer);

    // Commands

    void GetCommandPool(VkCommandPool &pool);
    void PopulateFrames(FramesInFlight &frames);

    void RecordCommandBuffer(const VkCommandBuffer &buffer, u32 idx);

    //

    static void OnWindowResize(GLFWwindow *window, int width, int height);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    void VkCleanup();
    void VkCleanupSwapChain();

  public:
    Render();
    ~Render();

    void Init();
    void Run();
    void Exit();

    i32vec2 GetWindowSize();
    GLFWwindow *GetWindow();
};
