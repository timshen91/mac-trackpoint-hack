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
  static int last_down = 0;
  if (type == kCGEventScrollWheel) {
      last_down = 0;
  } else if (type == kCGEventOtherMouseDown && CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber) == kCGMouseButtonCenter) {
      last_down = 1;
      return NULL;
  } else if (type == kCGEventOtherMouseUp && CGEventGetIntegerValueField(event, kCGMouseEventButtonNumber) == kCGMouseButtonCenter) {
      if (!last_down) {
	  return NULL;
      } else {
	  last_down = 0;
	  CGEventRef e = CGEventCreateCopy(event);
	  CGEventSetType(e, kCGEventOtherMouseDown);
	  CGEventTapPostEvent(proxy, e);
	  CFRelease(e);
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
