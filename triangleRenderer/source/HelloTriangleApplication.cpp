// include class definition
#include "..\headers\HelloTriangleApplication.h"


//
// Run application
//

void HelloTriangleApplication::run() {
    // initialise a glfw window
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

//
// Initialisation
//

void HelloTriangleApplication::initWindow() {
    // initialise glfw library
    glfwInit();

    // set parameters
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // initially for opengl, so tell it not to create opengl context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable resizing for now

    // create the window, 4th param refs a monitor, 5th param is opengl related
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan() {

}

//
// Main loop
//

void HelloTriangleApplication::mainLoop() {
    // loop keeps window open
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

//
// Cleanup
//

void HelloTriangleApplication::cleanup() {
    // destory the window
    glfwDestroyWindow(window);

    // terminate glfw
    glfwTerminate();
}

