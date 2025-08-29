#include "renderer.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#include "NativeParent_vk.h"
#include <stdexcept>
#include <array>
#include <cstdio>
#include <vector>


struct Impl {
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice phys = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkExtent2D swapchainExtent;
    VkCommandPool commandPool; // Create command pool first
    VkCommandBuffer commandBuffer;

    VkPipeline graphicsPipeline;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    void createRenderPass();
    void createFramebuffer(int w, int h);
    void createSwapchain(int w, int h);
    void createCmdBuffer();
    void createGraphicsPipeline();
    void createVertexBuffer();
};

struct Vertex { float pos[2]; float uv[2]; };

static const Vertex quadVerts[6] = {
    {{-0.5f,-0.5f},{0,1}},
    {{ 0.5f,-0.5f},{1,1}},
    {{ 0.5f, 0.5f},{1,0}},
    {{-0.5f,-0.5f},{0,1}},
    {{ 0.5f, 0.5f},{1,0}},
    {{-0.5f, 0.5f},{0,0}},
};


const char* getVulkanResultString(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        // Add more Vulkan error codes as needed
        default: return "UNRECOGNIZED VULKAN ERROR CODE";
    }
}

#define VK_CHECK(msg, result) { \
    if (result != VK_SUCCESS) { \
        throw std::runtime_error("Vulkan error: " + std::string( getVulkanResultString(result) ) + " in " + std::string( msg )); \
    } \
}

// Function to map format enum to a string (if needed) for logging purposes
const char* getFormatName(VkFormat format) {
  switch(format) {
    case VK_FORMAT_B8G8R8A8_UNORM: return "VK_FORMAT_B8G8R8A8_UNORM";
    case VK_FORMAT_R8G8B8A8_UNORM: return "VK_FORMAT_R8G8B8A8_UNORM";
    // Add mappings for other formats as needed
    default: return "UNKNOWN_FORMAT";
  }
}

#include <vulkan/vulkan.h>

const char* getColorspaceName(VkColorSpaceKHR colorSpace) {
    switch (colorSpace) {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR or VK_COLORSPACE_SRGB_NONLINEAR_KHR";
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT: return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT or VK_COLOR_SPACE_DCI_P3_LINEAR_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT: return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT";
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT: return "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT";
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT: return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT709_LINEAR_EXT: return "VK_COLOR_SPACE_BT709_LINEAR_EXT";
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT: return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT: return "VK_COLOR_SPACE_BT2020_LINEAR_EXT";
        case VK_COLOR_SPACE_HDR10_ST2084_EXT: return "VK_COLOR_SPACE_HDR10_ST2084_EXT";
        case VK_COLOR_SPACE_DOLBYVISION_EXT: return "VK_COLOR_SPACE_DOLBYVISION_EXT";
        case VK_COLOR_SPACE_HDR10_HLG_EXT: return "VK_COLOR_SPACE_HDR10_HLG_EXT";
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT: return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT";
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT: return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT";
        case VK_COLOR_SPACE_PASS_THROUGH_EXT: return "VK_COLOR_SPACE_PASS_THROUGH_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT: return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT";
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: return "VK_COLOR_SPACE_DISPLAY_NATIVE_AMD";
        // case VK_COLORSPACE_SRGB_NONLINEAR_KHR: return "VK_COLORSPACE_SRGB_NONLINEAR_KHR";
        // case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT: return "VK_COLOR_SPACE_DCI_P3_LINEAR_EXT";
        case VK_COLOR_SPACE_MAX_ENUM_KHR: return "VK_COLOR_SPACE_MAX_ENUM_KHR";

        // Add additional color spaces as needed
        default: return "UNKNOWN_COLOR_SPACE";
    }
}

static void logAvailableExtensions() {
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> props(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, props.data());

    printf("=== Available Vulkan instance extensions: ===\n");
    for (auto &p : props)
        printf("  %s\n", p.extensionName);
}

static void logAvailableLayers() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  printf("=== Available Vulkan layers: ===\n");
  for (const auto& layer : availableLayers) {
      printf( "  %s: %s\n", layer.layerName, layer.description );
  }
}

