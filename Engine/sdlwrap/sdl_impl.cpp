

#include "sdlwrap/allegro.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>


int *alw_allegro_errno = NULL;

char _debug_str[10000];

#define PRINT_STUB sprintf(_debug_str, "STUB %s:%d %s\n", __FILE__, __LINE__, __FUNCSIG__); OutputDebugString(_debug_str)

// ALW_PALETTE
// ============================================================================

// we might be able to get away with no setting palettes yet.

ALW_PALETTE alw_black_palette;

void alw_set_palette(const ALW_PALETTE p) { PRINT_STUB; }
void alw_get_palette(ALW_PALETTE p) { PRINT_STUB; }

void alw_set_palette_range(const ALW_PALETTE p, int from, int to, int vsync) { PRINT_STUB; }
void alw_get_palette_range(ALW_PALETTE p, int from, int to) { PRINT_STUB; }

void alw_fade_interpolate(const ALW_PALETTE source, const ALW_PALETTE dest, ALW_PALETTE output, int pos, int from, int to) {PRINT_STUB; }

void alw_select_palette(const ALW_PALETTE p) { PRINT_STUB; }
void alw_unselect_palette() { PRINT_STUB; }


// FILE
// ============================================================================

// path stuff works, findfirst and pack funcs are stubs.

const char *ALL_SEP_CHAR = "\\/";

const char SYS_SEP_CHAR = '\\';

char *alw_append_filename(char *dest, const char *path, const char *filename, int size) {
	size_t pathlen = strlen(path);

	if (pathlen == 0) {
		strncpy(dest, filename, size);
		return dest;
	} 

	char *tmp = (char*)malloc(pathlen + 1);
	strcpy(tmp, path);

	int lastch = -1;
	lastch = path[pathlen-1];

	if (strchr(ALL_SEP_CHAR, lastch) != 0)
		_snprintf (dest, size, "%s%s", tmp, filename);
	else
		_snprintf (dest, size, "%s%c%s", tmp, SYS_SEP_CHAR, filename);

	free(tmp);
	return dest;
}

char *alw_fix_filename_case(char *path) {
	// do nothing
	return path;
}

char *alw_fix_filename_slashes(char *path) {

	char *p = path;

	while (*p) {
		if (strchr(ALL_SEP_CHAR, *p) != 0) {
			*p = SYS_SEP_CHAR;
		}
		p++;
	}

	return path;
}

void alw_put_backslash(char *filename) {
	int append_slash = 0;

	size_t pathlen = strlen(filename);

	if (pathlen == 0) {
		append_slash = 1;
	} else {
		int lastch = filename[pathlen-1];
		append_slash = (strchr("\\/#:", lastch) == 0);
	}

	if (append_slash) {
		filename[pathlen] = SYS_SEP_CHAR;
		filename[pathlen+1] = 0;
	}



}



char* alw_get_filename(char *path) {
	char *result = path;
	char *p = path;

	while (*p) {
		if (strchr("\\/", *p) != 0)
			result = p+1;
		p++;
	}
	return result;
}

//=====

int alw_al_findfirst(const char *pattern, struct alw_al_ffblk *info, int attrib) {
	PRINT_STUB;
	return 1; //stub
}

int alw_al_findnext(struct alw_al_ffblk *info) {
	PRINT_STUB;
	return 1;
}

void alw_al_findclose(struct alw_al_ffblk *info) {PRINT_STUB;}

//=====

int alw_exists(const char *filename) {

	FILE *f = fopen(filename, "r");
	if (f) {
		fclose(f);
		return 1;
	}
	return 0;
}

int alw_file_exists(const char *filename, int attrib, int *aret) {
	return alw_exists(filename);
}

long alw_file_size_ex(const char *filename) {
	struct stat st;
	stat(filename, &st);
	return (long)st.st_size;
}



// ==========

ALW_PACKFILE *alw_pack_fopen(const char *filename, const char *mode) {
	PRINT_STUB;
	return NULL;
}
long alw_pack_fread(void *p, long n, ALW_PACKFILE *f) {
	PRINT_STUB;
	return -1;
}
int alw_pack_fseek(ALW_PACKFILE *f, int offset) {
	PRINT_STUB;
	return -1;
}
int alw_pack_fclose(ALW_PACKFILE *f) {
	PRINT_STUB;
	return 0;
}
extern "C" {
ALW_PACKFILE *__old_pack_fopen(char *,char *) {	PRINT_STUB; return 0;}
}


// ALW_BITMAP
// ============================================================================

ALW_BITMAP *alw_screen;

static int _colour_depth = 0;

void alw_set_color_depth(int depth) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
	_colour_depth = depth;
}

static ALW_BITMAP *wrap_sdl_surface(SDL_Surface *surf) {
	ALW_BITMAP *x = (ALW_BITMAP *)malloc(sizeof(ALW_BITMAP));
	x->surf = surf;
	x->w = surf->w;
	x->h = surf->h;

	x->line = (unsigned char **)malloc(x->h * sizeof(unsigned char*));
	int hcount = surf->h;
	unsigned char **l = x->line;
	unsigned char *p = (unsigned char*)surf->pixels;
	while(hcount) {
		*l = p;
		p += x->surf->w * x->surf->pitch;
		l += 1;
		hcount -= 1;
	}

	return x;
}

