//
// The class containing the application definition
//

#ifndef DUCK_APPLICATION_H
#define DUCK_APPLICATION_H

// include the vulkan window data
#include "../headers/VulkanSetup.h"
#include "../headers/Vertex.h"

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

    // the graphics pipeline, vulkan does not allow us to modify the pipeline so we need to create one 
    // from scratch if we want to add/remove stages, etc...
    void createGraphicsPipeline();

    void createRenderPass();

    void createImGuiRenderPass();

    void createDepthResources();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format);

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

    VkCommandBuffer beginSingleTimeCommands(VkCommandPool& commandBuffer);

    void endSingleTimeCommands(VkCommandBuffer* commandBuffer, VkCommandPool* commandPool);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createFrameBuffers();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    //--------------------------------------------------------------------//
    
    void loadModel(); 

    void createVertexBuffer();

    void createIndexBuffer();

    //--------------------------------------------------------------------//

    void createTextureImage();

    void createTextureImageView();

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);


    //--------------------------------------------------------------------//

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void createSwapChain();

    void recreateSwapChain();

    void cleanupSwapChain();

    void createImageViews();

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

    static std::vector<char> readFile(const std::string& filename);

    VkShaderModule createShaderModule(const std::vector<char>& code);

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

    // the swap chain and its data
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    // need views to use images in the render pipeline
    std::vector<VkImageView> swapChainImageViews;
    // a vector containing all the framebuffers
    std::vector<VkFramebuffer> swapChainFramebuffers;

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

    // depth image
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    // a texture
    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler; // lets us sample from an image, here the texture
    VkDeviceMemory textureImageMemory;

    // the geometry render pass
    VkRenderPass renderPass;
    // the imgui render pass
    VkRenderPass imGuiRenderPass;


    // layout used to specify fragment uniforms, still required even if not used
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorPool imGuiDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets; // descriptor set handles
    VkPipelineLayout pipelineLayout;
    // the pipeline
    VkPipeline graphicsPipeline;

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

