#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

// struct NativeParent {
//     void* nsView = nullptr;  // pointer to NSView on macOS
// };

struct vk_pimpl;
struct NativeParent;

class Renderer {
public:
    // Constructor creates instance, surface, device, queue, swapchain
    Renderer(NativeParent& np, int width, int height);
    ~Renderer();

    // Draw a frame (minimal placeholder)
    void drawFrame();

    void createRenderPass();
    void createFramebuffer(int w, int h);
    void createSwapchain(int w, int h);
    void createCmdBuffer();

    void createGraphicsPipeline();
    void createVertexBuffer();

private:
  vk_pimpl* vk = nullptr;
    // Optional: store image views, render passes, etc. later
};