ALW_BITMAP *alw_create_bitmap(int width, int height) {
	SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, _colour_depth, 0,0,0,0);
	return wrap_sdl_surface(surf);
}
ALW_BITMAP *alw_create_bitmap_ex(int color_depth, int width, int height) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
	SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, color_depth, 0,0,0,0);
	return wrap_sdl_surface(surf);
}

ALW_BITMAP *alw_create_sub_bitmap(ALW_BITMAP *parent, int x, int y, int width, int height) {
	PRINT_STUB;
	return 0;
}

void alw_destroy_bitmap(ALW_BITMAP *bitmap) {
	SDL_FreeSurface(bitmap->surf);
	free(bitmap->line);
	bitmap->line = 0;
	bitmap->surf =0 ;
	free(bitmap);
}

int alw_bitmap_color_depth(ALW_BITMAP *bmp) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
	return bmp->surf->format->BitsPerPixel;
}

int alw_bitmap_mask_color(ALW_BITMAP *bmp) {
	// not supported ?
	PRINT_STUB;
	return 0;
}

int alw_is_linear_bitmap(ALW_BITMAP *bmp) {
	PRINT_STUB;
	return 1;
}
int alw_is_memory_bitmap(ALW_BITMAP *bmp) {
	PRINT_STUB;
	return 1;
}
int alw_is_video_bitmap(ALW_BITMAP *bmp) {
	PRINT_STUB;
	return 0;
}

void alw_acquire_bitmap(ALW_BITMAP *bmp) {
	SDL_LockSurface(bmp->surf);
}
void alw_release_bitmap(ALW_BITMAP *bmp) {
	SDL_UnlockSurface(bmp->surf);
}
void alw_acquire_screen() {
	SDL_LockSurface(alw_screen->surf);
}
void alw_release_screen() {
	SDL_UnlockSurface(alw_screen->surf);
}
void alw_set_clip_rect(ALW_BITMAP *bitmap, int x1, int y1, int x2, int y2) {
	SDL_Rect rect = {x1, y1, x2-x1+1, y2-y1+1};
	SDL_SetClipRect(bitmap->surf, &rect);
}
void alw_set_clip_state(ALW_BITMAP *bitmap, int state) {
	PRINT_STUB;
	// i think the default is always clip:
}


// BLITS
// ============================================================================

void alw_blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height)  { PRINT_STUB; }
void alw_draw_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { PRINT_STUB; }
void alw_draw_lit_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int color) { PRINT_STUB; }
void alw_draw_trans_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { PRINT_STUB; }
void alw_draw_sprite_h_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { PRINT_STUB; }
void alw_draw_sprite_v_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { PRINT_STUB; }
void alw_draw_sprite_vh_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { PRINT_STUB; }
void alw_stretch_blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height){ PRINT_STUB; }
void alw_stretch_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int w, int h) { PRINT_STUB; }
void alw_rotate_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, alw_fixed angle) { PRINT_STUB; }
void alw_pivot_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int cx, int cy, alw_fixed angle) { PRINT_STUB; }

// COLOURS
// ============================================================================

int alw_makecol_depth(int color_depth, int r, int g, int b) { PRINT_STUB; return 0;}
int alw_makeacol_depth(int color_depth, int r, int g, int b, int a) { PRINT_STUB; return 0;}
int alw_getr_depth(int color_depth, int c){ PRINT_STUB; return 0;}
int alw_getg_depth(int color_depth, int c){ PRINT_STUB; return 0;}
int alw_getb_depth(int color_depth, int c){ PRINT_STUB; return 0;}
int alw_geta_depth(int color_depth, int c){ PRINT_STUB; return 0;}


// DRAWING
// ============================================================================

Uint32 _getpixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		return *p;

	case 2:
		return *(Uint16 *)p;

	case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			return p[0] << 16 | p[1] << 8 | p[2];
		else
			return p[0] | p[1] << 8 | p[2] << 16;

	case 4:
		return *(Uint32 *)p;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}

void _putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
	case 1:
		*p = pixel;
		break;

	case 2:
		*(Uint16 *)p = pixel;
		break;

	case 3:
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
			p[0] = (pixel >> 16) & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = pixel & 0xff;
		} else {
			p[0] = pixel & 0xff;
			p[1] = (pixel >> 8) & 0xff;
			p[2] = (pixel >> 16) & 0xff;
		}
		break;

	case 4:
		*(Uint32 *)p = pixel;
		break;
	}
}

