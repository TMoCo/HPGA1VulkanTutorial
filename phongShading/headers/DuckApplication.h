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
    // matrices for scene rendering
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    int uvToRgb;
    // a struct containing material properties for the phong shader
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec4 specular;
    // light position
    glm::vec3 lightPos = LIGHT_POS;
    // flag for setting the colour to texture coordinates
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

    void initImGui();

    void uploadFonts();

    void renderUI();

    //--------------------------------------------------------------------//

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createDescriptorSets();

    void createUniformBuffers();

    void createTextureSampler();

    //--------------------------------------------------------------------//

    void createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);

    void createCommandBuffers(std::vector<VkCommandBuffer>* commandBuffers, VkCommandPool& commandPool);

    void recordGemoetryCommandBuffer();

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

    // Variables changed by the UI
    float translateX = 0.0f;
    float translateY = 0.0f;
    float translateZ = 0.0f;

    float rotateX = 0.0f;
    float rotateY = 0.0f;
    float rotateZ = 0.0f;

    float zoom = 1.0f;

    bool enableDepthTest = true;
    bool uvToRgb = false;
    bool enableAlbedo = true;
    bool enableDiffuse = true;
    bool enableSpecular = true;


    // a texture
    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler; // lets us sample from an image, here the texture
    VkDeviceMemory textureImageMemory;


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
    // the index of the image retrieved from the swap chain
    uint32_t imageIndex;
    // resize window flag
    bool framebufferResized = false;
};

#endif

