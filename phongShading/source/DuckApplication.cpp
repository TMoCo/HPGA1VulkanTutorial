// include class definition
#include "..\headers\DuckApplication.h"

// include constants
#include "..\headers\Utils.h"

// transformations
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // because OpenGL uses depth range -1.0 - 1.0 and Vulkan uses 0.0 - 1.0
#include <glm/gtc/matrix_transform.hpp>

// image loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// model loading
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// time 
#include <chrono>

// min, max
#include <algorithm>

// file (shader) loading
#include <fstream>

// UINT32_MAX
#include <cstdint>

// set for queues
#include <set>


// ImGui includes for a nice gui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>


//
// Run application
//

void DuckApplication::run() {
    // initialise a glfw window
    initWindow();

    // initialise vulkan
    initVulkan();

    // initialise ImGui
    //initImGui();

    // run the main loop
    mainLoop();

    // clean up before exiting
    cleanup();
}

//
// ImGui Initialisation
//


//
// Vulkan Initialisation
//

void DuckApplication::initVulkan() {
    // create the vulkan core 
    vkSetup.initSetup(window);

    // create the swap chain
    createSwapChain();
    // initialise the swap chain image views
    createImageViews();

    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();

    createCommandPool(&renderCommandPool, 0);
    createDepthResources();


    createFrameBuffers();

    
    // textures can go in a separate class
    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    // model can go in a separate class
    loadModel();
    createVertexBuffer();
    createIndexBuffer();

    // uniforms as well?
    createUniformBuffers();
    createDescriptorPool(); 
    createDescriptorSets();

    createCommandBuffers(&renderCommandBuffers, renderCommandPool);

    createSyncObjects();
}

//
// Graphics pipeline setup
//

