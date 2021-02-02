#ifndef HELLO_TRIANGLE_APPLICATION_H
#define HELLO_TRIANGLE_APPLICATION_H

// functions, structs and enums
// #include <vulkan/vulkan.h> 

//#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

#include <vector>
#include <string>

// wrapper that contains no value until one is assigned
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
    VK_KHR_SWAPCHAIN_EXTENSION_NAME // swap chain is the infrastructure that owns the buffers we will render, a queue of images waiting to go on the screen
};

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

// a struct containing the details for support of a swap chain:
// surface capabilities (min/max number of images in swap chain, min/max width and height of images)
// surface formats (pixel format, colour space)
// available presentation modes
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// program wrapped in class where vulkan objects are stored as private members
class HelloTriangleApplication {
public:
    void run();

private:
    // functions to initiate the private members
    // init glfw window
    void initWindow();

    // init vulkan instance
    void initVulkan();

    //--------------------------------------------------------------------//

    // create a vulkan instance
    void createInstance();

    std::vector<const char*> getRequiredExtensions();

    //--------------------------------------------------------------------//

    // the graphics pipeline, vulkan does not allow us to modify the pipeline so we need to create one 
    // from scratch if we want to add/remove stages, etc... For the purpose of rendering a single triangle
    // however, we only need one pipeline
    void createGraphicsPipeline();

    void createRenderPass();

    //--------------------------------------------------------------------//

    void createFrameBuffers();

    void createCommandPool();

    void createCommandBuffers();

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

    // create a window surface
    void createSurface();

    void createImageViews();

    //--------------------------------------------------------------------//

    void createSemaphores();

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


    // layout used to specify fragment uniforms, still required even if not used
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    // the pipeline
    VkPipeline graphicsPipeline;

    // command buffers
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // syncing
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
};

#endif // !HELLO_TRIANGLE_APPLICATION_H

