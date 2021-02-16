//
// A convenience header file for constants etc. used in the application
//

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h> // uint32_t

#include <vector> // vector container
#include <string> // string class
#include <optional> // optional wrapper

#include <vulkan/vulkan_core.h> // vulkan core structs &c
//////////////////////
//
// Constants
//
//////////////////////

// constants for window dimensions
const uint32_t WIDTH =  800;
const uint32_t HEIGHT = 600;

// strings for the vulkan instance
const std::string APP_NAME =    "Basic application";
const std::string ENGINE_NAME = "No Engine";

// paths to the model and texture
const std::string MODEL_PATH =   "C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\assets\\mallard.obj";
const std::string TEXTURE_PATH = "C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\assets\\mallard.jpg";

// paths to the fragment and vertex shaders used
const std::string SHADER_VERT_PATH = "C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\source\\shaders\\vert.spv";
const std::string SHADER_FRAG_PATH = "C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\source\\shaders\\frag.spv";

// validation layers for debugging
const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

// required device extensions
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// in flight frames number
const size_t MAX_FRAMES_IN_FLIGHT = 2;

//////////////////////
//
// Debug preprocessor
//
//////////////////////

//#define NDEBUG // uncomment to remove validation layers for debug
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

//#define VERBOSE
#ifdef VERBOSE
const bool enableVerboseValidation = true;
#else
const bool enableVerboseValidation = false;
#endif

//////////////////////
//
// Utility structs
//
//////////////////////

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

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        // similar to physical device and extensions and layers....
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // create a vector to store queue families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        // store the queue families in the vector
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // iterate over queue family properties vector
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            // if the queue supports the desired queue operation, then the bitwise & operator returns true
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                // gaphics family was assigned a value! optional wrapper has_value now returns true.
                indices.graphicsFamily = i;
            }

            // start with false
            VkBool32 presentSupport = false;
            // function checks that device, queuefamily can present on the surface, sets presentSupport to true if so
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            // check the value in presentSupport
            if (presentSupport) {
                indices.presentFamily = i;
            }

            // return the first valid queue family
            if (indices.isComplete()) {
                break;
            }
            // increment i to get index of next queue family
            i++;
        }
        return indices;
    }
};

// a struct containing the details for support of a swap chain
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

// a POD struct containing the data for creating an image
struct CreateImageData {
    uint32_t              width       = 0;
    uint32_t              height      = 0;
    VkFormat              format      = VK_FORMAT_UNDEFINED;
    VkImageTiling         tiling      = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags     usage       = VK_NULL_HANDLE;
    VkMemoryPropertyFlags properties  = VK_NULL_HANDLE;
    VkImage*              image       = nullptr;
    VkDeviceMemory*       imageMemory = nullptr;
};

//////////////////////
//
// Utility functions namespace
//
//////////////////////

namespace utils {

    //
    // Memory type 
    //

    uint32_t findMemoryType(VkPhysicalDevice* physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        // GPUs allocate dufferent types of memory, varying in terms of allowed operations and performance. Combine buffer and application
        // requirements to find best type of memory
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memProperties);
        // two arrays in the struct, memoryTypes and memoryHeaps. Heaps are distinct ressources like VRAM and swap space in RAM
        // types exist within these heaps

