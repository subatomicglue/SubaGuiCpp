#pragma once

#include <cstdint>

struct NativeParent {
#ifdef _WIN32
  HWND hwnd{};
#elif defined(__APPLE__)
  void* nsView; // NSView*
  void* nsWindow; // store NSWindow*
#else
  ::Display* dpy{};
  ::Window   win{};
#endif
};

/// Creates a GL context bound to the native window (NSOpenGLContext)
uint64_t makeSurface(NativeParent parent, int width, int height);

/// Make this GL context current
void makeCurrent(uint64_t ctx);

/// Swap buffers (present)
void swapBuffers(uint64_t ctx);