static void logAvailableSurfaceFormats(VkPhysicalDevice phys, VkSurfaceKHR surface) {
    uint32_t formatCount = 0;

    // Get the number of surface formats
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &formatCount, nullptr);
    if (result != VK_SUCCESS) {
        printf("Failed to get surface format count\n");
        return; // Early exit on failure
    }

    // Create a vector to hold available formats
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &formatCount, surfaceFormats.data());
    if (result != VK_SUCCESS) {
        printf("Failed to get surface formats\n");
        return; // Early exit on failure
    }

    printf("=== Available Vulkan Surface Formats: ===\n");

    // Log each available surface format
    for (const auto& fmt : surfaceFormats) {
        printf("  Format: %d, Color Space: %d\n", fmt.format, fmt.colorSpace);
    }
}

Renderer::Renderer() : impl( new Impl() ) {}

Renderer::Renderer(NativeParent& np, int w, int h) : Renderer() {
    this->init( np, w, h );
}

void Renderer::init(NativeParent& np, int w, int h) {
  try {
    printf("=== Vulkan Renderer Init ===\n");

    // 1. Log extensions before creating instance
    logAvailableExtensions();
    logAvailableLayers();

    // 2. Instance
    VkApplicationInfo ai{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    ai.pApplicationName = "gfxkit";
    ai.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo = &ai;
    const char* exts[] = { 
      VK_KHR_SURFACE_EXTENSION_NAME,      // "VK_KHR_surface"
      VK_MVK_MACOS_SURFACE_EXTENSION_NAME,// "VK_MVK_macos_surface"
      VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME, // "VK_KHR_portability_enumeration"
    };
    ici.ppEnabledExtensionNames = exts;
    ici.enabledExtensionCount = 3;
    ici.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    const bool enableValidationLayers = true;
    const char* validationLayers[] = {};
    if (enableValidationLayers) {
        uint32_t availableLayerCount;
        vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(availableLayerCount);
        vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

        bool layerFound = false;
        for (const char* layerName : validationLayers) {
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                throw std::runtime_error("Requested validation layer not available: " + std::string(layerName));
            }
        }

        ici.enabledLayerCount = static_cast<uint32_t>(sizeof(validationLayers) / sizeof(validationLayers[0]));
        ici.ppEnabledLayerNames = validationLayers;
    } else {
        ici.enabledLayerCount = 0;
    }

    VkResult res = vkCreateInstance(&ici, nullptr, &impl->instance);
    if (res != VK_SUCCESS) {
        printf("vkCreateInstance failed with VkResult = %d = %s\n", res, getVulkanResultString(res));
        VK_CHECK( "vkCreateInstance", res );
    }
    printf("Vulkan instance created successfully\n");

    // 3. Surface
    try {
        impl->surface = makeSurface(impl->instance, np);
        printf("MoltenVK surface created successfully\n");
    } catch (const std::runtime_error& e) {
        printf("makeSurface failed: %s\n", e.what());
        throw;
    }

    // 4. Physical device
    uint32_t count = 0;
    res = vkEnumeratePhysicalDevices(impl->instance, &count, nullptr);
    if (res != VK_SUCCESS || count == 0) {
        printf("No Vulkan physical devices found (VkResult=%d, count=%d)\n", res, count);
        throw std::runtime_error("Failed to find physical device");
    }

    std::vector<VkPhysicalDevice> gpus(count);
    res = vkEnumeratePhysicalDevices(impl->instance, &count, gpus.data());
    if (res != VK_SUCCESS) {
        printf("vkEnumeratePhysicalDevices failed with VkResult=%d\n", res);
        throw std::runtime_error("Failed to enumerate physical devices");
    }

    impl->phys = gpus[0];
    printf("Using first physical device\n");

    // 5. Logical device + queue
    float qprio = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = 0; qci.queueCount = 1; qci.pQueuePriorities = &qprio;

    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1; dci.pQueueCreateInfos = &qci;
    const char* dc_exts[] = { 
      "VK_KHR_swapchain"
    };
    dci.ppEnabledExtensionNames = dc_exts;
    dci.enabledExtensionCount = 1;

    res = vkCreateDevice(impl->phys, &dci, nullptr, &impl->device);
    if (res != VK_SUCCESS) {
        printf("vkCreateDevice failed with VkResult=%d\n", res);
        throw std::runtime_error("vkCreateDevice failed");
    }
    vkGetDeviceQueue(impl->device, 0, 0, &impl->queue);
    printf("Logical device and queue created successfully\n");

    // Create Swapchain
    impl->createSwapchain(w, h);

    // Create Render Pass
    impl->createRenderPass();
    
    // Create Framebuffer
    impl->createFramebuffer(w, h);

    impl->createCmdBuffer();

    impl->createVertexBuffer();

    if (impl->renderPass == VK_NULL_HANDLE) {
        printf("Error: Render pass is not valid.\n");
        throw std::runtime_error("Invalid render pass");
    }

    if (impl->framebuffer == VK_NULL_HANDLE) {
        printf("Error: Framebuffer is not valid.\n");
        throw std::runtime_error("Invalid framebuffer");
    }

  } catch (const std::runtime_error& e) {
    printf( "Runtime error: %s\n", e.what() );
  } catch (...) {
    printf( "An unknown error occurred.\n" );
  }
}


