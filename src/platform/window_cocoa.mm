#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#include "window_cocoa.h"

@interface GfxWindowDelegate : NSObject <NSWindowDelegate>
@end
@implementation GfxWindowDelegate
- (BOOL)windowShouldClose:(id)sender {
    [NSApp terminate:nil];
    return YES;
}
@end

PlatformWindow::PlatformWindow(int w, int h, const char* title) {
    pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSRect frame = NSMakeRect(0, 0, w, h);
    NSUInteger style = NSWindowStyleMaskTitled |
                       NSWindowStyleMaskClosable |
                       NSWindowStyleMaskResizable;
    window = [[NSWindow alloc] initWithContentRect:frame
                                         styleMask:style
                                           backing:NSBackingStoreBuffered
                                             defer:NO];
    [window setTitle:[NSString stringWithUTF8String:title]];
    [window makeKeyAndOrderFront:nil];

    contentView = [window contentView];
    if (contentView == nil) printf( "ERROR: contentview is nil\n");
    contentView.wantsLayer = YES;
    contentView.layer = [CAMetalLayer layer]; // MoltenVK needs this

    GfxWindowDelegate* del = [[GfxWindowDelegate alloc] init];
    [window setDelegate:del];
    [NSApp activateIgnoringOtherApps:YES];
}

PlatformWindow::~PlatformWindow() {
    [pool drain];
}

void PlatformWindow::poll() {
    [NSApp nextEventMatchingMask:NSEventMaskAny
                       untilDate:[NSDate distantPast]
                          inMode:NSDefaultRunLoopMode
                         dequeue:YES];
    [NSApp updateWindows];
}

bool PlatformWindow::shouldClose() {
    return false; // Exit handled by NSApp terminate
}

void* PlatformWindow::nsView() {
    return (void*)contentView;
}

#include <vulkan/vulkan.h>
#include "vk_surface_from_native.h"

NativeParent& PlatformWindow::nativeParent() {
    static NativeParent np;   // static so we can return a reference
    np.nsView = (void*)contentView;
    return np;
}