int alw_getpixel ( ALW_BITMAP *bmp, int x, int y) { return _getpixel(bmp->surf, x, y); }
void alw_putpixel ( ALW_BITMAP *bmp, int x, int y, int color) { _putpixel(bmp->surf, x, y, color); }
void alw_clear_to_color(ALW_BITMAP *bitmap, int color) { SDL_FillRect(bitmap->surf, 0, color); }
void alw_clear_bitmap(ALW_BITMAP *bitmap) { SDL_FillRect(bitmap->surf, 0, 0); }
void alw_hline(ALW_BITMAP *bmp, int x1, int y, int x2, int color)  { PRINT_STUB; }
void alw_line(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color) { PRINT_STUB; }
void alw_do_line(ALW_BITMAP *bmp, int x1, int y1,int x2,int y2, int d, void (*proc)(ALW_BITMAP *bmp, int x, int y, int d))  { PRINT_STUB; }
void alw_rect(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color)  { PRINT_STUB; }
void alw_floodfill(ALW_BITMAP *bmp, int x, int y, int color) { PRINT_STUB; }
void alw_rectfill ( ALW_BITMAP *bmp, int x1, int y_1, int x2, int y2, int color)  { PRINT_STUB; }
void alw_triangle(ALW_BITMAP *bmp, int x1,int y1,int x2,int y2,int x3,int y3, int color)  { PRINT_STUB; }
void alw_circlefill(ALW_BITMAP *bmp, int x, int y, int radius, int color)  { PRINT_STUB; }


// MOUSE
// ============================================================================


static SDL_Rect _mouse_range_rect = {100, 100, 100, 100};

volatile int alw_mouse_x = 0;
volatile int alw_mouse_y = 0;
volatile int alw_mouse_z = 0;
volatile int alw_mouse_b = 0;

LPDIRECTINPUTDEVICE alw_mouse_dinput_device = 0;

static int _within_rect(SDL_Rect *rect, int x, int y) {
	return x >= rect->x && x < (rect->x+rect->w) && y >= rect->y && y < (rect->y+rect->h);
}

static int _custom_filter(const SDL_Event *event) {

	switch (event->type){
	   case SDL_MOUSEMOTION:
		   return _within_rect(&_mouse_range_rect, event->motion.x, event->motion.y);
	   case SDL_MOUSEBUTTONDOWN:
		   return _within_rect(&_mouse_range_rect, event->button.x, event->button.y);
	   case SDL_MOUSEBUTTONUP:
		   // don't actually filter mouse up events!
		   break;
	}
	return 1;
}

int alw_install_mouse() {
	SDL_SetEventFilter(&_custom_filter);
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
	_mouse_range_rect.x = x1;
	_mouse_range_rect.y = y1;
	_mouse_range_rect.w = x2-x1+1;
	_mouse_range_rect.h = y2-y1+1;
}


static void _handle_mouse_motion_event(SDL_MouseMotionEvent *motion_event){
	alw_mouse_x = motion_event->x;
	alw_mouse_y = motion_event->y;
}

static int _sdl_button_to_allegro_bit(int button) {
	switch (button) {
	case SDL_BUTTON_LEFT: return 0x1;
	case SDL_BUTTON_MIDDLE: return 0x4;
	case SDL_BUTTON_RIGHT: return 0x2;
	case SDL_BUTTON_X1: return 0x8;
	case SDL_BUTTON_X2: return 0x10;    
	}
	return 0x0;
}

static void _handle_mouse_button_down_event(SDL_MouseButtonEvent *button_event){


	if (button_event->button == SDL_BUTTON_WHEELUP){
		alw_mouse_z += 1;
	}
	else if (button_event->button == SDL_BUTTON_WHEELDOWN){
		alw_mouse_z -= 1;
	}
	else  {
		alw_mouse_b |= _sdl_button_to_allegro_bit(button_event->button);
	}
}

static void _handle_mouse_button_up_event(SDL_MouseButtonEvent *button_event){
	alw_mouse_b &= ~_sdl_button_to_allegro_bit(button_event->button);
}

static int _poll_everything() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Process event...
		switch (event.type) {
		case SDL_KEYDOWN:
			printf("The %s key was pressed!\n", SDL_GetKeyName(event.key.keysym.sym));
			//if (event.key.keysym.sym == SDLK_q)
			//	game_running =0;
			if (event.key.keysym.sym == SDLK_w)
				alw_position_mouse(10, 10);
			break;

		case SDL_MOUSEMOTION:
			_handle_mouse_motion_event(&event.motion);
			break;
		case SDL_MOUSEBUTTONDOWN:
			_handle_mouse_button_down_event(&event.button);
			break;
		case SDL_MOUSEBUTTONUP:
			_handle_mouse_button_up_event(&event.button);
			break;
		//case SDL_QUIT:
		//	game_running =0;
		}
	}
	return 0;
}

int alw_poll_mouse() {
	return _poll_everything();
}

// TRANSPARENCY
// ============================================================================

static ALW_COLOR_MAP *_alw_color_map;

void alw_set_alpha_blender() { PRINT_STUB; }
void alw_set_trans_blender(int r, int g, int b, int a) { PRINT_STUB; }
void alw_set_blender_mode (ALW_BLENDER_FUNC b15, ALW_BLENDER_FUNC b16, ALW_BLENDER_FUNC b24, int r, int g, int b, int a) { PRINT_STUB; }
void alw_create_light_table(ALW_COLOR_MAP *table, const ALW_PALETTE pal, int r, int g, int b, void (*callback)(int pos)) { PRINT_STUB; }

