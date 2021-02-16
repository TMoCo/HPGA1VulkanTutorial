//
// Definition of the SwapChain class
//

// include the class declaration
#include "../headers/SwapChain.h"

// exceptions
#include <iostream>
#include <stdexcept>

//////////////////////
//
// INITIALISATION AND DESTRUCTION
//
//////////////////////

void SwapChain::initSwapChain(VulkanSetup* vkSetup) {

}

void SwapChain::cleanupSwapChain() {

}

//////////////////////
//
// The swap chain
//
//////////////////////

void SwapChain::createSwapChain(VulkanSetup* vkSetup) {
    // create the swap chain by checking for swap chain support
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vkSetup->physicalDevice, vkSetup->surface);

    // set the swap chain properties using the above three methods for the format, presentation mode and capabilities
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, vkSetup->window);

    // the number of images we want to put in the swap chain, at least one more image than minimum so we don't have to wait for 
    // driver to complete internal operations before acquiring another image
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // also make sure not to exceed the maximum image count
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // start creating a structure for the swap chain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkSetup->surface; // specify the surface the swap chain should be tied to
    // set details
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // amount of layers each image consists of
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // bit field, specifies types of operations we'll use images in swap chain for

    // how to handle the swap chain images across multiple queue families (in case graphics queue is different to presentation queue)
    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(vkSetup->physicalDevice, vkSetup->surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    // if the queues differ
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // image owned by one queue family, ownership must be transferred explicilty
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // images can be used accross queue families with no explicit transfer
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    // a certain transform to apply to the image
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // specifiy if alpha channel should be blending with other windows
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode; // determined earlier
    createInfo.clipped = VK_TRUE; // ignore colour of obscured pixels
    // in case the swap chain is no longer optimal or invalid (if window was resized), need to recreate swap chain from scratch
    // and specify reference to old swap chain (ignore for now)
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // finally create the swap chain
    if (vkCreateSwapchainKHR(vkSetup->device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    // get size of swap chain images
    vkGetSwapchainImagesKHR(vkSetup->device, swapChain, &imageCount, nullptr);
    // resize accordingly
    images.resize(imageCount);
    // pull the images
    vkGetSwapchainImagesKHR(vkSetup->device, swapChain, &imageCount, images.data());

    // save format and extent
    imageFormat = surfaceFormat.format;
    extent = extent;
}

SwapChainSupportDetails SwapChain::querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
    SwapChainSupportDetails details;
    // query the surface capabilities and store in a VkSurfaceCapabilities struct
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities); // takes into account device and surface when determining capabilities

    // same as we have seen many times before
    uint32_t formatCount;
    // query the available formats, pass null ptr to just set the count
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    // if there are formats
    if (formatCount != 0) {
        // then resize the vector accordingly
        details.formats.resize(formatCount);
        // and set details struct fromats vector with the data pointer
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // exact same thing as format for presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

// next three functions are for setting the parameters of the swap chain

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // VkSurfaceFormatKHR entry contains a format and colorSpace member
    // format is colour channels and type eg VK_FORMAT_B8G8R8A8_SRGB (8 bit uint BGRA channels, 32 bits per pixel)
    // colorSpace is the coulour space that indicates if SRGB is supported with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR (used to be VK_COLORSPACE_SRGB_NONLINEAR_KHR)

    // loop through available formats
    for (const auto& availableFormat : availableFormats) {
        // if the correct combination of desired format and colour space exists then return the format
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    // if above fails, we could rank available formats based on how "good" they are for our task, settle for first element for now 
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // presentation mode, can be one of four possible values:
    // VK_PRESENT_MODE_IMMEDIATE_KHR -> image submitted by app is sent straight to screen, may result in tearing
    // VK_PRESENT_MODE_FIFO_KHR -> swap chain is a queue where display takes an image from front when display is refreshed. Program inserts rendered images at back. 
    // If queue full, program has to wait. Most similar vsync. Moment display is refreshed is "vertical blank".
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR -> Mode only differs from previous if application is late and queue empty at last vertical blank. Instead of waiting for next vertical blank, 
    // image is transferred right away when it finally arrives, may result tearing.
    // VK_PRESENT_MODE_MAILBOX_KHR -> another variation of second mode. Instead of blocking the app when queue is full, images that are already queued are replaced with newer ones.
    // Can be used to implement triple buffering, which allows to avoid tearing with less latency issues than standard vsync using double buffering.

    for (const auto& availablePresentMode : availablePresentModes) {
        // use triple buffering if available
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    // swap extent is the resolution of the swap chain images, almost alwawys = to window res we're drawing pixels in
    // match resolution by setting width and height in currentExtent member of VkSurfaceCapabilitiesKHR struct.
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        // get the dimensions of the window
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // prepare the struct with the height and width of the window
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // clamp the values between allowed min and max extents by the surface
        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

//////////////////////
//
// The swap chain image views
//
//////////////////////

void SwapChain::createSwapChainImageViews(const VkDevice& device) {
    // resize the vector of views to accomodate the images
    imageViews.resize(images.size());
    
    // loop to iterate through images and create a view for each
    for (size_t i = 0; i < images.size(); i++) {
        // helper function for creating image views with a specific format
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = images[i]; // different image, should refer to the texture
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // image as 1/2/3D textures and cube maps
        viewInfo.format = imageFormat;// how the image data should be interpreted
        // lets us swizzle colour channels around (here there is no swizzle)
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // describe what the image purpose is
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        // and what part of the image we want
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        // attemp to create the image view
        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        // and return the image view
        imageViews[i] = imageView;
    }
}




