#include "vk_surface_from_native.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_macos.h>
#include <stdexcept>

VkSurfaceKHR makeSurface(VkInstance inst, const NativeParent& np) {
  VkMacOSSurfaceCreateInfoMVK ci{VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK};
  ci.pView = (__bridge void*)np.nsView; // NSView*
  VkSurfaceKHR surface{};
  if (vkCreateMacOSSurfaceMVK(inst, &ci, nullptr, &surface) != VK_SUCCESS)
      throw std::runtime_error("Failed to create macOS surface");
  return surface;
}
