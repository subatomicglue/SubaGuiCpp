#include "NativeParent_gl.h"

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>

uint64_t makeSurface(NativeParent parent, int width, int height) {
    NSWindow* window = (__bridge NSWindow*)parent.nsWindow;

    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAColorSize,     24,
        NSOpenGLPFAAlphaSize,     8,
        NSOpenGLPFADoubleBuffer,
        0
    };

    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pixelFormat
                                                          shareContext:nil];

    NSView* contentView = [window contentView];
    [context setView:contentView];
    [context makeCurrentContext];

    // cast ObjC pointer to uint64_t opaque handle
    return reinterpret_cast<uint64_t>(context);
}


void makeCurrent(uint64_t ctx) {
    NSOpenGLContext* context = (__bridge NSOpenGLContext*)(void*)ctx;
    [context makeCurrentContext];
}

void swapBuffers(uint64_t ctx) {
    NSOpenGLContext* context = (__bridge NSOpenGLContext*)(void*)ctx;
    [context flushBuffer];
}


#endif // __APPLE__
