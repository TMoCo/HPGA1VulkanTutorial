//
// The class containing the application definition
//

#ifndef DUCK_APPLICATION_H
#define DUCK_APPLICATION_H

#include <Vertex.h> // the vertex struct
#include <VulkanSetup.h> // include the vulkan setup class
#include <SwapChainData.h> // the swap chain class
#include <FramebufferData.h> // the framebuffer data class

// glfw window library
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vectors, matrices ...
#include <glm/glm.hpp>

// reporting and propagating exceptions
#include <iostream> 
#include <stdexcept>

// very handy containers of objects
#include <vector>
#include <array>
// string for file name
#include <string>
// value wrapper
#include <optional>

//
// Helper structs
//

// a struct containing uniforms
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

//
// The application
//

// program wrapped in class where vulkan objects are stored as private members
class DuckApplication {

public:

    void run();

private:
    //--------------------------------------------------------------------//

    void initVulkan();

    //--------------------------------------------------------------------//


    void createImGuiRenderPass();

    //--------------------------------------------------------------------//

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createImGuiDescriptorPool();

    void createDescriptorSets();

    void createUniformBuffers();

    void createTextureSampler();

    //--------------------------------------------------------------------//

    void createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);

    void createCommandBuffers(std::vector<VkCommandBuffer>* commandBuffers, VkCommandPool& commandPool);

    //--------------------------------------------------------------------//
    
    void loadModel(); 

    void createVertexBuffer();

    void createIndexBuffer();

    //--------------------------------------------------------------------//

    void createTextureImage();

    void createTextureImageView();

    //--------------------------------------------------------------------//

    void recreateVulkanData();

    //--------------------------------------------------------------------//

    void createSyncObjects();

    //--------------------------------------------------------------------//

    // the main loop
    void mainLoop();

    void drawFrame();

    void drawImGuiFrame();

    void presentImGuiFrame();

    void updateUniformBuffer(uint32_t currentImage);

    //--------------------------------------------------------------------//

    // destroys everything properly
    void cleanup();

    //--------------------------------------------------------------------//

    // functions to initiate the private members
    // init glfw window
    void initWindow();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

// private members
private:
    // the window
    GLFWwindow* window;

    // the vulkan data (instance, surface, device)
    VulkanSetup vkSetup;

    // the swap chain and related data
    SwapChainData swapChainData;

    // the frame buffer and related data
    FramebufferData framebufferData;

    // object data
    // vertex buffer
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    // index buffer
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    // uniform buffers
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;


    // a texture
    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler; // lets us sample from an image, here the texture
    VkDeviceMemory textureImageMemory;

    // the imgui render pass
    VkRenderPass imGuiRenderPass;

    // layout used to specify fragment uniforms, still required even if not used
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorPool imGuiDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets; // descriptor set handles
   
    // command buffers
    VkCommandPool renderCommandPool;
    std::vector<VkCommandBuffer> renderCommandBuffers;

    // imgui command buffers
    VkCommandPool imGuiCommandPool;
    std::vector<VkCommandBuffer> imGuiCommandBuffers;

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