void alw_set_color_map(ALW_COLOR_MAP *alw_color_map) {_alw_color_map = alw_color_map;};
ALW_COLOR_MAP * alw_get_color_map(){return _alw_color_map;}

extern "C" {
	unsigned long _blender_trans16(unsigned long x, unsigned long y, unsigned long n){ PRINT_STUB; return 0;}
	unsigned long _blender_trans15(unsigned long x, unsigned long y, unsigned long n){ PRINT_STUB; return 0;}
}

// DIGIAL AUDIO
// ============================================================================
ALW_SAMPLE *alw_load_sample(const char *filename){ PRINT_STUB; return 0;}
void alw_destroy_sample(ALW_SAMPLE *spl){ PRINT_STUB; }
int alw_play_sample(const ALW_SAMPLE *spl, int vol, int pan, int freq, int loop){ PRINT_STUB; return 0;}
void alw_stop_sample(const ALW_SAMPLE *spl){ PRINT_STUB; }

// for controlling played samples
void alw_voice_start(int voice){ PRINT_STUB; }
void alw_voice_stop(int voice){ PRINT_STUB; }
int alw_voice_get_position(int voice){ PRINT_STUB; return 0;}
void alw_voice_set_position(int voice, int position){ PRINT_STUB; }
void alw_voice_set_volume(int voice, int volume){ PRINT_STUB; }
void alw_voice_set_pan(int voice, int pan) { PRINT_STUB; }
int alw_voice_get_frequency(int voice){ PRINT_STUB; return 0;}


// MIDI
// ============================================================================

volatile long alw_midi_pos = 0;

ALW_MIDI *alw_load_midi(const char *filename){ PRINT_STUB; return 0;}
void alw_destroy_midi(ALW_MIDI *midi){ PRINT_STUB; }
int alw_get_midi_length(ALW_MIDI *midi) { PRINT_STUB; return 0;}
int alw_play_midi(ALW_MIDI *midi, int loop) { PRINT_STUB; return -1;}
void alw_stop_midi(){ PRINT_STUB; }
void alw_midi_pause(){ PRINT_STUB; }
void alw_midi_resume(){ PRINT_STUB; }
int alw_midi_seek(int target) { PRINT_STUB; return 0;}


// SOUND INIT
// ============================================================================

void alw_reserve_voices(int digi_voices, int midi_voices){ PRINT_STUB; }
int alw_install_sound(int digi, int midi, const char *cfg_path){ PRINT_STUB; return 0;}
void alw_remove_sound(){ PRINT_STUB; }
void alw_set_volume(int digi_volume, int midi_volume){ PRINT_STUB; }
void alw_set_volume_per_voice(int scale){ PRINT_STUB; }


// COLOR FORMATS
// ============================================================================

ALW_RGB_MAP *alw_rgb_map;
void alw_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v){ PRINT_STUB; }
void alw_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b){ PRINT_STUB; }
void alw_create_rgb_table(ALW_RGB_MAP *table, const ALW_PALETTE pal, void (*callback)(int pos)){ PRINT_STUB; }


// AUDIO STREAM
// ============================================================================

void alw_stop_audio_stream(ALW_AUDIOSTREAM *stream){ PRINT_STUB; }



// WINALLEG
// ============================================================================
HWND alw_win_get_window(void){ PRINT_STUB;  return 0;}
int alw_wnd_call_proc(int (*proc)(void)){ PRINT_STUB;  return 0;}


// KEYBOARD
// ============================================================================

int alw_install_keyboard(){ PRINT_STUB; return 0;}
int alw_keyboard_needs_poll(){ PRINT_STUB; return 0;}
int alw_poll_keyboard(){ PRINT_STUB; return 0;}
int alw_keypressed(){ PRINT_STUB; return 0;}
int alw_readkey(){ PRINT_STUB; return 0;}
volatile char alw_key[KEY_MAX];
volatile int alw_key_shifts;
void alw_set_leds(int leds){ PRINT_STUB;}

