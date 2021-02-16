// 
// A helper class that creates an contains the vulkan data used in an application
//

#ifndef VULKAN_SETUP_H
#define VULKAN_SETUP_H

// vulkan definitions
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

// constants and structs
#include "../headers/Utils.h"


// glfw window library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vector container
#include <vector>
// the optional wrapper
#include <optional>

class VulkanSetup {

    //////////////////////
    //
    // DESTRUCTOR
    //
    //////////////////////

public:

    // destructor, handles the explicit destruction of the vulkan data
    ~VulkanSetup();

    //////////////////////
    //
    // MEMBER FUNCTIONS
    //
    //////////////////////

public:

    //
    // Initiate the setup
    //

    void setupVulkan(GLFWwindow* window);

private:

    //
    // Vulkan instance setup
    //

	void createInstance();
	
	// enumerates the extensions required when creating a vulkan instance
	std::vector<const char*> getRequiredExtensions();

    //
    // Validation layers setup
    //

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

    //
    // Vulkan surface setup
    //

    void createSurface();
    
    //
    // Vulkan physical and logical device setup
    //

    void pickPhysicalDevice();

    bool isDeviceSuitable(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    void createLogicalDevice();
    
    //
    // Swap chain setup
    //

    void createSwapChain();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    //////////////////////
    //
    // MEMBER VARIABLES
    //
    //////////////////////

	// make them public for now
public:
    //
    // The window
    //

    // a reference to the GLFW window
    GLFWwindow* window;

    //
    // Instance
    //

	// vulkan instance struct
	VkInstance instance;
    // a struct handle that manages debug callbacks
    VkDebugUtilsMessengerEXT debugMessenger;

    //
    // Surface
    //

    // the surface to render to
    VkSurfaceKHR surface;

    //
    // Device
    //

    // the physical device chosen for the application
    VkPhysicalDevice physicalDevice;
    // logical device that interfaces with the physical device
    VkDevice device;
    // queue handle for interacting with the graphics queue, implicitly cleaned up by destroying devices
    VkQueue graphicsQueue;
    // queue handle for interacting with the presentation queue
    VkQueue presentQueue;

};





#endif // !VULKAN_SETUP_H

