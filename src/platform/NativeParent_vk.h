#pragma once

#ifndef VKSURFFROMNATIVE_H
#define VKSURFFROMNATIVE_H

#include <vulkan/vulkan.h>


// vk_surface_from_native.h
struct NativeParent {
#ifdef _WIN32
  HWND hwnd{};
#elif defined(__APPLE__)
  void* nsView{}; // NSView*
#else
  ::Display* dpy{};
  ::Window   win{};
#endif
};

VkSurfaceKHR makeSurface(VkInstance instance, const NativeParent& np);

#endif

