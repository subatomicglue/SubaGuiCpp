#pragma once

// Forward declare Cocoa types in C++ header so we don’t pull in ObjC headers
#ifdef __OBJC__
@class NSWindow;
@class NSView;
@class NSAutoreleasePool;
#else
typedef struct objc_object NSWindow;
typedef struct objc_object NSView;
typedef struct objc_object NSAutoreleasePool;
#endif

class NativeParent;

/// Simple wrapper for an NSWindow + view, for use with Vulkan (MoltenVK)
struct PlatformWindow {
    NSWindow* window;
    NSView*   contentView;
    NSAutoreleasePool* pool;

    PlatformWindow(int w, int h, const char* title);
    ~PlatformWindow();

    NativeParent& nativeParent();

    void poll();
    bool shouldClose();
    void* nsView();  // returns NSView* as opaque void* for Vulkan surface creation
};
