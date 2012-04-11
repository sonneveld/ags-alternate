
#include "mac_alert.h"

#import <Cocoa/Cocoa.h>
#include "SDL.h"


extern "C" {

  
  void show_alert(const char *text, const char *caption) {
    SDL_Cursor *prevCursor = SDL_GetCursor();
    int prevCursorState = SDL_ShowCursor(SDL_QUERY);
    
    SDL_SetCursor(NULL);
    SDL_ShowCursor(SDL_ENABLE);
    
    @autoreleasepool {
      NSAlert* alert = [[NSAlert alloc] init];
      [alert setAlertStyle:NSCriticalAlertStyle];
      [alert setMessageText:[NSString stringWithCString:caption encoding:NSASCIIStringEncoding]];
      [alert setInformativeText:[NSString stringWithCString:text encoding:NSASCIIStringEncoding]];
      [alert addButtonWithTitle:@"Okay"];
      
      const NSInteger clicked = [alert runModal];
    }
    
    SDL_ShowCursor(prevCursorState);
    SDL_SetCursor(prevCursor);
  }

}