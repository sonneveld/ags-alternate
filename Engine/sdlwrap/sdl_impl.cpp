

#include "allegro.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

#include <vector>

#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <SDL_ttf.h>

#include "pcmgen.h"
#include "auderr.h"
#include "sampleloader.h"

// TODO: support 2x filters
// TODO: opengl version
// TODO: migrate from using MAC/WIN/LINUX_VERISON to feature specific defines.
// TODO: port to 64bit version
// TODO: port to ios

char _debug_str[10000];

//#define PRINT_STUB sprintf(_debug_str, "STUB %s:%d %s\n", __FILE__, __LINE__, __FUNCSIG__); OutputDebugString(_debug_str)
#define PRINT_STUB

static SDL_Surface *_actual_sdl_screen;

// we should be using new/delete, so hopefully temporary
template<class T> T* allocmem(size_t nelem=1, size_t extra=0)
{
  size_t s = nelem*sizeof(T) + extra;
  T* x = (T*)malloc(s);
  assert(x != 0);
  memset(x, 0, s);
  return x;
}


int _bestfit_color_for_current_palette(int r, int g, int b) {
  // TODO: implement bestfit color, if needed
  //return bestfit_color(_current_palette, r, g, b);
  return 0;
}

int _tmp_lookup_table[255] = {0};
int *_palette_expansion_table(int color_depth) {
  // TODO: implement palette expansion tables?
  return _tmp_lookup_table;
}



// RGB SHIFTS
// ============================================================================


/* default truecolor pixel format */
#define DEFAULT_RGB_R_SHIFT_15  0
#define DEFAULT_RGB_G_SHIFT_15  5
#define DEFAULT_RGB_B_SHIFT_15  10
#define DEFAULT_RGB_R_SHIFT_16  0
#define DEFAULT_RGB_G_SHIFT_16  5
#define DEFAULT_RGB_B_SHIFT_16  11
#define DEFAULT_RGB_R_SHIFT_24  0
#define DEFAULT_RGB_G_SHIFT_24  8
#define DEFAULT_RGB_B_SHIFT_24  16
#define DEFAULT_RGB_R_SHIFT_32  0
#define DEFAULT_RGB_G_SHIFT_32  8
#define DEFAULT_RGB_B_SHIFT_32  16
#define DEFAULT_RGB_A_SHIFT_32  24

int _rgb_r_shift_15 = DEFAULT_RGB_R_SHIFT_15;     /* truecolor pixel format */
int _rgb_g_shift_15 = DEFAULT_RGB_G_SHIFT_15;
int _rgb_b_shift_15 = DEFAULT_RGB_B_SHIFT_15;
int _rgb_r_shift_16 = DEFAULT_RGB_R_SHIFT_16;
int _rgb_g_shift_16 = DEFAULT_RGB_G_SHIFT_16;
int _rgb_b_shift_16 = DEFAULT_RGB_B_SHIFT_16;
int _rgb_r_shift_24 = DEFAULT_RGB_R_SHIFT_24;
int _rgb_g_shift_24 = DEFAULT_RGB_G_SHIFT_24;
int _rgb_b_shift_24 = DEFAULT_RGB_B_SHIFT_24;
int _rgb_r_shift_32 = DEFAULT_RGB_R_SHIFT_32;
int _rgb_g_shift_32 = DEFAULT_RGB_G_SHIFT_32;
int _rgb_b_shift_32 = DEFAULT_RGB_B_SHIFT_32;
int _rgb_a_shift_32 = DEFAULT_RGB_A_SHIFT_32;

/* lookup table for scaling 5 bit colors up to 8 bits */
 int _rgb_scale_5[32] =
{
	0,   8,   16,  24,  33,  41,  49,  57,
	66,  74,  82,  90,  99,  107, 115, 123,
	132, 140, 148, 156, 165, 173, 181, 189,
	198, 206, 214, 222, 231, 239, 247, 255
};


/* lookup table for scaling 6 bit colors up to 8 bits */
 int _rgb_scale_6[64] =
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



// INIT
// ============================================================================

char alw_allegro_error[ALW_ALLEGRO_ERROR_SIZE];

static void (*_on_close_callback)(void) = 0;

int alw_allegro_init() { 
  int result =  SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO);
  if (!result) {
    // if initialise
    SDL_EnableUNICODE(1);
  }
  return result;
}

void alw_allegro_exit() { 
  SDL_Quit(); 
}
void alw_set_window_title(const char *name){
  SDL_WM_SetCaption(name, 0);
}

int alw_set_close_button_callback(void (*proc)(void)){
  _on_close_callback = proc;
  return 0;
}

int alw_get_desktop_resolution(int *width, int *height){

  const SDL_VideoInfo *vidinfo;
  vidinfo = SDL_GetVideoInfo();
  *width = vidinfo->current_w;
  *height = vidinfo->current_h;
  return 0;
}




// ALW_BITMAP
// ============================================================================

ALW_COLOR_MAP * color_map;

ALW_BITMAP *alw_screen;

int _color_depth = 0;

void alw_set_color_depth(int depth) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
	_color_depth = depth;
}

GFX_VTABLE _default_vtable = {0};

static ALW_BITMAP *wrap_sdl_surface(SDL_Surface *surf) {
  // TODO: allow vtable overrides (like how the engine does in places)
  
  int nr_pointers = 2;
  if (surf->h > nr_pointers) nr_pointers = surf->h;
  
  ALW_BITMAP *x = allocmem<ALW_BITMAP>(1, sizeof(char *) * nr_pointers);
	x->surf = surf;
	x->w = surf->w;
	x->h = surf->h;

  x->vtable = allocmem<ALW_GFX_VTABLE>();
  x->vtable->mask_color = 0;
  x->vtable->color_depth = surf->format->BitsPerPixel;

  x->clip = 1;
  x->cl = 0;
  x->ct = 0;
  x->cr = surf->w;
  x->cb = surf->h;

	int hcount = surf->h;
	unsigned char **l = x->line;
	unsigned char *p = (unsigned char*)surf->pixels;
	while(hcount) {
		*l = p;
		p += surf->pitch;
		l += 1;
		hcount -= 1;
	}

	return x;
}

static void _add_palette_to_surface(SDL_Surface *surf) {
  SDL_Surface* videoSurf = SDL_GetVideoSurface();
  if (!videoSurf)
    return;

  if (!videoSurf->format->palette)
    return;
  
  SDL_SetColors(surf, videoSurf->format->palette->colors, 0, videoSurf->format->palette->ncolors);

}

static void _bmp_set_color_key(ALW_BITMAP *bmp) {
  //SDL_Surface *surf = bmp->surf;

  switch (bmp->vtable->color_depth) {
    case 8:
      //SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_8);
      bmp->vtable->mask_color = MASK_COLOR_8;
      break;
    case 15:
      //SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_15);
      bmp->vtable->mask_color =MASK_COLOR_15;
      break;
    case 16:
      //SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_16);
      bmp->vtable->mask_color =MASK_COLOR_16;
      break;
    case 24:
      //SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_24);
      bmp->vtable->mask_color =MASK_COLOR_24;
      break;
    case 32:
      //SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_32);
      bmp->vtable->mask_color =MASK_COLOR_32;
      break;
  }
}

ALW_BITMAP *alw_create_bitmap_ex(int color_depth, int width, int height) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
  
  Uint32 rmask=0, gmask=0, bmask=0, amask =0;
  
  switch (color_depth) {
    case 8: break;
    case 15:
      rmask = 0x1f << _rgb_r_shift_15;
      gmask = 0x1f << _rgb_g_shift_15;
      bmask = 0x1f << _rgb_b_shift_15;
      break;
    case 16:
      rmask = 0x1f << _rgb_r_shift_16;
      gmask = 0x3f << _rgb_g_shift_16;
      bmask = 0x1f << _rgb_b_shift_16;
      break;
    case 24:
      rmask = 0xff << _rgb_r_shift_32;
      gmask = 0xff << _rgb_g_shift_32;
      bmask = 0xff << _rgb_b_shift_32;
      break;
    case 32:
      rmask = 0xff << _rgb_r_shift_32;
      gmask = 0xff << _rgb_g_shift_32;
      bmask = 0xff << _rgb_b_shift_32;
      amask = 0xff << _rgb_a_shift_32;
      break;
  }
  
	SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, color_depth, rmask, gmask, bmask, amask);
  SDL_SetAlpha(surf, 0, 0xff);  // i think specifying amask sets the SDL_SRCALPHA flag which screws with sdl blits.
  
  _add_palette_to_surface(surf);
	ALW_BITMAP *bmp = wrap_sdl_surface(surf);
  _bmp_set_color_key(bmp);
  return bmp;
}



