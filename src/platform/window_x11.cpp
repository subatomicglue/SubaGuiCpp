
VkSurfaceKHR makeSurface(VkInstance inst, const NativeParent& np){
  VkSurfaceKHR surface{};
  VkXlibSurfaceCreateInfoKHR ci{VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR};
  ci.dpy = np.dpy;
  ci.window = np.win;
  vkCreateXlibSurfaceKHR(inst, &ci, nullptr, &surface);

  return surface;
}
