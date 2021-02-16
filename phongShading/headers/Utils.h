//
// a convenience header file for constants etc. used in the application
//

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h> // uint32_t

#include <vector> // vector container
#include <string> // string class
#include <optional> // optional wrapper

#include <vulkan/vulkan_core.h> // vulkan core structs &c

//
// Constants
//

// constants for window dimensions
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string APP_NAME = "Basic application";
const std::string ENGINE_NAME = "No Engine";

const std::string MODEL_PATH = "C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\assets\\mallard.obj";
const std::string TEXTURE_PATH = "C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\assets\\mallard.jpg";

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
// Utility structs
//

// a struct for the queue family index
struct QueueFamilyIndices {
    // queue family supporting drawing commands
    std::optional<uint32_t> graphicsFamily;
    // presentation of image to vulkan surface handled by the device
    std::optional<uint32_t> presentFamily;

    // returns true if the device supports the drawing commands AND the image can be presented to the surface
    inline bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        // similar to physical device and extensions and layers....
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // create a vector to store queue families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // store the queue families in the vector
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // iterate over queue family properties vector
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            // if the queue supports the desired queue operation, then the bitwise & operator returns true
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                // gaphics family was assigned a value! optional wrapper has_value now returns true.
                indices.graphicsFamily = i;
            }

            // start with false
            VkBool32 presentSupport = false;
            // function checks that device, queuefamily can present on the surface, sets presentSupport to true if so
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            // check the value in presentSupport
            if (presentSupport) {
                indices.presentFamily = i;
            }

            // return the first valid queue family
            if (indices.isComplete()) {
                break;
            }
            // increment i to get index of next queue family
            i++;
        }
        return indices;
    }
};

// a struct containing the details for support of a swap chain
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

//
// Utility functions
//



#endif // !UTILS_H
