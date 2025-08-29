VkSurfaceKHR makeSurface(VkInstance inst, const NativeParent& np){
  VkSurfaceKHR surface{};
  VkWin32SurfaceCreateInfoKHR ci{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
  ci.hinstance = GetModuleHandle(nullptr);
  ci.hwnd = np.hwnd;
  vkCreateWin32SurfaceKHR(inst, &ci, nullptr, &surface);
  return surface;
}
