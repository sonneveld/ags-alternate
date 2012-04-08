
#include "SDL.h"
#include "SDL_ttf.h"


int game_running = 0;
TTF_Font *font;



SDL_Rect mouse_range_rect = {100, 100, 100, 100};

int alw_mouse_x = 0;
int alw_mouse_y = 0;
int alw_mouse_z = 0;
int alw_mouse_b = 0;

int within_rect(SDL_Rect *rect, int x, int y) {
  return x >= rect->x && x < (rect->x+rect->w) && y >= rect->y && y < (rect->y+rect->h);
}

int custom_filter(const SDL_Event *event) {
  
  switch (event->type){
       case SDL_MOUSEMOTION:
         return within_rect(&mouse_range_rect, event->motion.x, event->motion.y);
       case SDL_MOUSEBUTTONDOWN:
         return within_rect(&mouse_range_rect, event->button.x, event->button.y);
       case SDL_MOUSEBUTTONUP:
          // don't actually filter mouse up events!
         break;
  }
  return 1;
}

int alw_install_mouse() {
  SDL_SetEventFilter(&custom_filter);
  return 0;
}

void alw_position_mouse(int x, int y) {
  SDL_WarpMouse(x, y);
  // the new pos appears in allegro, so make sure it is
  // set even if the event doesn't occur.
  alw_mouse_x = x;
  alw_mouse_y = y;
}



void alw_set_mouse_range(int x1, int y1, int x2, int y2) {
  mouse_range_rect.x = x1;
  mouse_range_rect.y = y1;
  mouse_range_rect.w = x2-x1+1;
  mouse_range_rect.h = y2-y1+1;
}


void handle_mouse_motion_event(SDL_MouseMotionEvent *motion_event){
  alw_mouse_x = motion_event->x;
  alw_mouse_y = motion_event->y;
}

int sdl_button_to_allegro_bit(int button) {
  switch (button) {
    case SDL_BUTTON_LEFT: return 0x1;
    case SDL_BUTTON_MIDDLE: return 0x4;
    case SDL_BUTTON_RIGHT: return 0x2;
    case SDL_BUTTON_X1: return 0x8;
    case SDL_BUTTON_X2: return 0x10;    
  }
  return 0x0;
}

void handle_mouse_button_down_event(SDL_MouseButtonEvent *button_event){
  

  if (button_event->button == SDL_BUTTON_WHEELUP){
    alw_mouse_z += 1;
  }
  else if (button_event->button == SDL_BUTTON_WHEELDOWN){
    alw_mouse_z -= 1;
  }
  else  {
    alw_mouse_b |= sdl_button_to_allegro_bit(button_event->button);
  }
}

void handle_mouse_button_up_event(SDL_MouseButtonEvent *button_event){
  alw_mouse_b &= ~sdl_button_to_allegro_bit(button_event->button);
}

int _poll_everything() {
	SDL_Event event;
    while (SDL_PollEvent(&event)) {
      // Process event...
      switch (event.type) {
        case SDL_KEYDOWN:
          printf("The %s key was pressed!\n", SDL_GetKeyName(event.key.keysym.sym));
          if (event.key.keysym.sym == SDLK_q)
            game_running =0;
          if (event.key.keysym.sym == SDLK_w)
            alw_position_mouse(10, 10);
          break;

        case SDL_MOUSEMOTION:
          handle_mouse_motion_event(&event.motion);
          break;
        case SDL_MOUSEBUTTONDOWN:
          handle_mouse_button_down_event(&event.button);
          break;
        case SDL_MOUSEBUTTONUP:
          handle_mouse_button_up_event(&event.button);
          break;
        case SDL_QUIT:
          game_running =0;
      }
    }
	return 0;
}

int alw_poll_mouse() {
  return _poll_everything();
}



int main(int argc, char *argv[]) {
  //The images
  SDL_Surface* hello = NULL;
  SDL_Surface* screen = NULL;

  //Start SDL
  SDL_Init( SDL_INIT_EVERYTHING );
  TTF_Init();

  font=TTF_OpenFont("Vera.ttf", 16);
  if(!font) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    return 0;
  }

  alw_install_mouse();

  //Set up screen
  screen = SDL_SetVideoMode( 640, 480, 32, SDL_SWSURFACE );

  //Load image
  hello = SDL_LoadBMP( "hello.bmp" );

  game_running = 1;
  while (game_running) {
  
	alw_poll_mouse();
	

    SDL_FillRect(screen, 0, 0);
    SDL_FillRect(screen, &mouse_range_rect, 0xff0000);

    //Apply image to screen
    SDL_BlitSurface( hello, NULL, screen, NULL );

    

    SDL_Color c = {0xff, 0xff, 0xff};
    char buf[1000];
    sprintf(buf, "%d %d %d 0x%x", alw_mouse_x, alw_mouse_y, alw_mouse_z, alw_mouse_b);
    SDL_Surface *possurf = TTF_RenderText_Solid( font, buf, c);
      SDL_BlitSurface( possurf, NULL, screen, NULL );
    SDL_FreeSurface(possurf);


    //Update Screen
    SDL_Flip( screen );

  }


  //Free the loaded image
  SDL_FreeSurface( hello );

  //Quit SDL
  SDL_Quit();

  return 0;
}