

#include "allegro.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <SDL_ttf.h>


char _debug_str[10000];

//#define PRINT_STUB sprintf(_debug_str, "STUB %s:%d %s\n", __FILE__, __LINE__, __FUNCSIG__); OutputDebugString(_debug_str)
#define PRINT_STUB

// INIT
// ============================================================================

char alw_allegro_error[ALW_ALLEGRO_ERROR_SIZE];

static void (*_on_close_callback)(void) = 0;

int alw_allegro_init() { 
  int result =  SDL_Init(SDL_INIT_EVERYTHING);
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

static int _colour_depth = 0;

void alw_set_color_depth(int depth) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
	_colour_depth = depth;
}

GFX_VTABLE _default_vtable = {0};

static ALW_BITMAP *wrap_sdl_surface(SDL_Surface *surf) {
	ALW_BITMAP *x = (ALW_BITMAP *)malloc(sizeof(ALW_BITMAP));
	x->surf = surf;
	x->w = surf->w;
	x->h = surf->h;

  x->vtable = (ALW_GFX_VTABLE *)malloc(sizeof(ALW_GFX_VTABLE));
  x->vtable->mask_color = 0;

  x->clip = 1;
  x->cl = 0;
  x->ct = 0;
  x->cr = surf->w;
  x->cb = surf->h;

	x->line = (unsigned char **)malloc(x->h * sizeof(unsigned char*));
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
  SDL_Surface *surf = bmp->surf;

  switch (surf->format->BitsPerPixel) {
    case 8:
      SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_8);
      bmp->vtable->mask_color = MASK_COLOR_8;
      break;
    case 15:
      SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_15);
      bmp->vtable->mask_color =MASK_COLOR_15;
      break;
    case 16:
      SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_16);
      bmp->vtable->mask_color =MASK_COLOR_16;
      break;
    case 24:
      SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_24);
      bmp->vtable->mask_color =MASK_COLOR_24;
      break;
    case 32:
      SDL_SetColorKey(surf, SDL_SRCCOLORKEY, MASK_COLOR_32);
      bmp->vtable->mask_color =MASK_COLOR_32;
      break;
  }
}

ALW_BITMAP *alw_create_bitmap_ex(int color_depth, int width, int height) {
	// depth (8, 15, 16, 24 or 32 bits per pixel)
	SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, color_depth, 0,0,0,0);
  _add_palette_to_surface(surf);
	ALW_BITMAP *bmp = wrap_sdl_surface(surf);
  _bmp_set_color_key(bmp);
  return bmp;
}



ALW_BITMAP *alw_create_bitmap(int width, int height) {
  return alw_create_bitmap_ex(_colour_depth, width, height);

  //SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, _colour_depth, 0,0,0,0);
  //SDL_SetColorKey(surf, SDL_SRCCOLORKEY, 0);
  //_add_palette_to_surface(surf);
  //return wrap_sdl_surface(surf);
}

ALW_BITMAP *alw_create_sub_bitmap(ALW_BITMAP *parent, int x, int y, int width, int height) {

	// from http://lists.libsdl.org/pipermail/sdl-libsdl.org/2005-June/050556.html

	SDL_Surface *pParent = parent->surf;
	int nTop = y;
	int nLeft = x;
	int nWidth = width;
	int nHeight = height;

	SDL_LockSurface(pParent);

	uint8_t* pPixels = (uint8_t*)pParent->pixels + pParent->pitch*nTop + pParent->format->BytesPerPixel * nLeft;

	SDL_Surface *m_pSurface = SDL_CreateRGBSurfaceFrom(pPixels, nWidth, nHeight, pParent->format->BitsPerPixel, pParent->pitch, pParent->format->Rmask, pParent->format->Gmask, pParent->format->Bmask, pParent->format->Amask);

	if (pParent->format->palette)
		SDL_SetColors(m_pSurface, pParent->format->palette->colors, 0, pParent->format->palette->ncolors);


	SDL_UnlockSurface(pParent);

	ALW_BITMAP *bmp =wrap_sdl_surface(m_pSurface);
  _bmp_set_color_key(bmp);
  return bmp;
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
	PRINT_STUB;
  return bmp->vtable->mask_color;
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

  bitmap->clip = 1;
  bitmap->cl = x1;
  bitmap->cr = x2;
  bitmap->ct = y1;
  bitmap->cb = y2;
}
void alw_set_clip_state(ALW_BITMAP *bitmap, int state) {
	PRINT_STUB;
	// i think the default is always clip:
}



