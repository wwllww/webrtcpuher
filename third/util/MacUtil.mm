
#import <Cocoa/Cocoa.h>

#include "MacUtil.h"

namespace macutil {

	void* windowFromView(void* view)
	{
		NSView * nsview = (__bridge NSView *) view;
		NSWindow * nsWin = [nsview window];
    	
		return (__bridge void*)nsWin;
	}
}

