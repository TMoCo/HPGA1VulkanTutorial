#ifndef HELLO_TRIANGLE_APPLICATION_H
#define HELLO_TRIANGLE_APPLICATION_H

// functions, structs and enums
// #include <vulkan/vulkan.h> 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// program wrapped in class where vulkan objects are stored as private members
class HelloTriangleApplication {
public:
    void run();

private:
    // functions to initiate the private members
    void initWindow();

    void initVulkan();

    void mainLoop();

    void cleanup();

private:
    // private members
    GLFWwindow* window;
};


#endif // !HELLO_TRIANGLE_APPLICATION_H