ALW_BITMAP *alw_create_bitmap(int width, int height) {
  return alw_create_bitmap_ex(_color_depth, width, height);

  //SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, _color_depth, 0,0,0,0);
  //SDL_SetColorKey(surf, SDL_SRCCOLORKEY, 0);
  //_add_palette_to_surface(surf);
  //return wrap_sdl_surface(surf);
}

extern ALW_BITMAP *create_sub_bitmap(ALW_BITMAP *parent, int x, int y, int width, int height);

ALW_BITMAP *alw_create_sub_bitmap(ALW_BITMAP *parent, int x, int y, int width, int height) {
  return create_sub_bitmap(parent, x, y, width, height);
}

void alw_destroy_bitmap(ALW_BITMAP *bitmap) {
  if (bitmap->surf) { // sub bitmaps don't link to surf.
    SDL_FreeSurface(bitmap->surf);
    bitmap->surf = 0;
    free(bitmap->vtable);
    bitmap->vtable = 0;
  }
  free(bitmap);
}

int alw_bitmap_color_depth(ALW_BITMAP *bmp) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
	return bmp->vtable->color_depth;
}

int alw_bitmap_mask_color(ALW_BITMAP *bmp) {
	PRINT_STUB;
  return bmp->vtable->mask_color;
}

int alw_is_same_bitmap(ALW_BITMAP *bmp1, ALW_BITMAP *bmp2)
{
  unsigned long m1;
  unsigned long m2;
  
  if ((!bmp1) || (!bmp2))
    return FALSE;
  
  if (bmp1 == bmp2)
    return TRUE;
  
  m1 = bmp1->id & ALW_BMP_ID_MASK;
  m2 = bmp2->id & ALW_BMP_ID_MASK;
  
  return ((m1) && (m1 == m2));
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
int alw_is_screen_bitmap(ALW_BITMAP *bmp) {
  PRINT_STUB;
	return 0;
}
int alw_is_planar_bitmap(ALW_BITMAP *bmp) {
  PRINT_STUB;
	return 0;
}

void alw_acquire_bitmap(ALW_BITMAP *bmp) {
  if (bmp->surf)
    SDL_LockSurface(bmp->surf);
}
void alw_release_bitmap(ALW_BITMAP *bmp) {
  if (bmp->surf)
    SDL_UnlockSurface(bmp->surf);
}
void alw_acquire_screen() {
  // TODO: shouldn't we be using the fake screen?
  SDL_LockSurface(_actual_sdl_screen);
}
void alw_release_screen() {
	SDL_UnlockSurface(_actual_sdl_screen);
}
void alw_set_clip_rect(ALW_BITMAP *bitmap, int x1, int y1, int x2, int y2) {
  /* internal clipping is inclusive-exclusive */
  x2++;
  y2++;
  
  bitmap->cl = MID(0, x1, bitmap->w-1);
  bitmap->ct = MID(0, y1, bitmap->h-1);
  bitmap->cr = MID(0, x2, bitmap->w);
  bitmap->cb = MID(0, y2, bitmap->h);
}
void alw_set_clip_state(ALW_BITMAP *bitmap, int state) {
	PRINT_STUB;
	// i think the default is always clip:
}



// GRAPHICS MODES
// ============================================================================

// original virtual screen (before alw_screen gets changed)
static ALW_BITMAP *_original_screen = 0;

//void alw_set_color_depth(int depth) { PRINT_STUB;}
int alw_set_gfx_mode(int card, int w, int h, int v_w, int v_h){
	assert(v_w == 0);
	assert(v_h == 0);

	_actual_sdl_screen = SDL_SetVideoMode(w, h, _color_depth, SDL_SWSURFACE|SDL_HWPALETTE);
	if (_actual_sdl_screen == NULL)
		return 1;
  
  // are we meant to set rgb_shifts after setting video mode?
  // TODO: investigate rgb_shifts.. how are they used when set by engine?
#if 0
  switch (_color_depth) {
    case 15:
      _rgb_r_shift_15 = _actual_sdl_screen->format->Rshift;
      _rgb_g_shift_15 = _actual_sdl_screen->format->Gshift;
      _rgb_b_shift_15 = _actual_sdl_screen->format->Bshift;
      break;
    case 16:
      _rgb_r_shift_16 = _actual_sdl_screen->format->Rshift;
      _rgb_g_shift_16 = _actual_sdl_screen->format->Gshift;
      _rgb_b_shift_16 = _actual_sdl_screen->format->Bshift;
      break;
    case 24:
      _rgb_r_shift_24 = _actual_sdl_screen->format->Rshift;
      _rgb_g_shift_24 = _actual_sdl_screen->format->Gshift;
      _rgb_b_shift_24 = _actual_sdl_screen->format->Bshift;
      break;
    case 32:
      _rgb_r_shift_32 = _actual_sdl_screen->format->Rshift;
      _rgb_g_shift_32 = _actual_sdl_screen->format->Gshift;
      _rgb_b_shift_32 = _actual_sdl_screen->format->Bshift;
      _rgb_a_shift_32 = _actual_sdl_screen->format->Ashift;
      break;
  }
#endif
  
  _original_screen = alw_create_bitmap_ex(_color_depth, w, h);
  alw_screen = _original_screen;
	return 0;
}

// TODO: support gfx mode lists?
ALW_GFX_MODE_LIST *alw_get_gfx_mode_list(int card){ PRINT_STUB; return 0;}
void alw_destroy_gfx_mode_list(ALW_GFX_MODE_LIST *mode_list) { PRINT_STUB;}

// TODO: support display switches?
int alw_set_display_switch_mode(int mode) { PRINT_STUB; return 0;}
int alw_set_display_switch_callback(int dir, void (*cb)()){ PRINT_STUB; return 0;}

void alw_vsync(){
  // TODO why does demo game require this, why is the screen not being cleared?
  SDL_FillRect(_actual_sdl_screen, 0, 0);
  SDL_BlitSurface(_original_screen->surf, 0, _actual_sdl_screen, 0);
	SDL_Flip(_actual_sdl_screen);
}


// ALW_PALETTE
// ============================================================================

// TODO: fix palette implementations

ALW_PALETTE alw_black_palette = {0};
ALW_PALETTE _current_palette; 


void alw_sys_set_palette_range(const ALW_PALETTE p, int from, int to, int vsync)
{
  SDL_Surface *surface = _actual_sdl_screen;
  int flags = SDL_PHYSPAL;
  int firstcolor = from;
  int ncolors = to-from+1;

  SDL_Color *colors = allocmem<SDL_Color>(ncolors);
  for (int i = 0; i< ncolors; i++) {
    ALW_RGB alcol = p[from+i];
    SDL_Color sdlcol = {alcol.r*4, alcol.g*4, alcol.b*4};
    colors[i] = sdlcol;
  }

  SDL_SetPalette(surface, flags, colors, firstcolor, ncolors);
  
  free(colors);
  
  if (vsync)
    alw_vsync();
}

extern void set_palette_range(const ALW_PALETTE p, int from, int to, int vsync);
void alw_set_palette_range(const ALW_PALETTE p, int from, int to, int vsync)
{
  PRINT_STUB;
  set_palette_range(p, from, to, vsync);
}

extern void get_palette_range(ALW_PALETTE p, int from, int to);
void alw_get_palette_range(ALW_PALETTE p, int from, int to)
{
  PRINT_STUB;
  get_palette_range(p, from, to);
}

void alw_set_palette(const ALW_PALETTE p)
{
  PRINT_STUB;
  set_palette_range(p, 0, ALW_PAL_SIZE-1, TRUE);
}
void alw_get_palette(ALW_PALETTE p)
{
  PRINT_STUB;
  get_palette_range(p, 0, ALW_PAL_SIZE-1);
}

extern void fade_interpolate(const ALW_PALETTE source, const ALW_PALETTE dest, ALW_PALETTE output, int pos, int from, int to);
void alw_fade_interpolate(const ALW_PALETTE source, const ALW_PALETTE dest, ALW_PALETTE output, int pos, int from, int to)
{
  PRINT_STUB;
  fade_interpolate(source, dest, output, pos, from, to);
}

// temp palette storage
extern void select_palette(const ALW_PALETTE p);
void alw_select_palette(const ALW_PALETTE p)
{
  PRINT_STUB;
  select_palette(p);
}
extern void unselect_palette(void);
void alw_unselect_palette()
{
  PRINT_STUB;
  unselect_palette();
}


// FILE
// ============================================================================

// path stuff works, findfirst and pack funcs are stubs.

const char *ALL_SEP_CHAR = "\\/";

#ifdef WINDOWS_VERSION
const char SYS_SEP_CHAR = '\\';
#else
const char SYS_SEP_CHAR = '/';
#endif

char *alw_append_filename(char *dest, const char *path, const char *filename, int size) {
	size_t pathlen = strlen(path);

	if (pathlen == 0) {
		strncpy(dest, filename, size);
		return dest;
	} 

  char *tmp = allocmem<char>(pathlen+1);
	strcpy(tmp, path);

	int lastch = -1;
	lastch = path[pathlen-1];

	if (strchr(ALL_SEP_CHAR, lastch) != 0)
		ac_snprintf (dest, size, "%s%s", tmp, filename);
	else
		ac_snprintf (dest, size, "%s%c%s", tmp, SYS_SEP_CHAR, filename);

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



const char* alw_get_filename(const char *path) {
	const char *result = path;
	const char *p = path;

	while (*p) {
		if (strchr("\\/", *p) != 0)
			result = p+1;
		p++;
	}
	return result;
}

//=====

extern int al_findnext(struct alw_al_ffblk *info);
extern void al_findclose(struct alw_al_ffblk *info);
extern int al_findfirst(const char *pattern, struct alw_al_ffblk *info, int attrib);

int alw_al_findfirst(const char *pattern, struct alw_al_ffblk *info, int attrib) {
	PRINT_STUB;
  return al_findfirst(pattern, info, attrib);
}

int alw_al_findnext(struct alw_al_ffblk *info) {
	PRINT_STUB;
	return al_findnext(info);
}

void alw_al_findclose(struct alw_al_ffblk *info) {
  PRINT_STUB;
  al_findclose(info);
}

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

//ALW_PACKFILE *alw_pack_fopen(const char *filename, const char *mode) {
//	PRINT_STUB;
//	return NULL;
//}
extern long pack_fread(void *p, long n, ALW_PACKFILE *f);
long alw_pack_fread(void *p, long n, ALW_PACKFILE *f) {
  return pack_fread(p, n, f);
}
extern int pack_fseek(ALW_PACKFILE *f, int offset);
int alw_pack_fseek(ALW_PACKFILE *f, int offset) {
  return pack_fseek(f, offset);
}
extern int pack_fclose(ALW_PACKFILE *f);
int alw_pack_fclose(ALW_PACKFILE *f) {
  return pack_fclose(f);
}



// BLITS
// ============================================================================

extern void blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void alw_blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
  PRINT_STUB;
  blit(source, dest, source_x, source_y, dest_x, dest_y, width, height);
}

void alw_draw_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  _vtable_draw_sprite(bmp, sprite, x, y);
}