const unsigned char alw_hw_to_mycode[256] = {
	/* 0x00 */ 0, KEY_ESC, KEY_1, KEY_2,
	/* 0x04 */ KEY_3, KEY_4, KEY_5, KEY_6,
	/* 0x08 */ KEY_7, KEY_8, KEY_9, KEY_0,
	/* 0x0C */ KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE, KEY_TAB,
	/* 0x10 */ KEY_Q, KEY_W, KEY_E, KEY_R,
	/* 0x14 */ KEY_T, KEY_Y, KEY_U, KEY_I,
	/* 0x18 */ KEY_O, KEY_P, KEY_OPENBRACE, KEY_CLOSEBRACE,
	/* 0x1C */ KEY_ENTER, KEY_LCONTROL, KEY_A, KEY_S,
	/* 0x20 */ KEY_D, KEY_F, KEY_G, KEY_H,
	/* 0x24 */ KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
	/* 0x28 */ KEY_QUOTE, KEY_TILDE, KEY_LSHIFT, KEY_BACKSLASH,
	/* 0x2C */ KEY_Z, KEY_X, KEY_C, KEY_V,
	/* 0x30 */ KEY_B, KEY_N, KEY_M, KEY_COMMA,
	/* 0x34 */ KEY_STOP, KEY_SLASH, KEY_RSHIFT, KEY_ASTERISK,
	/* 0x38 */ KEY_ALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1,
	/* 0x3C */ KEY_F2, KEY_F3, KEY_F4, KEY_F5,
	/* 0x40 */ KEY_F6, KEY_F7, KEY_F8, KEY_F9,
	/* 0x44 */ KEY_F10, KEY_NUMLOCK, KEY_SCRLOCK, KEY_7_PAD,
	/* 0x48 */ KEY_8_PAD, KEY_9_PAD, KEY_MINUS_PAD, KEY_4_PAD,
	/* 0x4C */ KEY_5_PAD, KEY_6_PAD, KEY_PLUS_PAD, KEY_1_PAD,
	/* 0x50 */ KEY_2_PAD, KEY_3_PAD, KEY_0_PAD, KEY_DEL_PAD,
	/* 0x54 */ KEY_PRTSCR, 0, KEY_BACKSLASH2, KEY_F11,
	/* 0x58 */ KEY_F12, 0, 0, KEY_LWIN,
	/* 0x5C */ KEY_RWIN, KEY_MENU, 0, 0,
	/* 0x60 */ 0, 0, 0, 0,
	/* 0x64 */ 0, 0, 0, 0,
	/* 0x68 */ 0, 0, 0, 0,
	/* 0x6C */ 0, 0, 0, 0,
	/* 0x70 */ KEY_KANA, 0, 0, KEY_ABNT_C1,
	/* 0x74 */ 0, 0, 0, 0,
	/* 0x78 */ 0, KEY_CONVERT, 0, KEY_NOCONVERT,
	/* 0x7C */ 0, KEY_YEN, 0, 0,
	/* 0x80 */ 0, 0, 0, 0,
	/* 0x84 */ 0, 0, 0, 0,
	/* 0x88 */ 0, 0, 0, 0,
	/* 0x8C */ 0, 0, 0, 0,
	/* 0x90 */ 0, KEY_AT, KEY_COLON2, 0,
	/* 0x94 */ KEY_KANJI, 0, 0, 0,
	/* 0x98 */ 0, 0, 0, 0,
	/* 0x9C */ KEY_ENTER_PAD, KEY_RCONTROL, 0, 0,
	/* 0xA0 */ 0, 0, 0, 0,
	/* 0xA4 */ 0, 0, 0, 0,
	/* 0xA8 */ 0, 0, 0, 0,
	/* 0xAC */ 0, 0, 0, 0,
	/* 0xB0 */ 0, 0, 0, 0,
	/* 0xB4 */ 0, KEY_SLASH_PAD, 0, KEY_PRTSCR,
	/* 0xB8 */ KEY_ALTGR, 0, 0, 0,
	/* 0xBC */ 0, 0, 0, 0,
	/* 0xC0 */ 0, 0, 0, 0,
	/* 0xC4 */ 0, KEY_PAUSE, 0, KEY_HOME,
	/* 0xC8 */ KEY_UP, KEY_PGUP, 0, KEY_LEFT,
	/* 0xCC */ 0, KEY_RIGHT, 0, KEY_END,
	/* 0xD0 */ KEY_DOWN, KEY_PGDN, KEY_INSERT, KEY_DEL,
	/* 0xD4 */ 0, 0, 0, 0,
	/* 0xD8 */ 0, 0, 0, KEY_LWIN,
	/* 0xDC */ KEY_RWIN, KEY_MENU, 0, 0,
	/* 0xE0 */ 0, 0, 0, 0,
	/* 0xE4 */ 0, 0, 0, 0,
	/* 0xE8 */ 0, 0, 0, 0,
	/* 0xEC */ 0, 0, 0, 0,
	/* 0xF0 */ 0, 0, 0, 0,
	/* 0xF4 */ 0, 0, 0, 0,
	/* 0xF8 */ 0, 0, 0, 0,
	/* 0xFC */ 0, 0, 0, 0
};
LPDIRECTINPUTDEVICE alw_key_dinput_device;


// AASTRETCH
// ============================================================================
void aa_stretch_sprite (ALW_BITMAP* dst, ALW_BITMAP* src, int dx, int dy, int dw, int dh){ PRINT_STUB;}

// FLI
// ============================================================================
int alw_play_fli(const char *filename, ALW_BITMAP *bmp, int loop, int (*callback)()){ PRINT_STUB; return ALW_FLI_OK;}


// IMAGE FILES
// ============================================================================
int alw_save_bitmap(const char *filename, ALW_BITMAP *bmp, const ALW_RGB *pal){ PRINT_STUB; return 0;}
ALW_BITMAP *alw_load_bitmap(const char *filename, ALW_RGB *pal){ PRINT_STUB; return 0;}
ALW_BITMAP *alw_load_pcx(const char *filename, ALW_RGB *pal) { PRINT_STUB; return 0;}
void alw_set_color_conversion(int mode) { PRINT_STUB;}


// GRAPHICS MODES
// ============================================================================
//void alw_set_color_depth(int depth) { PRINT_STUB;}
int alw_set_gfx_mode(int card, int w, int h, int v_w, int v_h){ PRINT_STUB; return 0;}

