#ifndef HELLO_TRIANGLE_APPLICATION_H
#define HELLO_TRIANGLE_APPLICATION_H

// functions, structs and enums
// #include <vulkan/vulkan.h> 

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// vector
#include <vector>

// wrapper that contains no value until one is assigned
#include <optional>

// constants for window dimensions
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// validation layers for debugging
const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

// uncomment to remove validation layers for debug
//#define NDEBUG
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

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

    // create a vulkan instance
    void createInstance();

    // create a window surface
    void createSurface();

    // picks a physical device (graphics card)
    void pickPhysicalDevice();

    // create a device interface
    void createLogicalDevice();

    // checks if the device is suitable for our operation
    bool isDeviceSuitable(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    // the main loop
    void mainLoop();

    // destroys everything properly
    void cleanup();

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

    std::vector<const char*> getRequiredExtensions();


private:
    // private members
    // the window
    GLFWwindow* window; 

    // vulkan instance struct
    VkInstance instance; 

    // a struct handle that manages debug callbacks
    VkDebugUtilsMessengerEXT debugMessenger;

    // the graphics card selected
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    // logical device that interfaces with the physical device
    VkDevice device;

    // queue handle for interacting with the graphics queue, implicitly cleaned up by destroying devices
    VkQueue graphicsQueue;

    // queue handle for interacting with the presentation queue
    VkQueue presentQueue;

    // platform agnostic handle to a surface
    VkSurfaceKHR surface;
};

#endif // !HELLO_TRIANGLE_APPLICATION_H

