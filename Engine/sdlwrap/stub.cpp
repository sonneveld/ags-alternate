
#include <allegro5/allegro.h>
#include "a4_aux.h"
#include "stub.h"

int cd_player_init() {
    printf("stub@%d\n", __LINE__);
    return 0;
}

int cd_player_control(int a, int b) {
    printf("stub@%d\n", __LINE__);
    return 0;
}


ALLEGRO_MOUSE_STATE _current_mouse_state;

void set_mouse_range (int x1, int y_1, int x2, int y2) {
    printf("stub@%d\n", __LINE__);
}

volatile int mouse_x;
volatile int mouse_y;
volatile int mouse_b;

int poll_mouse() {
    
    al_get_mouse_state(&_current_mouse_state);
    
    mouse_x = _current_mouse_state.x;
    mouse_y =  _current_mouse_state.y;
    mouse_b = _current_mouse_state.buttons;
    return 0;
}

void  position_mouse (int x, int y) {
    ALLEGRO_DISPLAY *disp = NULL;
    assert(disp != NULL);
    al_set_mouse_xy(disp, x, y);
}

int install_mouse() {
    return al_install_mouse();
}






void set_alpha_blender() {
    printf("stub@%d\n", __LINE__);
}

static BITMAP *_before;
static void _save_and_set_target(BITMAP *bmp){
    assert (_before == NULL);
    _before = al_get_target_bitmap();
    al_set_target_bitmap(bmp);
}

static void _restore_target(){
    al_set_target_bitmap(_before);   
    _before = NULL;
}


void draw_trans_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) {
    _save_and_set_target(bmp);
    al_draw_bitmap(sprite, (float)x, (float)y, 0);
    _restore_target();
}


void set_clip (BITMAP *bitmap, int x1, int y_1, int x2, int y2) {
    _save_and_set_target(bitmap);
    int w = x2-x1+1;
    int h = y2-y_1+1;
    al_set_clipping_rectangle(x1, y_1, w, h);
    _restore_target();
}

void set_clip_rect (BITMAP *bitmap, int x1, int y_1, int x2, int y2) {
    set_clip(bitmap, x1, y_1, x2, y2);
}

void destroy_bitmap(BITMAP *bitmap) {
    al_destroy_bitmap(bitmap);
}



BITMAP * create_bitmap_ex (int color_depth, int width, int height) {
    // we're ignoring depth which is dodge.
    BITMAP *result = al_create_bitmap(width, height);
    return result;
}

int _bitmap_get_cb(BITMAP *bitmap) {
    _save_and_set_target(bitmap);
    int x, y, w, h;
    al_get_clipping_rectangle(&x, &y, &w, &h);
    int result = y+h;
    _restore_target();
    return result;
}


void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
    _save_and_set_target(dest);
    al_draw_bitmap_region(source, source_x, source_y, width, height, dest_x, dest_y, 0);
    _restore_target();
}

void draw_sprite_h_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) {
    // ignoring mode for now.
    _save_and_set_target(bmp);
    al_draw_bitmap(sprite, x, y, ALLEGRO_FLIP_HORIZONTAL);
    _restore_target();
}

void draw_sprite_v_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) {
    // ignoring mode for now.
    _save_and_set_target(bmp);
    al_draw_bitmap(sprite, x, y, ALLEGRO_FLIP_VERTICAL);
    _restore_target();
}
void draw_sprite_vh_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) {
    // ignoring mode for now.
    _save_and_set_target(bmp);
    al_draw_bitmap(sprite, x, y, ALLEGRO_FLIP_VERTICAL|ALLEGRO_FLIP_HORIZONTAL);
    _restore_target();
}

void stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int source_width, int source_height, 
        int dest_x, int dest_y, int dest_width, int dest_height) {
    _save_and_set_target(dest);
    al_draw_scaled_bitmap(source, source_x, source_y, source_width, source_height, dest_x, dest_y, dest_width, dest_height, 0);
    _restore_target();
}

void clear(BITMAP *bitmap) {
    clear_bitmap(bitmap);
}

void clear_bitmap(BITMAP *bitmap) {
    // set to colour 0
    _save_and_set_target(bitmap);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    _restore_target();
}

void clear_to_color(BITMAP *bitmap, int color) {
    _save_and_set_target(bitmap);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    _restore_target();
}


BITMAP * create_bitmap (int width, int height) {
    // we're ignoring depth which is dodge.
    BITMAP *result = al_create_bitmap(width, height);
    return result;
}

 volatile int mouse_z;
 volatile long midi_pos = -1;
 volatile int key_shifts;
  RGB_MAP * rgb_map;
   COLOR_MAP * color_map;
  PALETTE black_palette = {0};
  char allegro_error[1000];
 
 
 
 
  int _rgb_r_shift_15;
  int _rgb_g_shift_15;
  int _rgb_b_shift_15;
  int _rgb_r_shift_16;
  int _rgb_g_shift_16;
  int _rgb_b_shift_16;
  int _rgb_r_shift_24;
  int _rgb_g_shift_24;
  int _rgb_b_shift_24;
  int _rgb_r_shift_32;
  int _rgb_g_shift_32;
  int _rgb_b_shift_32;
  int _rgb_a_shift_32;
 
    int _rgb_scale_5[1000];
    int _rgb_scale_6[1000];
    int key[ALLEGRO_KEY_MAX];


int keypressed() { printf("stub@%d\n", __LINE__); return 0;}


BITMAP *screen;
 
 
int osx_sys_question(AL_CONST char *msg, AL_CONST char *but1, AL_CONST char *but2) { printf("stub.h@%d\n", __LINE__); return  0; }
 bool PlayMovie (const char *name, int skip)  { printf("stub.h@%d\n", __LINE__); return  0; }
 
 void request_refresh_rate(int rate) {}
 
 extern "C" {
 
 PACKFILE *__old_pack_fopen(char * x,char *y) { return NULL; }
 }
 
 #include "acdebug.h"
 IAGSEditorDebugger * GetEditorDebugger(char const*) { return 0; }
 
 