void alw_draw_lit_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int color) { 
	PRINT_STUB;
  _vtable_draw_lit_sprite(bmp, sprite, x, y, color);
}
void alw_draw_trans_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  _vtable_draw_trans_sprite(bmp, sprite, x, y);
}
void alw_draw_sprite_h_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  _vtable_draw_sprite_h_flip(bmp, sprite, x, y);
}
void alw_draw_sprite_v_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  _vtable_draw_sprite_v_flip(bmp, sprite, x, y);
}
void alw_draw_sprite_vh_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  _vtable_draw_sprite_vh_flip(bmp, sprite, x, y);
}

// COLOURS
// ============================================================================

// TODO: fix 8bit makecol, getcol with palettes

int alw_makecol(int r, int g, int b) {
  return alw_makecol_depth(_color_depth, r, g, b);
}

int alw_makecol_depth(int color_depth, int r, int g, int b) {
  PRINT_STUB;
  switch (color_depth) {
    case 8:
      return 1; // fix later.
    case 15:
      return (((r >> 3) << _rgb_r_shift_15) |
              ((g >> 3) << _rgb_g_shift_15) |
              ((b >> 3) << _rgb_b_shift_15));
    case 16:
      return (((r >> 3) << _rgb_r_shift_16) |
              ((g >> 2) << _rgb_g_shift_16) |
              ((b >> 3) << _rgb_b_shift_16));
    case 24:
      return ((r << _rgb_r_shift_24) |
              (g << _rgb_g_shift_24) |
              (b << _rgb_b_shift_24));
    case 32:
      return ((r << _rgb_r_shift_32) |
              (g << _rgb_g_shift_32) |
              (b << _rgb_b_shift_32));
  }
  return 0;
}
int alw_makeacol_depth(int color_depth, int r, int g, int b, int a) { 
  PRINT_STUB;
  switch (color_depth) {
    case 32:
      return ((r << _rgb_r_shift_32) |
              (g << _rgb_g_shift_32) |
              (b << _rgb_b_shift_32) |
              (a << _rgb_a_shift_32));
    default:
      return alw_makecol_depth(color_depth, r,g, b);
  }

}
int alw_getr_depth(int color_depth, int c)
{
  PRINT_STUB;
  switch (color_depth) {
    case 8: return _rgb_scale_6[(int)_current_palette[c].r];
    case 15: return _rgb_scale_5[(c >> _rgb_r_shift_15) & 0x1F];
    case 16: return _rgb_scale_5[(c >> _rgb_r_shift_16) & 0x1F];
    case 24: return ((c >> _rgb_r_shift_24) & 0xFF);
    case 32: return ((c >> _rgb_r_shift_32) & 0xFF);
  }
  return 0;
}
int alw_getg_depth(int color_depth, int c)
{ 
  PRINT_STUB;
  switch (color_depth) {
    case 8: return _rgb_scale_6[(int)_current_palette[c].g];
    case 15: return _rgb_scale_5[(c >> _rgb_g_shift_15) & 0x1F];
    case 16: return _rgb_scale_6[(c >> _rgb_g_shift_16) & 0x3F];
    case 24: return ((c >> _rgb_g_shift_24) & 0xFF);
    case 32: return ((c >> _rgb_g_shift_32) & 0xFF);
  }
  return 0;
}
int alw_getb_depth(int color_depth, int c)
{ 
  PRINT_STUB;
  switch (color_depth) {
    case 8: return _rgb_scale_6[(int)_current_palette[c].b];
    case 15: return _rgb_scale_5[(c >> _rgb_b_shift_15) & 0x1F];
    case 16: return _rgb_scale_5[(c >> _rgb_b_shift_16) & 0x1F];
    case 24: return ((c >> _rgb_b_shift_24) & 0xFF);
    case 32: return ((c >> _rgb_b_shift_32) & 0xFF);
  }
  return 0;
}
int alw_geta_depth(int color_depth, int c)
{  
  PRINT_STUB;
  switch (color_depth) {
    case 32: return ((c >> _rgb_a_shift_32) & 0xFF);
  }
  return 0;
}


// DRAWING
// ============================================================================

int alw_getpixel ( ALW_BITMAP *bmp, int x, int y)
{ 
  return _vtable_getpixel(bmp, x, y);
}

void alw_putpixel ( ALW_BITMAP *bmp, int x, int y, int color) 
{ 
  _vtable_putpixel(bmp, x, y, color);
}

void alw_putpixel_ex ( ALW_BITMAP *bmp, int x, int y, int color, int alpha, int is_solid) 
{ 
  _vtable_putpixel_ex(bmp, x, y, color, is_solid);
}

void alw_clear_to_color(ALW_BITMAP *bitmap, int color) 
{
  _vtable_clear_to_color(bitmap, color);
}
void alw_clear_bitmap(ALW_BITMAP *bitmap) 
{
  _vtable_clear_to_color(bitmap, 0);
}
void alw_hline(ALW_BITMAP *bmp, int x1, int y, int x2, int color) 
{
  _vtable_hline(bmp, x1, y, x2, color);
}
void alw_vline(ALW_BITMAP *dst, int dx, int dy1, int dy2, int color)
{
  _vtable_vline(dst, dx, dy1, dy2, color);
}

