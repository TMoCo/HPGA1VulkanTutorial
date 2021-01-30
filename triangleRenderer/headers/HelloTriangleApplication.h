#ifndef HELLO_TRIANGLE_APPLICATION_H
#define HELLO_TRIANGLE_APPLICATION_H

// functions, structs and enums
// #include <vulkan/vulkan.h> 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vector
#include <vector>

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
};


#endif // !HELLO_TRIANGLE_APPLICATION_H