        // loop over the device memory types to find the right one
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            // we want a memory type that is suitable for the vertex buffer, but also able to write our vertex data to memory
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        // otherwise we can't find the right type!
        throw std::runtime_error("failed to find suitable memory type!");
    }

    //
    // Image and image view creation
    //

    void createImage(VkPhysicalDevice* physicalDevice, VkDevice* device, const CreateImageData& info) {
        // create the struct for creating an image
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D; // coordinate system of the texels
        imageInfo.extent.width = info.width; // the dimensions of the image
        imageInfo.extent.height = info.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1; // no mip mapping yet
        imageInfo.arrayLayers = 1; // not in array yet
        imageInfo.format = info.format; // same format as the pixels is best
        imageInfo.tiling = info.tiling; // tiling of the pixels, let vulkan lay them out
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = info.usage; // same semantics as during buffer creation, here as destination for the buffer copy
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // for multisampling
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // used by a single queue family
        imageInfo.flags = 0; // Optional, for sparse data

        // create the image. The hardware could fail for the format we have specified. We should have a list of acceptable formats and choose the best one depending
        // on the selection of formats supported by the device
        if (vkCreateImage(*device, &imageInfo, nullptr, info.image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        // allocate memory for an image, similar to a buffer allocation
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(*device, *info.image, &memRequirements);

        // info on the allocation
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // attempt to create an image
        if (vkAllocateMemory(*device, &allocInfo, nullptr, info.imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        // associate the memory to the image
        vkBindImageMemory(*device, *info.image, *info.imageMemory, 0);
    }

    VkImageView createImageView(VkDevice* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        // helper function for creating image views with a specific format
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image; // different image, should refer to the texture
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // image as 1/2/3D textures and cube maps
        viewInfo.format = format;// how the image data should be interpreted
        // lets us swizzle colour channels around (here there is no swizzle)
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // describe what the image purpose is
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        // and what part of the image we want
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        // attemp to create the image view
        VkImageView imageView;
        if (vkCreateImageView(*device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        // and return the image view
        return imageView;
    }

    //
    // Transitioning between image layouyts
    //

    void transitionImageLayout(VkDevice* device, VkQueue* graphicsQueue, VkImage image, VkCommandPool renderCommandPool, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        // images may have different layout that affect how pixels are organised in memory, so we need to specify which layout we are transitioning
        // to and from to lake sure we have the optimal layout for our task
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, renderCommandPool);
        // a common way to perform layout transitions is using an image memory barrier, generally for suncing acces to a ressourcem
        // eg make sure write completes before subsequent read, but can transition image layout and transfer queue family ownership

        // the barrier struct, useful for synchronising access resources in the pipeline (no read / write conflicts)
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        // specify layout transition
        barrier.oldLayout = oldLayout; // can use VK_IMAGE_LAYOUT_UNDEFINED if old layout is of no importance
        barrier.newLayout = newLayout;
        // indices of the two queue families
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // explicit if we don't want to as not default value
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        // info about the image
        barrier.image = image; // the image

        // Determine which aspects of the image are included in the view!
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            // in the case the image is a depth image, then we want the view to contain only the depth aspect
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponent(format)) {
                // also need to include the stencil aspect if avaialble
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            // otherwise we are interested in the colour aspect of the image
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        // not an array and no mipmap 
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        // we need to set the access mask and pipeline stages based on the layout in the transition
        // we need to handle two transitions: 
        // undefined -> transfer dest : transfer writes that don't need to wait on anything
        // transfer dest -> shader reading : share read should wait on transfer write

        // declare the stages 
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        // determine which of the two transitions we are executing
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            // first transition, where the layout is not important, to a layout optimal as destination in a transfer operation
            barrier.srcAccessMask = 0; // don't have to wait on anything for pre barrier operation
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // transfer writes must occur in the pipeline transfer stage, a pseudo-stage where transfers happen
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            // second transition, where the src layout is optimal as the destination of a transfer operation, to a layout optimal for sampling by a shader
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // wait on the transfer to finish writing
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            // third transition, where the src layout is not important and the dst layout is optimal for depth/stencil operations
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        // extend this function for other transitions
        else {
            // unrecognised transition
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            // arrays of pipeline barriers of three available types
            0, nullptr, // memory barriers
            0, nullptr, // buffer memory barriers
            1, &barrier // image memory barriers
        );

        endSingleTimeCommands(device, graphicsQueue, &commandBuffer, &renderCommandPool);
    }

    bool hasStencilComponent(VkFormat format) {
        // simple helper function that returns true if the specified format has a stencil buffer component
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    //
    // Begining and ending single use command buffers
    //

    VkCommandBuffer beginSingleTimeCommands(VkDevice* device, VkCommandPool& commandPool) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        // allocate the command buffer
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(*device, &allocInfo, &commandBuffer);

        // set the struct for the command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // tell the driver about how the command buffer will be used for optimisation

        // start recording the command buffer
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkDevice* device, VkQueue* graphicsQueue, VkCommandBuffer* commandBuffer, VkCommandPool* commandPool) {
        // end recording
        vkEndCommandBuffer(*commandBuffer);

        // execute the command buffer by completing the submitinfo struct
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffer;

        // submit the queue for execution
        vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        // here we could use a fence to schedule multiple transfers simultaneously and wait for them to complete instead
        // of executing all at the same time, alternatively use wait for the queue to execute
        vkQueueWaitIdle(*graphicsQueue);

        // free the command buffer once the queue is no longer in use
        vkFreeCommandBuffers(*device, *commandPool, 1, commandBuffer);
    }


}

#endif // !UTILS_H
