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

  static int last_down = 0;
  if (type == kCGEventScrollWheel) {
      last_down = 0;
      return event;
  } else if (type == kCGEventOtherMouseDown) {
      last_down = 1;
      return NULL;
  } else if (type == kCGEventOtherMouseUp) {
      if (!last_down) {
	  return NULL;
      } else {
	  last_down = 0;
	  CGEventRef e = CGEventCreateCopy(event);
	  CGEventSetType(e, kCGEventOtherMouseDown);
	  CGEventTapPostEvent(proxy, e);
	  CFRelease(e);
	  return event;
      }
  }
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
