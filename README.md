
# Subatomic Sonic Device

Subatomic Sonographic Device - virtual audio device for standalone and plugins

![logo](logo.png)

Framework for building sonic devices running as VST, AU, CLAP, standalone.
C++ toolkit for audio instruments and effects, framework providing audio plugins & standalone, with GUI and input.

## Details

A lean, C++17, 2D windows + quad-and-texture graphics layer for audio synthesizer GUIs (knobs, lights, buttons, text), audio plugins (like vst, au, clap), and audio devices with touchscreens (raspberry pi):

 - standalone (opens its own window)
 - inside VST3, AU (macOS), and CLAP plugin editors
 - on Windows, macOS (Apple Silicon + Intel), Linux/Raspberry Pi
 - GPU-accelerated with one graphics backend


## tech stack

 - Swappable Renderers
   - **OpenGL ES 2**   (status: works)
   - **Vulkan 1.2**    (status: compiles, doesn't run)
     - **macOS** via **MoltenVK** (Vulkan over Metal).
     - **Windows/Linux/Raspberry Pi** with the standard Vulkan loader/ICD.
     - **Dependencies (small)**: Vulkan SDK (loader + headers), **stb_image.h** (textures), optional **volk** (Vulkan function loader), optional **VMA** (allocator). Shaders precompiled to SPIR-V at build time (glslangValidator or shaderc), so no runtime compiler dependency.
   - **Single draw primitive**: batched textured quads.
 - Windows
   - We implement tiny native windows for
     - Win32 (**status:**  not started)
     - Cocoa (**status:**  works)
     - X11 (**status:**  not started)
 - Widgets
   - Layout and definition in JSON configuration


## project layout

```
/
  /cmake/
  /src/
    /core/        (platform-agnostic)
      device.hpp, device.cpp
      swapchain.hpp, swapchain.cpp
      buffer.hpp, image.hpp, sampler.hpp
      pipeline.hpp
      renderer.hpp, renderer.cpp         // batched-textured-quad renderer
      quad_batch.hpp, quad_batch.cpp
      shaders/                           // *.vert *.frag + CMake to make SPIR-V
    /platform/
      /win32/ window_win32.hpp/.cpp      // Standalone window + surface
      /cocoa/ window_cocoa.mm            // Standalone NSWindow + CAMetalLayer-backed VkSurface via MVK
      /x11/   window_x11.cpp             // or wayland/ if preferred
      vk_surface_from_native.hpp/.cpp    // (HWND, NSView*, X11 Window) -> VkSurfaceKHR
    /guikit/
    /plugin/
      /vst3/
        CMakeLists.txt
        vst3_entry.cpp
        MyProcessor.cpp/.hpp
        MyController.cpp/.hpp
        MyEditorView.cpp/.hpp            // wraps host window and calls Renderer
      /au/
        au_component.mm                  // AUv2 Cocoa view (NSView*)
      /clap/
        clap_entry.c
        clap_gui.cpp/.hpp                // implements clap_gui and uses native parent
  /examples/
    /standalone_app/ main.cpp
  CMakeLists.txt
```


## install
```
brew install conan
```
