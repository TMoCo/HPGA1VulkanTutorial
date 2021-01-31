// include class definition
#include "..\headers\HelloTriangleApplication.h"

// reporting and propagating exceptions
#include <iostream> 
#include <stdexcept>

//
// Run application
//

void HelloTriangleApplication::run() {
    // initialise a glfw window
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

//
// Initialisation
//

void HelloTriangleApplication::initWindow() {
    // initialise glfw library
    glfwInit();

    // set parameters
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // initially for opengl, so tell it not to create opengl context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable resizing for now

    // create the window, 4th param refs a monitor, 5th param is opengl related
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan() {
    // create the instance
    createInstance();
    setupDebugMessenger();
}

void HelloTriangleApplication::createInstance() {
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


    // include valdation layers if enables
    if (enableValidationLayers) {
        // save layer count, cast size_t to uin32_t
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    // we can now create the instance (pointer to struct, pointer to custom allocator callbacks, 
    // pointer to handle that stores the new object)
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) { // check everything went well by comparing returned value
        throw std::runtime_error("failed to create a vulkan instance!");
    }

    // create a 
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    /*
    // check for optional functionality, so far we only queried for essential window functionality
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr); // get just the extension count

    // create a vector of VkExtensionProperties of size extensionCount
    std::vector<VkExtensionProperties> extensions(extensionCount);

    // query extension details and store in the vector by passing pointer to vector's internal array (data())
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // print extensions
    std::cout << "available extensions:\n";
    for (const auto& extension : extensions) { // for each extension struct in extensions vector
        std::cout << '\t' << extension.extensionName << '\n'; // struct stores extension name, print it
    }
    */
}

void HelloTriangleApplication::pickPhysicalDevice() {
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

void HelloTriangleApplication::createLogicalDevice() {
    // query the queue families available on the device
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    // specify which queue we want to create, initialise struct at 0
    VkDeviceQueueCreateInfo queueCreateInfo{};
    // information in the struct
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    // the family index
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    // number of queues
    queueCreateInfo.queueCount = 1;

    // between 0 and 1, influences sheduling of queue commands 
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // queries support certain features (like geometry shaders, other things in the vulkan pipeline...)
    VkPhysicalDeviceFeatures deviceFeatures{};

    // the struct containing the device info
    VkDeviceCreateInfo createInfo{};
    // inform on type of struct
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    // pointer to queue info
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    // the number of queues
    createInfo.queueCreateInfoCount = 1;

    // desired device features
    createInfo.pEnabledFeatures = &deviceFeatures;
    // setting validation layers and extensions is per device
    createInfo.enabledExtensionCount = 0;

    // older implementation compatibility, no disitinction instance and device specific validations
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device) {
    // if we wanted to look at the device properties and features in more detail:
    // VkPhysicalDeviceProperties deviceProperties;
    // vkGetPhysicalDeviceProperties(device, &deviceProperties);
    // VkPhysicalDeviceFeatures deviceFeatures;
    // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    // we could determine which device is the most suitable by giving each a score
    // based on the contents of the structs

    // get the queues
    QueueFamilyIndices indices = findQueueFamilies(device);

    // for now just return true
    return indices.isComplete();
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    // similar to physical device and extensions and layers....
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // iterate over queue family properties vector
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        // if the queue supports the desired queue operation, then the bitwise & operator returns true
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            // gaphics family was assigned a value! optional wrapper has_value now returns true.
            indices.graphicsFamily = i;
        }
        // increment i
        i++;
    }

    return indices;
}

//
// Main loop
//

void HelloTriangleApplication::mainLoop() {
    // loop keeps window open
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

//
// Cleanup
//

void HelloTriangleApplication::cleanup() {
    // if debug activated, remove the messenger
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
           
    // only called before program exits, destroys the vulkan instance
    vkDestroyInstance(instance, nullptr);

    // destory the window
    glfwDestroyWindow(window);

    // terminate glfw
    glfwTerminate();
}

//
// Debug
//

void HelloTriangleApplication::setupDebugMessenger() {
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
VkResult HelloTriangleApplication::CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    // vkGetInstanceProcAddr returns nullptr if the function couldn't be loaded
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        // return the pointer to the function
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// similar to above function but for destroying a debug messenger
void HelloTriangleApplication::DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
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
    // pMessage: The debug message as a null - terminated string
    // pObjects : Array of Vulkan object handles related to the message
    // objectCount : Number of objects in array
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    // the creation of a messenger create info is put in a separate function for use to debug the creation and destruction of 
    // a VkInstance object
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // types of callbacks to be called for
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    // filter which message type filtered by callback
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    // pointer to call back function
    createInfo.pfnUserCallback = debugCallback;
}

bool HelloTriangleApplication::checkValidationLayerSupport() {
    uint32_t layerCount;
    // get the layer count
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    // create a vector of layers of size layercount
    std::vector<VkLayerProperties> availableLayers(layerCount);
    // get all available layers and store in the vector
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // iterate over the validation layers 
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        // iterate over the available layers
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

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions() {
    // start by getting the glfw extensions, nescessary for displaying something in a window.
    // platform agnostic, so need an extension to interface with window system. Use GLFW to return
    // the extensions needed for platform and passed to createInfo struct
    uint32_t glfwExtensionCount = 0; // initialise extension count to 0, changed later
    const char** glfwExtensions; // array of strings with extension names
    // get extension count
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    // a vector containing the values from glfwExtensions. 
    // glfwExtensions is an array of strings, we give the vector constructor a range of values from glfwExtensions to 
    // copy (first value at glfwExtensions, a pointer, to last value, pointer to first + nb of extensions)
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    // add the VK_EXT_debug_utils with macro on condition debug is activated
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    // return the vector
    return extensions;
}