// GRAPHICS MODES
// ============================================================================

static SDL_Surface *_actual_sdl_screen;

//void alw_set_color_depth(int depth) { PRINT_STUB;}
int alw_set_gfx_mode(int card, int w, int h, int v_w, int v_h){
	assert(v_w == 0);
	assert(v_h == 0);

	_actual_sdl_screen = SDL_SetVideoMode(w, h, _colour_depth, SDL_SWSURFACE|SDL_HWPALETTE);
	if (_actual_sdl_screen == NULL)
		return 1;

	alw_screen = wrap_sdl_surface(_actual_sdl_screen);

	return 0;
}

ALW_GFX_MODE_LIST *alw_get_gfx_mode_list(int card){ PRINT_STUB; return 0;}
void alw_destroy_gfx_mode_list(ALW_GFX_MODE_LIST *mode_list) { PRINT_STUB;}

int alw_set_display_switch_mode(int mode) { PRINT_STUB; return 0;}
int alw_set_display_switch_callback(int dir, void (*cb)()){ PRINT_STUB; return 0;}

void alw_vsync(){
	SDL_Flip(_actual_sdl_screen);
}


// ALW_PALETTE
// ============================================================================

// we might be able to get away with no setting palettes yet.

ALW_PALETTE alw_black_palette;

void alw_set_palette(const ALW_PALETTE p) { PRINT_STUB; }
void alw_get_palette(ALW_PALETTE p) { PRINT_STUB; }

void alw_set_palette_range(const ALW_PALETTE p, int from, int to, int vsync) {
  SDL_Surface *surface = _actual_sdl_screen;
  int flags = SDL_PHYSPAL;
  int firstcolor = from;
  int ncolors = to-from+1;

  SDL_Color *colors = (SDL_Color *)malloc(sizeof(SDL_Color)*ncolors);
  for (int i = 0; i< ncolors; i++) {
    ALW_RGB alcol = p[from+i];
    SDL_Color sdlcol = {alcol.r*4, alcol.g*4, alcol.b*4};
    colors[i] = sdlcol;
  }

  SDL_SetPalette(surface, flags, colors, firstcolor, ncolors);
  
  if (vsync)
    SDL_Flip(_actual_sdl_screen);
}
void alw_get_palette_range(ALW_PALETTE p, int from, int to) { PRINT_STUB; }

void alw_fade_interpolate(const ALW_PALETTE source, const ALW_PALETTE dest, ALW_PALETTE output, int pos, int from, int to) {PRINT_STUB; }

void alw_select_palette(const ALW_PALETTE p) { PRINT_STUB; }
void alw_unselect_palette() { PRINT_STUB; }


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

	char *tmp = (char*)malloc(pathlen + 1);
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

