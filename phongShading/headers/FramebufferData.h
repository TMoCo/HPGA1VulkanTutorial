//
// A class that contains the framebuffer data
//

#ifndef FRAMEBUFFER_DATA_H
#define FRAMEBUFFER_DATA_H

#include <VulkanSetup.h> // for referencing the device
#include <DepthResource.h> // for referencing the depth resource
#include <SwapChainData.h> // for referencing the swap chain

#include <vulkan/vulkan_core.h>

class FramebufferData {
    //////////////////////
    //
    // MEMBER FUNCTIONS
    //
    //////////////////////
public:

    void initFramebufferData(const VulkanSetup* vkSetup, const SwapChainData* swapChainData, const VkCommandPool& commandPool);

    void cleanupFrambufferData();

private:

    //
    // Framebuffers creation
    //

    void createFrameBuffers(const VulkanSetup* vkSetup, const SwapChainData* swapChainData);


    //////////////////////
    //
    // MEMBER VARIABLES
    //
    //////////////////////

public:

    //
    // Framebuffers
    //

    // a vector containing all the framebuffers
    std::vector<VkFramebuffer> framebuffers;

    //
    // Depth component
    //

    // the depth resource for depth buffer and testing
    DepthResource depthResource;
};


#endif // !FRAMEBUFFER_DATA_H

