//
// a convenience header file for constants etc. used in the application
//

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>

//
// Constants
//

// constants for window dimensions
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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

#endif // !UTILS_H
