//
// A class that contains the swap chain setup and data. It has a create function
// to facilitate the recreation when a window is resized. It contains all the variables
// that depend on the VkSwapChainKHR object
//

#ifndef VULKAN_SWAP_CHAIN_H
#define VULKAN_SWAP_CHAIN_H

#include "VulkanSetup.h" // for referencing the device

#include <vector> // vector container

#include <vulkan/vulkan_core.h>


class SwapChainData {
    //////////////////////
    //
    // MEMBER FUNCTIONS
    //
    //////////////////////

public:

    //
    // Initiate and cleanup the swap chain
    //

    void initSwapChainData(VulkanSetup* pVkSetup, VkDescriptorSetLayout* descriptorSetLayout);

    void cleanupSwapChainData();

private:

    //
    // swap chain creation and helper functions
    //

    void createSwapChain();

    SwapChainSupportDetails querySwapChainSupport();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    //
    // Image and image views creation
    //

    void createSwapChainImageViews();

    //
    // Render pass creation
    //

    void createRenderPass();

    //
    // Pipeline creation
    //

    void createGraphicsPipeline(VkDescriptorSetLayout* descriptorSetLayout);

    //
    // Framebuffers creation
    //

    void createFrameBuffers();

    //
    // Depth resources 
    //

    void createDepthResources();

    VkFormat findDepthFormat();


    //
    // Helper functions
    //

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    //////////////////////
    //
    // MEMBER VARIABLES
    //
    //////////////////////

public:
    // a reference to the vulkan setup (instance, devices, surface)
    VulkanSetup* vkSetup;

    // the swap chain
    VkSwapchainKHR swapChain;

    // the swap chain images
    std::vector<VkImage> images;

    // the image formats
    VkFormat imageFormat;

    // the extent of the swap chain
    VkExtent2D extent;

    // a vector containing the views needed to use images in the render pipeline
    std::vector<VkImageView> imageViews;

    // a vector containing all the framebuffers
    std::vector<VkFramebuffer> framebuffers;

    // the render pass
    VkRenderPass renderPass;

    // the layout of the graphics pipeline, for binding descriptor sets
    VkPipelineLayout graphicsPipelineLayout;

    // the graphics pipeline
    VkPipeline graphicsPipeline;

    // depth image
    VkImage depthImage;

    // the depth image's view for interfacing
    VkImageView depthImageView;

    // the memory containing the depth image data
    VkDeviceMemory depthImageMemory;
};

#endif // !VULKAN_SWAP_CHAIN_H
