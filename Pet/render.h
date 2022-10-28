#pragma once

#include "core.h"

using namespace glm;

class Render
{
  private:
    static Render Instance;

    static GLFWwindow *Window;
    static const char *WindowTitle;
    static i32vec2 WindowSize;

    static VkInstance VulkanInstance;
    static VkDebugUtilsMessengerEXT VulkanMessenger;
    static VkPhysicalDevice VulkanDevice;

  public:
  private:
    Render() = default;
    Render(const Render &) = delete;
    ~Render() = default;

    static GLFWwindow *InitializeGLFW();
    static VkApplicationInfo PopulateVkAppInfo();

    static list<const char *> GetRequiredExtensions();
    static list<VkExtensionProperties> GetAvailableExtensions();
    static bool ValidateExtensions(const list<const char *> &required, const list<VkExtensionProperties> &available);

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
