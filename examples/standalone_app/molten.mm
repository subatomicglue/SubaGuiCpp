// hello_moltenvk.cpp
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#import <Cocoa/Cocoa.h>
#include <iostream>
#include <stdexcept>
#include <vector>

struct AppWindow {
    NSWindow* window;
    NSView* contentView;
};

AppWindow createCocoaWindow(int width, int height, const char* title) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
        NSRect rect = NSMakeRect(0, 0, width, height);
        NSWindow* window = [[NSWindow alloc] initWithContentRect:rect
                                                       styleMask:style
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        [window setTitle:[NSString stringWithUTF8String:title]];
        [window makeKeyAndOrderFront:nil];
        return { window, window.contentView };
    }
}

int main() {
    AppWindow win = createCocoaWindow(640, 480, "MoltenVK Hello Triangle");

    // 1. Vulkan instance
    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "HelloMoltenVK";
    appInfo.apiVersion = VK_API_VERSION_1_1;

    const char* exts[] = { VK_KHR_SURFACE_EXTENSION_NAME,
                            VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
                            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME };

    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ici.pApplicationInfo = &appInfo;
    ici.enabledExtensionCount = 3;
    ici.ppEnabledExtensionNames = exts;
    ici.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkInstance instance;
    if (vkCreateInstance(&ici, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");

    // 2. MoltenVK surface
    VkMacOSSurfaceCreateInfoMVK sci{VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK};
    sci.pView = (__bridge void*)win.contentView;
    VkSurfaceKHR surface;
    if (vkCreateMacOSSurfaceMVK(instance, &sci, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create MoltenVK surface");

    // 3. Pick physical device
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    if (gpuCount == 0) throw std::runtime_error("No Vulkan devices found");
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());
    VkPhysicalDevice phys = gpus[0];

    // 4. Find queue family that supports graphics + present
    uint32_t qCount;
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &qCount, nullptr);
    std::vector<VkQueueFamilyProperties> qProps(qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(phys, &qCount, qProps.data());

    uint32_t graphicsFamily = -1;
    for (uint32_t i = 0; i < qCount; ++i) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phys, i, surface, &presentSupport);
        if ((qProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
            graphicsFamily = i;
            break;
        }
    }
    if (graphicsFamily == -1) throw std::runtime_error("No compatible queue family found");

    // 5. Logical device
    float qprio = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = graphicsFamily;
    qci.queueCount = 1;
    qci.pQueuePriorities = &qprio;

    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;

    VkDevice device;
    if (vkCreateDevice(phys, &dci, nullptr, &device) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");

    VkQueue queue;
    vkGetDeviceQueue(device, graphicsFamily, 0, &queue);

    std::cout << "MoltenVK setup complete!" << std::endl;
    std::cout << "Press Cmd+Q in the window to exit..." << std::endl;

    // Minimal event loop
    @autoreleasepool {
        while (true) {
            NSEvent* event;
            while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                              untilDate:[NSDate distantPast]
                                                 inMode:NSDefaultRunLoopMode
                                                dequeue:YES])) {
                [NSApp sendEvent:event];
                [NSApp updateWindows];
                if (event.type == NSEventTypeApplicationDefined) break;
            }
        }
    }

    // Cleanup
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}
