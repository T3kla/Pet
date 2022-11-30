#pragma once

#include "core.h"

using namespace glm;

struct QueueFamilyIndices
{
    opt<uint32_t> graphicsFamily;
    opt<uint32_t> presentFamily;

    bool IsComplete();
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    list<VkSurfaceFormatKHR> formats;
    list<VkPresentModeKHR> presentModes;
};

class Render
{
    static Render Instance;

  private:
    static GLFWwindow *Window;
    static const char *WindowTitle;
    static i32vec2 WindowSize;

    static VkInstance VkInstance;
    static VkDebugUtilsMessengerEXT VkMessenger;

    static VkPhysicalDevice VkPhyDevice;
    static QueueFamilyIndices VkPhyDeviceIndices;
    static VkDevice VkLogDevice;

    static VkQueue VkGraphicsQueue;
    static VkSurfaceKHR VkSurface;

    static VkSwapchainKHR VkCurSwapChain;
    static VkSwapchainKHR VkOldSwapChain;
    static list<VkImage> VkSwapChainImages;
    static VkFormat VkSwapChainImageFormat;
    static VkExtent2D VkSwapChainExtent;
    static list<VkImageView> VkSwapChainImageViews;

  public:
  private:
    Render() = default;
    Render(const Render &) = delete;
    ~Render() = default;

    static GLFWwindow *InitializeGLFW();
    static VkApplicationInfo PopulateVkAppInfo();
    static VkInstanceCreateInfo PopulateVkInstanceInfo(const VkApplicationInfo &vkAppInfo,
                                                       const list<const char *> &requiredExtensions,
                                                       const VkDebugUtilsMessengerCreateInfoEXT &messengerInfo);
    static VkDebugUtilsMessengerCreateInfoEXT PopulateVkMessengerInfo();
    static VkWin32SurfaceCreateInfoKHR PopulateVkSurface();

    // Extension validation

    static list<const char *> GetRequiredExtensions();
    static list<VkExtensionProperties> GetAvailableExtensions();
    static bool ValidateExtensions(const list<const char *> &required, const list<VkExtensionProperties> &available);

    // Layer validation

    static list<VkLayerProperties> GetAvailableLayers();
    static bool ValidateLayers(const list<const char *> &required, const list<VkLayerProperties> &available);

    // Physical Device validation

    static VkPhysicalDevice GetMostSuitableDevice();
    static u32 RateDevice(const VkPhysicalDevice &device);
    static bool RateAvailableQueueFamilies(const VkPhysicalDevice &device);
    static bool RateExtensionSupport(const VkPhysicalDevice &device);
    static bool RateSwapChainDetails(const VkPhysicalDevice &device);

    // Queue family validation

    static QueueFamilyIndices GetAvailableQueuesFamilies(const VkPhysicalDevice &device);

    // Logic Device validation

    static VkDevice GetLogicalDevice(const VkPhysicalDevice &vkPhyDevice, const QueueFamilyIndices &vkPhyDeviceIndices);

    // Graphics queue

    static VkQueue GetGraphicsQueue(const VkDevice &vkLogDevice, const QueueFamilyIndices &indices);

    // SwapChain

    static SwapChainSupportDetails GetSwapChainSupportDetails(const VkPhysicalDevice &device);
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const list<VkSurfaceFormatKHR> &availableFormats);
    static VkPresentModeKHR ChooseSwapPresentMode(const list<VkPresentModeKHR> &availablePresentModes);
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    static VkSwapchainKHR GetSwapChain(VkSwapchainKHR *oldSwapChain = nullptr);

    // Image views

    static list<VkImageView> GetImageViews();

    // Graphics pipeline

    static void GetGraphicsPipeline();

    //

    static void OnWindowResize(GLFWwindow *window, int width, int height);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

  public:
    static void Init();
    static void Run();
    static void Exit();

    static i32vec2 GetWindowSize();
    static GLFWwindow *GetWindow();
};
