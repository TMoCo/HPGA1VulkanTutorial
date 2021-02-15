#ifndef DUCK_APPLICATION_H
#define DUCK_APPLICATION_H

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

// a simple vertex struct
struct Vertex {
    glm::vec3 pos; // position 
    glm::vec3 normal; // normal
    glm::vec3 color; // colour
    glm::vec2 texCoord; // texture coordinate


    // binding description of a vertex
    static VkVertexInputBindingDescription getBindingDescription() {
        // a struct containing info on how to store vertex data
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0; // index of binding in array of bindings
        bindingDescription.stride = sizeof(Vertex); // bytes in one entry
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // how to move the next data
        // other option for instancing -> VK_VERTEX_INPUT_RATE_INSTANCE
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        // we have two attributes: position and colour. The struct describes how to extract an attribute
        // from a chunk of vertex data from a binding description
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        // position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        // normal
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);
        // colour
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, color);
        // tex coord
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
};

// a struct containing uniforms
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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
};

// a struct containing the details for support of a swap chain
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


//
// The application
//

// program wrapped in class where vulkan objects are stored as private members
class DuckApplication {
public:
    void run();

private:
    void initImGui();

    //--------------------------------------------------------------------//

    void initVulkan();

    //--------------------------------------------------------------------//

    // create a vulkan instance
    void createInstance();

    std::vector<const char*> getRequiredExtensions();

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

    // create a device interface
    void createLogicalDevice();

    // picks a physical device (graphics card)
    void pickPhysicalDevice();

    // checks if the device is suitable for our operation
    bool isDeviceSuitable(VkPhysicalDevice device);

    // checks if a device supports a certain extension
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    //--------------------------------------------------------------------//

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    void createSwapChain();

    void recreateSwapChain();

    void cleanupSwapChain();

    // create a window surface
    void createSurface();

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

    //--------------------------------------------------------------------//

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

// private members
private:
    // the window
    GLFWwindow* window;

    // vulkan instance struct
    VkInstance instance;
    // a struct handle that manages debug callbacks
    VkDebugUtilsMessengerEXT debugMessenger;
    // platform agnostic handle to a surface
    VkSurfaceKHR surface;

    // the graphics card selected
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    // logical device that interfaces with the physical device
    VkDevice device;

    // queue handle for interacting with the graphics queue, implicitly cleaned up by destroying devices
    VkQueue graphicsQueue;
    // queue handle for interacting with the presentation queue
    VkQueue presentQueue;

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