void Impl::createSwapchain(int w, int h) {
    logAvailableSurfaceFormats( phys, surface);


    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, surface, &capabilities);
    if(capabilities.minImageCount > capabilities.maxImageCount) {
        capabilities.minImageCount = capabilities.maxImageCount; // Adjust if needed
        printf( "BAD?   capabilities.minImageCount > capabilities.maxImageCount (%d > %d)\n", capabilities.minImageCount, capabilities.maxImageCount );
    }


    // select a surface format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(phys, surface, &formatCount, surfaceFormats.data());
    VkSurfaceFormatKHR surfaceFormat = {};
    // Check available formats
    for (const auto& format : surfaceFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = format; // Found preferred format with sRGB color space
            break;
        }
    }
    // If preferred format is not found, choose the first one available
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED) {
        surfaceFormat = surfaceFormats[0]; // Fallback to the first available format
        printf("Selected fallback surface format: %d = %s\n", surfaceFormat.format, getFormatName(surfaceFormat.format));
        printf("Selected fallback surface ColorSpace: %d = %s\n", surfaceFormat.colorSpace, getColorspaceName(surfaceFormat.colorSpace));
    } else {
        printf("Selected surface format: %d = %s\n", surfaceFormat.format, getFormatName(surfaceFormat.format));
        printf("Selected surface ColorSpace: %d = %s\n", surfaceFormat.colorSpace, getColorspaceName(surfaceFormat.colorSpace));
    }

    // 6. Swapchain
    VkSwapchainCreateInfoKHR sci = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = surface;
    if (surface == VK_NULL_HANDLE) {
        printf("Error: Vulkan surface is not valid.\n");
        throw std::runtime_error("Invalid surface");
    }
    sci.minImageCount = capabilities.minImageCount > 1 ? capabilities.minImageCount : 2;
    sci.imageFormat = surfaceFormat.format;//VK_FORMAT_B8G8R8A8_UNORM;
    sci.imageColorSpace = surfaceFormat.colorSpace;//VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    if (w == 0 || h == 0) {
      throw std::runtime_error( "width or height is 0" );
    }
    if (capabilities.currentExtent.width == UINT32_MAX) {
        // Swapchain extent can be defined by the user
        sci.imageExtent = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
        printf( "Swapchain extent can be defined by the user\n" );
    } else {
        // If currentExtent is defined, use it
        sci.imageExtent = capabilities.currentExtent;
        printf( "If currentExtent is defined, use it\n" );
    }
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = capabilities.currentTransform; // VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;

    // Log swapchain parameters
    printf("Creating Swapchain with the following parameters:\n");
    printf(" Surface: %p\n", surface); // Log the surface pointer
    printf(" Min Image Count: %d\n", sci.minImageCount);
    printf(" Image Format: %d\n", sci.imageFormat);
    printf(" Color Space: %d\n", sci.imageColorSpace);
    printf(" Extent: [%d, %d]\n", sci.imageExtent.width, sci.imageExtent.height);


    if (capabilities.maxImageCount < 2) {
        printf("Error: Max image count for swapchain is less than 2\n");
        throw std::runtime_error("Swapchain cannot be created");
    }

    printf( "...about to call vkCreateSwapchainKHR...\n");
    VkResult res = vkCreateSwapchainKHR(device, &sci, nullptr, &swapchain);
    printf( "...after call to vkCreateSwapchainKHR...\n");
    if (res != VK_SUCCESS) {
        printf("vkCreateSwapchainKHR failed with VkResult = %d = %s\n", res, getVulkanResultString( res ) );
        VK_CHECK( "vkCreateSwapchainKHR", res )
    }
    printf("Swapchain created successfully\n");
}