void DuckApplication::createGraphicsPipeline() {
    // std::vector<char> 
    auto vertShaderCode = readFile("C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\source\\shaders\\vert.spv");
    auto fragShaderCode = readFile("C:\\Users\\Tommy\\Documents\\COMP4\\5822HighPerformanceGraphics\\A1\\HPGA1VulkanTutorial\\phongShading\\source\\shaders\\frag.spv");


    // compiling and linking of shaders doesnt happen until the pipeline is created, they are also destroyed along
    // with the pipeline so we don't need them to be member variables of the class
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // need to assign shaders to stages in the pipeline
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // for the vertex shader, we'll asign it to the vertex stage
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    // set the vertex shader
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main"; // the entry point, or the function to invoke in the shader
    // vertShaderStageInfo.pSpecializationInfo : can specify values for shader constants. Can use a singleshader module 
    // whose behaviour can be configured at pipeline creation by specifying different values for the constants used 
    // better than at render time because compiler can optimise if statements dependent on these values. Watch this space

    // similar gist as vertex shader for fragment shader
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // assign to the fragment stage
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main"; // also use main as the entry point

    // use this array for future reference
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // setup pipeline to accept vertex data
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    // format of the vertex data, describe the binding and the attributes
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // because vertex data is in the shader, we don't have to specify anything here. We would otherwise
    // need arrays of structs that describe the details for loading vertex data
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    // tells what kind of geometry to draw from the vertices
    // VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
    // VK_PRIMITIVE_TOPOLOGY_LINE_LIST : line from every 2 vertices without reuse
    // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : end vertex of every line used as start vertex for next line
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : triangle from every 3 vertices without reuse
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : second and third vertex of every triangle used as first two vertices of next triangle
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE; // can break up lines and triangles in strip using special id 0xFFF or 0xFFFFFFF

    // the region of the framebuffer of output that will be rendered to ... usually always (0,0) to (width,height)
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width; // don't use WIDTH and HEIGHT as the swap chain may have differing values
    viewport.height = (float)swapChainExtent.height;
    viewport.minDepth = 0.0f; // in range [0,1]
    viewport.maxDepth = 1.0f; // in range [0,1]

    // viewport describes transform from image to framebuffer, but scissor rectangles defiles wich regions pixels are actually stored
    // pixels outisde are ignored by the rasteriser. Here the scissor covers the entire framebuffer
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    // once viewport and scissor have been defined, they need to be combined. Can use multiple viewports and scissors on some GPUs
    // (requires enqbling a GPU feature, so changes the logical device creation)
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // rateriser takes geometry and turns it into fragments. Also performs depth test, face culling and scissor test
    // can be configured to output wireframe, full polygon 
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // if true,  fragments outside of near and far are clamped rather than discarded
    rasterizer.rasterizerDiscardEnable = VK_FALSE; // if true, disables passing geometry to framebuffer, so disables output
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // like opengl, can also be VK_POLYGON_MODE_LINE (wireframe) or VK_POLYGON_MODE_POINT
    // NB any other mode than fill requires enabling a GPU feature
    rasterizer.lineWidth = 1.0f; // larger than 1.0f requires the wideLines GPU feature
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // type of face culling to use (disable, cull front, cull back, cull both)
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // vertex order for faces to be front facing (CW and CCW)
    rasterizer.depthBiasEnable = VK_FALSE; // alter the depth by adding a constant or based onthe fragment's slope
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // multisampling is a way to perform antialiasing, less expensive than rendering a high res poly then donwscaling
    // also requires enabling a GPU feature (in logical device creation) so disable for now
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // could also set depth and stencil testing, but not needed now so it can stay a nullptr (already set
    // thanks to the {} when creating the struct)

    // after fragment shader has returned a result, needs to be combined with what is already in framebuffer
    // can either mix old and new values or combine with bitwise operation
    VkPipelineColorBlendAttachmentState colorBlendAttachment{}; // contains the config per attached framebuffer
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; // disable because we don't want colour blending

    // this struct references array of structures for all framebuffers and sets constants to use as blend factors
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1; // the previously declared attachment
    colorBlending.pAttachments = &colorBlendAttachment;

    // enable and configure depth testing
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // spec if the depth should be compared to the depth buffer
    depthStencil.depthTestEnable = VK_TRUE;
    // specifies if the new depth that pass the depth test should be written to the buffer
    depthStencil.depthWriteEnable = VK_TRUE;
    // pecifies the comparison that is performed to keep or discard fragments (here lower depth = closer so keep less)
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    // boundary on the depth test, only keep fragments within the specified depth range
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    // stencil buffer operations
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional


    // create the pipeline layout, where uniforms are specified, also push constants another way of passing dynamic values
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // refernece to the descriptor layout (uniforms)
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(vkSetup.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    // now use all the structs we have constructed to build the pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // reference the array of shader stage structs
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    // reference the structures describing the fixed function pipeline
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
    // the pipeline layout is a vulkan handle rather than a struct pointer
    pipelineInfo.layout = pipelineLayout;
    // and a reference to the render pass
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // index of desired sub pass where pipeline will be used

    if (vkCreateGraphicsPipelines(vkSetup.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // destroy the shader modules, as we don't need them once the shaders have been compiled
    vkDestroyShaderModule(vkSetup.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(vkSetup.device, vertShaderModule, nullptr);
}

void DuckApplication::createRenderPass() {
    // need to tell vulkan about framebuffer attachments used while rendering
    // how many colour and depth buffers, how many samples for each and how contents
    // handled though rendering operations... Wrapped in render pass object
    VkAttachmentDescription colorAttachment{}; // attachment 
    colorAttachment.format = swapChainImageFormat; // colour attachment format should match swap chain images format
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // > 1 for multisampling
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // what to do with data in attachment pre rendering
    // VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
    // VK_ATTACHMENT_LOAD_OP_CLEAR : Clear the values to a constant at the start
    // VK_ATTACHMENT_LOAD_OP_DONT_CARE : Existing contents are undefined; we don't care about them
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // post rendering
    // VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memoryand can be read later
    // VK_ATTACHMENT_STORE_OP_DONT_CARE : Contents of the framebuffer will be undefined after the rendering
    // above ops for colour, below for stencil data... not currenyly in use so just ignore
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // textures and framebuffers are VkImages with certain pixel formats, but layout in memory can change based on image use
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout before render pass begins (we don't care, not guaranteed to be preserved)
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout to transition to post render pass (image should be ready for drawing over by imgui)
    // common layouts
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : Images to be presented in the swap chain
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Images to be used as destination for a memory copy operation

    // specify a depth attachment to the render pass
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat(); // the same format as the depth image itself
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // like the colour buffer, we don't care about the previous depth contents so use layout undefined
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    // a single render pass consists of multiple subpasses, which are subsequent rendering operations depending on content of
    // framebuffers on previous passes (eg post processing). Grouping subpasses into a single render pass lets Vulkan optimise
    // every subpass references 1 or more attachments (see above) with structs:
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // forst attachment so refer to attachment index 0
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // which layout we want the attachment to have during a subpass

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1; // the second attachment
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // the subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // be explicit that this is a graphics subpass (Vulkan supports compute subpasses)
    // specify the reference to the colour attachment 
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // other types of attachments can also be referenced
    subpass.pDepthStencilAttachment = &depthAttachmentRef; // only a single depth/stencil attachment, no sense in depth tests on multiple buffers


    // subpass dependencies control the image layout transitions. They specify memory and execution of dependencies between subpasses
    // there are implicit subpasses right before and after the render pass
    // There are two built-in dependencies that take care of the transition at the start of the render pass and at the end, but the former 
    // does not occur at the right time as it assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet 
    // there are two ways to deal with the problem:
    // - change waitStages of the imageAvailableSemaphore (in the drawframe function) to VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT -> ensures that the
    // render pass does not start until image is available
    // - make the render pass wait for the VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage
    VkSubpassDependency dependency{};
    // indices of the dependency and dependent subpasses, dstSubpass > srcSubpass at all times to prevent cycles in dependency graph
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // special refers to implicit subpass before or after renderpass 
    dependency.dstSubpass = 0; // index 0 refers to our subpass, first and only one
    // specify the operations to wait on and stag when ops occur
    // need to wait for swap chain to finish reading, can be accomplished by waiting on the colour attachment output stage
    // need to make sure there are no conflicts between transitionning og the depth image and it being cleared as part of its load operation
    // The depth image is first accessed in the early fragment test pipeline stage and because we have a load operation that clears, we should specify the access mask for writes.
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    // ops that should wait are in colour attachment stage and involve writing of the colour attachment
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // now create the render pass, can be created by filling the structure with references to arrays for multiple subpasses, attachments and dependencies
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment }; // store the attachments
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data(); // the colour attachment for the renderpass
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass; // the associated supass
    // specify the dependency
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // explicitly create the renderpass
    if (vkCreateRenderPass(vkSetup.device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void DuckApplication::createImGuiRenderPass() {
    // creat the ImGui render pass
    VkAttachmentDescription attachment = {};
    attachment.format = swapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // the final layout is the presentation to the window

    VkAttachmentReference colorAttachment = {};
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    if (vkCreateRenderPass(vkSetup.device, &info, nullptr, &imGuiRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }
}

void DuckApplication::createDepthResources() {
    // depth image should have the same resolution as the colour attachment, defined by swap chain extent
    VkFormat depthFormat = findDepthFormat(); // find a depth format

    // we have the information needed to create an image (the format, usage etc) and an image view
    createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    // undefined layout is used as the initial layout as there are no existing depth image contents that matter
    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

VkFormat DuckApplication::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    // instead of a fixed format, get a list of formats ranked from most to least desirable and iterate through it
    for (VkFormat format : candidates) {
        // query the support of the format by the device
        VkFormatProperties props; // contains three fields
        // linearTilingFeatures: Use cases that are supported with linear tiling
        // optimalTilingFeatures: Use cases that are supported with optimal tiling
        // bufferFeatures : Use cases that are supported for buffer
        vkGetPhysicalDeviceFormatProperties(vkSetup.physicalDevice, format, &props);

        // test if the format is supported
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    // could either return a special value or throw an exception
    throw std::runtime_error("failed to find supported format!");
}

VkFormat DuckApplication::findDepthFormat() {
    // return a certain depth format if available
    return findSupportedFormat(
        // list of candidate formats
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        // the desired tiling
        VK_IMAGE_TILING_OPTIMAL,
        // the device format properties flag that we want 
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool DuckApplication::hasStencilComponent(VkFormat format) {
    // simple helper function that returns true if the specified format has a stencil buffer component
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

//
// Descriptors
//

void DuckApplication::createDescriptorSetLayout() {
    // provide details about every descriptor binding used in the shaders
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // this is a uniform descriptor
    // specify binding used
    uboLayoutBinding.binding = 0; // the first descriptor
    uboLayoutBinding.descriptorCount = 1; // single uniform buffer object so just 1, could be used to specify a transform for each bone in a skeletal animation
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // in which shader stage is the descriptor going to be referenced
    uboLayoutBinding.pImmutableSamplers = nullptr; // relevant to image sampling related descriptors

    // same as above but for a texture sampler rather than for uniforms
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // this is a sampler descriptor
    samplerLayoutBinding.binding = 1; // the second descriptor
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; ;// the shader stage we wan the descriptor to be used in, ie the fragment shader stage
    // can use the texture sampler in the vertex stage as part of a height map to deform the vertices in a grid
    samplerLayoutBinding.pImmutableSamplers = nullptr;


    // put the descriptors in an array
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    // descriptor set bindings combined into a descriptor set layour object, created the same way as other vk objects by filling a struct in
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());; // number of bindings
    layoutInfo.pBindings = bindings.data(); // pointer to the bindings

    if (vkCreateDescriptorSetLayout(vkSetup.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DuckApplication::createDescriptorPool() {
    // descriptor layout describes descriptors that can be bound. Create a descriptor set for each buffer. We need 
    // to create a descriptor pool to get the descriptor set (much like the command pool for command queues)
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    // we will allocate one pool for each frame
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // the uniform buffer descriptor
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // the sampler descriptor
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size()); // max nb of individual descriptors 
    poolInfo.pPoolSizes = poolSizes.data(); // the descriptors
    // the maximum number of descriptor sets that may be allocated
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

    // create the descirptor pool
    if (vkCreateDescriptorPool(vkSetup.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DuckApplication::createImGuiDescriptorPool() {

    // for ImGui
    std::array<VkDescriptorPoolSize, 11> poolSizes{};
    uint32_t swapChainImagesSize = static_cast<uint32_t>(swapChainImages.size());
    poolSizes[0] = { VK_DESCRIPTOR_TYPE_SAMPLER, swapChainImagesSize };
    poolSizes[1] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapChainImagesSize };
    poolSizes[2] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, swapChainImagesSize };
    poolSizes[3] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, swapChainImagesSize };
    poolSizes[4] = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, swapChainImagesSize };
    poolSizes[5] = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, swapChainImagesSize };
    poolSizes[6] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChainImagesSize };
    poolSizes[7] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, swapChainImagesSize };
    poolSizes[8] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, swapChainImagesSize };
    poolSizes[9] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, swapChainImagesSize };
    poolSizes[10] = { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, swapChainImagesSize };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size()); // max nb of individual descriptors 
    poolInfo.pPoolSizes = poolSizes.data(); // the descriptors
    // the maximum number of descriptor sets that may be allocated
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

    // create the descirptor pool
    if (vkCreateDescriptorPool(vkSetup.device, &poolInfo, nullptr, &imGuiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DuckApplication::createDescriptorSets() {
    // create the descriptor set
    std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    // specify the descriptor pool to allocate from
    allocInfo.descriptorPool = descriptorPool;
    // the number of descriptors to allocate
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    // a pointer to the descriptor layout to base them on
    allocInfo.pSetLayouts = layouts.data();

    // resize the descriptor set container to accomodate for the descriptor sets, as many as there are frames
    descriptorSets.resize(swapChainImages.size());

    // now attempt to create them. We don't need to explicitly clear the descriptor sets because they will be freed
    // when the desciptor set is destroyed. The function may fail if the pool is not sufieciently large, but succeed other times 
    // if the driver can solve the problem internally... so sometimes the driver will let us get away with an allocation
    // outside of the limits of the desciptor pool, and other times fail! Goes without saying that this is different for every machine...
    if (vkAllocateDescriptorSets(vkSetup.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // loop over the created descriptor sets to configure them
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        // the buffer and the region of it that contain the data for the descriptor
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i]; // contents of buffer for image i
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject); // here this is the size of the whole buffer, we can use VK_WHOLE_SIZE instead

        // bind the actual image and sampler to the descriptors in the descriptor set
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        // the struct configuring the descriptor set
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        // the uniform buffer
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i]; // wich set to update
        descriptorWrites[0].dstBinding = 0; // uniform buffer has binding 0
        descriptorWrites[0].dstArrayElement = 0; // descriptors can be arrays, only one element so first index
        // type of descriptor again
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1; // can update multiple descriptors at once starting at dstArrayElement, descriptorCount specifies how many elements

        descriptorWrites[0].pBufferInfo = &bufferInfo; // for descriptors that use buffer data
        descriptorWrites[0].pImageInfo = nullptr; // for image data
        descriptorWrites[0].pTexelBufferView = nullptr; // desciptors refering to buffer views

        // the texture sampler
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1; // sampler has binding 1
        descriptorWrites[1].dstArrayElement = 0; // only one element so index 0
        // type of descriptor again
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;

        descriptorWrites[1].pBufferInfo = nullptr; // for descriptors that use buffer data
        descriptorWrites[1].pImageInfo = &imageInfo; // for image data
        descriptorWrites[1].pTexelBufferView = nullptr; // desciptors refering to buffer views

        // update according to the configuration
        vkUpdateDescriptorSets(vkSetup.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void DuckApplication::createUniformBuffers() {
    // specify what the size of the buffer is
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // resize the uniform buffer to be as big as the swap chain, each image has its own se of uniforms
    uniformBuffers.resize(swapChainImages.size());
    uniformBuffersMemory.resize(swapChainImages.size());

    // loop over the images and create a uniform buffer for each
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
    }
}

void DuckApplication::createTextureSampler() {
    // configure the sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // how to interpolate texels that are magnified or minified
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    // addressing mode
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    // VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions.
    // VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts the coordinates to mirror the image when going beyond the dimensions.
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : Take the color of the edge closest to the coordinate beyond the image dimensions.
    // VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE : Like clamp to edge, but instead uses the edge opposite to the closest edge.
    // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER : Return a solid color when sampling beyond the dimensions of the image

    samplerInfo.anisotropyEnable = VK_TRUE; // use unless performance is a concern (IT WILL BE)
    VkPhysicalDeviceProperties properties{}; // can query these here or at beginning for reference
    vkGetPhysicalDeviceProperties(vkSetup.physicalDevice, &properties);
    // limites the amount of texel samples that can be used to calculate final colours, obtain from the device properties
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    // self ecplanatory, can't be an arbitrary colour
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // which coordinate system we want to use to address texels! usually always normalised
    // if comparison enabled, texels will be compared to a value and result is used in filtering (useful for shadow maps)
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    // mipmapping fields
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    // now create the configured sampler
    if (vkCreateSampler(vkSetup.device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

//
// Buffers (command, frame) setup
//

void DuckApplication::createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags) {
    // submit command buffers by submitting to one of the device queues, like graphics and presentation
    // each command pool can only allocate command buffers submitted on a single type of queue
    QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::findQueueFamilies(vkSetup.physicalDevice, vkSetup.surface);

    // command pool needs two parameters
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // the queue to submit to
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    // flag for command pool, influences how command buffers are rerecorded
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT -> rerecorded with new commands often
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT -> let command buffers be rerecorded individually rather than together
    poolInfo.flags = flags; // in our case, we only record at beginning of program so leave empty

    // and create the command pool, we therfore ave to destroy it explicitly in cleanup
    if (vkCreateCommandPool(vkSetup.device, &poolInfo, nullptr, commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void DuckApplication::createCommandBuffers(std::vector<VkCommandBuffer>* commandBuffers, VkCommandPool& commandPool) {
    // resize the command buffers container to the same size as the frame buffers container
    commandBuffers->resize(swapChainFramebuffers.size());

    // create the struct
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // identify the struct type
    allocInfo.commandPool = commandPool; // specify the command pool
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // level specifes is the buffers are primary or secondary
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY -> can be submitted to a queue for exec but not called from other command buffers
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY -> cannot be submitted directly, but can be called from primary command buffers
    allocInfo.commandBufferCount = (uint32_t)commandBuffers->size(); // the number of buffers to allocate

    if (vkAllocateCommandBuffers(vkSetup.device, &allocInfo, commandBuffers->data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    // start recording a command buffer 
    for (size_t i = 0; i < commandBuffers->size(); i++) {
        // the following struct used as argument specifying details about the usage of specific command buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // how we are going to use the command buffer
        // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT -> command buffer will be immediately rerecorded
        // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT -> is a secondary command buffer entirely within a single render pass
        // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT -> command buffer can be resbmitted while also already pending execution
        beginInfo.pInheritanceInfo = nullptr; // relevant to secondary comnmand buffers

        // creating implicilty resets the command buffer if it was already recorded once, cannot append
        // commands to a buffer at a later time!
        if (vkBeginCommandBuffer((*commandBuffers)[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // create a render pass, initialised with some params in the following struct
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass; // handle to the render pass and the attachments to bind
        renderPassInfo.framebuffer = swapChainFramebuffers[i]; // the framebuffer created for each swapchain image view
        renderPassInfo.renderArea.offset = { 0, 0 }; // some offset for the render area
        // best performance if same size as attachment
        renderPassInfo.renderArea.extent = swapChainExtent; // size of the render area (where shaders load and stores occur, pixels outside are undefined)

        // because we used the VK_ATTACHMENT_LOAD_OP_CLEAR for load operations of the render pass, we need to set clear colours
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // black with max opacity
        clearValues[1].depthStencil = { 1.0f, 0 }; // initialise the depth value to far (1 in the range of 0 to 1)
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); // only use a single value
        renderPassInfo.pClearValues = clearValues.data(); // the colour to use for clear operation

        // begin the render pass. All vkCmd functions are void, so error handling occurs at the end
        // first param for all cmd are the command buffer to record command to, second details the render pass we've provided
        vkCmdBeginRenderPass((*commandBuffers)[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // final parameter controls how drawing commands within the render pass will be provided 
        // VK_SUBPASS_CONTENTS_INLINE -> render pass cmd embedded in primary command buffer and no secondary command buffers will be executed
        // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS -> render pass commands executed from secondary command buffers

            // bind the graphics pipeline, second param determines if the object is a graphics or compute pipeline
        vkCmdBindPipeline((*commandBuffers)[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        // bind the vertex buffer, can have many vertex buffers
        vkCmdBindVertexBuffers((*commandBuffers)[i], 0, 1, vertexBuffers, offsets);
        // bind the index buffer, can only have a single index buffer 
        // params (-the nescessary cmd) bufferindex buffer, byte offset into it, type of data
        vkCmdBindIndexBuffer((*commandBuffers)[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        // bind the uniform descriptor sets
        vkCmdBindDescriptorSets((*commandBuffers)[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        // command to draw the vertices in the vertex buffer
        //vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0); 
        // params :
        // the command buffer
        // instance count, for instance rendering, so only one here
        // first vertex, offset into the vertex buffer. Defines lowest value of gl_VertexIndex
        // first instance, offset for instance rendering. Defines lowest value of gl_InstanceIndex

        vkCmdDrawIndexed((*commandBuffers)[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        // params :
        // the command buffer
        // the indices
        // no instancing so only a single instance
        // offset into index buffer
        // offest to add to the indices in the index buffer
        // offest for instancing

        // /!\ about vertex and index buffers /!\
            // The previous chapter already mentioned that should allocate multiple resources like buffers 
            // from a single memory allocation. Even better, Driver developers recommend to store multiple buffers, 
            // like the vertex and index buffer, into a single VkBuffer and use  offsets in commands like vkCmdBindVertexBuffers. 
            // The advantage is that your data is more cache friendly, because it's closer together. 

        // end the render pass
        vkCmdEndRenderPass((*commandBuffers)[i]);

        // we've finished recording, so end recording and check for errors
        if (vkEndCommandBuffer((*commandBuffers)[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

VkCommandBuffer DuckApplication::beginSingleTimeCommands(VkCommandPool& commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    // allocate the command buffer
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vkSetup.device, &allocInfo, &commandBuffer);

    // set the struct for the command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // tell the driver about how the command buffer will be used for optimisation

    // start recording the command buffer
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void DuckApplication::endSingleTimeCommands(VkCommandBuffer* commandBuffer, VkCommandPool* commandPool) {
    // end recording
    vkEndCommandBuffer(*commandBuffer);

    // execute the command buffer by completing the submitinfo struct
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer;

    // submit the queue for execution
    vkQueueSubmit(vkSetup.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    // here we could use a fence to schedule multiple transfers simultaneously and wait for them to complete instead
    // of executing all at the same time, alternatively use wait for the queue to execute
    vkQueueWaitIdle(vkSetup.graphicsQueue);

    // free the command buffer once the queue is no longer in use
    vkFreeCommandBuffers(vkSetup.device, *commandPool, 1, commandBuffer);
}

void DuckApplication::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    // images may have different layout that affect how pixels are organised in memory, so we need to specify which layout we are transitioning
    // to and from to lake sure we have the optimal layout for our task
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderCommandPool);
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

    endSingleTimeCommands(&commandBuffer, &renderCommandPool);
}

void DuckApplication::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    // copying buffer to image
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderCommandPool);
    
    // need to specify which parts of the buffer we are going to copy to which part of the image
    VkBufferImageCopy region{};
    
    // about the buffer area
    region.bufferOffset = 0; // byte offset after which the pixels start
    // how pixels are laid out in memory
    region.bufferRowLength = 0; // 0 = tightly packed
    region.bufferImageHeight = 0;

    // about the image area, which part of te image we want to copy the pixels
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    // buffer to image copy operations are enqueued thus
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // which layout the image is currently using
        1,
        &region
    );
    endSingleTimeCommands(&commandBuffer, &renderCommandPool);
}

void DuckApplication::createFrameBuffers() {
    // resize the container to hold all the framebuffers, or image views, in the swap chain
    swapChainFramebuffers.resize(swapChainImageViews.size());

    // now loop over the image views and create the framebuffers, also bind the image to the attachment
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        // get the attachment 
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass; // which renderpass the framebuffer needs, only one at the moment
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // the number of attachments, or VkImageView objects, to bind to the buffer
        framebufferInfo.pAttachments = attachments.data(); // pointer to the attachment(s)
        framebufferInfo.width = swapChainExtent.width; // specify dimensions of framebuffer depending on swapchain dimensions
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1; // single images so only one layer

        // attempt to create the framebuffer and place in the framebuffer container
        if (vkCreateFramebuffer(vkSetup.device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void DuckApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
    VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // fill in the corresponding struct
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // allocate a buffer of the right size in bytes
    bufferInfo.usage = usage; // what the data in the buffer is used for
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffers can be owned by a single queue or shared between many
    // bufferInfo.flags = 0; // to configure sparse memory

    // attempt to create a buffer
    if (vkCreateBuffer(vkSetup.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    // created a buffer, but haven't assigned any memory yet, also get the right memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkSetup.device, buffer, &memRequirements);

    // allocate the memory for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // allocate memory for the buffer. In a real world application, not supposed to actually call vkAllocateMemory for every individual buffer. 
    // The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit. The right way to 
    // allocate memory for large number of objects at the same time is to create a custom allocator that splits up a single allocation among many 
    // different objects by using the offset parameters seen in other functions
    if (vkAllocateMemory(vkSetup.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    // associate memory with buffer
    vkBindBufferMemory(vkSetup.device, buffer, bufferMemory, 0);
}

void DuckApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // memory transfer operations are executed using command buffers, like drawing commands. We need to allocate a temporary command buffer
    // could use a command pool for these short lived operations using the flag VK_COMMAND_POOL_CREATE_TRANSIENT_BIT 
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderCommandPool);

    // defines the region of 
    VkBufferCopy copyRegion{};
    // offsets in the source and destination, we may want to keep certain values
    copyRegion.srcOffset = 0; 
    copyRegion.dstOffset = 0; 
    copyRegion.size = size; // can't be VK_WHOLE_SIZE here like vkMapMemory
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(&commandBuffer, &renderCommandPool);
}

uint32_t DuckApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // GPUs allocate dufferent types of memory, varying in terms of allowed operations and performance. Combine buffer and application
    // requirements to find best type of memory
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkSetup.physicalDevice, &memProperties);
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
// model
//

void DuckApplication::loadModel() {
    // setup variables to get model info
    tinyobj::attrib_t attrib; // contains all the positions, normals, textures and faces
    std::vector<tinyobj::shape_t> shapes; // all the separate objects and their faces
    std::vector<tinyobj::material_t> materials; // object materials
    std::string warn, err;

    // load the model, show error if not
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(warn + err);
    }

    // combine all the shapes into a single model
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            // set vertex data
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
    }
}

void DuckApplication::createVertexBuffer() {
    // precompute buffer size
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    // call our helper buffer creation function

    // use a staging buffer for mapping and copying 
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    // in host memory (cpu)
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    // VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.

    void* data;
    // access a region in memory ressource defined by offset and size (0 and bufferInfo.size), can use special value VK_WHOLE_SIZE to map all of the memory
    // second to last is for flages (none in current API so set to 0), last is output for pointer to mapped memory
    vkMapMemory(vkSetup.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize); // memcpy the data in the vertex list to that region in memory
    vkUnmapMemory(vkSetup.device, stagingBufferMemory); // unmap the memory 
    // possible issues as driver may not immediately copy data into buffer memory, writes to buffer may not be visible in mapped memory yet...
    // either use a heap that is host coherent (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT in memory requirements)
    // or call vkFlushMappedMemoryRanges after writing to mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory

    // create the vertex buffer, now the memory is device local (faster)
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    // VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    // cleanup after using the staging buffer
    vkDestroyBuffer(vkSetup.device, stagingBuffer, nullptr);
    vkFreeMemory(vkSetup.device, stagingBufferMemory, nullptr);
}

void DuckApplication::createIndexBuffer() {
    // almost identical to the vertex buffer creation process except where commented
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vkSetup.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(vkSetup.device, stagingBufferMemory);

    // different usage bit flag VK_BUFFER_USAGE_INDEX_BUFFER_BIT instead of VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(vkSetup.device, stagingBuffer, nullptr);
    vkFreeMemory(vkSetup.device, stagingBufferMemory, nullptr);
}

//
// Textures
//

void DuckApplication::createTextureImage() {
    // uses command buffer so should be called after create command pool
    int texWidth, texHeight, texChannels;

    // load the file 
    // forces image to be loaded with an alpha channel, returns ptr to first element in an array of pixels
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha); 
    // laid out row by row with 4 bytes by pixel in case of STBI_rgb_alpha
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // create a staging buffer in host visible memory so we can map it, not device memory although that is our destination
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory);

    // directly copy the pixels in the array from the image loading library to the buffer
    void* data;
    vkMapMemory(vkSetup.device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(vkSetup.device, stagingBufferMemory);

    // and cleanup pixels after copying in the data
    stbi_image_free(pixels);

    // now create the image
    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    // next step is to copy the staging buffer to the texture image using our helper functions
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // specify the initial layout VK_IMAGE_LAYOUT_UNDEFINED
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    // need another transfer to give the shader access to the texture
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // cleanup the staging buffer and its memory
    vkDestroyBuffer(vkSetup.device, stagingBuffer, nullptr);
    vkFreeMemory(vkSetup.device, stagingBufferMemory, nullptr);
}

void DuckApplication::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    // create the struct for creating an image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D; // coordinate system of the texels
    imageInfo.extent.width = width; // the dimensions of the image
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1; // no mip mapping yet
    imageInfo.arrayLayers = 1; // not in array yet
    imageInfo.format = format; // same format as the pixels is best
    imageInfo.tiling = tiling; // tiling of the pixels, let vulkan lay them out
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage; // same semantics as during buffer creation, here as destination for the buffer copy
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // for multisampling
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // used by a single queue family
    imageInfo.flags = 0; // Optional, for sparse data

    // create the image. The hardware could fail for the format we have specified. We should have a list of acceptable formats and choose the best one depending
    // on the selection of formats supported by the device
    if (vkCreateImage(vkSetup.device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    // allocate memory for an image, similar to a buffer allocation
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkSetup.device, image, &memRequirements);

    // info on the allocation
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // attempt to create an image
    if (vkAllocateMemory(vkSetup.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    // associate the memory to the image
    vkBindImageMemory(vkSetup.device, image, imageMemory, 0);
}

void DuckApplication::createTextureImageView() {
    // just like we need views for the swap chain images, so do we need a view for the texture image view
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView DuckApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
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
    if (vkCreateImageView(vkSetup.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    // and return the image view
    return imageView;
}

//
// Swap chain and surface setup
//

SwapChainSupportDetails DuckApplication::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    // query the surface capabilities and store in a VkSurfaceCapabilities struct
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSetup.surface, &details.capabilities); // takes into account device and surface when determining capabilities

    // same as we have seen many times before
    uint32_t formatCount;
    // query the available formats, pass null ptr to just set the count
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSetup.surface, &formatCount, nullptr);

    // if there are formats
    if (formatCount != 0) {
        // then resize the vector accordingly
        details.formats.resize(formatCount);
        // and set details struct fromats vector with the data pointer
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSetup.surface, &formatCount, details.formats.data());
    }

    // exact same thing as format for presentation modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSetup.surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSetup.surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

// next three functions are for setting the parameters of the swap chain

VkSurfaceFormatKHR DuckApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
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

VkPresentModeKHR DuckApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
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

VkExtent2D DuckApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

void DuckApplication::createSwapChain() {
    // create the swap chain by checking for swap chain support
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vkSetup.physicalDevice);

    // set the swap chain properties using the above three methods for the format, presentation mode and capabilities
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

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
    createInfo.surface = vkSetup.surface; // specify the surface the swap chain should be tied to
    // set details
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // amount of layers each image consists of
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // bit field, specifies types of operations we'll use images in swap chain for

    // how to handle the swap chain images across multiple queue families (in case graphics queue is different to presentation queue)
    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(vkSetup.physicalDevice, vkSetup.surface);
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
    if (vkCreateSwapchainKHR(vkSetup.device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    // get size of swap chain images
    vkGetSwapchainImagesKHR(vkSetup.device, swapChain, &imageCount, nullptr);
    // resize accordingly
    swapChainImages.resize(imageCount);
    // pull the images
    vkGetSwapchainImagesKHR(vkSetup.device, swapChain, &imageCount, swapChainImages.data());

    // save format and extent
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void DuckApplication::recreateSwapChain() {
    // for handling window minimisation, we get the size of the windo through the glfw framebuffer dimensions
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    // start an infinite loop to hang the process
    while (width == 0 || height == 0) {
        // continually evaluate the window dimensions, if the window is no longer hidder the loop will terminate
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    // wait before touching if in use by the device
    vkDeviceWaitIdle(vkSetup.device);

    // destroy the previous swap chain
    cleanupSwapChain();

    // all creation functions for objects that depend on swap chain / window size
    createSwapChain(); // recreate the swap chain itself
    createImageViews(); // because views are based on swap chain images, they need to be recreated as well
    createRenderPass(); // render pass needs to be recreated because it depends on format of swap chain images
    // rare for swapchain image format to change but needs handling, viewport and scissors specified in pipeline 
    // creation so needs to be recreated. NB we can avoid this using dynamic states
    createGraphicsPipeline();
    createDepthResources();
    createFrameBuffers(); // directly depend on swap chain images so recreate
    createUniformBuffers(); // depends on swap chain size so recreate
    createDescriptorPool();
    //createImGuiDescriptorPool();
    createDescriptorSets();
    createCommandBuffers(&renderCommandBuffers, renderCommandPool); // directly depend on swap chain images so recreate

    // tell ImGui to update the swap chain information
    //ImGui_ImplVulkan_SetMinImageCount(static_cast<uint32_t>(swapChainImages.size()));
}

void DuckApplication::cleanupSwapChain() {
    // destroy the depth image and related stuff (view and free memory)
    vkDestroyImageView(vkSetup.device, depthImageView, nullptr);
    vkDestroyImage(vkSetup.device, depthImage, nullptr);
    vkFreeMemory(vkSetup.device, depthImageMemory, nullptr);

    // destroy the frame buffers
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(vkSetup.device, swapChainFramebuffers[i], nullptr);
    }

    // destroy the command buffers. This lets us preserve the command pool rather than wastefully creating and deestroying repeatedly
    vkFreeCommandBuffers(vkSetup.device, renderCommandPool, static_cast<uint32_t>(renderCommandBuffers.size()), renderCommandBuffers.data());

    // destroy pipeline and related data
    vkDestroyPipeline(vkSetup.device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vkSetup.device, pipelineLayout, nullptr);
    vkDestroyRenderPass(vkSetup.device, renderPass, nullptr);

    // loop over the image views and destroy them. NB we don't destroy the images because they are implicilty created
    // and destroyed by the swap chain
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(vkSetup.device, swapChainImageViews[i], nullptr);
    }

    // destroy the swap chain proper
    vkDestroySwapchainKHR(vkSetup.device, swapChain, nullptr);

    // also destroy the uniform buffers that worked with the swap chain
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vkDestroyBuffer(vkSetup.device, uniformBuffers[i], nullptr);
        vkFreeMemory(vkSetup.device, uniformBuffersMemory[i], nullptr);
    }

    // cleanup the descriptor pools
    //vkDestroyDescriptorPool(vkSetup.device, imGuiDescriptorPool, nullptr);
    vkDestroyDescriptorPool(vkSetup.device, descriptorPool, nullptr);
}

void DuckApplication::createImageViews() {
    // resize the vector of views to accomodate the images
    swapChainImageViews.resize(swapChainImages.size());
    // loop to iterate through images and create a view for each
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

//
// Synchronisation
//

void DuckApplication::createSyncObjects() {
    // resize the semaphores to the number of simultaneous frames, each has its own semaphores
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE); // explicitly initialise the fences in this vector to no fence

    VkSemaphoreCreateInfo semaphoreInfo{};
    // only required field at the moment, may change in the future
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // initialise in the signaled state

    // simply loop over each frame and create semaphores for them
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // attempt to create the semaphors
        if (vkCreateSemaphore(vkSetup.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vkSetup.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vkSetup.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

//
// Main loop and drawing
//

void DuckApplication::mainLoop() {
    // loop keeps window open
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();


        // the ui stuff
        /*
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();
        */

        // magic stuff with ImGui happening here
        //memcpy(&wd->ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
        
        // the vulkan stuff
        drawFrame();
    }
    vkDeviceWaitIdle(vkSetup.device);
}

void DuckApplication::drawFrame() {
    // will acquire an image from swap chain, exec commands in command buffer with images as attachments in the frameBuffer
    // return the image to the swap buffer. These tasks are started simultaneously but executed asynchronously.
    // However we want these to occur in sequence because each relies on the previous task success
    // For syncing can use semaphores or fences and coordinate operations by having one op signal another
    // op and another operation wait for a fence or semaphor to go from unsignaled to signaled.
    // we can access fence state with vkWaitForFences and not semaphores.
    // fences are mainly for syncing app with rendering op, use here to synchronise the frame rate
    // semaphores are for syncing ops within or across cmd queues. We want to sync queue op to draw cmds and presentation so pref semaphores here

    // at the start of the frame, make sure that the previous frame has finished which will signal the fence
    vkWaitForFences(vkSetup.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // retrieve an image from the swap chain
    uint32_t imageIndex;
    // swap chain is an extension so use the vk*KHR function
    VkResult result = vkAcquireNextImageKHR(vkSetup.device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); // params:
    // the logical device and the swap chain we want to restrieve image from
    // a timeout in ns. Using UINT64_MAX disables it
    // synchronisation objects, so a semaphore
    // handle to another sync object (which we don't use so nul handle)
    // variable to output the swap chain image that has become available

    // Vulkan tells us if the swap chain is out of date with this result value (the swap chain is incompatible with the surface, eg window resize)
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // if so then recreate the swap chain and try to acquire the image from the new swap chain
        recreateSwapChain();
        return; // return to acquire the image again
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // both values here are considered "success", even if partial, values
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(vkSetup.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    // update the unifrom buffer before submitting
    updateUniformBuffer(imageIndex);

    // info needed to submit the command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // which semaphores to wait on before execution begins
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    // which stages of the pipeline to wait at (here at the stage where we write colours to the attachment)
    // we can in theory start work on vertex shader etc while image is not yet available
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages; // for each sempahore we provide a wait stage
    // which command buffer to submit to, submit the cmd buffer that binds the swap chain image we acquired as color attachment
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderCommandBuffers[imageIndex];

    // which semaphores to signal once the command buffer(s) has finished, we are using the renderFinishedSemaphore for that
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // reset the fence to block next frame just before using the fence
    vkResetFences(vkSetup.device, 1, &inFlightFences[currentFrame]);

    // submit the command buffer to the graphics queue, takes an array of submitinfo when work load is much larger
    // last param is a fence, should be signaled when the cmd buffer finished executing so use to signal frame has finished
    if (vkQueueSubmit(vkSetup.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // submitting the result back to the swap chain to have it shown onto the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // which semaphores to wait on 
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    // specify the swap chains to present image to and index of image for each swap chain
    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    // allows to specify an array of vKResults to check for every individual swap chain if presentation is succesful
    presentInfo.pResults = nullptr; // Optional

    // submit the request to put an image from the swap chain to the presentation queue
    result = vkQueuePresentKHR(vkSetup.presentQueue, &presentInfo);

    // similar to when acquiring the swap chain image, check that the presentation queue can accept the image, also check for resizing
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // after the ImGui frame is drawn, increment current frame count (% loops around)
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void DuckApplication::updateUniformBuffer(uint32_t currentImage) {
    // compute the time elapsed since rendering began
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    // translate the model in the positive z axis
    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -30.0f, -85.0f));
    ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // a simple rotation around the z axis
    ubo.model = glm::rotate(ubo.model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // make the camera view the geometry from above at a 45° angle (eye pos, subject pos, up direction)
    ubo.view = glm::mat4(1.0f); //glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    // proect the scene with a 45° fov, use current swap chain extent to compute aspect ratio, near, far
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
    // designed for openGL, so y coordinates are inverted
    ubo.proj[1][1] *= -1;

    // copy the uniform buffer object into the uniform buffer
    void* data;
    vkMapMemory(vkSetup.device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(vkSetup.device, uniformBuffersMemory[currentImage]);
}

//
// Shaders
//

std::vector<char> DuckApplication::readFile(const std::string& filename) {
    // create an input file stream, place cursor at the end and read in binary
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    // check that the stream was succesfully opened
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    // reading in the file at the end, we can use read position to determine how big the file is
    // and allocate a buffer accordingly. tellg returns position of current character (ate)
    size_t fileSize = (size_t)file.tellg();
    // now allocate the buffer to accomodate for the file size and the data (bytes)
    std::vector<char> buffer(fileSize);
    // place cursor back at the begining of the file
    file.seekg(0);
    // then read the data, pointer to vector data and fileSIze informs where to place data and how much to read
    file.read(buffer.data(), fileSize);
    // good practice, always close the file!
    file.close();
    // and return the file in the buffer (bytes)
    return buffer;
}

VkShaderModule DuckApplication::createShaderModule(const std::vector<char>& code) {
    // need to wrap the shader code into a shader module through this helper function, takes 
    // pointer to the byte code as argument
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    // the pointer to the bytecode needs to be a unit32_t, but it is currently a char. The reinterpret cast
    // needs the data to satisfy the alignment of the uint32_t... But apparently the vector guarantees that
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    // create the shader module 
    if (vkCreateShaderModule(vkSetup.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

//
// Cleanup
//

void DuckApplication::cleanup() {

    // call the function we created for destroying the swap chain
    cleanupSwapChain();

    // destroy the texture image view and sampler
    vkDestroySampler(vkSetup.device, textureSampler, nullptr);
    vkDestroyImageView(vkSetup.device, textureImageView, nullptr);

    // destroy the texture
    vkDestroyImage(vkSetup.device, textureImage, nullptr);
    vkFreeMemory(vkSetup.device, textureImageMemory, nullptr);

    // destroy the descriptor layout
    vkDestroyDescriptorSetLayout(vkSetup.device, descriptorSetLayout, nullptr);

    // destroy the index buffer and free its memory
    vkDestroyBuffer(vkSetup.device, indexBuffer, nullptr);
    vkFreeMemory(vkSetup.device, indexBufferMemory, nullptr);

    // destroy the vertex buffer and free its memory
    vkDestroyBuffer(vkSetup.device, vertexBuffer, nullptr);
    vkFreeMemory(vkSetup.device, vertexBufferMemory, nullptr);

    // loop over each frame and destroy its semaphores 
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vkSetup.device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vkSetup.device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vkSetup.device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(vkSetup.device, renderCommandPool, nullptr);

    vkSetup.cleanupSetup();

    // destory the window
    glfwDestroyWindow(window);

    // terminate glfw
    glfwTerminate();
}

//
// GLFW
//

void DuckApplication::initWindow() {
    // initialise glfw library
    glfwInit();

    // set parameters
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // initially for opengl, so tell it not to create opengl context
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable resizing for now

    // create the window, 4th param refs a monitor, 5th param is opengl related
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    // store an arbitrary pointer to a glfwwindow because glfw does not know how to call member function from a ptr to an instance of this class
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback); // tell the window what the call back for resizing is
}

void DuckApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    // pointer to this application class obtained from glfw, it doesnt know that it is a DuckApplication but we do so we can cast to it
    auto app = reinterpret_cast<DuckApplication*>(glfwGetWindowUserPointer(window));
    // and set the resize flag to true
    app->framebufferResized = true;
}