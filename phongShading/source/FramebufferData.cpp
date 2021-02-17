//
// FramebufferData class definition
//

#include <FramebufferData.h>

// reporting and propagating exceptions
#include <iostream> 
#include <stdexcept>
#include <array> // array container

//////////////////////
//
// Create and destroy the framebuffer data
//
//////////////////////

void FramebufferData::initFramebufferData(const VulkanSetup* vkSetup, const SwapChainData* swapChainData, const VkCommandPool& commandPool) {
    // first create the depth resource
    depthResource.createDepthResource(vkSetup, swapChainData->extent, commandPool);
    // then create the framebuffers
    createFrameBuffers(vkSetup, swapChainData);
}

void FramebufferData::cleanupFrambufferData() {

}

//////////////////////
//
// The framebuffers
//
//////////////////////

void FramebufferData::createFrameBuffers(const VulkanSetup* vkSetup, const SwapChainData* swapChainData) {
    // resize the container to hold all the framebuffers, or image views, in the swap chain
    framebuffers.resize(swapChainData->imageViews.size());

    // now loop over the image views and create the framebuffers, also bind the image to the attachment
    for (size_t i = 0; i < swapChainData->imageViews.size(); i++) {
        // get the attachment 
        std::array<VkImageView, 2> attachments = {
            swapChainData->imageViews[i],
            depthResource.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = swapChainData->renderPass; // which renderpass the framebuffer needs, only one at the moment
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // the number of attachments, or VkImageView objects, to bind to the buffer
        framebufferInfo.pAttachments = attachments.data(); // pointer to the attachment(s)
        framebufferInfo.width = swapChainData->extent.width; // specify dimensions of framebuffer depending on swapchain dimensions
        framebufferInfo.height = swapChainData->extent.height;
        framebufferInfo.layers = 1; // single images so only one layer

        // attempt to create the framebuffer and place in the framebuffer container
        if (vkCreateFramebuffer(vkSetup->device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