void Impl::createRenderPass() {
    // Define a render pass (dummy example; modify as needed)
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM; // The same format as the swapchain
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkResult res = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    if (res != VK_SUCCESS) {
        printf("Failed to create render pass: %s\n", getVulkanResultString(res));
        throw std::runtime_error("Failed to create render pass");
    }
    printf("Render pass created successfully\n");
}

void Impl::createFramebuffer(int w, int h) {
    // Create framebuffer for the swapchain image
    VkImageView imageView; // You should create an image view based on swapchain images first
    // Ensure you correctly create and manage VkImageView resources, here’s a basic example to create a framebuffer
    // This is a simplified version; ensure you have an actual image view for your swapchain image

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass; // Use the previously created render pass
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &imageView; // Use the image view here (it must be valid)
    framebufferInfo.width = w; // Width of the swapchain image
    framebufferInfo.height = h; // Height of the swapchain image
    framebufferInfo.layers = 1;

    VkResult res = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
    if (res != VK_SUCCESS) {
        printf("Failed to create framebuffer: %s\n", getVulkanResultString(res));
        throw std::runtime_error("Failed to create framebuffer");
    }
    printf("Framebuffer created successfully\n");
}

void Impl::createCmdBuffer() {
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = 0; // Ensure this is the correct family index
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create command pool!");
  }

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
  }
}



// Define the vertices for a 100x100 quad centered at the origin
// struct Vertex {
//     float pos[2]; // Position
//     float uv[2];  // Texture coordinates
// };

static const Vertex quadVertices[] = {
    {{-50.0f, -50.0f}, {0.0f, 1.0f}}, // Bottom left
    {{ 50.0f, -50.0f}, {1.0f, 1.0f}}, // Bottom right
    {{ 50.0f,  50.0f}, {1.0f, 0.0f}}, // Top right
    {{-50.0f, -50.0f}, {0.0f, 1.0f}}, // Bottom left
    {{ 50.0f,  50.0f}, {1.0f, 0.0f}}, // Top right
    {{-50.0f,  50.0f}, {0.0f, 0.0f}}  // Top left
};


// Helper function to create a single-time command buffer
// Helper function to create a single-time command buffer
VkCommandBuffer vkBeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    if (result != VK_SUCCESS) {
        printf("Failed to allocate command buffer: %s\n", getVulkanResultString(result));
        throw std::runtime_error("Failed to allocate command buffer");
    }

    // Begin recording commands
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        printf("Failed to begin command buffer: %s\n", getVulkanResultString(result));
        throw std::runtime_error("Failed to begin command buffer");
    }

    return commandBuffer;
}