void _linear_draw_sprite8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite8(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite15(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite16(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite24(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);

void _linear_draw_sprite32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_v_flip32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_h_flip32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_sprite_vh_flip32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_trans_sprite32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy);
void _linear_draw_lit_sprite32(ALW_BITMAP *dst, ALW_BITMAP *src, int dx, int dy, int color);


unsigned char _my_blit_col = 16;

void alw_blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) {
	PRINT_STUB; 
	SDL_Surface *src = source->surf;
	SDL_Surface *dst = dest->surf;
	//dst = _actual_sdl_screen;

	void *srcpixels = source->surf->pixels;
	void *destpixels = dest->surf->pixels;

	SDL_Rect srcrect = {source_x, source_y, width, height};
	SDL_Rect dstrect = {dest_x, dest_y, width, height};

  // alw_blit actually ignores alpha
  int origalphaflag = src->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
  int origalpha = src->format->alpha;
  int origcolkeyflag = src->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
  int origcolkey = src->format->colorkey;
  SDL_SetAlpha(src, 0, 255);
  SDL_SetColorKey(src, 0, 0);

  SDL_BlitSurface(src, &srcrect, dst, &dstrect);

  if (origalphaflag)
    SDL_SetAlpha(src, origalphaflag, origalpha);
  if (origcolkeyflag)
    SDL_SetColorKey(src, origcolkeyflag, origcolkey);

  //if (dest_x != 0 || dest_y != 0) {
//    SDL_FillRect(dst, &dstrect, _my_blit_col);
    //_my_blit_col = (_my_blit_col +1)% 40;
  //}

	//SDL_Flip(_actual_sdl_screen);

  //void;
}
void alw_draw_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;

  switch (sprite->surf->format->BitsPerPixel) {
    case 8:
      _linear_draw_sprite8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite32(bmp, sprite, x, y);
      break;
  }

  return;


  SDL_Surface *src = sprite->surf;
  SDL_Surface *dst = bmp->surf;
  //dst = _actual_sdl_screen;

  void *srcpixels = sprite->surf->pixels;
  void *destpixels = bmp->surf->pixels;

  SDL_Rect srcrect = {0, 0, sprite->w, sprite->h};
  SDL_Rect dstrect = {x, y, sprite->w, sprite->h};

  SDL_BlitSurface(src, &srcrect, dst, &dstrect);

  //SDL_Flip(_actual_sdl_screen);
}