ALW_GFX_MODE_LIST *alw_get_gfx_mode_list(int card){ PRINT_STUB; return 0;}
void alw_destroy_gfx_mode_list(ALW_GFX_MODE_LIST *mode_list) { PRINT_STUB;}

int alw_set_display_switch_mode(int mode) { PRINT_STUB; return 0;}
int alw_set_display_switch_callback(int dir, void (*cb)()){ PRINT_STUB; return 0;}

void alw_vsync(){ PRINT_STUB;}


// UNICODE
// ============================================================================
char *alw_uconvert(const char *s, int type, char *buf, int newtype, int size){ PRINT_STUB; return 0;}
void alw_set_uformat(int type){ PRINT_STUB;}




// TIMERS
// ============================================================================

static SDL_TimerID _timer_id;
static int _timer_set = 0;
static volatile int _timer_interval = -1;
static void (*_allegro_callback)() = 0;
static int _last_interval = 0;


Uint32 _timer_callback(Uint32 interval, void *param) {
	(*_allegro_callback)();
	return _timer_interval;
};

int alw_install_timer() {
	return 0;
}

int alw_install_int_ex(void (*proc)(), int speed) {

	if (!_timer_set) {
		_timer_interval = (long)speed * 1000L / ALW_TIMERS_PER_SECOND ;
		_allegro_callback = proc;
		SDL_AddTimer(_timer_interval, _timer_callback, 0);
		_timer_set = 1;
	} else {
		_timer_interval = (long)speed * 1000L / ALW_TIMERS_PER_SECOND ;
	}
	return 0;
}

void alw_rest(unsigned int time) {
	SDL_Delay(time);
}


// INIT
// ============================================================================

int alw_install_allegro(int system_id, int *errno_ptr, int (*atexit_ptr)( void (__cdecl *func )( void ))) { PRINT_STUB; return 0;}
void alw_allegro_exit() { PRINT_STUB;}
void alw_set_window_title(const char *name){ PRINT_STUB;}
int alw_set_close_button_callback(void (*proc)(void)){ PRINT_STUB; return 0;}
int alw_get_desktop_resolution(int *width, int *height){ PRINT_STUB; return 0;}
char alw_allegro_error[ALW_ALLEGRO_ERROR_SIZE];



// MISC
// ============================================================================

extern "C" {
	ALW_GFX_VTABLE *_get_vtable(int color_depth){ PRINT_STUB; return 0;}
}


/* lookup table for scaling 5 bit colors up to 8 bits */
static int _rgb_scale_5[32] =
{
	0,   8,   16,  24,  33,  41,  49,  57,
	66,  74,  82,  90,  99,  107, 115, 123,
	132, 140, 148, 156, 165, 173, 181, 189,
	198, 206, 214, 222, 231, 239, 247, 255
};


/* lookup table for scaling 6 bit colors up to 8 bits */
static int _rgb_scale_6[64] =
{
	0,   4,   8,   12,  16,  20,  24,  28,
	32,  36,  40,  44,  48,  52,  56,  60,
	65,  69,  73,  77,  81,  85,  89,  93,
	97,  101, 105, 109, 113, 117, 121, 125,
	130, 134, 138, 142, 146, 150, 154, 158,
	162, 166, 170, 174, 178, 182, 186, 190,
	195, 199, 203, 207, 211, 215, 219, 223,
	227, 231, 235, 239, 243, 247, 251, 255
};

int alw_get_rgb_scale_5(int x) { return _rgb_scale_5[x]; }
int alw_get_rgb_scale_6(int x){ return _rgb_scale_6[x]; }


static int  _rgb_r_shift_15;
static int  _rgb_g_shift_15;
static int  _rgb_b_shift_15;
static int  _rgb_r_shift_16;
static int  _rgb_g_shift_16;
static int  _rgb_b_shift_16;
static int  _rgb_r_shift_32;
static int  _rgb_g_shift_32;
static int  _rgb_b_shift_32;
static int  _rgb_a_shift_32;

int  alw_get_rgb_r_shift_15() {return _rgb_r_shift_15;}
int  alw_get_rgb_g_shift_15() {return _rgb_g_shift_15;}
int  alw_get_rgb_b_shift_15() {return _rgb_b_shift_15;}
int  alw_get_rgb_r_shift_16() {return _rgb_r_shift_16;}
int  alw_get_rgb_g_shift_16() {return _rgb_g_shift_16;}
int  alw_get_rgb_b_shift_16() {return _rgb_b_shift_16;}
int  alw_get_rgb_r_shift_32() {return _rgb_r_shift_32;}
int  alw_get_rgb_g_shift_32() {return _rgb_g_shift_32;}
int  alw_get_rgb_b_shift_32() {return _rgb_b_shift_32;}
int  alw_get_rgb_a_shift_32() {return _rgb_a_shift_32;}
void  alw_set_rgb_r_shift_15(int x) {_rgb_r_shift_15 = x;}
void  alw_set_rgb_g_shift_15(int x) {_rgb_g_shift_15 = x;}
void  alw_set_rgb_b_shift_15(int x) {_rgb_b_shift_15 = x;}
void  alw_set_rgb_r_shift_16(int x) {_rgb_r_shift_16 = x;}
void  alw_set_rgb_g_shift_16(int x) {_rgb_g_shift_16 = x;}
void  alw_set_rgb_b_shift_16(int x) {_rgb_b_shift_16 = x;}
void  alw_set_rgb_r_shift_32(int x) {_rgb_r_shift_32 = x;}
void  alw_set_rgb_g_shift_32(int x) {_rgb_g_shift_32 = x;}
void  alw_set_rgb_b_shift_32(int x) {_rgb_b_shift_32 = x;}
void  alw_set_rgb_a_shift_32(int x) {_rgb_a_shift_32 = x;}

