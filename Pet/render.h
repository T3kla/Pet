#pragma once

#include "core.h"

#include "shaderc/shaderc.hpp"


struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription desc;
        desc.binding = 0;
        desc.stride = sizeof(Vertex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return desc;
    }

    static arr<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        arr<VkVertexInputAttributeDescription, 2> desc{};
        desc[0].binding = 0;
        desc[0].location = 0;
        desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        desc[0].offset = offsetof(Vertex, pos);
        desc[1].binding = 0;
        desc[1].location = 1;
        desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        desc[1].offset = offsetof(Vertex, color);
        return desc;
    }
};

struct QueueFamilyIndices
{
    opt<uint32_t> graphicsFamily;
    opt<uint32_t> presentFamily;

    bool IsComplete() const;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    list<VkSurfaceFormatKHR> formats;
    list<VkPresentModeKHR> presentModes;
};

struct FramesInFlight
{
    list<VkCommandBuffer> cmdBuffers; // draw commands containers
    list<VkSemaphore> imgSemaphores;  // image available
    list<VkSemaphore> rndSemaphores;  // render finished
    list<VkFence> fences;             // sync with cpu
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
  public:
    void Init();
    void Run();
    void Exit();

    glm::i32vec2 GetWindowSize();
    GLFWwindow *GetWindow();

  private:
    GLFWwindow *window = nullptr;
    const char *windowTitle = "PetProject";
    glm::i32vec2 windowSize = glm::i32vec2(800, 600);

    const list<const char *> vkValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const list<const char *> vkDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkInstance vkInstance{};
    VkInstanceCreateInfo vkInstanceInfo{};
    VkApplicationInfo vkAppInfo{};
    VkDebugUtilsMessengerEXT vkMessenger{};
    VkDebugUtilsMessengerCreateInfoEXT vkMessengerInfo{};
    VkPhysicalDevice vkPhyDevice{};
    QueueFamilyIndices vkPhyDeviceIndices;
    VkDevice vkLogDevice{};
    VkQueue vkGraphicsQueue{};
    VkSurfaceKHR vkSurface{};
    VkSwapchainKHR vkCurSwapChain{};
    VkSwapchainKHR vkOldSwapChain{};
    list<VkImage> vkSwapChainImages{};
    VkFormat vkSwapChainImageFormat{};
    VkExtent2D vkSwapChainExtent{};
    list<VkImageView> vkImageViews;
    list<VkFramebuffer> vkFramesBuffer;
    bool frameBufferResized = false;
    VkBuffer vkVertexBuffer{};
    VkDeviceMemory vkVertexMemory{};
    VkRenderPass vkRenderPass{};
    VkPipelineLayout vkPipeLayout{};
    VkPipeline vkPipe{};
    VkCommandPool vkCmdPool{};
    FramesInFlight frames = FramesInFlight(2);

    const list<Vertex> vertices = {          //
        {{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},  //
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   //
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}}; //

    GLFWwindow *InitializeGLFW();

    void PopulateVkAppInfo(VkApplicationInfo &info);
    void PopulateVkMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &info);
    void AttachDebugMessenger();

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

    list<char> GenerateShader(const str& path, shaderc_shader_kind kind);
    VkShaderModule GetShaderModule(const list<char> &shader);

    void GetFramesBuffer(list<VkFramebuffer> &buffer);
    void GetVertexBuffer(VkBuffer &buffer, VkDeviceMemory &memory);

    void GetCommandPool(VkCommandPool &pool);
    void PopulateFrames(FramesInFlight &framesInFlight);

    void RecordCommandBuffer(const VkCommandBuffer &buffer, u32 idx);

    //

    static void OnWindowResize(GLFWwindow* window, int width, int height);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData);

    u32 FindMemoryType(const u32 &typeFilter, const VkMemoryPropertyFlags &flags);

    void VkCleanup();
    void VkCleanupSwapChain();
};
