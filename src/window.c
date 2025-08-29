#include <CoreGraphics/CoreGraphics.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // usleep

typedef unsigned long NSUInteger;
#ifndef YES
#define YES ((BOOL)1)
#define NO ((BOOL)0)
#endif

#define MSGSEND0(ret, obj, sel) ((ret (*)(id, SEL))objc_msgSend)((id)(obj), sel)
#define MSGSEND1(ret, obj, sel, a)                                             \
  ((ret (*)(id, SEL, typeof(a)))objc_msgSend)((id)(obj), sel, a)
#define MSGSEND2(ret, obj, sel, a, b)                                          \
  ((ret (*)(id, SEL, typeof(a), typeof(b)))objc_msgSend)((id)(obj), sel, a, b)
#define MSGSEND4(ret, obj, sel, a, b, c, d)                                    \
  ((ret (*)(id, SEL, typeof(a), typeof(b), typeof(c),                          \
            typeof(d)))objc_msgSend)((id)(obj), sel, a, b, c, d)

static size_t WIDTH = 800;
static size_t HEIGHT = 600;
static uint32_t *framebuffer = NULL;

// drawRect: method for our custom view (blits the raw buffer)
static void drawRectIMP(id self, SEL _cmd, CGRect rect) {
  id gc = MSGSEND0(id, objc_getClass("NSGraphicsContext"),
                   sel_registerName("currentContext"));
  CGContextRef ctx =
      (CGContextRef)MSGSEND0(id, gc, sel_registerName("CGContext"));
  if (!ctx)
    return;

  CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  CGDataProviderRef provider =
      CGDataProviderCreateWithData(NULL, framebuffer, WIDTH * HEIGHT * 4, NULL);

  CGImageRef img = CGImageCreate(
      (size_t)WIDTH, (size_t)HEIGHT, 8, 32, (size_t)WIDTH * 4, cs,
      kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst, provider,
      NULL, false, kCGRenderingIntentDefault);

  CGContextDrawImage(ctx, CGRectMake(0, 0, (CGFloat)WIDTH, (CGFloat)HEIGHT),
                     img);

  CGImageRelease(img);
  CGDataProviderRelease(provider);
  CGColorSpaceRelease(cs);
}

int main(void) {
  framebuffer = (uint32_t *)malloc(WIDTH * HEIGHT * sizeof(uint32_t));
  if (!framebuffer)
    return 1;
  memset(framebuffer, 0, WIDTH * HEIGHT * 4);

  // Initial gradient (BGRA little-endian in a 32-bit word)
  for (size_t y = 0; y < HEIGHT; y++) {
    for (size_t x = 0; x < WIDTH; x++) {
      uint8_t r = (uint8_t)((x * 255) / WIDTH);
      uint8_t g = (uint8_t)((y * 255) / HEIGHT);
      uint8_t b = 128;
      framebuffer[y * WIDTH + x] =
          0xFF000000u | ((uint32_t)b << 16) | ((uint32_t)g << 8) | (uint32_t)r;
    }
  }

  // Setup
  id setupPool =
      MSGSEND0(id, objc_getClass("NSAutoreleasePool"), sel_registerName("new"));
  id app = MSGSEND0(id, objc_getClass("NSApplication"),
                    sel_registerName("sharedApplication"));

  // Create custom NSView subclass
  Class NSViewClass = objc_getClass("NSView");
  Class MyViewClass = objc_allocateClassPair(NSViewClass, "MyPixelView", 0);
  class_addMethod(MyViewClass, sel_registerName("drawRect:"), (IMP)drawRectIMP,
                  "v@:{CGRect={CGPoint=dd}{CGSize=dd}}");
  objc_registerClassPair(MyViewClass);

  // Window + view
  CGRect rect = {{100, 100}, {(CGFloat)WIDTH, (CGFloat)HEIGHT}};
  id window = MSGSEND4(
      id, MSGSEND0(id, objc_getClass("NSWindow"), sel_registerName("alloc")),
      sel_registerName("initWithContentRect:styleMask:backing:defer:"), rect,
      (1 << 0) | (1 << 1) | (1 << 3), // titled, closable, resizable
      (NSUInteger)2,                  // NSBackingStoreBuffered
      NO);

  id view = MSGSEND0(id, MyViewClass, sel_registerName("new"));
  MSGSEND1(void, window, sel_registerName("setContentView:"), view);
  MSGSEND1(void, window, sel_registerName("setTitle:"),
           (id)CFSTR("Manual Pump Blit"));
  MSGSEND1(void, window, sel_registerName("makeKeyAndOrderFront:"), nil);

  // Foreground app + minimal menu so Spaces/focus behave
  MSGSEND1(BOOL, app, sel_registerName("setActivationPolicy:"),
           0 /* Regular */);
  id menubar = MSGSEND0(id, objc_getClass("NSMenu"), sel_registerName("new"));
  MSGSEND1(void, app, sel_registerName("setMainMenu:"), menubar);
  MSGSEND1(void, app, sel_registerName("activateIgnoringOtherApps:"), YES);

  // Finish launching sequence (rough equivalent to what -run would do)
  MSGSEND0(void, app, sel_registerName("finishLaunching"));
  MSGSEND0(void, setupPool, sel_registerName("drain"));

  // --- Manual pump: while(true)
  // ------------------------------------------------
  uint64_t t = 0;
  id mode = (id)CFSTR("kCFRunLoopDefaultMode");

  while (1) {
    // Autorelease pool per tick (like @autoreleasepool { ... })
    id pool = MSGSEND0(id, objc_getClass("NSAutoreleasePool"),
                       sel_registerName("new"));

    // 1) Poll one event (non-blocking)
    id distantPast =
        MSGSEND0(id, objc_getClass("NSDate"), sel_registerName("distantPast"));
    id event = MSGSEND4(
        id, app,
        sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
        (NSUInteger)-1, // NSEventMaskAny
        distantPast, mode, YES);
    if (event) {
      MSGSEND1(void, app, sel_registerName("sendEvent:"), event);
      MSGSEND0(void, app, sel_registerName("updateWindows"));
    }

    // 2) Update framebuffer (simple animated stripe)
    //    Keep it cheap so CPU stays low; tweak as you like.
    for (size_t y = 0; y < HEIGHT; y++) {
      for (size_t x = 0; x < WIDTH; x++) {
        uint8_t r = (uint8_t)(((x + (t >> 2)) * 255) / WIDTH);
        uint8_t g = (uint8_t)(((y + (t >> 3)) * 255) / HEIGHT);
        uint8_t b = (uint8_t)(128 + ((x ^ y ^ t) & 0x3F)); // small shimmer
        framebuffer[y * WIDTH + x] = 0xFF000000u | ((uint32_t)b << 16) |
                                     ((uint32_t)g << 8) | (uint32_t)r;
      }
    }
    t++;

    // 3) Blit synchronously (invoke drawRect: now)
    MSGSEND0(void, view, sel_registerName("display"));

    // 4) Small sleep to avoid pegging a core (target ~60 Hz)
    usleep(16000);

    // Drain per-frame pool
    MSGSEND0(void, pool, sel_registerName("drain"));
  }

  // Unreachable in this demo
  return 0;
}