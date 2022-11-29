#pragma once

#include "core.h"

using namespace glm;

struct QueueFamilyIndices
{
    opt<uint32_t> graphicsFamily;

    bool IsComplete();
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

    // Queue family validation

    static QueueFamilyIndices GetAvailableQueuesFamilies(const VkPhysicalDevice &device);

    // Logic Device validation

    static VkDevice GetLogicalDevice(const VkPhysicalDevice &vkPhyDevice, const QueueFamilyIndices &vkPhyDeviceIndices,
                                     u32 extensionNum, const list<const char *> &extensions, u32 layerNum,
                                     const list<const char *> &layers);

    // Graphics queue

    static VkQueue GetGraphicsQueue(const VkDevice &vkLogDevice, const QueueFamilyIndices &indices);

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