void alw_draw_lit_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int color) { 
	PRINT_STUB;
  // we assume that bmp and sprite have same color depth

  switch (sprite->surf->format->BitsPerPixel) {
    case 8:
      _linear_draw_lit_sprite8(bmp, sprite, x, y, color);
      break;
    case 15:
      _linear_draw_lit_sprite15(bmp, sprite, x, y, color);
      break;
    case 16:
      _linear_draw_lit_sprite16(bmp, sprite, x, y, color);
      break;
    case 24:
      _linear_draw_lit_sprite24(bmp, sprite, x, y, color);
      break;
    case 32:
      _linear_draw_lit_sprite32(bmp, sprite, x, y, color);
      break;
  }
  //alw_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}
void alw_draw_trans_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;


  switch (sprite->surf->format->BitsPerPixel) {
    case 8:
      _linear_draw_trans_sprite8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_trans_sprite15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_trans_sprite16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_trans_sprite24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_trans_sprite32(bmp, sprite, x, y);
      break;
  }

  //alw_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}
void alw_draw_sprite_h_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  // we assume that bmp and sprite have same color depth
  switch (sprite->surf->format->BitsPerPixel) {
    case 8:
      _linear_draw_sprite_h_flip8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite_h_flip15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite_h_flip16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite_h_flip24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite_h_flip32(bmp, sprite, x, y);
      break;
  }
  //alw_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}
void alw_draw_sprite_v_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  // we assume that bmp and sprite have same color depth
  switch (sprite->surf->format->BitsPerPixel) {
    case 8:
      _linear_draw_sprite_v_flip8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite_v_flip15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite_v_flip16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite_v_flip24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite_v_flip32(bmp, sprite, x, y);
      break;
  }
	//alw_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}
void alw_draw_sprite_vh_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) { 
	PRINT_STUB;
  // we assume that bmp and sprite have same color depth
  switch (sprite->surf->format->BitsPerPixel) {
    case 8:
      _linear_draw_sprite_vh_flip8(bmp, sprite, x, y);
      break;
    case 15:
      _linear_draw_sprite_vh_flip15(bmp, sprite, x, y);
      break;
    case 16:
      _linear_draw_sprite_vh_flip16(bmp, sprite, x, y);
      break;
    case 24:
      _linear_draw_sprite_vh_flip24(bmp, sprite, x, y);
      break;
    case 32:
      _linear_draw_sprite_vh_flip32(bmp, sprite, x, y);
      break;
  }
  //alw_blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}
//void alw_stretch_blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height){ PRINT_STUB; }
//void alw_stretch_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int w, int h) { PRINT_STUB;   alw_blit(sprite, bmp, 0, 0, x, y, w, h); }
//void alw_rotate_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, alw_fixed angle) { PRINT_STUB; }
//void alw_pivot_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int cx, int cy, alw_fixed angle) { PRINT_STUB; }

// COLOURS
// ============================================================================

int alw_makecol_depth(int color_depth, int r, int g, int b) {
  PRINT_STUB; 
  return SDL_MapRGB(_actual_sdl_screen->format, r, g, b);
  return 0;
}
int alw_makeacol_depth(int color_depth, int r, int g, int b, int a) { PRINT_STUB; return 0;}
int alw_getr_depth(int color_depth, int c)
{ 
  PRINT_STUB;
  Uint8 r,g,b;
  SDL_GetRGB(c, _actual_sdl_screen->format, &r, &g, &b);
  return r;
}
int alw_getg_depth(int color_depth, int c)
{ 
  PRINT_STUB;
  Uint8 r,g,b;
  SDL_GetRGB(c, _actual_sdl_screen->format, &r, &g, &b);
  return g;
}
int alw_getb_depth(int color_depth, int c)
{ 
  PRINT_STUB;
  Uint8 r,g,b;
  SDL_GetRGB(c, _actual_sdl_screen->format, &r, &g, &b);
  return b;
}
int alw_geta_depth(int color_depth, int c)
{  
  PRINT_STUB;
  Uint8 r,g,b,a;
  SDL_GetRGBA(c, _actual_sdl_screen->format, &r, &g, &b, &a);
  return a;
}


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
//void alw_do_line(ALW_BITMAP *bmp, int x1, int y1,int x2,int y2, int d, void (*proc)(ALW_BITMAP *bmp, int x, int y, int d))  { PRINT_STUB; }
void alw_rect(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color)  { PRINT_STUB; }
extern void _soft_floodfill(ALW_BITMAP *bmp, int x, int y, int color);
void alw_floodfill(ALW_BITMAP *bmp, int x, int y, int color) {
  _soft_floodfill(bmp, x, y, color);
}
void alw_rectfill ( ALW_BITMAP *bmp, int x1, int y_1, int x2, int y2, int color)  { PRINT_STUB; }
void alw_triangle(ALW_BITMAP *bmp, int x1,int y1,int x2,int y2,int x3,int y3, int color)  { PRINT_STUB; }
void alw_circlefill(ALW_BITMAP *bmp, int x, int y, int radius, int color)  { PRINT_STUB; }

void alw_hfill(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color) {
  SDL_Rect dstrect = {dx1, dy, dx2-dx1+1, 1};
  SDL_FillRect(dst->surf, &dstrect, color);
}

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
  SDL_ShowCursor(SDL_DISABLE);
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

static void CheckALError (const char *operation) {
	ALenum alErr = alGetError();
	if (alErr == AL_NO_ERROR) return;
	const char *errFormat = NULL;
	switch (alErr) {
		case AL_INVALID_NAME: errFormat = "OpenAL Error: %s (AL_INVALID_NAME)"; break;
		case AL_INVALID_VALUE:  errFormat = "OpenAL Error: %s (AL_INVALID_VALUE)"; break;
		case AL_INVALID_ENUM:  errFormat = "OpenAL Error: %s (AL_INVALID_ENUM)"; break;
		case AL_INVALID_OPERATION: errFormat = "OpenAL Error: %s (AL_INVALID_OPERATION)"; break;
		case AL_OUT_OF_MEMORY: errFormat = "OpenAL Error: %s (AL_OUT_OF_MEMORY)"; break;
		default: errFormat = "OpenAL Error: %s (unknown error code)"; break;
	}
	fprintf (stderr, errFormat, operation);
	exit(1);
	
}

//static int _snd_digi_voices = -1;
//static int _snd_midi_voices = -1;


static ALCdevice* _openal_device;
static ALCcontext* _openal_context;
//#define _OPENAL_NUM_SOURCES (32)

//static int _openal_num_sources = -1;
//static   ALuint	*_openal_sources=0;

/*
static ALuint _openal_next_available_source() {
  for (int i = 0; i < _openal_num_sources; i++) {
    ALint srcType;
    alGetSourcei(_openal_sources[i], AL_SOURCE_TYPE, &srcType);
    CheckALError ("Couldn't read source type");
    if (srcType == AL_UNDETERMINED)
      return _openal_sources[i];
  }
  return 0; // none found.
}*/

static ALuint _openal_new_source() {
  ALuint source;
  alGenSources(1, &source);
  alSourcef(source, AL_GAIN, AL_MAX_GAIN);
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
  //_openal_num_sources = _OPENAL_NUM_SOURCES;

	// set up OpenAL buffers
  _openal_device = alcOpenDevice(NULL);
	CheckALError ("Couldn't open AL device"); // default device
	_openal_context = alcCreateContext(_openal_device, 0);
	CheckALError ("Couldn't open AL context");
	alcMakeContextCurrent (_openal_context);
	CheckALError ("Couldn't make AL context current");
  
	// set up streaming source
  /*
  _openal_sources = (ALuint*)malloc(_openal_num_sources*sizeof(ALuint));
	alGenSources(_openal_num_sources, _openal_sources);
	CheckALError ("Couldn't generate sources");
  
  for (int i = 0; i < _openal_num_sources; i++) {
    alSourcef(_openal_sources[i], AL_GAIN, AL_MAX_GAIN);
    CheckALError("Couldn't set source gain");
  }*/
  
  return 0;
}

void alw_remove_sound()
{ 
  PRINT_STUB; 
  
  /*
  if (_openal_sources != 0){
    alDeleteSources(_openal_num_sources, _openal_sources);
    free(_openal_sources);
    _openal_sources = 0;
  } */
  
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

// AUDIO STREAM
// ============================================================================

void alw_stop_audio_stream(ALW_AUDIOSTREAM *stream){ PRINT_STUB; }


// COLOR FORMATS
// ============================================================================

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

struct ogg_static_buffer {
  void *data;
  int pos;
  int data_len;
};

struct ALOGG_OGG {
  // openal
   ALuint source;
  ALuint buffer;
  
  // ogg
  OggVorbis_File vf;
   int current_section;
  int channels;
  ALenum format;
  long rate;
  
  // static buff
  ogg_static_buffer statbuf;
};


struct ALOGG_OGGSTREAM {} ;


size_t _bufread(void *ptr, size_t size, size_t count, ogg_static_buffer *statbuf) {
  size_t avail = (statbuf->data_len-statbuf->pos)/size;
  
  if (avail < count)
    count = avail;
  
  memcpy(ptr, ((char*)statbuf->data)+statbuf->pos, count*size);
  statbuf->pos += count*size;
  
  return count;
}
int _bufseek(ogg_static_buffer *statbuf, ogg_int64_t offset, int origin) {
  switch (origin) {
    case SEEK_SET: //0
      statbuf->pos = offset;
      break;
    case SEEK_CUR: // 1
      statbuf->pos += offset;
      break;
    case SEEK_END: // 2
      statbuf->pos = statbuf->data_len - 1 + offset;
      break;
    default:
      return -1;
  }
  return 0;
}
long _buftell(ogg_static_buffer *statbuf) {
  return statbuf->pos;
}

static ov_callbacks OV_CALLBACKS_BUF = {
  (size_t (*)(void *, size_t, size_t, void *))  _bufread,
  (int (*)(void *, ogg_int64_t, int))           _bufseek,
  (int (*)(void *))                             NULL,
  (long (*)(void *))                            _buftell
};

//#define OGGBUFSIZE (4096*8)
//static char _pcmout[OGGBUFSIZE]; /* take 4k out of the data segment, not the stack */


void FillStaticBuffer(ALOGG_OGG *alogg) {
  
  ogg_int64_t ov_pcm_total(OggVorbis_File *vf,int i);
  long pcm_samples = ov_pcm_total(&alogg->vf, -1);
  long pcm_bytes = pcm_samples * 2*2;
  
  printf("bytes: %ld", pcm_bytes);
  char *buf = (char*) malloc(pcm_bytes);
  
  long filled = 0;
  while (filled < pcm_bytes) {
    long count = ov_read(&alogg->vf,buf+filled,pcm_bytes-filled,0,2,1,&alogg->current_section);
    filled += count;
  }
  
	// copy from sampleBuffer to AL buffer
	alBufferData(alogg->buffer,
               alogg->format,
               buf,
               filled,//player->bufferSizeBytes,
               alogg->rate);
}

// xtern: my_load_static_ogg
ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len)
{ 
  PRINT_STUB; 

  ALuint source = _openal_new_source();
  if (source == 0)
    return NULL;
  
  ALOGG_OGG *alogg = (ALOGG_OGG*)malloc(sizeof(ALOGG_OGG));
  alogg->source = source;
  
  //FILE *f = fopen(oggpath, "r");
  alogg->statbuf.data = data;
  alogg->statbuf.data_len = data_len;
  alogg->statbuf.pos = 0;
  
  if(ov_open_callbacks(&alogg->statbuf, &alogg->vf, NULL, 0, OV_CALLBACKS_BUF) < 0) {
    fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
    exit(1);
  }  
  
  
  {
    //char **ptr=ov_comment(&alogg->vf,-1)->user_comments;
    vorbis_info *vi=ov_info(&alogg->vf,-1);
    alogg->channels = vi->channels;// == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    alogg->rate = vi->rate;
  }
  
  switch (alogg->channels) {
    case 1:
      alogg->format = AL_FORMAT_MONO16;
      break;
    case 2:
      alogg->format = AL_FORMAT_STEREO16;
      break;
    default:
      return 0;
  }
  
  // only one buffer cause its static.
  //ALuint buffer;
	alGenBuffers(1, &alogg->buffer);
	CheckALError ("Couldn't generate buffer");
  
  FillStaticBuffer(alogg);
  
  // queue up the buffers on the source
	alSourceQueueBuffers(source, 1, &alogg->buffer);

  // parse ogg
  // get audio info
  return alogg;
}

// xtern: MYSTATICOGG:Destroy
void alogg_destroy_ogg(ALOGG_OGG *ogg)
{ 
  
  alogg_stop_ogg(ogg);
  
  // release
  alDeleteSources(1, &ogg->source);
  
  PRINT_STUB;
}

// xtern: MYSTATICOGG:play_from, Restart
int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop)
{ 
  // start playing ogg with these parameters
  PRINT_STUB; 
  
  alSourcePlayv (1, &ogg->source);
	CheckALError ("Couldn't play");
  
  return 0;
}

