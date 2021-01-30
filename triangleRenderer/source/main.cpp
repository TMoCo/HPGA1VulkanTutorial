// functions, structs and enums
// #include <vulkan/vulkan.h> 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// reporting and propagating exceptions
#include <iostream> 
#include <stdexcept>

// macros
#include <cstdlib>

// include the application definition
#include "../headers/HelloTriangleApplication.h"


int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}