static HWND _allegro_wnd = NULL;
void alw_set_allegro_wnd(HWND allegro_wnd) {_allegro_wnd = allegro_wnd;}
HWND alw_get_allegro_wnd() {return _allegro_wnd;}

extern "C" LPDIRECTDRAW2 alw_get_directdraw() { PRINT_STUB; return 0;}
extern "C" LPDIRECTSOUND alw_get_directsound(){ PRINT_STUB; return 0;}
extern "C" DDRAW_SURFACE *alw_get_gfx_directx_primary_surface(){ PRINT_STUB; return 0;}

ALW_BITMAP *alw_gfx_directx_create_system_bitmap(int width, int height){ PRINT_STUB; return 0;}

// FONTS
// ============================================================================

int alfont_init(void){ PRINT_STUB; return 0;}
void alfont_exit(void){ PRINT_STUB;}

ALFONT_FONT *alfont_load_font_from_mem(const char *data, int data_len){ PRINT_STUB; return 0;}
void alfont_destroy_font(ALFONT_FONT *f){ PRINT_STUB;}

int alfont_set_font_size(ALFONT_FONT *f, int h){ PRINT_STUB; return 0;}

int alfont_text_mode(int mode){ PRINT_STUB; return 0;}

void alfont_textout_aa(ALW_BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color){ PRINT_STUB;}
void alfont_textout(ALW_BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color){ PRINT_STUB;}

int alfont_text_height(ALFONT_FONT *f){ PRINT_STUB; return 0;}
int alfont_text_length(ALFONT_FONT *f, const char *str){ PRINT_STUB; return 0;}


// CDLIB
// ============================================================================

extern "C" {
	int cd_init(void){ PRINT_STUB; return 0;}
	void cd_exit(void){ PRINT_STUB;}
	int cd_play_from(int track){ PRINT_STUB; return 0;}
	int cd_current_track(void){ PRINT_STUB; return 0;}
	void cd_pause(void){ PRINT_STUB;}
	void cd_resume(void){ PRINT_STUB;}
	int cd_get_tracks(int *first, int *last){ PRINT_STUB; return 0;}
	void cd_eject(void){ PRINT_STUB;}
	void cd_close(void){ PRINT_STUB;}
}


// DUMB
// ============================================================================

DUMB_IT_SIGRENDERER *duh_get_it_sigrenderer(DUH_SIGRENDERER *sigrenderer){ PRINT_STUB; return 0;}
int dumb_it_sr_get_current_order(DUMB_IT_SIGRENDERER *sr){ PRINT_STUB; return 0;}

DUH_SIGRENDERER *dumb_it_start_at_order(DUH *duh, int n_channels, int startorder){ PRINT_STUB; return 0;}
void dumb_it_set_loop_callback(DUMB_IT_SIGRENDERER *sigrenderer, int (*callback)(void *data), void *data){ PRINT_STUB;}
int dumb_it_callback_terminate(void *data){ PRINT_STUB; return 0;}
void duh_end_sigrenderer(DUH_SIGRENDERER *sigrenderer){ PRINT_STUB;}

DUH *dumb_load_it(const char *filename){ PRINT_STUB; return 0;}
DUH *dumb_load_xm(const char *filename){ PRINT_STUB; return 0;}
DUH *dumb_load_s3m(const char *filename){ PRINT_STUB; return 0;}
DUH *dumb_load_mod(const char *filename){ PRINT_STUB; return 0;}
void unload_duh(DUH *duh){ PRINT_STUB;}
long duh_get_length(DUH *duh){ PRINT_STUB; return 0;}

void dumb_exit(void){ PRINT_STUB;}



void dumb_register_packfiles(void){ PRINT_STUB;}
AL_DUH_PLAYER *al_start_duh(DUH *duh, int n_channels, long pos, float volume, long bufsize, int freq){ PRINT_STUB; return 0;}
void al_stop_duh(AL_DUH_PLAYER *dp){ PRINT_STUB;}
void al_pause_duh(AL_DUH_PLAYER *dp){ PRINT_STUB;}
void al_resume_duh(AL_DUH_PLAYER *dp){ PRINT_STUB;}
void al_duh_set_volume(AL_DUH_PLAYER *dp, float volume){ PRINT_STUB;}
int al_poll_duh(AL_DUH_PLAYER *dp){ PRINT_STUB; return 0;}
AL_DUH_PLAYER *al_duh_encapsulate_sigrenderer(DUH_SIGRENDERER *sigrenderer, float volume, long bufsize, int freq){ PRINT_STUB; return 0;}
DUH_SIGRENDERER *al_duh_get_sigrenderer(AL_DUH_PLAYER *dp){ PRINT_STUB; return 0;}