// xtern:MYSTATICOGG: destroy, seek, restart
void alogg_stop_ogg(ALOGG_OGG *ogg)
{
  // stop playing
  alSourceStop(ogg->source);
  PRINT_STUB;
}


int _tmp_count = 0;
// xtern: MYSTATICOGG:Poll
int alogg_poll_ogg(ALOGG_OGG *ogg)
{ 
  // poll, do work, return 1 if finished.
  
  ALint bytesoffset;
  alGetSourcei( ogg->source, AL_BYTE_OFFSET, &bytesoffset );
  CheckALError ("Couldn't get offset.");
  

  if (_tmp_count %200 == 0)
    printf("play ogg %d bytes\n", bytesoffset);
  _tmp_count += 1;
  
  PRINT_STUB; 
  return 0;
}


// extern: MYSTATICOGG: set_volume
void alogg_adjust_ogg(ALOGG_OGG *ogg, int vol, int pan, int speed, int loop)
{ 
  
  ALfloat newVolume = vol/255.0f;
  alSourcef(ogg->source, AL_GAIN, newVolume);
  
  ALint alloop = loop?AL_TRUE:AL_FALSE;
  alSourcei(ogg->source, AL_LOOPING, alloop);
  
  // adjust vol, speed, etc.
  PRINT_STUB;
}


