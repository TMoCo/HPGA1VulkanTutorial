// include class definition
#include "..\headers\DuckApplication.h"

// reporting and propagating exceptions
#include <iostream> 
#include <stdexcept>

// min, max
#include <algorithm>

// file (shader) loading
#include <fstream>

// UINT32_MAX
#include <cstdint>

// set for queues
#include <set>


// a triangle of vertices, interleaving vertex attributes
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

// vertex index container
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

//
// Run application
//

void DuckApplication::run() {
    // initialise a glfw window
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

//
// Initialisation
//

void DuckApplication::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFrameBuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createCommandBuffers();
    createSyncObjects();
}

//
// Vulkan instance 
//

void DuckApplication::createInstance() {
    // if we have enabled validation layers and some requested layers aren't available, throw error
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    // create a 0 initialised (curly braces) VkApplicationInfo, technically optional
    VkApplicationInfo appInfo{};
    // tells the driver how to optimise for our purpose
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // what kind of vulkan struct this appinfo belongs to
    appInfo.pApplicationName = "Hello Triangle"; // name of the application
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // app version 1.0.0
    appInfo.pEngineName = "No Engine"; // no engine used
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); // no engine version 1.0.0
    appInfo.apiVersion = VK_API_VERSION_1_0; // version of API used
    // pNext initialised to nullptr

    // create a VkInstanceCreateInfo struct, not optional!
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // type of instance we are creating
    createInfo.pApplicationInfo = &appInfo; // pointer to 

    auto extensions = getRequiredExtensions();
    // update createInfo
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // create a debug messenger before the instance is created to capture any errors in creation process
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    // include valdation layers if enables
    if (enableValidationLayers) {
        // save layer count, cast size_t to uin32_t
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // we can now create the instance (pointer to struct, pointer to custom allocator callbacks, 
    // pointer to handle that stores the new object)
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) { // check everything went well by comparing returned value
        throw std::runtime_error("failed to create a vulkan instance!");
    }
}

