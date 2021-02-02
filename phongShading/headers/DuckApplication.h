#ifndef DUCK_APPLICATION_H
#define DUCK_APPLICATION_H

// glfw window library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vectors, matrices ...
#include <glm/glm.hpp>

// very handy containers of objects
#include <vector>
#include <array>
// string for file name
#include <string>
// value wrapper
#include <optional>

//
// Constants
//

// constants for window dimensions
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// validation layers for debugging
const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

// required device extensions
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME 
};

// in flight frames number
const size_t MAX_FRAMES_IN_FLIGHT = 2;

//
// Debug preprocessor
//

//#define NDEBUG // uncomment to remove validation layers for debug
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//#define VERBOSE
#ifdef VERBOSE
const bool enableVerboseValidation = true;
#else
const bool enableVerboseValidation = false;
#endif


//
// Helper structs
//

// a simple vertex struct
struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    // binding description of a vertex
    static VkVertexInputBindingDescription getBindingDescription() {
        // a struct containing info on how to store vertex data
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; // index of binding in array of bindings
        bindingDescription.stride = sizeof(Vertex); // bytes in one entry
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // how to move the next data
        // other option for instancing -> VK_VERTEX_INPUT_RATE_INSTANCE
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        // we have two attributes: position and colour. The struct describes how to extract an attribute
        // from a chunk of vertex data from a binding description
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        // position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        // colour
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }
};

// a struct for the queue family index
struct QueueFamilyIndices {
    // queue family supporting drawing commands
    std::optional<uint32_t> graphicsFamily;
    // presentation of image to vulkan surface handled by the device
    std::optional<uint32_t> presentFamily;

    // returns true if the device supports the drawing commands AND the image can be presented to the surface
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// a struct containing the details for support of a swap chain
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

//
// The application
//

// program wrapped in class where vulkan objects are stored as private members
class DuckApplication {
public:
    void run();

private:
    // init vulkan instance
    void initVulkan();

    //--------------------------------------------------------------------//

    // create a vulkan instance
    void createInstance();

    std::vector<const char*> getRequiredExtensions();

    //--------------------------------------------------------------------//

    // the graphics pipeline, vulkan does not allow us to modify the pipeline so we need to create one 
    // from scratch if we want to add/remove stages, etc...
    void createGraphicsPipeline();

    void createRenderPass();

    //--------------------------------------------------------------------//

    void createFrameBuffers();

    void createCommandPool();

    void createCommandBuffers();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createVertexBuffer();

    void createIndexBuffer();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    //--------------------------------------------------------------------//

    // create a device interface
    void createLogicalDevice();

    // picks a physical device (graphics card)
    void pickPhysicalDevice();

    // checks if the device is suitable for our operation
    bool isDeviceSuitable(VkPhysicalDevice device);

    // checks if a device supports a certain extension
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    //--------------------------------------------------------------------//

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void createSwapChain();

    void recreateSwapChain();

    void cleanupSwapChain();

    // create a window surface
    void createSurface();

    void createImageViews();

    //--------------------------------------------------------------------//

    void createSyncObjects();

    //--------------------------------------------------------------------//

    // the main loop
    void mainLoop();

    void drawFrame();

    //--------------------------------------------------------------------//

    static std::vector<char> readFile(const std::string& filename);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    //--------------------------------------------------------------------//

    // destroys everything properly
    void cleanup();

    //--------------------------------------------------------------------//

    // functions to initiate the private members
    // init glfw window
    void initWindow();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    //--------------------------------------------------------------------//

    // sets up the debug messenger when in debug mode
    void setupDebugMessenger();

    // finds where the debug messenger creator is located and calls it (if available)
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    // same as creator, but for destroying
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    bool checkValidationLayerSupport();

// private members
private:
    // the window
    GLFWwindow* window;

    // vulkan instance struct
    VkInstance instance;
    // a struct handle that manages debug callbacks
    VkDebugUtilsMessengerEXT debugMessenger;
    // platform agnostic handle to a surface
    VkSurfaceKHR surface;

    // the graphics card selected
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    // logical device that interfaces with the physical device
    VkDevice device;

    // queue handle for interacting with the graphics queue, implicitly cleaned up by destroying devices
    VkQueue graphicsQueue;
    // queue handle for interacting with the presentation queue
    VkQueue presentQueue;

    // the swap chain and its data
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    // need views to use images in the render pipeline
    std::vector<VkImageView> swapChainImageViews;
    // a vector containing all the framebuffers
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // vertex buffer
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    // index buffer
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    // layout used to specify fragment uniforms, still required even if not used
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    // the pipeline
    VkPipeline graphicsPipeline;

    // command buffers
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // each frame has it's own semaphores
    // semaphores are for GPU-GPU synchronisation
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    // fences are for CPU-GPU synchronisation
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    // keep track of the current frame
    size_t currentFrame = 0;
    bool framebufferResized = false;
};

#endif