extern void _normal_line(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void alw_line(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{ 
  PRINT_STUB;
  _normal_line(bmp, x1, y1, x2, y2, color);
}

extern void do_line(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(ALW_BITMAP *, int, int, int));
void alw_do_line(ALW_BITMAP *bmp, int x1, int y1,int x2,int y2, int d, void (*proc)(ALW_BITMAP *bmp, int x, int y, int d))
{ 
  PRINT_STUB; 
  do_line(bmp, x1, y1, x2, y2, d, proc);
}

extern void _soft_rect(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void alw_rect(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{ 
  PRINT_STUB; 
  _soft_rect(bmp, x1, y1, x2, y2, color);
}

extern void _soft_floodfill(ALW_BITMAP *bmp, int x, int y, int color);
void alw_floodfill(ALW_BITMAP *bmp, int x, int y, int color) {
  _soft_floodfill(bmp, x, y, color);
}

extern void _normal_rectfill(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void alw_rectfill ( ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{ 
  PRINT_STUB; 
  _normal_rectfill(bmp, x1, y1, x2, y2, color);
}

void alw_triangle(ALW_BITMAP *bmp, int x1,int y1,int x2,int y2,int x3,int y3, int color)
{ 
  PRINT_STUB; 
  // TODO implement triangle?
  // requires polygon.c from allegro.  do we need this too?
}

extern void _soft_circlefill(ALW_BITMAP *bmp, int x, int y, int radius, int color);
void alw_circlefill(ALW_BITMAP *bmp, int x, int y, int radius, int color)
{ 
  PRINT_STUB; 
  _soft_circlefill(bmp, x, y, radius, color);
}



// MOUSE
// ============================================================================


static int _mouse_range_rect_set = 0;
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
       if (_mouse_range_rect_set)
		     return _within_rect(&_mouse_range_rect, event->motion.x, event->motion.y);
	   case SDL_MOUSEBUTTONDOWN:
       if (_mouse_range_rect_set)
		     return _within_rect(&_mouse_range_rect, event->button.x, event->button.y);
	   case SDL_MOUSEBUTTONUP:
		   // don't actually filter mouse up events!
		   break;
	}
	return 1;
}

int alw_install_mouse() {
	SDL_SetEventFilter(&_custom_filter);
  SDL_ShowCursor(SDL_DISABLE);
	return 0;
}

void alw_position_mouse(int x, int y) {
  // disabled cause it makes it difficult to actually close the window if we're
  // constantly warping the mouse.
#if 0
	SDL_WarpMouse(x, y);
	// the new pos appears in allegro, so make sure it is
	// set even if the event doesn't occur.
	alw_mouse_x = x;
	alw_mouse_y = y;
#endif
}



void alw_set_mouse_range(int x1, int y1, int x2, int y2) {
  _mouse_range_rect_set = 1;
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

static void _handle_key_down_event(SDL_KeyboardEvent *key_event);
static void _handle_key_up_event(SDL_KeyboardEvent *key_event);

static int _poll_everything() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// Process event...
		switch (event.type) {
      case SDL_KEYDOWN:
        _handle_key_down_event(&event.key);
        break;
      case SDL_KEYUP:
        _handle_key_up_event(&event.key);
        break;
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
        
		case SDL_QUIT:
      if (_on_close_callback)
        _on_close_callback();
      break;
		}
	}
	return 0;
}

int alw_poll_mouse() {
	return _poll_everything();
}


// KEYBOARD
// ============================================================================

// TODO: what does it mean when readkey returns 0?

// TODO: can we handle events asyncronously?

static int _scancode_sdl_to_allegro(int sdl_key_sym);
static int _scancode_allegro_to_sdl(int allegro_scancode);

static int _sdl_key_state[SDLK_LAST] = {0};

#define _READKEY_BUF_SIZE 32
SDL_keysym _readkey_buf[_READKEY_BUF_SIZE];
int _readkey_start = 0;  // oldest
int _readkey_end = 0;    // spot to write next one.

int _readkey_buffer_is_full() {
	return (_readkey_end+1)%_READKEY_BUF_SIZE == _readkey_start;
}

int _readkey_buffer_is_empty() {
	return _readkey_start == _readkey_end;
}

void _readkey_buffer_push(SDL_keysym value)  {
	assert(!_readkey_buffer_is_full());
	
	_readkey_buf[_readkey_end] = value;
	_readkey_end = (_readkey_end+1)%_READKEY_BUF_SIZE;
	
	assert (_readkey_start != _readkey_end);
}

SDL_keysym _readkey_buffer_pop() {
  assert(!_readkey_buffer_is_empty());
	SDL_keysym value = _readkey_buf[_readkey_start];
	_readkey_start = (_readkey_start+1)%_READKEY_BUF_SIZE;
  return value;
}

void _readkey_buffer_clear() {
  _readkey_start = 0;
  _readkey_end = 0;
  
}


int alw_install_keyboard(){ 
  PRINT_STUB; 
  alw_clear_keybuf();
  return 0; 
}

/* Returns TRUE if the current keyboard driver is operating in polling mode. */
int alw_keyboard_needs_poll(){ PRINT_STUB; return 1; }

/* poll keyboard, only need for alw_key and alw_key_shifts */
int alw_poll_keyboard(){ _poll_everything(); return 0; }

/* Returns TRUE if there are keypresses waiting in the input buffer. You can use 
 this to see if the next call to readkey() is going to block or to simply wait 
 for the user to press a key. */
int alw_keypressed(){ 
  PRINT_STUB;
  alw_poll_keyboard(); 
  return !_readkey_buffer_is_empty();
}

/* Returns the next character from the keyboard buffer, in ASCII format. If the 
 buffer is empty, it waits until a key is pressed.
 The low byte of the return value contains the ASCII code of the key, and the 
 high byte the scancode. */
int alw_readkey(){ 
  PRINT_STUB;
  
  for (;;) {
    alw_poll_keyboard(); 
    if (!_readkey_buffer_is_empty())
      break;
    SDL_Delay(1);
    
  }
  
  SDL_keysym sdlkeysym = _readkey_buffer_pop();
  int allegro_scancode = _scancode_sdl_to_allegro(sdlkeysym.sym);
  int allegro_ascii = sdlkeysym.unicode & 0x7f;
  int value = allegro_ascii | (allegro_scancode << 8);
  return value;
}

void alw_clear_keybuf() { 
  PRINT_STUB; 
  alw_poll_keyboard(); // read anything in sdl event buffer.
  _readkey_buffer_clear();
  memset(_sdl_key_state, 0, sizeof(_sdl_key_state));
}

/* Array of flags indicating the state of each key, ordered by scancode */
//volatile char alw_key[KEY_MAX];
int alw_get_key(int allegro_scancode) {
  alw_poll_keyboard();
  int sdl_scancode = _scancode_allegro_to_sdl(allegro_scancode);
  if (sdl_scancode <= 0)
    return 0;
  return _sdl_key_state[sdl_scancode];
}

/* Bitmask containing the current state of shift/ctrl/alt, */
int alw_get_key_shifts(int allegro_shift) {
  alw_poll_keyboard();
  switch (allegro_shift) {
    case KB_NUMLOCK_FLAG:
      return _sdl_key_state[SDLK_NUMLOCK];
    case KB_CAPSLOCK_FLAG:
      return _sdl_key_state[SDLK_CAPSLOCK];
    case KB_SCROLOCK_FLAG:
      return _sdl_key_state[SDLK_SCROLLOCK];
  }
  return 0;
}

static void _handle_key_down_event(SDL_KeyboardEvent *key_event) {
  _sdl_key_state[key_event->keysym.sym] = 1;
	
  // we get weirdness if there isn't an ascii version.. ac engine expects to be
  // able to read an extended char?
  //if (key_event->keysym.unicode >= 0x80 || key_event->keysym.unicode < 0)
  //  return;
  
  if (_readkey_buffer_is_full())
    return;
  
  int allegro_scancode = _scancode_sdl_to_allegro(key_event->keysym.sym);
  if (allegro_scancode <=0)
    return;

  _readkey_buffer_push(key_event->keysym);
}

static void _handle_key_up_event(SDL_KeyboardEvent *key_event) {
  _sdl_key_state[key_event->keysym.sym] = 0;
}

static int _scancode_sdl_to_allegro(int sdl_key_sym) {
  switch (sdl_key_sym) {
    case SDLK_BACKSPACE:    return KEY_BACKSPACE;
    case SDLK_TAB:          return KEY_TAB;
    case SDLK_RETURN:       return KEY_ENTER;
    case SDLK_PAUSE:        return KEY_PAUSE;
    case SDLK_ESCAPE:       return KEY_ESC;
    case SDLK_SPACE:        return KEY_SPACE;
    case SDLK_QUOTE:        return KEY_QUOTE;
    case SDLK_COMMA:        return KEY_COMMA;
    case SDLK_MINUS:        return KEY_MINUS;
    case SDLK_PERIOD:       return KEY_STOP;
    case SDLK_SLASH:        return KEY_SLASH;
    case SDLK_0:            return KEY_0;
    case SDLK_1:            return KEY_1;
    case SDLK_2:            return KEY_2;
    case SDLK_3:            return KEY_3;
    case SDLK_4:            return KEY_4;
    case SDLK_5:            return KEY_5;
    case SDLK_6:            return KEY_6;
    case SDLK_7:            return KEY_7;
    case SDLK_8:            return KEY_8;
    case SDLK_9:            return KEY_9;
    case SDLK_COLON:        return KEY_COLON;
    case SDLK_SEMICOLON:    return KEY_SEMICOLON;
    case SDLK_EQUALS:       return KEY_EQUALS;
    case SDLK_LEFTBRACKET:  return KEY_OPENBRACE;
    case SDLK_BACKSLASH:    return KEY_BACKSLASH;
    case SDLK_RIGHTBRACKET: return KEY_CLOSEBRACE;
    case SDLK_BACKQUOTE:    return KEY_TILDE;
    case SDLK_a:            return KEY_A;
    case SDLK_b:            return KEY_B;
    case SDLK_c:            return KEY_C;
    case SDLK_d:            return KEY_D;
    case SDLK_e:            return KEY_E;
    case SDLK_f:            return KEY_F;
    case SDLK_g:            return KEY_G;
    case SDLK_h:            return KEY_H;
    case SDLK_i:            return KEY_I;
    case SDLK_j:            return KEY_J;
    case SDLK_k:            return KEY_K;
    case SDLK_l:            return KEY_L;
    case SDLK_m:            return KEY_M;
    case SDLK_n:            return KEY_N;
    case SDLK_o:            return KEY_O;
    case SDLK_p:            return KEY_P;
    case SDLK_q:            return KEY_Q;
    case SDLK_r:            return KEY_R;
    case SDLK_s:            return KEY_S;
    case SDLK_t:            return KEY_T;
    case SDLK_u:            return KEY_U;
    case SDLK_v:            return KEY_V;
    case SDLK_w:            return KEY_W;
    case SDLK_x:            return KEY_X;
    case SDLK_y:            return KEY_Y;
    case SDLK_z:            return KEY_Z;
    case SDLK_DELETE:       return KEY_DEL;
    case SDLK_KP0:          return KEY_0_PAD;
    case SDLK_KP1:          return KEY_1_PAD;
    case SDLK_KP2:          return KEY_2_PAD;
    case SDLK_KP3:          return KEY_3_PAD;
    case SDLK_KP4:          return KEY_4_PAD;
    case SDLK_KP5:          return KEY_5_PAD;
    case SDLK_KP6:          return KEY_6_PAD;
    case SDLK_KP7:          return KEY_7_PAD;
    case SDLK_KP8:          return KEY_8_PAD;
    case SDLK_KP9:          return KEY_9_PAD;
    case SDLK_KP_PERIOD:    return KEY_DEL_PAD;
    case SDLK_KP_DIVIDE:    return KEY_SLASH_PAD;
    case SDLK_KP_MULTIPLY:  return KEY_ASTERISK;
    case SDLK_KP_MINUS:     return KEY_MINUS_PAD;
    case SDLK_KP_PLUS:      return KEY_PLUS_PAD;
    case SDLK_KP_ENTER:     return KEY_ENTER_PAD;
    case SDLK_KP_EQUALS:    return KEY_EQUALS_PAD;
    case SDLK_UP:           return KEY_UP;
    case SDLK_DOWN:         return KEY_DOWN;
    case SDLK_RIGHT:        return KEY_RIGHT;
    case SDLK_LEFT:         return KEY_LEFT;
    case SDLK_INSERT:       return KEY_INSERT;
    case SDLK_HOME:         return KEY_HOME;
    case SDLK_END:          return KEY_END;
    case SDLK_PAGEUP:       return KEY_PGUP;
    case SDLK_PAGEDOWN:     return KEY_PGDN;
    case SDLK_F1:           return KEY_F1;
    case SDLK_F2:           return KEY_F2;
    case SDLK_F3:           return KEY_F3;
    case SDLK_F4:           return KEY_F4;
    case SDLK_F5:           return KEY_F5;
    case SDLK_F6:           return KEY_F6;
    case SDLK_F7:           return KEY_F7;
    case SDLK_F8:           return KEY_F8;
    case SDLK_F9:           return KEY_F9;
    case SDLK_F10:          return KEY_F10;
    case SDLK_F11:          return KEY_F11;
    case SDLK_F12:          return KEY_F12;
    case SDLK_NUMLOCK:      return KEY_NUMLOCK;
    case SDLK_CAPSLOCK:     return KEY_CAPSLOCK;
    case SDLK_SCROLLOCK:    return KEY_SCRLOCK;
    case SDLK_RSHIFT:       return KEY_RSHIFT;
    case SDLK_LSHIFT:       return KEY_LSHIFT;
    case SDLK_RCTRL:        return KEY_RCONTROL;
    case SDLK_LCTRL:        return KEY_LCONTROL;
    case SDLK_RALT:         return KEY_ALT;
    case SDLK_LALT:         return KEY_ALT;
    case SDLK_LSUPER:       return KEY_LWIN;
    case SDLK_RSUPER:       return KEY_RWIN;
    case SDLK_PRINT:        return KEY_PRTSCR;
    default:                return -1;
  }
}

static int _scancode_allegro_to_sdl(int allegro_scancode) {
  switch (allegro_scancode) {
    case KEY_A:             return SDLK_a;
    case KEY_B:             return SDLK_b;
    case KEY_C:             return SDLK_c;
    case KEY_D:             return SDLK_d;
    case KEY_E:             return SDLK_e;
    case KEY_F:             return SDLK_f;
    case KEY_G:             return SDLK_g;
    case KEY_H:             return SDLK_h;
    case KEY_I:             return SDLK_i;
    case KEY_J:             return SDLK_j;
    case KEY_K:             return SDLK_k;
    case KEY_L:             return SDLK_l;
    case KEY_M:             return SDLK_m;
    case KEY_N:             return SDLK_n;
    case KEY_O:             return SDLK_o;
    case KEY_P:             return SDLK_p;
    case KEY_Q:             return SDLK_q;
    case KEY_R:             return SDLK_r;
    case KEY_S:             return SDLK_s;
    case KEY_T:             return SDLK_t;
    case KEY_U:             return SDLK_u;
    case KEY_V:             return SDLK_v;
    case KEY_W:             return SDLK_w;
    case KEY_X:             return SDLK_x;
    case KEY_Y:             return SDLK_y;
    case KEY_Z:             return SDLK_z;
    case KEY_0:             return SDLK_0;
    case KEY_1:             return SDLK_1;
    case KEY_2:             return SDLK_2;
    case KEY_3:             return SDLK_3;
    case KEY_4:             return SDLK_4;
    case KEY_5:             return SDLK_5;
    case KEY_6:             return SDLK_6;
    case KEY_7:             return SDLK_7;
    case KEY_8:             return SDLK_8;
    case KEY_9:             return SDLK_9;
    case KEY_0_PAD:         return SDLK_KP0;
    case KEY_1_PAD:         return SDLK_KP1;
    case KEY_2_PAD:         return SDLK_KP2;
    case KEY_3_PAD:         return SDLK_KP3;
    case KEY_4_PAD:         return SDLK_KP4;
    case KEY_5_PAD:         return SDLK_KP5;
    case KEY_6_PAD:         return SDLK_KP6;
    case KEY_7_PAD:         return SDLK_KP7;
    case KEY_8_PAD:         return SDLK_KP8;
    case KEY_9_PAD:         return SDLK_KP9;
    case KEY_F1:            return SDLK_F1;
    case KEY_F2:            return SDLK_F2;
    case KEY_F3:            return SDLK_F3;
    case KEY_F4:            return SDLK_F4;
    case KEY_F5:            return SDLK_F5;
    case KEY_F6:            return SDLK_F6;
    case KEY_F7:            return SDLK_F7;
    case KEY_F8:            return SDLK_F8;
    case KEY_F9:            return SDLK_F9;
    case KEY_F10:           return SDLK_F10;
    case KEY_F11:           return SDLK_F11;
    case KEY_F12:           return SDLK_F12;
    case KEY_ESC:           return SDLK_ESCAPE;
    case KEY_TILDE:         return SDLK_BACKQUOTE;
    case KEY_MINUS:         return SDLK_MINUS;
    case KEY_EQUALS:        return SDLK_EQUALS;
    case KEY_BACKSPACE:     return SDLK_BACKSPACE;
    case KEY_TAB:           return SDLK_TAB;
    case KEY_OPENBRACE:     return SDLK_LEFTBRACKET;
    case KEY_CLOSEBRACE:    return SDLK_RIGHTBRACKET;
    case KEY_ENTER:         return SDLK_RETURN;
    case KEY_COLON:         return SDLK_COLON;
    case KEY_QUOTE:         return SDLK_QUOTE;
    case KEY_BACKSLASH:     return SDLK_BACKSLASH;
    case KEY_COMMA:         return SDLK_COMMA;
    case KEY_STOP:          return SDLK_PERIOD;
    case KEY_SLASH:         return SDLK_SLASH;
    case KEY_SPACE:         return SDLK_SPACE;
    case KEY_INSERT:        return SDLK_INSERT;
    case KEY_DEL:           return SDLK_DELETE;
    case KEY_HOME:          return SDLK_HOME;
    case KEY_END:           return SDLK_END;
    case KEY_PGUP:          return SDLK_PAGEUP;
    case KEY_PGDN:          return SDLK_PAGEDOWN;
    case KEY_LEFT:          return SDLK_LEFT;
    case KEY_RIGHT:         return SDLK_RIGHT;
    case KEY_UP:            return SDLK_UP;
    case KEY_DOWN:          return SDLK_DOWN;
    case KEY_SLASH_PAD:     return SDLK_KP_DIVIDE;
    case KEY_ASTERISK:      return SDLK_KP_MULTIPLY;
    case KEY_MINUS_PAD:     return SDLK_KP_MINUS;
    case KEY_PLUS_PAD:      return SDLK_KP_PLUS;
    case KEY_DEL_PAD:       return SDLK_KP_PERIOD;
    case KEY_ENTER_PAD:     return SDLK_KP_ENTER;
    case KEY_PRTSCR:        return SDLK_PRINT;
    case KEY_PAUSE:         return SDLK_PAUSE;
    case KEY_EQUALS_PAD:    return SDLK_KP_EQUALS;
    case KEY_SEMICOLON:     return SDLK_SEMICOLON;
    case KEY_LSHIFT:        return SDLK_LSHIFT;
    case KEY_RSHIFT:        return SDLK_RSHIFT;
    case KEY_LCONTROL:      return SDLK_LCTRL;
    case KEY_RCONTROL:      return SDLK_RCTRL;
    case KEY_ALT:           return SDLK_LALT;
    //case KEY_ALT:           return SDLK_RALT;
    case KEY_LWIN:          return SDLK_LSUPER;
    case KEY_RWIN:          return SDLK_RSUPER;
    case KEY_SCRLOCK:       return SDLK_SCROLLOCK;
    case KEY_NUMLOCK:       return SDLK_NUMLOCK;
    case KEY_CAPSLOCK:      return SDLK_CAPSLOCK;
    default:                return -1;
  }
}

// TODO: do we still need this?
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


// TRANSPARENCY
// ============================================================================

// TODO: fix light table... and check where color_map is used.
// 256-color transparency
static ALW_COLOR_MAP *_alw_color_map;
void alw_create_light_table(ALW_COLOR_MAP *table, const ALW_PALETTE pal, int r, int g, int b, void (*callback)(int pos)) { PRINT_STUB; }
void alw_set_color_map(ALW_COLOR_MAP *alw_color_map) {_alw_color_map = alw_color_map;};
int alw_has_color_map(){return _alw_color_map != 0;}

extern unsigned long _blender_black(unsigned long x, unsigned long y, unsigned long n);
// truecolor transparency
// modes set before calling draw_trans_sprite, alw_draw_lit_sprite
//void alw_set_alpha_blender() { PRINT_STUB; }   // see colblend
//void alw_set_trans_blender(int r, int g, int b, int a) { PRINT_STUB; }   // see colblend
void alw_set_blender_mode (ALW_BLENDER_FUNC b15, ALW_BLENDER_FUNC b16, ALW_BLENDER_FUNC b24, int r, int g, int b, int a) 
{ 
  PRINT_STUB; 
  _blender_func15 = b15;
  _blender_func16 = b16;
  _blender_func24 = b24;
  _blender_func32 = b24;

  _blender_func15x = _blender_black;
  _blender_func16x = _blender_black;
  _blender_func24x = _blender_black;

  _blender_col_15 = alw_makecol15(r, g, b);
  _blender_col_16 = alw_makecol16(r, g, b);
  _blender_col_24 = alw_makecol24(r, g, b);
  _blender_col_32 = alw_makecol32(r, g, b);

  _blender_alpha = a;
}
void alw_set_blender_mode_ex(ALW_BLENDER_FUNC b15,ALW_BLENDER_FUNC b16,ALW_BLENDER_FUNC b24,ALW_BLENDER_FUNC b32,ALW_BLENDER_FUNC b15x,ALW_BLENDER_FUNC b16x,ALW_BLENDER_FUNC b24x, int r, int g,int b,int a) 
{ 
  PRINT_STUB; 
  _blender_func15 = b15;
  _blender_func16 = b16;
  _blender_func24 = b24;
  _blender_func32 = b32;

  _blender_func15x = b15x;
  _blender_func16x = b16x;
  _blender_func24x = b24x;

  _blender_col_15 = alw_makecol15(r, g, b);
  _blender_col_16 = alw_makecol16(r, g, b);
  _blender_col_24 = alw_makecol24(r, g, b);
  _blender_col_32 = alw_makecol32(r, g, b);

  _blender_alpha = a;
}

ALW_BLENDER_FUNC _blender_func15 = 0;   /* truecolor pixel blender routines */
ALW_BLENDER_FUNC _blender_func16 = 0;
ALW_BLENDER_FUNC _blender_func24 = 0;
ALW_BLENDER_FUNC _blender_func32 = 0;

ALW_BLENDER_FUNC _blender_func15x = 0;
ALW_BLENDER_FUNC _blender_func16x = 0;
ALW_BLENDER_FUNC _blender_func24x = 0;

int _blender_col_15 = 0;               /* for truecolor lit sprites */
int _blender_col_16 = 0;
int _blender_col_24 = 0;
int _blender_col_32 = 0;

int _blender_alpha = 0;                /* for truecolor translucent drawing */

// SOUND INIT
// ============================================================================

//static int _snd_digi_voices = -1;
//static int _snd_midi_voices = -1;

// TODO: create sound manager that runs in seperate thread

static ALCdevice* _openal_device;
static ALCcontext* _openal_context;

static ALuint _openal_new_source() {
  ALuint source;
  alGenSources(1, &source);
  CheckALError("Couldn't generate source");
  alSourcef(source, AL_GAIN, AL_MAX_GAIN);
  CheckALError("Couldn't set max gain on source");
  return source;
}

void alw_reserve_voices(int digi_voices, int midi_voices)
{ 
  PRINT_STUB; 
  //_snd_digi_voices = digi_voices;
  //_snd_midi_voices = midi_voices;
}
int alw_install_sound(int digi, int midi, const char *cfg_path)
{ 
  // ignore these.
  (void)digi;
  (void)midi;
  (void)cfg_path;
  
  PRINT_STUB; 

	// set up OpenAL buffers
  _openal_device = alcOpenDevice(NULL);
	CheckALError ("Couldn't open AL device"); // default device
	_openal_context = alcCreateContext(_openal_device, 0);
	CheckALError ("Couldn't open AL context");
	alcMakeContextCurrent (_openal_context);
	CheckALError ("Couldn't make AL context current");
  
  return 0;
}

void alw_remove_sound()
{ 
  PRINT_STUB; 
  
  if (_openal_context != 0){
    alcDestroyContext(_openal_context);
    _openal_context = 0;
  }
  
  if (_openal_device != 0){
    alcCloseDevice(_openal_device);
    _openal_device = 0;
  }
}
void alw_set_volume(int digi_volume, int midi_volume){ PRINT_STUB; }
void alw_set_volume_per_voice(int scale){ PRINT_STUB; }


// DIGIAL AUDIO
// ============================================================================


AlwSample::AlwSample() {
    source = 0;
  }
  
AlwSample::~AlwSample() {
  PRINT_STUB;
  alSourceStop(source);
  CheckALError("Couldn't stop source");
  alSourcei(source, AL_BUFFER, NULL);  // unqueue all buffers
  CheckALError("Couldn't unqueue all buffers from source");
  
  for(std::vector<ALuint>::iterator it = buffers.begin(); it != buffers.end(); ++it) {
    ALuint bid = *it;
    alDeleteBuffers(1, &bid);
    CheckALError("Couldn't delete buffer");
  }
  buffers.clear();
  alDeleteSources(1, &source);
  CheckALError("Couldn't delete source");
  source = 0;
}

int AlwSample::play(int vol, int pan, int freq, int loop) {
  
  alGenSources(1, &source);
  CheckALError("Couldn't generate source");
  
  for(std::vector<ALuint>::iterator it = buffers.begin(); it != buffers.end(); ++it) {
    ALuint bid = *it;
    alSourceQueueBuffers(source, 1, &bid);
    CheckALError("Couldn't enqueue buffer");
  }

  set_volume(vol);
  set_pan(pan);
  set_loop(loop);
  
  alSourcePlay(source);
  CheckALError("Couldn't play source");
  
  return 1; //?
}

int AlwSample::stop() {
  alSourceStop(source);
  CheckALError("Couldn't stop source");
  alSourceRewind(source);
  CheckALError("Couldn't rewing source");
  return 0;
}

int AlwSample::pause() {
  alSourceStop(source);
  CheckALError("Couldn't stop source");
  return 0;
}

int AlwSample::resume() {
  alSourcePlay(source);
  CheckALError("Couldn't play source");
  return 0;
}


void AlwSample::set_position(int pos) {
  alSourcei(source, AL_SAMPLE_OFFSET, pos);
  CheckALError("Couldn't set source position");

}

void AlwSample::set_volume(int vol) {
  ALfloat newVolume = vol/255.0f;
  alSourcef(source, AL_GAIN, newVolume);
  CheckALError ("Couldn't set volume");
}

void AlwSample::set_pan(int pan) {
  // TODO: implement panning
}

void AlwSample::set_loop(int isloop) {
  alSourcei(source, AL_LOOPING, isloop?AL_TRUE:AL_FALSE);
  CheckALError("Couldn't set source looping");
  
}

double AlwSample::get_position_ms() {
  ALfloat secOffset;
  alGetSourcef(source, AL_SEC_OFFSET, &secOffset);
  CheckALError("Couldn't get source time offset");
  return secOffset * 1000.0;
}

int AlwSample::get_position() {
  ALint sampleOffset;
  alGetSourcei(source, AL_SAMPLE_OFFSET,&sampleOffset);
  CheckALError("Couldn't get source sample offset");
  return sampleOffset;
}


double AlwSample::get_length_ms() {
  double length_s = 0.0;
  
  for(std::vector<ALuint>::iterator it = buffers.begin(); it != buffers.end(); ++it) {
    ALuint bid = *it;
   
    ALint size, bits, channels, freq;
    
    alGetBufferi(bid, AL_SIZE, &size);
    CheckALError("Couldn't get buffer size");
    alGetBufferi(bid, AL_BITS, &bits);
    CheckALError("Couldn't get buffer bits");
    alGetBufferi(bid, AL_CHANNELS, &channels);
    CheckALError("Couldn't get buffer channels");
    alGetBufferi(bid, AL_FREQUENCY, &freq);
    CheckALError("Couldn't get buffer frequency");
    
    length_s += (double)size / channels / (bits/8) / freq;
  }
  
  return length_s * 1000.0;
}

int AlwSample::is_done() {
  ALint srcstate;
  alGetSourcei(source, AL_SOURCE_STATE, &srcstate);
  CheckALError("Couldn't get source state");
  CheckALError ("Couldn't get state");
  return srcstate != AL_PLAYING;
}


AlwSample *alw_load_sample(const char *filename){   //***
  PRINT_STUB; 
  AlwSample *sample = new AlwSample();
  
  if (CreateOpenAlBuffersFromSample(filename, sample->buffers) != 0)
    return 0;
  
  return sample;
}


// MIDI
// ============================================================================

// TODO implement midi playing

volatile long alw_midi_pos = 0;

ALW_MIDI *alw_load_midi(const char *filename){ PRINT_STUB; return 0;}
void alw_destroy_midi(ALW_MIDI *midi){ PRINT_STUB; }
int alw_get_midi_length(ALW_MIDI *midi) { PRINT_STUB; return 0;}
int alw_play_midi(ALW_MIDI *midi, int loop) { PRINT_STUB; return -1;}
void alw_stop_midi(){ PRINT_STUB; }
void alw_midi_pause(){ PRINT_STUB; }
void alw_midi_resume(){ PRINT_STUB; }
int alw_midi_seek(int target) { PRINT_STUB; return 0;}

// AUDIO STREAM
// ============================================================================

void alw_stop_audio_stream(ALW_AUDIOSTREAM *stream){ PRINT_STUB; }


// COLOR FORMATS
// ============================================================================

// TODO: fix rgb maps

//ALW_RGB_MAP *alw_rgb_map;
void alw_set_rgb_map(ALW_RGB_MAP *rgb_map) { PRINT_STUB; }
//void alw_rgb_to_hsv(int r, int g, int b, float h, float *s, float *v){ PRINT_STUB; }
//void alw_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b){ PRINT_STUB; }
void alw_create_rgb_table(ALW_RGB_MAP *table, const ALW_PALETTE pal, void (*callback)(int pos)){ PRINT_STUB; }


// WINALLEG
// ============================================================================
#ifdef WINDOWS_VERSION
HWND alw_win_get_window(void){ PRINT_STUB;  return 0;}
#endif
int alw_wnd_call_proc(int (*proc)(void)){ PRINT_STUB;  return 0;}



// FLI
// ============================================================================

// TODO: FLI support

int alw_play_fli(const char *filename, ALW_BITMAP *bmp, int loop, int (*callback)()){ PRINT_STUB; return ALW_FLI_OK;}


// IMAGE FILES
// ============================================================================

// TODO: save/load bitmaps, color conversions...

int alw_save_bitmap(const char *filename, ALW_BITMAP *bmp, const ALW_RGB *pal){ PRINT_STUB; return 0;}
ALW_BITMAP *alw_load_bitmap(const char *filename, ALW_RGB *pal){ PRINT_STUB; return 0;}
ALW_BITMAP *alw_load_pcx(const char *filename, ALW_RGB *pal) { PRINT_STUB; return 0;}


static int _color_conv;
void alw_set_color_conversion(int mode) 
{ 
  PRINT_STUB;
  _color_conv = mode;
}
int alw_get_color_conversion() {
  return _color_conv;
}



// UNICODE
// ============================================================================

// TODO: how to support unicode...

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



// MISC
// ============================================================================

// TODO: find where _get_vtable used.. find why used.
extern "C" {
	ALW_GFX_VTABLE *_get_vtable(int color_depth){ PRINT_STUB; return 0;}
}

#ifdef WINDOWS_VERSION
static HWND _allegro_wnd = NULL;
void alw_set_allegro_wnd(HWND allegro_wnd) {_allegro_wnd = allegro_wnd;}
HWND alw_get_allegro_wnd() {return _allegro_wnd;}

extern "C" LPDIRECTDRAW2 alw_get_directdraw() { PRINT_STUB; return 0;}
extern "C" LPDIRECTSOUND alw_get_directsound(){ PRINT_STUB; return 0;}
extern "C" DDRAW_SURFACE *alw_get_gfx_directx_primary_surface(){ PRINT_STUB; return 0;}
#endif

ALW_BITMAP *alw_gfx_directx_create_system_bitmap(int width, int height){ PRINT_STUB; return 0;}

// FONTS
// ============================================================================

#if 0
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
#endif

// CDLIB
// ============================================================================

// TODO: what to do with cd playing?

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

// TODO: support dumb

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

static const int _ALOGG_BUFFER_SIZE = 16*1024;
static const int _ALOGG_NUM_BUFFER = 8;

struct ogg_static_buffer {
  void *data;
  int pos;
  int data_len;
};

struct ALOGG_OGG {
  // openal
   ALuint source;
  ALuint buffers[_ALOGG_BUFFER_SIZE];
  int buffers_init;
  
  // ogg
  int past_byte_count;
  
  // pcmgen
  IStaticPcmGenerator *pcmgen;
};


// xtern: my_load_static_ogg
ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len)
{ 
  ALuint source = 0;
  ALOGG_OGG *alogg = 0;
  
  PRINT_STUB; 

  source = _openal_new_source();
  if (source == 0) return 0;
  
  alogg = allocmem<ALOGG_OGG>();
  
  alogg->source = source;
  alogg->pcmgen = pcmgen_from_ogg_buffer(data, data_len);
  alogg->buffers_init = 0;
    
  return alogg;
}

// xtern: MYSTATICOGG:Destroy
void alogg_destroy_ogg(ALOGG_OGG *alogg)
{ 
  PRINT_STUB;
  
  alogg_stop_ogg(alogg);
  
  // release
  alogg->pcmgen->Close();
  
  alSourcei(alogg->source, AL_BUFFER, NULL);
  CheckALError ("Couldn't remove buffers from source.");
  
  for(int i=0; i < _ALOGG_NUM_BUFFER; i++) {
    if (alIsBuffer(alogg->buffers[i])) {
      alSourcei(alogg->source, AL_BUFFER, NULL);
      alDeleteBuffers(1, &alogg->buffers[i]);
      CheckALError ("Couldn't delete buffer");
    }
  }
  
  if (alIsSource(alogg->source)) {
    alDeleteSources(1, &alogg->source);
    CheckALError ("Couldn't delete source");
  }
  
  if (alogg != 0) {
    free(alogg);
  }
}

// xtern: MYSTATICOGG:play_from, Restart
int alogg_play_ex_ogg(ALOGG_OGG *alogg, int buffer_len, int vol, int pan, int speed, int loop)
{ 
  // start playing ogg with these parameters
  PRINT_STUB; 
  
  if (alogg_is_playing_ogg(alogg))
    return ALOGG_OK;
  
  // set is looping first, so that we can loop while prefilling buffers
  alogg_adjust_ogg(alogg, vol, pan, speed, loop);
  
  if (!alogg->buffers_init) {
    // only one buffer cause its static.
    //ALuint buffer;
    alGenBuffers(_ALOGG_NUM_BUFFER, alogg->buffers);
    CheckALError ("Couldn't generate buffer");
    FillBufferResult result;
    int filledBuffers = 0;
    for (int i=0; i<_ALOGG_NUM_BUFFER; i++) {
      // error or stopped early.
       result = alogg->pcmgen->FillOpenAlBuffer(alogg->buffers[i]);
      
      if (result == FBR_EOF)
        break;
      if (result == FBR_FAILURE)
        return -1;
      
      filledBuffers += 1;
    }
    
    if (filledBuffers <= 0) {
      printf("no buffers!\n");
      alDeleteBuffers(_ALOGG_NUM_BUFFER, alogg->buffers);
      CheckALError ("Couldn't delete buffers");
      return -1;
    }
    
    // queue up the buffers on the source
    alSourceQueueBuffers(alogg->source, filledBuffers, alogg->buffers);
    CheckALError ("Couldn't add buffers to queue");
    alogg->buffers_init = 1;
  }
 
  alSourcePlay(alogg->source);
	CheckALError ("Couldn't play");
  
  return 0;
}

// xtern:MYSTATICOGG: destroy, seek, restart
void alogg_stop_ogg(ALOGG_OGG *ogg)
{
  // stop playing
  alSourceStop(ogg->source);
  CheckALError ("Couldn't stop source");
  PRINT_STUB;
}

// xtern: MYSTATICOGG:Poll
int alogg_poll_ogg(ALOGG_OGG *alogg)
{ 
  // poll, do work, return 1 if finished.
	ALint processed;
	alGetSourcei (alogg->source, AL_BUFFERS_PROCESSED, &processed);
	CheckALError ("couldn't get al_buffers_processed");
	
  int added_buffers = 0;
  
	while (processed > 0) {
    
		ALuint freeBuffer;
		alSourceUnqueueBuffers(alogg->source, 1, &freeBuffer);
		CheckALError("couldn't unqueue buffer");
    
    ALint bytesdone = 0;
    alGetBufferi(freeBuffer, AL_SIZE, &bytesdone);
    alogg->past_byte_count += bytesdone;
    
    FillBufferResult result = alogg->pcmgen->FillOpenAlBuffer(freeBuffer);
    if (result != FBR_OK)
      break;
    
		alSourceQueueBuffers(alogg->source, 1, &freeBuffer);
		CheckALError ("couldn't queue refilled buffer");

    added_buffers = 1;
		processed--;
	}
  
  ALint srcstate;
  alGetSourcei(alogg->source, AL_SOURCE_STATE, &srcstate);
  CheckALError ("Couldn't get state");
  if (srcstate != AL_PLAYING) {
    
    // TODO: check that we've buffered the last of the ogg.
    // or if its on repeat.
    
    if (added_buffers) {
      //	printf("replaying\n");
      alSourcePlay (alogg->source);
      CheckALError ("Couldn't play");
    } else {
      //printf ("stopping?\n");
    }
  }
	
  return alogg_is_playing_ogg(alogg) ? 0: ALOGG_POLL_PLAYJUSTFINISHED;
}


// extern: MYSTATICOGG: set_volume
void alogg_adjust_ogg(ALOGG_OGG *ogg, int vol, int pan, int speed, int loop)
{ 
  ALfloat newVolume = vol/255.0f;
  alSourcef(ogg->source, AL_GAIN, newVolume);
  CheckALError ("Couldn't set volume");
  
  ogg->pcmgen->SetLoopMode(loop != 0);
  
  // adjust vol, speed, etc.
  PRINT_STUB;
}


// extern: MYSTATICOGG: Restart
void alogg_rewind_ogg(ALOGG_OGG *alogg)
{
  // jump back to start. replay
  PRINT_STUB;
  alogg->pcmgen->SeekMs(0.0);
}

// xtern: MYSTATICOGG:play_from
void alogg_seek_abs_msecs_ogg(ALOGG_OGG *alogg, int msecs) {
  PRINT_STUB;
  alogg->pcmgen->SeekMs((double)msecs);
}


// xtern: MYSTATICOGG: get_length_ms
int alogg_get_length_msecs_ogg(ALOGG_OGG *alogg)
{
  PRINT_STUB;
  return (int)alogg->pcmgen->GetTotalMs();
}


// xtern: MYSTATICOGG: get_pos_ms
int alogg_get_pos_msecs_ogg(ALOGG_OGG *alogg)
{
  PRINT_STUB;
  
  ALint bytesoffset;
  alGetSourcei( alogg->source, AL_BYTE_OFFSET, &bytesoffset );
  CheckALError ("Couldn't get offset.");
 
  int bytes_processed = alogg->past_byte_count + bytesoffset;
  double time = alogg->pcmgen->BytesPosInMs(bytes_processed);
  return (int)time;
}


// xtern: MYSTATICOGG: get_pos_ms
int alogg_is_playing_ogg(ALOGG_OGG *alogg)
{
  // is the ogg currently playing?
  // seems to get other stuff called to calculate position.
  PRINT_STUB; 
  
  ALint srcstate;
  alGetSourcei(alogg->source, AL_SOURCE_STATE, &srcstate);
  CheckALError ("Couldn't get state");
  
  if (srcstate == AL_INITIAL)
    return 0;

  int playing = (srcstate == AL_PLAYING) || !alogg->pcmgen->HasDecodeEof();
  
  return playing;
}
// xtern: MYSTATICOGG: get_pos_ms, get_voice
ALW_AUDIOSTREAM *alogg_get_audiostream_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}





ALOGG_OGGSTREAM *alogg_create_oggstream_from_packfile(ALW_PACKFILE *packfile) {
  ALuint source = 0;
  ALOGG_OGG *alogg = 0;
  
  PRINT_STUB; 
  
  source = _openal_new_source();
  if (source == 0) return 0;
  
  alogg = allocmem<ALOGG_OGG>();
  
  alogg->source = source;
  alogg->pcmgen = pcmgen_from_ogg_packfile(packfile);
  
  return alogg;
}

void alogg_destroy_oggstream(ALOGG_OGGSTREAM *ogg)
{ 
  PRINT_STUB;
  alogg_destroy_ogg(ogg);
}
int alogg_play_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan)
{ 
  PRINT_STUB; 
  return alogg_play_ex_ogg(ogg, 0, vol, pan, 1000, 0);
}
void alogg_stop_oggstream(ALOGG_OGGSTREAM *ogg)
{ 
  PRINT_STUB;
  alogg_stop_ogg(ogg);
}
void alogg_adjust_oggstream(ALOGG_OGGSTREAM *ogg, int vol, int pan, int speed)
{
  PRINT_STUB;
  alogg_adjust_ogg(ogg, vol, pan, speed, 0);
}
int alogg_poll_oggstream(ALOGG_OGGSTREAM *ogg)
{ 
  PRINT_STUB; 
  return alogg_poll_ogg(ogg);
}
int alogg_get_pos_msecs_oggstream(ALOGG_OGGSTREAM *ogg)
{ 
  PRINT_STUB; 
  return alogg_get_pos_msecs_ogg(ogg);
}
int alogg_is_playing_oggstream(ALOGG_OGGSTREAM *ogg) 
{ 
  PRINT_STUB; 
  return alogg_is_playing_ogg(ogg);
}
ALW_AUDIOSTREAM *alogg_get_audiostream_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}



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


// DIRECT ACCESS
// ============================================================================

unsigned long alw_bmp_write_line(ALW_BITMAP *bmp, int line) {
  PRINT_STUB;
  return (unsigned long)bmp->line[line];
}
unsigned long alw_bmp_read_line(ALW_BITMAP *bmp, int line) {
  PRINT_STUB;
  return (unsigned long)bmp->line[line];
}

void alw_bmp_unwrite_line(ALW_BITMAP *bmp) { PRINT_STUB; }
void alw_bmp_select(ALW_BITMAP *bmp) { PRINT_STUB; }


int bmp_read24 (uintptr_t addr)
{
  unsigned char *p = (unsigned char *)addr;
  int c;

  c = READ3BYTES(p);

  return c;
}

void bmp_write24 (uintptr_t addr, int c)
{
  unsigned char *p = (unsigned char *)addr;

  WRITE3BYTES(p, c);
}


// OSX
// ============================================================================
int osx_sys_question(const char *m, const char*b1, const char*b2) {
    return 0;
}