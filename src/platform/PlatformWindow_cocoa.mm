#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h> // for VK only
#include "PlatformWindow_cocoa.h"

#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#include "NativeParent_vk.h"
#endif

#ifdef USE_OPENGL
#include "NativeParent_gl.h"
#endif

// ---------------------------
// Delegate for window actions
// ---------------------------
@interface GfxWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) BOOL shouldClose;
@end

@implementation GfxWindowDelegate
- (instancetype)init {
    if (self = [super init]) {
        _shouldClose = NO;
    }
    return self;
}

- (BOOL)windowShouldClose:(id)sender {
    Event e{EventType::Quit};
    PlatformWindow* pw = (PlatformWindow*)[[sender contentView] owner];
    if (pw) pw->pubsub.dispatch(e);
    return YES;
}

- (void)windowDidResize:(NSNotification *)notification {
    NSWindow* win = [notification object];
    NSRect frame = [win contentRectForFrameRect:[win frame]];
    Event e{EventType::Resize, 0, 0, (int)frame.size.width, (int)frame.size.height};
    PlatformWindow* pw = (PlatformWindow*)[[win contentView] owner];
    if (pw) pw->pubsub.dispatch(e);
}
@end

// ---------------------------
// Custom view for mouse input
// ---------------------------
@interface GfxContentView : NSView
@property(nonatomic, assign) PlatformWindow* owner;
@end

@implementation GfxContentView
- (BOOL)acceptsFirstResponder { return YES; }
- (void)dispatchMouseEvent:(NSEvent*)event type:(EventType)etype button:(MouseButton)b {
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    Event e{etype, (int)p.x, (int)p.y};
    e.button = b;
    self.owner->pubsub.dispatch(e);
}
- (void)mouseDown:(NSEvent*)event { 
    [self dispatchMouseEvent:event type:EventType::MouseDown button:MouseButton::Left]; 
}
- (void)mouseUp:(NSEvent*)event { 
    [self dispatchMouseEvent:event type:EventType::MouseUp button:MouseButton::Left]; 
}
- (void)rightMouseDown:(NSEvent*)event { 
    [self dispatchMouseEvent:event type:EventType::MouseDown button:MouseButton::Right]; 
}
- (void)rightMouseUp:(NSEvent*)event { 
    [self dispatchMouseEvent:event type:EventType::MouseUp button:MouseButton::Right]; 
}
- (void)otherMouseDown:(NSEvent*)event { 
    [self dispatchMouseEvent:event type:EventType::MouseDown button:MouseButton::Middle]; 
}
- (void)otherMouseUp:(NSEvent*)event { 
    [self dispatchMouseEvent:event type:EventType::MouseUp button:MouseButton::Middle]; 
}
- (void)mouseMoved:(NSEvent*)event {
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    Event e{EventType::MouseMove, (int)p.x, (int)p.y};
    self.owner->pubsub.dispatch(e);
}
- (void)mouseDragged:(NSEvent*)event {
    [self mouseMoved:event];
}
- (void)keyDown:(NSEvent*)event {
    Event e{EventType::KeyDown};
    e.key = [event keyCode];                 // hardware key code
    e.character = [[event charactersIgnoringModifiers] characterAtIndex:0]; // normalized char
    e.keyRepeat = [event isARepeat];
    self.owner->pubsub.dispatch(e);
}
- (void)keyUp:(NSEvent*)event {
    Event e{EventType::KeyUp};
    e.key = [event keyCode];
    e.character = [[event charactersIgnoringModifiers] characterAtIndex:0];
    e.keyRepeat = false;
    self.owner->pubsub.dispatch(e);
}
@end



// ---------------------------
// PlatformWindow
// ---------------------------
PlatformWindow::PlatformWindow(int w, int h, const char* title) {
    pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // Install quit shortcuts (⌘Q, ⌘W)
    NSMenu* menubar = [[NSMenu alloc] init];
    NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@""];
    [appMenu addItemWithTitle:@"Quit"
                       action:@selector(terminate:)
                keyEquivalent:@"q"];
    [appMenu addItemWithTitle:@"Close Window"
                       action:@selector(performClose:)
                keyEquivalent:@"w"];
    [appMenuItem setSubmenu:appMenu];

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

    // Install custom contentView for mouse events
    contentView = [[GfxContentView alloc] initWithFrame:frame];
    ((GfxContentView*)contentView).owner = this;
    [window setContentView:contentView];

#if USE_VULKAN
    contentView.wantsLayer = YES;
    contentView.layer = [CAMetalLayer layer]; // MoltenVK needs this
#elif USE_OPENGL
    contentView.wantsLayer = NO;
#endif

    GfxWindowDelegate* del = [[GfxWindowDelegate alloc] init];
    [window setDelegate:del];
    [NSApp activateIgnoringOtherApps:YES];
}

PlatformWindow::~PlatformWindow() {
    [pool drain];
}

void PlatformWindow::poll() {
    NSEvent* event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
        [NSApp sendEvent:event];
    }
    [NSApp updateWindows];
}

bool PlatformWindow::shouldClose() {
    GfxWindowDelegate* del = (GfxWindowDelegate*)[window delegate];
    return del.shouldClose;
}

void* PlatformWindow::nsView() {
    return (void*)contentView;
}

NativeParent& PlatformWindow::nativeParent() {
    static NativeParent np;
    np.nsView = (void*)contentView;
    np.nsWindow = (void*)window;
    return np;
}
