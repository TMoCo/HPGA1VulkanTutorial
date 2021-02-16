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


class SwapChain {
    //////////////////////
    //
    // MEMBER FUNCTIONS
    //
    //////////////////////

public:

    //
    // Initiate and cleanup the swap chain
    //

    void initSwapChain(VulkanSetup* vkSetup);

    void cleanupSwapChain();

private:

    //
    // swap chain creation and helper functions
    //

    void createSwapChain(VulkanSetup* vkSetup);

    SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    //
    // image and image views creation
    //

    void createSwapChainImageViews(const VkDevice& device);

    //////////////////////
    //
    // MEMBER VARIABLES
    //
    //////////////////////

public:

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
};

#endif // !VULKAN_SWAP_CHAIN_H