// End command buffer and submit
// End command buffer and submit
void vkEndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue) {
    // End recording of the command buffer
    VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS) {
        printf("Failed to end command buffer: %s\n", getVulkanResultString(result));
        throw std::runtime_error("Failed to end command buffer");
    }

    // Prepare to submit the command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        printf("Failed to submit draw command buffer: %s\n", getVulkanResultString(result));
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Wait for the queue to be idle
    vkQueueWaitIdle(queue);

    // Clean up the command buffer
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void vkCopyBuffer(VkDevice device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkQueue queue, VkCommandPool commandPool) {
    VkCommandBuffer commandBuffer = vkBeginSingleTimeCommands(device, commandPool); // Create a command buffer for copying

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = sizeof(quadVerts); // Size of the vertex data to copy
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndSingleTimeCommands(device, commandPool, commandBuffer, queue); // Submit the command buffer
}


VkShaderModule createShaderModule(VkDevice device, const std::string& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.length();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.c_str());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, Impl* impl) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(impl->phys, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i; // Return the first suitable memory type
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void Impl::createVertexBuffer() {
    printf("VertexBuf createVertexBuffer\n");

    VkDeviceSize bufferSize = sizeof(quadVerts);

    // Create a staging buffer for the vertex data
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // Create the staging buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK("vkCreateBuffer (staging)", vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this); // Host-visible

    VK_CHECK("vkAllocateMemory (staging)", vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory));
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    // Copy the vertex data to the staging buffer
    void* data;
    VK_CHECK("vkMapMemory", vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data));
    memcpy(data, quadVerts, (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    // Create the vertex buffer
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VK_CHECK("vkCreateBuffer (vertex)", vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer));

    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this); // Use device-local memory

    VK_CHECK("vkAllocateMemory (vertex)", vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory));
    vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    // Copy the data from the staging buffer to the vertex buffer
    vkCopyBuffer(device, stagingBuffer, vertexBuffer, queue, commandPool);

    // Cleanup the staging buffer and memory
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    printf("VertexBuf created successfully\n");
}

void Impl::createGraphicsPipeline() {
  const std::string vertex_shader = "#version 450\
\
layout(location = 0) in vec2 inPosition;\
layout(location = 1) in vec2 inUV;\
\
layout(location = 0) out vec2 fragUV;\
\
void main() {\
    fragUV = inUV;\
    gl_Position = vec4(inPosition, 0.0, 1.0);\
}\
";

  const std::string fragment_shader = "#version 450\
\
layout(location = 0) in vec2 fragUV;\
layout(location = 0) out vec4 outColor;\
\
void main() {\
    outColor = vec4(1.0, 0.0, 0.0, 1.0); // Set color to red\
}\
";

    VkShaderModule vertShaderModule = createShaderModule(device, vertex_shader);
    VkShaderModule fragShaderModule = createShaderModule(device, fragment_shader);

    VkPipelineShaderStageCreateInfo vertStageInfo = {};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertShaderModule;
    vertStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragStageInfo = {};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragShaderModule;
    fragStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };



    /////////////////////////////////////////////////////////////////
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[2]; // We have 2 attributes
    attributeDescriptions[0].binding = 0; // Binding = 0
    attributeDescriptions[0].location = 0; // Location = 0
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // Position format
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0; // Binding = 0
    attributeDescriptions[1].location = 1; // Location = 1
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT; // UV format
    attributeDescriptions[1].offset = offsetof(Vertex, uv);
    /////////////////////////////////////////////////////////////////////


    // Set up vertex input info
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and scissor state
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // or none if needed
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Set for single sampling

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Pipeline layout setup
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // No descriptor sets for this example

    VkResult res = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (res != VK_SUCCESS) {
        printf("Failed to create pipeline layout: %s\n", getVulkanResultString(res));
        throw std::runtime_error("Failed to create pipeline layout");
    }

    // Graphics pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass; // Use the render pass created earlier
    pipelineInfo.subpass = 0; // Use the first subpass

    res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
    if (res != VK_SUCCESS) {
        printf("Failed to create graphics pipeline: %s\n", getVulkanResultString(res));
        throw std::runtime_error("Failed to create graphics pipeline");
    }

    printf("Graphics pipeline created successfully\n");

    // Cleanup shader modules after pipeline creation
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