// extern: MYSTATICOGG: Restart
void alogg_rewind_ogg(ALOGG_OGG *ogg)
{
  // jump back to start. replay
  PRINT_STUB;
}

// xtern: MYSTATICOGG:play_from
void alogg_seek_abs_msecs_ogg(ALOGG_OGG *ogg, int msecs){ PRINT_STUB;}

// xtern: MYSTATICOGG:play_from
int alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}

// xtern: MYSTATICOGG:play_from
int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}


// xtern: MYSTATICOGG: get_length_ms
int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}


// xtern: MYSTATICOGG: get_pos_ms
int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}


// xtern: MYSTATICOGG: get_pos_ms
int alogg_is_playing_ogg(ALOGG_OGG *ogg)
{
  // is the ogg currently playing?
  // seems to get other stuff called to calculate position.
  PRINT_STUB; 
  return 0;
}
// xtern: MYSTATICOGG: get_pos_ms, get_voice
ALW_AUDIOSTREAM *alogg_get_audiostream_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}


ALOGG_OGGSTREAM *alogg_create_oggstream(void *first_data_buffer, int data_buffer_len, int last_block)
{ 
  PRINT_STUB; 
  // parse ogg
  // get audio info
  return (ALOGG_OGGSTREAM*)malloc(sizeof(ALOGG_OGGSTREAM));
}
void alogg_destroy_oggstream(ALOGG_OGGSTREAM *ogg)
{ 
  // destroy!
  PRINT_STUB;
}
int alogg_play_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan)
{ 
  PRINT_STUB; 
  return 0;
}
void alogg_stop_oggstream(ALOGG_OGGSTREAM *ogg)
{ 
  // stop oggstream
  PRINT_STUB;
}
void alogg_adjust_oggstream(ALOGG_OGGSTREAM *ogg, int vol, int pan, int speed){ PRINT_STUB;}
int alogg_poll_oggstream(ALOGG_OGGSTREAM *ogg)
{ 
  // polling
  PRINT_STUB; 
  return 0;
}
void *alogg_get_oggstream_buffer(ALOGG_OGGSTREAM *ogg)
{
  // used to fill next bit of streaming buffer
  PRINT_STUB;
  return 0;
}  
void alogg_free_oggstream_buffer(ALOGG_OGGSTREAM *ogg, int bytes_used){ PRINT_STUB;}
int alogg_get_pos_msecs_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}
int alogg_is_playing_oggstream(ALOGG_OGGSTREAM *ogg) 
{ 
  PRINT_STUB; 
  return 0;
}
ALW_AUDIOSTREAM *alogg_get_audiostream_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}

int alogg_is_end_of_oggstream(ALOGG_OGGSTREAM *ogg){ PRINT_STUB; return 0;}
int alogg_is_end_of_ogg(ALOGG_OGG *ogg){ PRINT_STUB; return 0;}


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