// ALOGG
// ============================================================================
extern "C" {

	ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len){ PRINT_STUB; return 0;}
	void alogg_destroy_ogg(ALOGG_OGG *ogg){ PRINT_STUB;}
	int alogg_play_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan){ PRINT_STUB; return 0;}
	int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop){ PRINT_STUB; return 0;}
	void alogg_stop_ogg(ALOGG_OGG *ogg){ PRINT_STUB;}
	void alogg_adjust_ogg(ALOGG_OGG *ogg, int vol, int pan, int speed, int loop){ PRINT_STUB;}
	void alogg_rewind_ogg(ALOGG_OGG *ogg){ PRINT_STUB;}
	void alogg_seek_abs_msecs_ogg(ALOGG_OGG *ogg, int msecs){ PRINT_STUB;}
	int alogg_poll_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}
	int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}
	int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}
	int alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}
	int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}
	int alogg_is_playing_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}
	ALW_AUDIOSTREAM *alogg_get_audiostream_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}

	ALOGG_OGGSTREAM *alogg_create_oggstream(void *first_data_buffer, int data_buffer_len, int last_block){ PRINT_STUB; return 0;}
	void alogg_destroy_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB;}
	int alogg_play_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan){ PRINT_STUB; return 0;}
	void alogg_stop_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB;}
	void alogg_adjust_oggstream(ALOGG_OGGSTREAM *ogg, int vol, int pan, int speed){ PRINT_STUB;}
	int alogg_poll_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}
	void *alogg_get_oggstream_buffer(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}
	void alogg_free_oggstream_buffer(ALOGG_OGGSTREAM *ogg, int bytes_used){ PRINT_STUB;}
	int alogg_get_pos_msecs_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}
	int alogg_is_playing_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}
	ALW_AUDIOSTREAM *alogg_get_audiostream_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}

	int alogg_is_end_of_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}
	int alogg_is_end_of_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}
}

// ALMP3
// ============================================================================

ALMP3_MP3 *almp3_create_mp3(void *data, int data_len){ PRINT_STUB; return 0;}
void almp3_destroy_mp3(ALMP3_MP3 *mp3){ PRINT_STUB;}

int almp3_play_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan){ PRINT_STUB; return 0;}
int almp3_play_ex_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan, int speed, int loop){ PRINT_STUB; return 0;}
void almp3_stop_mp3(ALMP3_MP3 *mp3){ PRINT_STUB;}
void almp3_rewind_mp3(ALMP3_MP3 *mp3){ PRINT_STUB;}
void almp3_seek_abs_msecs_mp3(ALMP3_MP3 *mp3, int msecs){ PRINT_STUB;}
void almp3_adjust_mp3(ALMP3_MP3 *mp3, int vol, int pan, int speed, int loop){ PRINT_STUB;}

int almp3_poll_mp3(ALMP3_MP3 *mp3){ PRINT_STUB; return 0;}

int almp3_get_pos_msecs_mp3(ALMP3_MP3 *mp3){ PRINT_STUB; return 0;}
int almp3_get_length_msecs_mp3(ALMP3_MP3 *mp3){ PRINT_STUB; return 0;}

ALW_AUDIOSTREAM *almp3_get_audiostream_mp3(ALMP3_MP3 *mp3){ PRINT_STUB; return 0;}

ALMP3_MP3STREAM *almp3_create_mp3stream(void *first_data_buffer, int data_buffer_len, int last_block){ PRINT_STUB; return 0;}
void almp3_destroy_mp3stream(ALMP3_MP3STREAM *mp3){ PRINT_STUB;}

int almp3_play_mp3stream(ALMP3_MP3STREAM *mp3, int buffer_len, int vol, int pan){ PRINT_STUB; return 0;}
void almp3_stop_mp3stream(ALMP3_MP3STREAM *mp3){ PRINT_STUB;}
void almp3_adjust_mp3stream(ALMP3_MP3STREAM *mp3, int vol, int pan, int speed){ PRINT_STUB;}

int almp3_poll_mp3stream(ALMP3_MP3STREAM *mp3){ PRINT_STUB; return 0;}
void *almp3_get_mp3stream_buffer(ALMP3_MP3STREAM *mp3){ PRINT_STUB; return 0;}
void almp3_free_mp3stream_buffer(ALMP3_MP3STREAM *mp3, int bytes_used){ PRINT_STUB;}

int almp3_get_length_msecs_mp3stream(ALMP3_MP3STREAM *mp3, int total_size){ PRINT_STUB; return 0;}
int almp3_get_pos_msecs_mp3stream(ALMP3_MP3STREAM *mp3){ PRINT_STUB; return 0;}

ALW_AUDIOSTREAM *almp3_get_audiostream_mp3stream(ALMP3_MP3STREAM *mp3){ PRINT_STUB; return 0;}