Renderer::~Renderer() {
    // Clean up resources
    if (impl->framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(impl->device, impl->framebuffer, nullptr);
    }
    if (impl->renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(impl->device, impl->renderPass, nullptr);
    }
    if (impl->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(impl->device, impl->swapchain, nullptr);
    }
    if (impl->device != VK_NULL_HANDLE) {
        vkDestroyDevice(impl->device, nullptr);
    }
    if (impl->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(impl->instance, impl->surface, nullptr);
    }
    if (impl->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(impl->instance, nullptr);
    }
    // if (impl->commandBuffer != VK_NULL_HANDLE) {
    //     vkDestroyCommandBuffer(impl->commandBuffer, nullptr);
    // }
    // if (impl->commandPool != VK_NULL_HANDLE) {
    //     vkDestroyCommandPool(impl->commandPool, nullptr);
    // }
    impl = nullptr;
}



// Ensure you have a valid VkRenderPass created earlier in your setup
void Renderer::drawFrame() {
    // Step 1: Acquire the next image from the swapchain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(impl->device, impl->swapchain, UINT64_MAX,
                                             VK_NULL_HANDLE, VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        printf("Swapchain out of date. Need to recreate swapchain.\n");
        return; 
    } else if (result != VK_SUCCESS) {
        printf("Failed to acquire swapchain image: %d\n", result);
        return; 
    }

    // Step 2: Begin recording commands into the command buffer
    // Start command buffer recording
    result = vkBeginCommandBuffer(impl->commandBuffer, nullptr);
    if (result != VK_SUCCESS) {
        printf("Failed to begin recording command buffer.\n");
        VK_CHECK( "vkBeginCommandBuffer", result )
        return;
    }

    // Step 3: Set the viewport and scissor for the render pass
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(impl->swapchainExtent.width); // Use the swapchain extent
    viewport.height = static_cast<float>(impl->swapchainExtent.height); // Use the swapchain extent
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(impl->commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = impl->swapchainExtent; // Match the swapchain image size
    vkCmdSetScissor(impl->commandBuffer, 0, 1, &scissor);

    // Step 4: Set up render pass info
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = impl->renderPass; // Ensure this is initialized
    renderPassBeginInfo.framebuffer = impl->framebuffer; // You need to have a valid framebuffer
    renderPassBeginInfo.renderArea = { { 0, 0 }, impl->swapchainExtent }; // Use swapchain extent

    // Clear values for the render pass
    VkClearValue clearColor = {};
    clearColor.color.float32[0] = 0.0f; // Red clear color
    clearColor.color.float32[1] = 0.0f; // Green clear color
    clearColor.color.float32[2] = 0.0f; // Blue clear color
    clearColor.color.float32[3] = 1.0f; // Alpha clear color

    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    // Step 5: Begin the render pass
    vkCmdBeginRenderPass(impl->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Step 6: Bind your graphics pipeline
    vkCmdBindPipeline(impl->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, impl->graphicsPipeline); // Ensure this is initialized
    
    // Step 7: Bind vertex buffer
    VkDeviceSize offsets[] = { 0 }; // Offset for the first vertex buffer
    vkCmdBindVertexBuffers(impl->commandBuffer, 0, 1, &impl->vertexBuffer, offsets);

    // Step 8: Draw your vertices
    vkCmdDraw(impl->commandBuffer, 6, 1, 0, 0); // Example for drawing the quad

    // Step 9: End the render pass
    vkCmdEndRenderPass(impl->commandBuffer);

    // Step 10: Stop recording
    if (vkEndCommandBuffer(impl->commandBuffer) != VK_SUCCESS) {
        printf("Failed to record command buffer.\n");
        return;
    }

    // Step 11: Submit the command buffer to the queue
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &impl->commandBuffer;

    result = vkQueueSubmit(impl->queue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        printf("Failed to submit draw command buffer: %d\n", result);
        return;
    }

    // Step 12: Present the frame
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &impl->swapchain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(impl->queue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        printf("Swapchain out of date during presentation. Need to recreate.\n");
    } else if (result != VK_SUCCESS) {
        printf("Failed to present swapchain image: %d\n", result);
    }
}
