// Thank you http://osxbook.com
//
// Complile using the following command line:
// gcc -Wall hack.c -framework ApplicationServices
//
// For happy hacking, see: https://developer.apple.com/library/mac/documentation/Carbon/Reference/QuartzEventServicesRef/Reference/reference.html

#include <ApplicationServices/ApplicationServices.h>

CGEventMask eventMask = (1 << kCGEventOtherMouseUp) | (1 << kCGEventOtherMouseDown) | (1 << kCGEventScrollWheel);

// My own custom for ThinkPad keyboard. Suppress mouse center button before scrolling.
CGEventRef myCGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
  if (!((1 << type) & eventMask)) {
    return event;
  }
  if (type != kCGEventScrollWheel && CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber) != kCGMouseButtonCenter) {
    return event;
  }

  static CGEventType events[3];
  static int i = 0;
#define NEXT_M3(i) ((i + 1) % 3)
#define PREV_M3(i) ((i + 2) % 3)
  if (events[PREV_M3(PREV_M3(i))] == kCGEventOtherMouseDown &&
      events[PREV_M3(i)] == kCGEventScrollWheel &&
      type == kCGEventOtherMouseUp) {
    return NULL;
  }
  events[i] = type;
  i = NEXT_M3(i);
#undef PREV_M3
#undef NEXT_M3

  return event;
}

int main(void) {
  CFMachPortRef eventTap;
  CFRunLoopSourceRef runLoopSource;

  eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, myCGEventCallback, NULL);
  if (!eventTap) {
      exit(1);
  }

  runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);

  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);

  CGEventTapEnable(eventTap, true);

  CFRunLoopRun();

  exit(0);
}