std::vector<const char*> DuckApplication::getRequiredExtensions() {
    // start by getting the glfw extensions, nescessary for displaying something in a window.
    // platform agnostic, so need an extension to interface with window system. Use GLFW to return
    // the extensions needed for platform and passed to createInfo struct
    uint32_t glfwExtensionCount = 0; // initialise extension count to 0, changed later
    const char** glfwExtensions; // array of strings with extension names
    // get extension count
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // glfwExtensions is an array of strings, we give the vector constructor a range of values from glfwExtensions to 
    // copy (first value at glfwExtensions, a pointer, to last value, pointer to first + nb of extensions)
    // a vector containing the values from glfwExtensions. 
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // add the VK_EXT_debug_utils with macro on condition debug is activated
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // return the vector
    return extensions;
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
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // vertex order for faces to be front facing (CW and CCW)
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

    // this struct references array of stgructures for all framebuffers and sets constants to use as blend factors
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1; // the previously declared attachment
    colorBlending.pAttachments = &colorBlendAttachment;

    // create the pipeline layout, where uniforms are specified, also push constants another way of passing dynamic values
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
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
    // the pipeline layout is a vulkan handle rather than a struct pointer
    pipelineInfo.layout = pipelineLayout;
    // and a reference to the render pass
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0; // index of desired sub pass where pipeline will be used

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // destroy the shader modules, as we don't need them once the shaders have been compiled
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
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
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // layout to transition to post render pass (image should be ready for presentation)
    // common layouts
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : Images to be presented in the swap chain
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Images to be used as destination for a memory copy operation

    // a single render pass consists of multiple subpasses, which are subsequent rendering operations depending on content of
    // framebuffers on previous passes (eg post processing). Grouping subpasses into a single render pass lets Vulkan optimise
    // every subpass references 1 or more attachments (see above) with structs:
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // only one attachment at the moment so refer to attachment index 0
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // which layout we want to attachment to have during a subpass

    // the subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // be explicit that this is a graphics subpass (Vulkan supports compute subpasses)

    // specify the reference to the colour attachment 
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // other types of attachments can also be referenced

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
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    // ops that should wait are in colour attachment stage and involve writing of the colour attachment
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // now create the render pass, can be created by filling the structure with references to arrays for multiple subpasses, attachments and dependencies
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment; // the colour attachment for the renderpass
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass; // the associated supass
    // specify the dependency
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // explicitly create the renderpass
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

//
// Buffers (command, frame, vertex) setup
//

void DuckApplication::createFrameBuffers() {
    // resize the container to hold all the framebuffers, or image views, in the swap chain
    swapChainFramebuffers.resize(swapChainImageViews.size());

    // now loop over the image views and create framebuffers from them
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        // get the attachment 
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass; // which renderpass the framebuffer needs, only one at the moment
        framebufferInfo.attachmentCount = 1; // the number of attachments, or VkImageView objects, to bind to the buffer
        framebufferInfo.pAttachments = attachments; // pointer to the attachment(s)
        framebufferInfo.width = swapChainExtent.width; // specify dimensions of framebuffer depending on swapchain dimensions
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1; // single images so only one layer

        // attempt to create the framebuffer and place in the framebuffer container
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void DuckApplication::createCommandPool() {
    // submit command buffers by submitting to one of the device queues, like graphics and presentation
    // each command pool can only allocate command buffers submitted on a single type of queue
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    // command pool needs two parameters
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // the queue to submit to
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    // flag for command pool, influences how command buffers are rerecorded
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT -> rerecorded with new commands often
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT -> let command buffers be rerecorded individually rather than together
    poolInfo.flags = 0; // in our case, we only record at beginning of program so leave empty

    // and create the command pool, we therfore ave to destroy it explicitly in cleanup
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void DuckApplication::createCommandBuffers() {
    // resize the command buffers container to the same size as the frame buffers container
    commandBuffers.resize(swapChainFramebuffers.size());

    // create the struct
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; // identify the struct type
    allocInfo.commandPool = commandPool; // specify the command pool
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // level specifes is the buffers are primary or secondary
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY -> can be submitted to a queue for exec but not called from other command buffers
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY -> cannot be submitted directly, but can be called from primary command buffers
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size(); // the number of buffers to allocate

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    // start recording a command buffer 
    for (size_t i = 0; i < commandBuffers.size(); i++) {
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
        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
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

        // because we used the VK_ATTACHMENT_LOAD_OP_CLEAR for the load operation of the render pass, we need to set a clear colour
        VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // black with max opacity
        renderPassInfo.clearValueCount = 1; // only use a single value
        renderPassInfo.pClearValues = &clearColor; // the colour to use for clear operation

        // begin the render pass. All vkCmd functions are void, so error handling occurs at the end
        // first param for all cmd are the command buffer to record command to, second details the render pass we've provided
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // final parameter controls how drawing commands within the render pass will be provided 
        // VK_SUBPASS_CONTENTS_INLINE -> render pass cmd embedded in primary command buffer and no secondary command buffers will be executed
        // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS -> render pass commands executed from secondary command buffers

            // bind the graphics pipeline, second param determines if the object is a graphics or compute pipeline
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer vertexBuffers[] = { vertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            // bind the vertex buffer, can have many vertex buffers
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
            // bind the index buffer, can only have a single index buffer 
            // params (-the nescessary cmd) bufferindex buffer, byte offset into it, type of data
            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16); 

            // command to draw the vertices in the vertex buffer
            //vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0); 
            // params :
            // the command buffer
            // instance count, for instance rendering, so only one here
            // first vertex, offset into the vertex buffer. Defines lowest value of gl_VertexIndex
            // first instance, offset for instance rendering. Defines lowest value of gl_InstanceIndex

            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
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
        vkCmdEndRenderPass(commandBuffers[i]);

        // we've finished recording, so end recording and check for errors
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
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
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    // created a buffer, but haven't assigned any memory yet, also get the right memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    // allocate the memory for the buffer
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // allocate memory for the buffer. In a real world application, not supposed to actually call vkAllocateMemory for every individual buffer. 
    // The maximum number of simultaneous memory allocations is limited by the maxMemoryAllocationCount physical device limit. The right way to 
    // allocate memory for large number of objects at the same time is to create a custom allocator that splits up a single allocation among many 
    // different objects by using the offset parameters seen in other functions
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    // associate memory with buffer
    vkBindBufferMemory(device, buffer, bufferMemory, 0);

}

void DuckApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // memory transfer operations are executed using command buffers, like drawing commands. We need to allocate a temporary command buffer
    // could use a command pool for these short lived operations using the flag VK_COMMAND_POOL_CREATE_TRANSIENT_BIT 
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    // allocate the command buffer
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    // set the struct for the command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // tell the driver about how the command buffer will be used for optimisation

    // start recording the command buffer
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // defines the region of 
    VkBufferCopy copyRegion{};
    // offsets in the source and destination, we may want to keep certain values
    copyRegion.srcOffset = 0; 
    copyRegion.dstOffset = 0; 
    copyRegion.size = size; // can't be VK_WHOLE_SIZE here like vkMapMemory
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // end recording
    vkEndCommandBuffer(commandBuffer);

    // execute the command buffer by completing the submitinfo struct
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // submit the queue for execution
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    // here we could use a fence to schedule multiple transfers simultaneously and wait for them to complete instead
    // of executing all at the same time, alternatively use wait for the queue to execute
    vkQueueWaitIdle(graphicsQueue);

    // free the command buffer once the queue is no longer in use
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
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
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize); // memcpy the data in the vertex list to that region in memory
    vkUnmapMemory(device, stagingBufferMemory); // unmap the memory 
    // possible issues as driver may not immediately copy data into buffer memory, writes to buffer may not be visible in mapped memory yet...
    // either use a heap that is host coherent (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT in memory requirements)
    // or call vkFlushMappedMemoryRanges after writing to mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory
    
    // create the vertex buffer, now the memory is device local (faster)
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    // VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    // cleanup after using the staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void DuckApplication::createIndexBuffer() {
    // almost identical to the vertex buffer creation process except where commented
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size(); 

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // different usage bit flag VK_BUFFER_USAGE_INDEX_BUFFER_BIT instead of VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

uint32_t DuckApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // GPUs allocate dufferent types of memory, varying in terms of allowed operations and performance. Combine buffer and application
    // requirements to find best type of memory
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
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
// Device setup
//

void DuckApplication::createLogicalDevice() {
    // query the queue families available on the device
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    // create a vector containing VkDeviceQueueCreqteInfo structs
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // using a set makes sure that there are no dulpicate references to a same queue!
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    // queue priority, for now give quueues the same priority
    float queuePriority = 1.0f;
    // loop over the queue families in the set
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        // specify which queue we want to create, initialise struct at 0
        VkDeviceQueueCreateInfo queueCreateInfo{};
        // information in the struct
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        // the family index
        queueCreateInfo.queueFamilyIndex = queueFamily;
        // number of queues
        queueCreateInfo.queueCount = 1;
        // between 0 and 1, influences sheduling of queue commands 
        queueCreateInfo.pQueuePriorities = &queuePriority;
        // push the info on the vector
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // queries support certain features (like geometry shaders, other things in the vulkan pipeline...)
    VkPhysicalDeviceFeatures deviceFeatures{};

    // the struct containing the device info
    VkDeviceCreateInfo createInfo{};
    // inform on type of struct
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // the number of queues
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    // pointer to queue(s) info, here the raw underlying array in a vector (guaranteed contiguous!)
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    // desired device features
    createInfo.pEnabledFeatures = &deviceFeatures;
    // setting validation layers and extensions is per device
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); // the number of desired extensions
    createInfo.ppEnabledExtensionNames = deviceExtensions.data(); // pointer to the vector containing the desired extensions 

    // older implementation compatibility, no disitinction instance and device specific validations
    if (enableValidationLayers) {
        // these fields are ignored by newer vulkan implementations
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    // instantiate a logical device from the create info we've determined
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    // set the graphics queue handle, only want a single queue so use index 0
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    // set the presentation queue handle like above
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void DuckApplication::pickPhysicalDevice() {
    // similar to extensions, gets the physical devices available
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    // handles no devices
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    // get a vector of physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    // store all physical devices in the vector
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // iterate over available physical devices and check that they are suitable
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    // if the physicalDevice handle is still null, then no suitable devices were found
    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool DuckApplication::isDeviceSuitable(VkPhysicalDevice device) {
    // if we wanted to look at the device properties and features in more detail:
    // VkPhysicalDeviceProperties deviceProperties;
    // vkGetPhysicalDeviceProperties(device, &deviceProperties);
    // VkPhysicalDeviceFeatures deviceFeatures;
    // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    // we could determine which device is the most suitable by giving each a score
    // based on the contents of the structs

    // get the queues
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);
    // NB the availability of a presentation queue implies that swap chain extension is supported, but best to be explicit about this

    bool swapChainAdequate = false;
    if (extensionsSupported) { // if extension supported, in our case extension for the swap chain
        // find out more about the swap chain details
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        // at least one supported image format and presentation mode is sufficient for now
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    // return the queue family index (true if a value was initialised), device supports extension and swap chain is adequate (phew)
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool DuckApplication::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    // intit the extension count
    uint32_t extensionCount;
    // set the extension count using the right vulkan enumerate function 
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    // a vector container for the properties of the available extensions
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    // same function used to get extension count, but add the pointer to vector data to store the properties struct
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // wrap the const vector of extensions deviceExtensions defined at top of header file into a set, to get unique extensions names
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // loop through available extensions, erasing any occurence of the required extension(s)
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    // if the required extensions vector is empty, then they were erased because they are available, so return true with empty() 
    return requiredExtensions.empty();
}

QueueFamilyIndices DuckApplication::findQueueFamilies(VkPhysicalDevice device) {
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

//
// Swap chain and surface setup
//

SwapChainSupportDetails DuckApplication::querySwapChainSupport(VkPhysicalDevice device) {
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
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

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
    createInfo.surface = surface; // specify the surface the swap chain should be tied to
    // set details
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // amount of layers each image consists of
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // bit field, specifies types of operations we'll use images in swap chain for

    // how to handle the swap chain images across multiple queue families (in case graphics queue is different to presentation queue)
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    // get size of swap chain images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    // resize accordingly
    swapChainImages.resize(imageCount);
    // pull the images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

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
    vkDeviceWaitIdle(device);

    // destroy the previous swap chain
    cleanupSwapChain();

    // all creation functions for objects that depend on swap chain / window size
    createSwapChain(); // recreate the swap chain itself
    createImageViews(); // because views are based on swap chain images, they need to be recreated as well
    createRenderPass(); // render pass needs to be recreated because it depends on format of swap chain images
    // rare for swapchain image format to change but needs handling, viewport and scissors specified in pipeline 
    // creation so needs to be recreated. NB we can avoid this using dynamic states
    createGraphicsPipeline();
    createFrameBuffers(); // directly depend on swap chain images so recreate
    createCommandBuffers(); // directly depend on swap chain images so recreate
}

void DuckApplication::cleanupSwapChain() {
    // destroy the frame buffers
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    // destroy the command buffers. This lets us preserve the command pool rather than wastefully creating and deestroying repeatedly
    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    // destroy pipeline and related data
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    // loop over the image views and destroy them. NB we don't destroy the images because they are implicilty created
    // and destroyed by the swap chain
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    // destroy the swap chain proper
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void DuckApplication::createSurface() {
    // takes simple arguments instead of structs
    // object is platform agnostic but creation is not, this is handled by the glfw method
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void DuckApplication::createImageViews() {
    // resize the vector of views to accomodate the images
    swapChainImageViews.resize(swapChainImages.size());

    // loop to iterate through images
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        // a view struct for each image
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i]; // the image this is a view of
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // image as 1/2/3D textures and cube maps
        createInfo.format = swapChainImageFormat; // how the image data should be interpreted
        // lets us swizzle colour channels around (here there is no swizzle)
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // describe what the image purpose is and what part of the image 
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // create the view
        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
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
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
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
        drawFrame();
    }
    vkDeviceWaitIdle(device);
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
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // retrieve an image from the swap chain
    uint32_t imageIndex;
    // swap chain is an extension so use the vk*KHR function
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); // params:
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
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

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
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    // which semaphores to signal once the command buffer(s) has finished, we are using the renderFinishedSemaphore for that
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // reset the fence to block next frame just before using the fence
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // submit the command buffer to the graphics queue, takes an array of submitinfo when work load is much larger
    // last param is a fence, should be signaled when the cmd buffer finished executing so use to signal frame has finished
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
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
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    // similar to when acquiring the swap chain image, check that the presentation queue can accept the image, also check for resizing
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // after a frame is drawn, increment current frame count (% loops around)
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
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

    // destroy the index buffer and free its memory
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    // destroy the vertex buffer and free its memory
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    // loop over each frame and destroy its semaphores 
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);
    // remove the logical device, no direct interaction with instance so not passed as argument
    vkDestroyDevice(device, nullptr);

    // if debug activated, remove the messenger
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    // destroy the window surface
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // only called before program exits, destroys the vulkan instance
    vkDestroyInstance(instance, nullptr);

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

//
// Debug
//

void DuckApplication::setupDebugMessenger() {
    // do nothing if we are not in debug mode
    if (!enableValidationLayers) return;

    // create messenger info
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    // set create info's parameters
    populateDebugMessengerCreateInfo(createInfo);

    // create the debug messenger
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

// proxy function handles finding the extension function vkCreateDebugUtilsMessengerEXT 
VkResult DuckApplication::CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    // vkGetInstanceProcAddr returns nullptr if the function couldn't be loaded, otherwise a pointer to the function
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        // return the result of the function
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// similar to above function but for destroying a debug messenger
void DuckApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL DuckApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, // 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) { // some user data
    // message severity flags, values can be used to check how message compares to a certain level of severity
    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT : Informational message like the creation of a resource
    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : Message about behavior that is not necessarily an error, but very likely a bug in your application
    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT : Message about behavior that is invalid and may cause crashes
    // message type flags
    // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
    // VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : Something has happened that violates the specification or indicates a possible mistake
    // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : Potential non - optimal use of Vulkan
    // refers to a struct with the details of the message itself
    // pMessage : The debug message as a null - terminated string
    // pObjects : Array of Vulkan object handles related to the message
    // objectCount : Number of objects in array
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void DuckApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    // the creation of a messenger create info is put in a separate function for use to debug the creation and destruction of 
    // a VkInstance object
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // types of callbacks to be called for
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    if (enableVerboseValidation) {
        createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    }
    // filter which message type filtered by callback
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    // pointer to call back function
    createInfo.pfnUserCallback = debugCallback;
}

bool DuckApplication::checkValidationLayerSupport() {
    uint32_t layerCount;
    // get the layer count
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    // create a vector of layers of size layercount
    std::vector<VkLayerProperties> availableLayers(layerCount);
    // get all available layers and store in the vector
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // iterate over the desired validation layers
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        // iterate over the available layers (We could have used a set as is the case for device extensions check)
        for (const auto& layerProperties : availableLayers) {
            // check that the validation layers exist in available layers
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        // if not found, then we can't use that layer
        if (!layerFound) {
            return false;
        }
    }

    return true;
}
