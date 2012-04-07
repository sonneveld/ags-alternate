#ifndef _ALLEGRO_WRAPPER_H
#define _ALLEGRO_WRAPPER_H

// use to get an idea of what's being used so we can port it to SDL or Allegro3

// ignore stuff in the graphics drivers since we'd probably have to write from scratch
// ignore external libraries, they'd have to be rewritten anyway.


// include windows.h, with funny business since allegro ALSO uses BITMAP type.
#define BITMAP WINDOWS_BITMAP
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define WINDOWS_RGB(r,g,b)  ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#undef BITMAP
#undef RGB

#include <direct.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <dsound.h>


#define ALLEGRO_DATE 20120101

#define AL_ID(a,b,c,d)     (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))
#define U_ASCII         AL_ID('A','S','C','8')
#define U_UNICODE       AL_ID('U','N','I','C')



#include "SDL.h"

#include "stdint.h"

#define MASK_COLOR_8       0
#define MASK_COLOR_15      0x7C1F
#define MASK_COLOR_16      0xF81F
#define MASK_COLOR_24      0xFF00FF
#define MASK_COLOR_32      0xFF00FF

typedef struct ALW_GFX_VTABLE        /* functions for drawing onto bitmaps */
{
	//int color_depth;
	int mask_color;
	//void *unwrite_bank;  /* C function on some machines, asm on i386 */
} GFX_VTABLE;

typedef struct ALW_GFX_MODE
{
	int width, height, bpp;
} ALW_GFX_MODE;

typedef struct ALW_GFX_MODE_LIST
{
	int num_modes;                /* number of gfx modes */
	ALW_GFX_MODE *mode;               /* pointer to the actual mode list array */
} ALW_GFX_MODE_LIST;

struct alw_al_ffblk {
	//int attrib;         /* actual attributes of the file found */
	time_t time;        /* modification time of file */
	//long size;          /* size of file */
	char name[512];     /* name of file */
	//void *ff_data;      /* private hook */
};

struct ALW_BITMAP {
	int w, h;                     /* width and height in pixels */
	int clip;                     /* flag if clipping is turned on */
	int cl, cr, ct, cb;           /* clip left, right, top and bottom values */
	GFX_VTABLE *vtable;           /* drawing functions */
	//void *write_bank;             /* C func on some machines, asm on i386 */
	//void *read_bank;              /* C func on some machines, asm on i386 */
	//void *dat;                    /* the memory we allocated for the bitmap */
	//unsigned long id;             /* for identifying sub-bitmaps */
	void *extra;                  /* points to a structure with more info */
	//int x_ofs;                    /* horizontal offset (for sub-bitmaps) */
	//int y_ofs;                    /* vertical offset (for sub-bitmaps) */
	//int seg;                      /* bitmap segment */

	// actually used
	SDL_Surface *surf;
	unsigned char **line;
};

struct _al_normal_packfile_details
{
	//int hndl;                           /* DOS file handle */
	//int flags;                          /* PACKFILE_FLAG_* constants */
	//unsigned char *buf_pos;             /* position in buffer */
	//int buf_size;                       /* number of bytes in the buffer */
	long todo;                          /* number of bytes still on the disk */
	//struct PACKFILE *parent;            /* nested, parent file */
	//struct LZSS_PACK_DATA *pack_data;   /* for LZSS compression */
	//struct LZSS_UNPACK_DATA *unpack_data; /* for LZSS decompression */
	//char *filename;                     /* name of the file */
	//char *passdata;                     /* encryption key data */
	//char *passpos;                      /* current key position */
	//unsigned char buf[F_BUF_SIZE];      /* the actual data buffer */
};

struct ALW_PACKFILE {
   //AL_CONST PACKFILE_VTABLE *vtable;
   //void *userdata;
   //int is_normal_packfile;

   struct _al_normal_packfile_details normal;
};

typedef struct ALW_SAMPLE
{
	//int bits;                           /* 8 or 16 */
	//int stereo;                         /* sample type flag */
	int freq;                           /* sample frequency */
	//int priority;                       /* 0-255 */
	unsigned long len;                  /* length (in samples) */
	//unsigned long loop_start;           /* loop start position */
	//unsigned long loop_end;             /* loop finish position */
	//unsigned long param;                /* for internal use by the driver */
	//void *data;                         /* sample data */
} ALW_SAMPLE;

struct ALW_AUDIOSTREAM {
	int voice;                          /* the voice we are playing on */
	struct ALW_SAMPLE *samp;                /* the sample we are using */
	//int len;                            /* buffer length */
	//int bufcount;                       /* number of buffers per sample half */
	//int bufnum;                         /* current refill buffer */
	int active;                         /* which half is currently playing */
	void *locked;                       /* the locked buffer */
};

typedef struct ALW_MIDI                    /* a midi file */
{
	//int divisions;                      /* number of ticks per quarter note */
	//struct {
		//unsigned char *data;             /* MIDI message stream */
		//int len;                         /* length of the track data */
	//} track[MIDI_TRACKS];
} ALW_MIDI;

typedef struct ALW_RGB
{
  unsigned char r, g, b;
  unsigned char filler;
} RGB;

#define ALW_PAL_SIZE     256

typedef ALW_RGB ALW_PALETTE[ALW_PAL_SIZE];


typedef struct {
  unsigned char data[32][32][32];
} ALW_RGB_MAP;

typedef struct {
  unsigned char data[ALW_PAL_SIZE][ALW_PAL_SIZE];
} ALW_COLOR_MAP;

typedef int32_t alw_fixed;

#define ALW_DIGI_AUTODETECT       -1     
#define ALW_DIGI_NONE             0

#define ALW_MIDI_AUTODETECT       -1
#define ALW_MIDI_NONE             0


// MISC

extern int alw_get_rgb_scale_5(int x);
extern int alw_get_rgb_scale_6(int x);

extern int  alw_get_rgb_r_shift_15();
extern int  alw_get_rgb_g_shift_15();
extern int  alw_get_rgb_b_shift_15();
extern int  alw_get_rgb_r_shift_16();
extern int  alw_get_rgb_g_shift_16();
extern int  alw_get_rgb_b_shift_16();
extern int  alw_get_rgb_r_shift_32();
extern int  alw_get_rgb_g_shift_32();
extern int  alw_get_rgb_b_shift_32();
extern int  alw_get_rgb_a_shift_32();
extern void  alw_set_rgb_r_shift_15(int x);
extern void  alw_set_rgb_g_shift_15(int x);
extern void  alw_set_rgb_b_shift_15(int x);
extern void  alw_set_rgb_r_shift_16(int x);
extern void  alw_set_rgb_g_shift_16(int x);
extern void  alw_set_rgb_b_shift_16(int x);
extern void  alw_set_rgb_r_shift_32(int x);
extern void  alw_set_rgb_g_shift_32(int x);
extern void  alw_set_rgb_b_shift_32(int x);
extern void  alw_set_rgb_a_shift_32(int x);

extern void alw_set_allegro_wnd(HWND allegro_wnd);
extern HWND alw_get_allegro_wnd();
//HWND allegro_wnd = NULL;

extern "C" extern LPDIRECTDRAW2 alw_get_directdraw();
extern "C" extern LPDIRECTSOUND alw_get_directsound();

typedef struct DDRAW_SURFACE {
	LPDIRECTDRAWSURFACE2 id;
	int flags;
	int lock_nesting;
	ALW_BITMAP *parent_bmp;  /* only used by the flipping chain */
	struct DDRAW_SURFACE *next;
	struct DDRAW_SURFACE *prev;
} DDRAW_SURFACE;

extern "C" extern DDRAW_SURFACE *alw_get_gfx_directx_primary_surface();

ALW_BITMAP *alw_gfx_directx_create_system_bitmap(int width, int height);

#define DIGI_DIRECTX(n)          AL_ID('D','X','A'+(n),' ')
#define DIGI_DIRECTAMX(n)        AL_ID('A','X','A'+(n),' ')
#define DIGI_WAVOUTID(n)         AL_ID('W','O','A'+(n),' ')
#define MIDI_WIN32MAPPER         AL_ID('W','3','2','M')
#define MIDI_WIN32(n)            AL_ID('W','3','2','A'+(n))
#define MIDI_WIN32_IN(n)         AL_ID('W','3','2','A'+(n))
#define SYSTEM_AUTODETECT  0
#define SYSTEM_NONE        AL_ID('N','O','N','E')


// WINALLEG
HWND alw_win_get_window(void);
int alw_wnd_call_proc(int (*proc)(void));

// INIT
#define ALW_ALLEGRO_DATE ALLEGRO_DATE
int alw_install_allegro(int system_id, int *errno_ptr, int (*atexit_ptr)( void (__cdecl *func )( void )));
void alw_allegro_exit();
void alw_set_window_title(const char *name);
int alw_set_close_button_callback(void (*proc)(void));
int alw_get_desktop_resolution(int *width, int *height);
#define ALW_ALLEGRO_ERROR_SIZE 256
extern char alw_allegro_error[ALW_ALLEGRO_ERROR_SIZE];

#define ALW_END_OF_MAIN()

// UNICODE
char *alw_uconvert(const char *s, int type, char *buf, int newtype, int size);
void alw_set_uformat(int type);

// MOUSE
typedef void *LPDIRECTINPUTDEVICE;

extern volatile int alw_mouse_x;
extern volatile int alw_mouse_y;
extern volatile int alw_mouse_z;
extern volatile int alw_mouse_b;
extern int alw_install_mouse();
extern int alw_poll_mouse();
extern void alw_set_mouse_range (int x1, int y_1, int x2, int y2) ;
extern void alw_position_mouse (int x, int y);

extern LPDIRECTINPUTDEVICE alw_mouse_dinput_device;


// TIMER
#define ALW_TIMERS_PER_SECOND     1193181L
#define ALW_SECS_TO_TIMER(x)      ((long)(x) * ALW_TIMERS_PER_SECOND)
#define ALW_MSEC_TO_TIMER(x)      ((long)(x) * (ALW_TIMERS_PER_SECOND / 1000))

extern int alw_install_timer();
extern int alw_install_int_ex(void (*proc)(), int speed);
extern void alw_rest(unsigned int time);

#define ALW_END_OF_FUNCTION(x)
#define ALW_LOCK_FUNCTION(x)
#define ALW_LOCK_VARIABLE(x)

// KEYBOARD
int alw_install_keyboard();
int alw_keyboard_needs_poll();
int alw_poll_keyboard();
int alw_keypressed();
int alw_readkey();
extern volatile char alw_key[];
extern volatile int alw_key_shifts;
void alw_set_leds(int leds);

extern const unsigned char alw_hw_to_mycode[256];
extern LPDIRECTINPUTDEVICE alw_key_dinput_device;



// GRAPHICS MODES
#define SWITCH_NONE           0
#define SWITCH_PAUSE          1
#define SWITCH_AMNESIA        2
#define SWITCH_BACKGROUND     3
#define SWITCH_BACKAMNESIA    4

#define SWITCH_IN             0
#define SWITCH_OUT            1

#define GFX_TEXT				-1

#define GFX_DIRECTX              AL_ID('D','X','A','C')
#define GFX_DIRECTX_ACCEL        AL_ID('D','X','A','C')
#define GFX_DIRECTX_SAFE         AL_ID('D','X','S','A')
#define GFX_DIRECTX_SOFT         AL_ID('D','X','S','O')
#define GFX_DIRECTX_WIN          AL_ID('D','X','W','N')
#define GFX_DIRECTX_OVL          AL_ID('D','X','O','V')
#define GFX_GDI                  AL_ID('G','D','I','B')

//void alw_set_color_depth(int depth);
int alw_set_gfx_mode(int card, int w, int h, int v_w, int v_h);

ALW_GFX_MODE_LIST *alw_get_gfx_mode_list(int card);
void alw_destroy_gfx_mode_list(ALW_GFX_MODE_LIST *mode_list);

int alw_set_display_switch_mode(int mode);
int alw_set_display_switch_callback(int dir, void (*cb)());

void alw_vsync();



// BITMAP
extern ALW_BITMAP *alw_screen;

void alw_set_color_depth(int depth);
ALW_BITMAP *alw_create_bitmap(int width, int height) ;
ALW_BITMAP *alw_create_bitmap_ex(int color_depth, int width, int height) ;
ALW_BITMAP *alw_create_sub_bitmap(ALW_BITMAP *parent, int x, int y, int width, int height) ;
void alw_destroy_bitmap(ALW_BITMAP *bitmap) ;
int alw_bitmap_color_depth(ALW_BITMAP *bmp) ;
int alw_bitmap_mask_color(ALW_BITMAP *bmp) ;
int alw_is_linear_bitmap(ALW_BITMAP *bmp) ;
int alw_is_memory_bitmap(ALW_BITMAP *bmp) ;
int alw_is_video_bitmap(ALW_BITMAP *bmp) ;
void alw_acquire_bitmap(ALW_BITMAP *bmp) ;
void alw_release_bitmap(ALW_BITMAP *bmp);
void alw_acquire_screen() ;
void alw_release_screen() ;
void alw_set_clip_rect(ALW_BITMAP *bitmap, int x1, int y1, int x2, int y2) ;
void alw_set_clip_state(ALW_BITMAP *bitmap, int state) ;


// IMAGE FILES
int alw_save_bitmap(const char *filename, ALW_BITMAP *bmp, const ALW_RGB *pal);
ALW_BITMAP *alw_load_bitmap(const char *filename, ALW_RGB *pal);
ALW_BITMAP *alw_load_pcx(const char *filename, ALW_RGB *pal) ;
void alw_set_color_conversion(int mode) ;

// ALW_PALETTE
extern ALW_PALETTE alw_black_palette;
extern void alw_set_palette(const ALW_PALETTE p);
extern void alw_get_palette(ALW_PALETTE p) ;
extern void alw_set_palette_range(const ALW_PALETTE p, int from, int to, int vsync) ;
extern void alw_get_palette_range(ALW_PALETTE p, int from, int to) ;
extern void alw_fade_interpolate(const ALW_PALETTE source, const ALW_PALETTE dest, ALW_PALETTE output, int pos, int from, int to) ;
extern void alw_select_palette(const ALW_PALETTE p) ;
extern void alw_unselect_palette() ;

// TRUE COLOUR PIXELS
#define ALW_MASK_COLOR_8       0
#define ALW_MASK_COLOR_15      0x7C1F
#define ALW_MASK_COLOR_16      0xF81F
#define ALW_MASK_COLOR_24      0xFF00FF
#define ALW_MASK_COLOR_32      0xFF00FF

int alw_makecol_depth(int color_depth, int r, int g, int b) ;
int alw_makeacol_depth(int color_depth, int r, int g, int b, int a) ;
int alw_getr_depth(int color_depth, int c);
int alw_getg_depth(int color_depth, int c);
int alw_getb_depth(int color_depth, int c);
int alw_geta_depth(int color_depth, int c);

#define alw_makecol8(r,g,b) alw_makecol_depth(8,r,g,b)
#define alw_makecol15(r,g,b) alw_makecol_depth(15,r,g,b)
#define alw_makecol16(r,g,b) alw_makecol_depth(16,r,g,b)
#define alw_makecol24(r,g,b) alw_makecol_depth(24,r,g,b)
#define alw_makecol32(r,g,b) alw_makecol_depth(32,r,g,b)
#define alw_makeacol32(r,g,b,a) alw_makeacol_depth(32,r,g,b,a)

#define alw_getr15(c) alw_getr_depth(15,c)
#define alw_getg15(c) alw_getg_depth(15,c)
#define alw_getb15(c) alw_getb_depth(15,c)

#define alw_getr16(c) alw_getr_depth(16,c)
#define alw_getg16(c) alw_getg_depth(16,c)
#define alw_getb16(c) alw_getb_depth(16,c)

#define alw_getr24(c) alw_getr_depth(24,c)
#define alw_getg24(c) alw_getg_depth(24,c)
#define alw_getb24(c) alw_getb_depth(24,c)

#define alw_getr32(c) alw_getr_depth(32,c)
#define alw_getg32(c) alw_getg_depth(32,c)
#define alw_getb32(c) alw_getb_depth(32,c)
#define alw_geta32(c) alw_geta_depth(32,c)

extern ALW_COLOR_MAP *color_map;

#define ALLEGRO_COLOR8
#define ALLEGRO_COLOR15
#define ALLEGRO_COLOR16
#define ALLEGRO_COLOR24
#define ALLEGRO_COLOR32


#define MASK_COLOR_8       0
#define MASK_COLOR_15      0x7C1F
#define MASK_COLOR_16      0xF81F
#define MASK_COLOR_24      0xFF00FF
#define MASK_COLOR_32      0xFF00FF

#define bmp_write8(addr, c)         (*((uint8_t  *)(addr)) = (c))
#define bmp_write15(addr, c)        (*((uint16_t *)(addr)) = (c))
#define bmp_write16(addr, c)        (*((uint16_t *)(addr)) = (c))
#define bmp_write32(addr, c)        (*((uint32_t *)(addr)) = (c))

#define bmp_read8(addr)             (*((uint8_t  *)(addr)))
#define bmp_read15(addr)            (*((uint16_t *)(addr)))
#define bmp_read16(addr)            (*((uint16_t *)(addr)))
#define bmp_read32(addr)            (*((uint32_t *)(addr)))

int bmp_read24 (uintptr_t addr);
void bmp_write24 (uintptr_t addr, int c);

#define READ3BYTES(p)  ((*(unsigned char *)(p))               \
  | (*((unsigned char *)(p) + 1) << 8)  \
  | (*((unsigned char *)(p) + 2) << 16))

#define WRITE3BYTES(p,c)  ((*(unsigned char *)(p) = (c)),             \
  (*((unsigned char *)(p) + 1) = (c) >> 8),  \
  (*((unsigned char *)(p) + 2) = (c) >> 16))

// DIRECT ACCESS

unsigned long alw_bmp_write_line(ALW_BITMAP *bmp, int line);
unsigned long alw_bmp_read_line(ALW_BITMAP *bmp, int line);
void alw_bmp_unwrite_line(ALW_BITMAP *bmp);
void alw_bmp_select(ALW_BITMAP *bmp);


// DRAWING

int alw_getpixel ( ALW_BITMAP *bmp, int x, int y);
void alw_putpixel ( ALW_BITMAP *bmp, int x, int y, int color);
void alw_clear_to_color(ALW_BITMAP *bitmap, int color);
void alw_clear_bitmap(ALW_BITMAP *bitmap);
void alw_hline(ALW_BITMAP *bmp, int x1, int y, int x2, int color) ;
void alw_line(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void alw_do_line(ALW_BITMAP *bmp, int x1, int y1,int x2,int y2, int d, void (*proc)(ALW_BITMAP *bmp, int x, int y, int d)) ;
void alw_rect(ALW_BITMAP *bmp, int x1, int y1, int x2, int y2, int color) ;
void alw_floodfill(ALW_BITMAP *bmp, int x, int y, int color);
void alw_rectfill ( ALW_BITMAP *bmp, int x1, int y_1, int x2, int y2, int color) ;
void alw_triangle(ALW_BITMAP *bmp, int x1,int y1,int x2,int y2,int x3,int y3, int color) ;
void alw_circlefill(ALW_BITMAP *bmp, int x, int y, int radius, int color) ;
void alw_hfill(ALW_BITMAP *dst, int dx1, int dy, int dx2, int color);

#define alw__getpixel alw_getpixel
#define alw__putpixel alw_putpixel


// BLITTING AND SPRITES
void alw_blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) ;
void alw_draw_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) ;
void alw_draw_lit_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int color) ;
void alw_draw_trans_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) ;
void alw_draw_sprite_h_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) ;
void alw_draw_sprite_v_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) ;
void alw_draw_sprite_vh_flip(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y) ;
void alw_stretch_blit(ALW_BITMAP *source, ALW_BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height);
void alw_stretch_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int w, int h) ;
void alw_rotate_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, alw_fixed angle) ;
void alw_pivot_sprite(ALW_BITMAP *bmp, ALW_BITMAP *sprite, int x, int y, int cx, int cy, alw_fixed angle) ;


// TRANSPARENCY

// 256-color transparency
void alw_create_light_table(ALW_COLOR_MAP *table, const ALW_PALETTE pal, int r, int g, int b, void (*callback)(int pos)) ;
void alw_set_color_map(ALW_COLOR_MAP *alw_color_map);
int alw_has_color_map();

// truecolor transparency
typedef unsigned long (*ALW_BLENDER_FUNC)(unsigned long x, unsigned long y, unsigned long n);
void alw_set_alpha_blender() ;
void alw_set_trans_blender(int r, int g, int b, int a) ;
void alw_set_blender_mode (ALW_BLENDER_FUNC b15, ALW_BLENDER_FUNC b16, ALW_BLENDER_FUNC b24, int r, int g, int b, int a) ;
void alw_set_blender_mode_ex(ALW_BLENDER_FUNC b15,ALW_BLENDER_FUNC b16,ALW_BLENDER_FUNC b24,ALW_BLENDER_FUNC b32,ALW_BLENDER_FUNC b15x,ALW_BLENDER_FUNC b16x,ALW_BLENDER_FUNC b24x, int r, int g,int b,int a);

extern ALW_BLENDER_FUNC _blender_func15;
extern ALW_BLENDER_FUNC _blender_func16;
extern ALW_BLENDER_FUNC _blender_func24;
extern ALW_BLENDER_FUNC _blender_func32;
extern ALW_BLENDER_FUNC _blender_func15x;
extern ALW_BLENDER_FUNC _blender_func16x;
extern ALW_BLENDER_FUNC _blender_func24x;

extern int _blender_col_15;
extern int _blender_col_16;
extern int _blender_col_24;
extern int _blender_col_32;
extern int _blender_alpha;


// COLOUR FORMATS
//extern ALW_RGB_MAP *_alw_rgb_map;
void alw_set_rgb_map(ALW_RGB_MAP *rgb_map);
void alw_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v);
void alw_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b);
void alw_create_rgb_table(ALW_RGB_MAP *table, const ALW_PALETTE pal, void (*callback)(int pos));


// FLI
#define ALW_FLI_OK          0 
#define ALW_FLI_ERROR       -2
int alw_play_fli(const char *filename, ALW_BITMAP *bmp, int loop, int (*callback)());

// SOUND INIT
void alw_reserve_voices(int digi_voices, int midi_voices);
int alw_install_sound(int digi, int midi, const char *cfg_path);
void alw_remove_sound();
void alw_set_volume(int digi_volume, int midi_volume);
void alw_set_volume_per_voice(int scale);

// DIGIAL AUDIO
extern ALW_SAMPLE *alw_load_sample(const char *filename);
extern void alw_destroy_sample(ALW_SAMPLE *spl);
extern int alw_play_sample(const ALW_SAMPLE *spl, int vol, int pan, int freq, int loop);
extern void alw_stop_sample(const ALW_SAMPLE *spl);

// for controlling played samples
void alw_voice_start(int voice);
void alw_voice_stop(int voice);
int alw_voice_get_position(int voice);
void alw_voice_set_position(int voice, int position);
void alw_voice_set_volume(int voice, int volume);
void alw_voice_set_pan(int voice, int pan);
int alw_voice_get_frequency(int voice);



// MIDI
ALW_MIDI *alw_load_midi(const char *filename);
void alw_destroy_midi(ALW_MIDI *midi);
int alw_get_midi_length(ALW_MIDI *midi);
int alw_play_midi(ALW_MIDI *midi, int loop);
void alw_stop_midi();
void alw_midi_pause();
void alw_midi_resume();
int alw_midi_seek(int target);
extern volatile long alw_midi_pos;


// AUDIO STREAMS
void alw_stop_audio_stream(ALW_AUDIOSTREAM *stream);


// FILE
char *alw_append_filename(char *dest, const char *path, const char *filename, int size) ;
char *alw_fix_filename_case(char *path) ;
char *alw_fix_filename_slashes(char *path) ;
void alw_put_backslash(char *filename) ;
char* alw_get_filename(char *path) ;

// also involve pack files.
int alw_exists(const char *filename) ;
#define FA_RDONLY       1
#define FA_HIDDEN       2
#define FA_SYSTEM       4
#define FA_LABEL        8
#define FA_DIREC        16
#define FA_ARCH         32
#define FA_NONE         0
#define FA_ALL          (~FA_NONE)
int alw_file_exists(const char *filename, int attrib, int *aret) ;
long alw_file_size_ex(const char *filename) ;

// could probably stub
int alw_al_findfirst(const char *pattern, struct alw_al_ffblk *info, int attrib) ;
int alw_al_findnext(struct alw_al_ffblk *info) ;
void alw_al_findclose(struct alw_al_ffblk *info) ;

// used only by audio.  stub?
ALW_PACKFILE *alw_pack_fopen(const char *filename, const char *mode) ;
long alw_pack_fread(void *p, long n, ALW_PACKFILE *f) ;
int alw_pack_fseek(ALW_PACKFILE *f, int offset) ;
int alw_pack_fclose(ALW_PACKFILE *f) ;
extern "C" {
extern ALW_PACKFILE *__old_pack_fopen(char *,char *);
}


// FIXED
/*fmaths.inl */
alw_fixed alw_itofix (int x)  ;
/* math.c */ 
double alw_fixtof (alw_fixed x);
alw_fixed alw_fixdiv (alw_fixed x, alw_fixed y);
alw_fixed alw_fixmul (alw_fixed x, alw_fixed y);
alw_fixed alw_fixsin (alw_fixed x);
alw_fixed alw_fixcos (alw_fixed x);
alw_fixed alw_fixatan(alw_fixed x);


// ALOGG
extern "C" {

#define ALOGG_OK                     0
#define ALOGG_POLL_PLAYJUSTFINISHED  1

typedef struct ALOGG_OGG ALOGG_OGG;
typedef struct ALOGG_OGGSTREAM ALOGG_OGGSTREAM;

extern ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len);
extern void alogg_destroy_ogg(ALOGG_OGG *ogg);
extern int alogg_play_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan);
extern int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop);
extern void alogg_stop_ogg(ALOGG_OGG *ogg);
extern void alogg_adjust_ogg(ALOGG_OGG *ogg, int vol, int pan, int speed, int loop);
extern void alogg_rewind_ogg(ALOGG_OGG *ogg);
extern void alogg_seek_abs_msecs_ogg(ALOGG_OGG *ogg, int msecs);
extern int alogg_poll_ogg(ALOGG_OGG *ogg);
extern int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg);
extern int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg);
extern int alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg);
extern int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg);
extern int alogg_is_playing_ogg(ALOGG_OGG *ogg);
extern ALW_AUDIOSTREAM *alogg_get_audiostream_ogg(ALOGG_OGG *ogg);

extern ALOGG_OGGSTREAM *alogg_create_oggstream(void *first_data_buffer, int data_buffer_len, int last_block);
extern void alogg_destroy_oggstream(ALOGG_OGGSTREAM *ogg);
extern int alogg_play_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan);
extern void alogg_stop_oggstream(ALOGG_OGGSTREAM *ogg);
extern void alogg_adjust_oggstream(ALOGG_OGGSTREAM *ogg, int vol, int pan, int speed);
extern int alogg_poll_oggstream(ALOGG_OGGSTREAM *ogg);
extern void *alogg_get_oggstream_buffer(ALOGG_OGGSTREAM *ogg);
extern void alogg_free_oggstream_buffer(ALOGG_OGGSTREAM *ogg, int bytes_used);
extern int alogg_get_pos_msecs_oggstream(ALOGG_OGGSTREAM *ogg);
extern int alogg_is_playing_oggstream(ALOGG_OGGSTREAM *ogg);
extern ALW_AUDIOSTREAM *alogg_get_audiostream_oggstream(ALOGG_OGGSTREAM *ogg);

int alogg_is_end_of_oggstream(ALOGG_OGGSTREAM *ogg);
int alogg_is_end_of_ogg(ALOGG_OGG *ogg);
}

// ALMP3

#define ALMP3_OK                     0
#define ALMP3_POLL_PLAYJUSTFINISHED  1

typedef struct ALMP3_MP3 ALMP3_MP3;

extern ALMP3_MP3 *almp3_create_mp3(void *data, int data_len);
extern void almp3_destroy_mp3(ALMP3_MP3 *mp3);

extern int almp3_play_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan);
extern int almp3_play_ex_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan, int speed, int loop);
extern void almp3_stop_mp3(ALMP3_MP3 *mp3);
extern void almp3_rewind_mp3(ALMP3_MP3 *mp3);
extern void almp3_seek_abs_msecs_mp3(ALMP3_MP3 *mp3, int msecs);
extern void almp3_adjust_mp3(ALMP3_MP3 *mp3, int vol, int pan, int speed, int loop);

extern int almp3_poll_mp3(ALMP3_MP3 *mp3);

extern int almp3_get_pos_msecs_mp3(ALMP3_MP3 *mp3);
extern int almp3_get_length_msecs_mp3(ALMP3_MP3 *mp3);

extern ALW_AUDIOSTREAM *almp3_get_audiostream_mp3(ALMP3_MP3 *mp3);


typedef struct ALMP3_MP3STREAM ALMP3_MP3STREAM;

extern ALMP3_MP3STREAM *almp3_create_mp3stream(void *first_data_buffer, int data_buffer_len, int last_block);
extern void almp3_destroy_mp3stream(ALMP3_MP3STREAM *mp3);

extern int almp3_play_mp3stream(ALMP3_MP3STREAM *mp3, int buffer_len, int vol, int pan);
extern void almp3_stop_mp3stream(ALMP3_MP3STREAM *mp3);
extern void almp3_adjust_mp3stream(ALMP3_MP3STREAM *mp3, int vol, int pan, int speed);

extern int almp3_poll_mp3stream(ALMP3_MP3STREAM *mp3);
extern void *almp3_get_mp3stream_buffer(ALMP3_MP3STREAM *mp3);
extern void almp3_free_mp3stream_buffer(ALMP3_MP3STREAM *mp3, int bytes_used);

extern int almp3_get_length_msecs_mp3stream(ALMP3_MP3STREAM *mp3, int total_size);
extern int almp3_get_pos_msecs_mp3stream(ALMP3_MP3STREAM *mp3);

extern ALW_AUDIOSTREAM *almp3_get_audiostream_mp3stream(ALMP3_MP3STREAM *mp3);


// DUMB

// we can probably use the first half, and only reimplement the 2nd.
typedef struct DUH DUH;
typedef struct DUH_SIGRENDERER DUH_SIGRENDERER;

typedef struct DUMB_IT_SIGDATA DUMB_IT_SIGDATA;
typedef struct DUMB_IT_SIGRENDERER DUMB_IT_SIGRENDERER;

typedef struct DUMBFILE DUMBFILE;

DUMB_IT_SIGRENDERER *duh_get_it_sigrenderer(DUH_SIGRENDERER *sigrenderer);
int dumb_it_sr_get_current_order(DUMB_IT_SIGRENDERER *sr);

DUH_SIGRENDERER *dumb_it_start_at_order(DUH *duh, int n_channels, int startorder);
void dumb_it_set_loop_callback(DUMB_IT_SIGRENDERER *sigrenderer, int (*callback)(void *data), void *data);
int dumb_it_callback_terminate(void *data);
void duh_end_sigrenderer(DUH_SIGRENDERER *sigrenderer);

DUH *dumb_load_it(const char *filename);
DUH *dumb_load_xm(const char *filename);
DUH *dumb_load_s3m(const char *filename);
DUH *dumb_load_mod(const char *filename);
void unload_duh(DUH *duh);
long duh_get_length(DUH *duh);

void dumb_exit(void);


typedef struct AL_DUH_PLAYER AL_DUH_PLAYER;
void dumb_register_packfiles(void);
AL_DUH_PLAYER *al_start_duh(DUH *duh, int n_channels, long pos, float volume, long bufsize, int freq);
void al_stop_duh(AL_DUH_PLAYER *dp);
void al_pause_duh(AL_DUH_PLAYER *dp);
void al_resume_duh(AL_DUH_PLAYER *dp);
void al_duh_set_volume(AL_DUH_PLAYER *dp, float volume);
int al_poll_duh(AL_DUH_PLAYER *dp);
AL_DUH_PLAYER *al_duh_encapsulate_sigrenderer(DUH_SIGRENDERER *sigrenderer, float volume, long bufsize, int freq);
DUH_SIGRENDERER *al_duh_get_sigrenderer(AL_DUH_PLAYER *dp);

// cdlib stuff
extern "C" {
	int cd_init(void);
	void cd_exit(void);
	int cd_play_from(int track);
	int cd_current_track(void);
	void cd_pause(void);
	void cd_resume(void);
	int cd_get_tracks(int *first, int *last);
	void cd_eject(void);
	void cd_close(void);
}



enum {
	__allegro_KB_SHIFT_FLAG    = 0x0001,
	__allegro_KB_CTRL_FLAG     = 0x0002,
	__allegro_KB_ALT_FLAG      = 0x0004,
	__allegro_KB_LWIN_FLAG     = 0x0008,
	__allegro_KB_RWIN_FLAG     = 0x0010,
	__allegro_KB_MENU_FLAG     = 0x0020,
	__allegro_KB_COMMAND_FLAG  = 0x0040,
	__allegro_KB_SCROLOCK_FLAG = 0x0100,
	__allegro_KB_NUMLOCK_FLAG  = 0x0200,
	__allegro_KB_CAPSLOCK_FLAG = 0x0400,
	__allegro_KB_INALTSEQ_FLAG = 0x0800,
	__allegro_KB_ACCENT1_FLAG  = 0x1000,
	__allegro_KB_ACCENT2_FLAG  = 0x2000,
	__allegro_KB_ACCENT3_FLAG  = 0x4000,
	__allegro_KB_ACCENT4_FLAG  = 0x8000
};

enum {
	__allegro_KEY_A            = 1,
	__allegro_KEY_B            = 2,
	__allegro_KEY_C            = 3,
	__allegro_KEY_D            = 4,
	__allegro_KEY_E            = 5,
	__allegro_KEY_F            = 6,
	__allegro_KEY_G            = 7,
	__allegro_KEY_H            = 8,
	__allegro_KEY_I            = 9,
	__allegro_KEY_J            = 10,
	__allegro_KEY_K            = 11,
	__allegro_KEY_L            = 12,
	__allegro_KEY_M            = 13,
	__allegro_KEY_N            = 14,
	__allegro_KEY_O            = 15,
	__allegro_KEY_P            = 16,
	__allegro_KEY_Q            = 17,
	__allegro_KEY_R            = 18,
	__allegro_KEY_S            = 19,
	__allegro_KEY_T            = 20,
	__allegro_KEY_U            = 21,
	__allegro_KEY_V            = 22,
	__allegro_KEY_W            = 23,
	__allegro_KEY_X            = 24,
	__allegro_KEY_Y            = 25,
	__allegro_KEY_Z            = 26,
	__allegro_KEY_0            = 27,
	__allegro_KEY_1            = 28,
	__allegro_KEY_2            = 29,
	__allegro_KEY_3            = 30,
	__allegro_KEY_4            = 31,
	__allegro_KEY_5            = 32,
	__allegro_KEY_6            = 33,
	__allegro_KEY_7            = 34,
	__allegro_KEY_8            = 35,
	__allegro_KEY_9            = 36,
	__allegro_KEY_0_PAD        = 37,
	__allegro_KEY_1_PAD        = 38,
	__allegro_KEY_2_PAD        = 39,
	__allegro_KEY_3_PAD        = 40,
	__allegro_KEY_4_PAD        = 41,
	__allegro_KEY_5_PAD        = 42,
	__allegro_KEY_6_PAD        = 43,
	__allegro_KEY_7_PAD        = 44,
	__allegro_KEY_8_PAD        = 45,
	__allegro_KEY_9_PAD        = 46,
	__allegro_KEY_F1           = 47,
	__allegro_KEY_F2           = 48,
	__allegro_KEY_F3           = 49,
	__allegro_KEY_F4           = 50,
	__allegro_KEY_F5           = 51,
	__allegro_KEY_F6           = 52,
	__allegro_KEY_F7           = 53,
	__allegro_KEY_F8           = 54,
	__allegro_KEY_F9           = 55,
	__allegro_KEY_F10          = 56,
	__allegro_KEY_F11          = 57,
	__allegro_KEY_F12          = 58,
	__allegro_KEY_ESC          = 59,
	__allegro_KEY_TILDE        = 60,
	__allegro_KEY_MINUS        = 61,
	__allegro_KEY_EQUALS       = 62,
	__allegro_KEY_BACKSPACE    = 63,
	__allegro_KEY_TAB          = 64,
	__allegro_KEY_OPENBRACE    = 65,
	__allegro_KEY_CLOSEBRACE   = 66,
	__allegro_KEY_ENTER        = 67,
	__allegro_KEY_COLON        = 68,
	__allegro_KEY_QUOTE        = 69,
	__allegro_KEY_BACKSLASH    = 70,
	__allegro_KEY_BACKSLASH2   = 71,
	__allegro_KEY_COMMA        = 72,
	__allegro_KEY_STOP         = 73,
	__allegro_KEY_SLASH        = 74,
	__allegro_KEY_SPACE        = 75,
	__allegro_KEY_INSERT       = 76,
	__allegro_KEY_DEL          = 77,
	__allegro_KEY_HOME         = 78,
	__allegro_KEY_END          = 79,
	__allegro_KEY_PGUP         = 80,
	__allegro_KEY_PGDN         = 81,
	__allegro_KEY_LEFT         = 82,
	__allegro_KEY_RIGHT        = 83,
	__allegro_KEY_UP           = 84,
	__allegro_KEY_DOWN         = 85,
	__allegro_KEY_SLASH_PAD    = 86,
	__allegro_KEY_ASTERISK     = 87,
	__allegro_KEY_MINUS_PAD    = 88,
	__allegro_KEY_PLUS_PAD     = 89,
	__allegro_KEY_DEL_PAD      = 90,
	__allegro_KEY_ENTER_PAD    = 91,
	__allegro_KEY_PRTSCR       = 92,
	__allegro_KEY_PAUSE        = 93,
	__allegro_KEY_ABNT_C1      = 94,
	__allegro_KEY_YEN          = 95,
	__allegro_KEY_KANA         = 96,
	__allegro_KEY_CONVERT      = 97,
	__allegro_KEY_NOCONVERT    = 98,
	__allegro_KEY_AT           = 99,
	__allegro_KEY_CIRCUMFLEX   = 100,
	__allegro_KEY_COLON2       = 101,
	__allegro_KEY_KANJI        = 102,
	__allegro_KEY_EQUALS_PAD   = 103,  /* MacOS X */
	__allegro_KEY_BACKQUOTE    = 104,  /* MacOS X */
	__allegro_KEY_SEMICOLON    = 105,  /* MacOS X */
	__allegro_KEY_COMMAND      = 106,  /* MacOS X */
	__allegro_KEY_UNKNOWN1     = 107,
	__allegro_KEY_UNKNOWN2     = 108,
	__allegro_KEY_UNKNOWN3     = 109,
	__allegro_KEY_UNKNOWN4     = 110,
	__allegro_KEY_UNKNOWN5     = 111,
	__allegro_KEY_UNKNOWN6     = 112,
	__allegro_KEY_UNKNOWN7     = 113,
	__allegro_KEY_UNKNOWN8     = 114,

	__allegro_KEY_MODIFIERS    = 115,

	__allegro_KEY_LSHIFT       = 115,
	__allegro_KEY_RSHIFT       = 116,
	__allegro_KEY_LCONTROL     = 117,
	__allegro_KEY_RCONTROL     = 118,
	__allegro_KEY_ALT          = 119,
	__allegro_KEY_ALTGR        = 120,
	__allegro_KEY_LWIN         = 121,
	__allegro_KEY_RWIN         = 122,
	__allegro_KEY_MENU         = 123,
	__allegro_KEY_SCRLOCK      = 124,
	__allegro_KEY_NUMLOCK      = 125,
	__allegro_KEY_CAPSLOCK     = 126,

	__allegro_KEY_MAX          = 127
};

#ifndef ALLEGRO_NO_KEY_DEFINES

#define KB_SHIFT_FLAG         __allegro_KB_SHIFT_FLAG
#define KB_CTRL_FLAG          __allegro_KB_CTRL_FLAG
#define KB_ALT_FLAG           __allegro_KB_ALT_FLAG
#define KB_LWIN_FLAG          __allegro_KB_LWIN_FLAG
#define KB_RWIN_FLAG          __allegro_KB_RWIN_FLAG
#define KB_MENU_FLAG          __allegro_KB_MENU_FLAG
#define KB_COMMAND_FLAG       __allegro_KB_COMMAND_FLAG
#define KB_SCROLOCK_FLAG      __allegro_KB_SCROLOCK_FLAG
#define KB_NUMLOCK_FLAG       __allegro_KB_NUMLOCK_FLAG
#define KB_CAPSLOCK_FLAG      __allegro_KB_CAPSLOCK_FLAG
#define KB_INALTSEQ_FLAG      __allegro_KB_INALTSEQ_FLAG
#define KB_ACCENT1_FLAG       __allegro_KB_ACCENT1_FLAG
#define KB_ACCENT2_FLAG       __allegro_KB_ACCENT2_FLAG
#define KB_ACCENT3_FLAG       __allegro_KB_ACCENT3_FLAG
#define KB_ACCENT4_FLAG       __allegro_KB_ACCENT4_FLAG

#define KEY_A                 __allegro_KEY_A
#define KEY_B                 __allegro_KEY_B
#define KEY_C                 __allegro_KEY_C
#define KEY_D                 __allegro_KEY_D
#define KEY_E                 __allegro_KEY_E
#define KEY_F                 __allegro_KEY_F
#define KEY_G                 __allegro_KEY_G
#define KEY_H                 __allegro_KEY_H
#define KEY_I                 __allegro_KEY_I
#define KEY_J                 __allegro_KEY_J
#define KEY_K                 __allegro_KEY_K
#define KEY_L                 __allegro_KEY_L
#define KEY_M                 __allegro_KEY_M
#define KEY_N                 __allegro_KEY_N
#define KEY_O                 __allegro_KEY_O
#define KEY_P                 __allegro_KEY_P
#define KEY_Q                 __allegro_KEY_Q
#define KEY_R                 __allegro_KEY_R
#define KEY_S                 __allegro_KEY_S
#define KEY_T                 __allegro_KEY_T
#define KEY_U                 __allegro_KEY_U
#define KEY_V                 __allegro_KEY_V
#define KEY_W                 __allegro_KEY_W
#define KEY_X                 __allegro_KEY_X
#define KEY_Y                 __allegro_KEY_Y
#define KEY_Z                 __allegro_KEY_Z
#define KEY_0                 __allegro_KEY_0
#define KEY_1                 __allegro_KEY_1
#define KEY_2                 __allegro_KEY_2
#define KEY_3                 __allegro_KEY_3
#define KEY_4                 __allegro_KEY_4
#define KEY_5                 __allegro_KEY_5
#define KEY_6                 __allegro_KEY_6
#define KEY_7                 __allegro_KEY_7
#define KEY_8                 __allegro_KEY_8
#define KEY_9                 __allegro_KEY_9
#define KEY_0_PAD             __allegro_KEY_0_PAD
#define KEY_1_PAD             __allegro_KEY_1_PAD
#define KEY_2_PAD             __allegro_KEY_2_PAD
#define KEY_3_PAD             __allegro_KEY_3_PAD
#define KEY_4_PAD             __allegro_KEY_4_PAD
#define KEY_5_PAD             __allegro_KEY_5_PAD
#define KEY_6_PAD             __allegro_KEY_6_PAD
#define KEY_7_PAD             __allegro_KEY_7_PAD
#define KEY_8_PAD             __allegro_KEY_8_PAD
#define KEY_9_PAD             __allegro_KEY_9_PAD
#define KEY_F1                __allegro_KEY_F1
#define KEY_F2                __allegro_KEY_F2
#define KEY_F3                __allegro_KEY_F3
#define KEY_F4                __allegro_KEY_F4
#define KEY_F5                __allegro_KEY_F5
#define KEY_F6                __allegro_KEY_F6
#define KEY_F7                __allegro_KEY_F7
#define KEY_F8                __allegro_KEY_F8
#define KEY_F9                __allegro_KEY_F9
#define KEY_F10               __allegro_KEY_F10
#define KEY_F11               __allegro_KEY_F11
#define KEY_F12               __allegro_KEY_F12
#define KEY_ESC               __allegro_KEY_ESC
#define KEY_TILDE             __allegro_KEY_TILDE
#define KEY_MINUS             __allegro_KEY_MINUS
#define KEY_EQUALS            __allegro_KEY_EQUALS
#define KEY_BACKSPACE         __allegro_KEY_BACKSPACE
#define KEY_TAB               __allegro_KEY_TAB
#define KEY_OPENBRACE         __allegro_KEY_OPENBRACE
#define KEY_CLOSEBRACE        __allegro_KEY_CLOSEBRACE
#define KEY_ENTER             __allegro_KEY_ENTER
#define KEY_COLON             __allegro_KEY_COLON
#define KEY_QUOTE             __allegro_KEY_QUOTE
#define KEY_BACKSLASH         __allegro_KEY_BACKSLASH
#define KEY_BACKSLASH2        __allegro_KEY_BACKSLASH2
#define KEY_COMMA             __allegro_KEY_COMMA
#define KEY_STOP              __allegro_KEY_STOP
#define KEY_SLASH             __allegro_KEY_SLASH
#define KEY_SPACE             __allegro_KEY_SPACE
#define KEY_INSERT            __allegro_KEY_INSERT
#define KEY_DEL               __allegro_KEY_DEL
#define KEY_HOME              __allegro_KEY_HOME
#define KEY_END               __allegro_KEY_END
#define KEY_PGUP              __allegro_KEY_PGUP
#define KEY_PGDN              __allegro_KEY_PGDN
#define KEY_LEFT              __allegro_KEY_LEFT
#define KEY_RIGHT             __allegro_KEY_RIGHT
#define KEY_UP                __allegro_KEY_UP
#define KEY_DOWN              __allegro_KEY_DOWN
#define KEY_SLASH_PAD         __allegro_KEY_SLASH_PAD
#define KEY_ASTERISK          __allegro_KEY_ASTERISK
#define KEY_MINUS_PAD         __allegro_KEY_MINUS_PAD
#define KEY_PLUS_PAD          __allegro_KEY_PLUS_PAD
#define KEY_DEL_PAD           __allegro_KEY_DEL_PAD
#define KEY_ENTER_PAD         __allegro_KEY_ENTER_PAD
#define KEY_PRTSCR            __allegro_KEY_PRTSCR
#define KEY_PAUSE             __allegro_KEY_PAUSE
#define KEY_ABNT_C1           __allegro_KEY_ABNT_C1
#define KEY_YEN               __allegro_KEY_YEN
#define KEY_KANA              __allegro_KEY_KANA
#define KEY_CONVERT           __allegro_KEY_CONVERT
#define KEY_NOCONVERT         __allegro_KEY_NOCONVERT
#define KEY_AT                __allegro_KEY_AT
#define KEY_CIRCUMFLEX        __allegro_KEY_CIRCUMFLEX
#define KEY_COLON2            __allegro_KEY_COLON2
#define KEY_KANJI             __allegro_KEY_KANJI
#define KEY_EQUALS_PAD        __allegro_KEY_EQUALS_PAD
#define KEY_BACKQUOTE         __allegro_KEY_BACKQUOTE
#define KEY_SEMICOLON         __allegro_KEY_SEMICOLON
#define KEY_COMMAND           __allegro_KEY_COMMAND
#define KEY_UNKNOWN1          __allegro_KEY_UNKNOWN1
#define KEY_UNKNOWN2          __allegro_KEY_UNKNOWN2
#define KEY_UNKNOWN3          __allegro_KEY_UNKNOWN3
#define KEY_UNKNOWN4          __allegro_KEY_UNKNOWN4
#define KEY_UNKNOWN5          __allegro_KEY_UNKNOWN5
#define KEY_UNKNOWN6          __allegro_KEY_UNKNOWN6
#define KEY_UNKNOWN7          __allegro_KEY_UNKNOWN7
#define KEY_UNKNOWN8          __allegro_KEY_UNKNOWN8

#define KEY_MODIFIERS         __allegro_KEY_MODIFIERS

#define KEY_LSHIFT            __allegro_KEY_LSHIFT
#define KEY_RSHIFT            __allegro_KEY_RSHIFT
#define KEY_LCONTROL          __allegro_KEY_LCONTROL
#define KEY_RCONTROL          __allegro_KEY_RCONTROL
#define KEY_ALT               __allegro_KEY_ALT
#define KEY_ALTGR             __allegro_KEY_ALTGR
#define KEY_LWIN              __allegro_KEY_LWIN
#define KEY_RWIN              __allegro_KEY_RWIN
#define KEY_MENU              __allegro_KEY_MENU
#define KEY_SCRLOCK           __allegro_KEY_SCRLOCK
#define KEY_NUMLOCK           __allegro_KEY_NUMLOCK
#define KEY_CAPSLOCK          __allegro_KEY_CAPSLOCK

#define KEY_MAX               __allegro_KEY_MAX

#endif /* ALLEGRO_NO_KEY_DEFINES */


// ALFONT

#define ALFONT_OK                   0
#define ALFONT_ERROR                -1

typedef struct ALFONT_FONT ALFONT_FONT;

extern int alfont_init(void);
extern void alfont_exit(void);

extern ALFONT_FONT *alfont_load_font_from_mem(const char *data, int data_len);
extern void alfont_destroy_font(ALFONT_FONT *f);

extern int alfont_set_font_size(ALFONT_FONT *f, int h);

extern int alfont_text_mode(int mode);

extern void alfont_textout_aa(ALW_BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color);
extern void alfont_textout(ALW_BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color);

extern int alfont_text_height(ALFONT_FONT *f);
extern int alfont_text_length(ALFONT_FONT *f, const char *str);


// AASTRETCH

void aa_stretch_sprite (ALW_BITMAP* dst, ALW_BITMAP* src, int dx, int dy, int dw, int dh);





#define COLORCONV_NONE              0

#define COLORCONV_8_TO_15           1
#define COLORCONV_8_TO_16           2
#define COLORCONV_8_TO_24           4
#define COLORCONV_8_TO_32           8

#define COLORCONV_15_TO_8           0x10
#define COLORCONV_15_TO_16          0x20
#define COLORCONV_15_TO_24          0x40
#define COLORCONV_15_TO_32          0x80

#define COLORCONV_16_TO_8           0x100
#define COLORCONV_16_TO_15          0x200
#define COLORCONV_16_TO_24          0x400
#define COLORCONV_16_TO_32          0x800

#define COLORCONV_24_TO_8           0x1000
#define COLORCONV_24_TO_15          0x2000
#define COLORCONV_24_TO_16          0x4000
#define COLORCONV_24_TO_32          0x8000

#define COLORCONV_32_TO_8           0x10000
#define COLORCONV_32_TO_15          0x20000
#define COLORCONV_32_TO_16          0x40000
#define COLORCONV_32_TO_24          0x80000

#define COLORCONV_32A_TO_8          0x100000
#define COLORCONV_32A_TO_15         0x200000
#define COLORCONV_32A_TO_16         0x400000
#define COLORCONV_32A_TO_24         0x800000

#define COLORCONV_DITHER_PAL        0x1000000
#define COLORCONV_DITHER_HI         0x2000000
#define COLORCONV_KEEP_TRANS        0x4000000

#define COLORCONV_DITHER            (COLORCONV_DITHER_PAL |          \
	COLORCONV_DITHER_HI)

#define COLORCONV_EXPAND_256        (COLORCONV_8_TO_15 |             \
	COLORCONV_8_TO_16 |             \
	COLORCONV_8_TO_24 |             \
	COLORCONV_8_TO_32)

#define COLORCONV_REDUCE_TO_256     (COLORCONV_15_TO_8 |             \
	COLORCONV_16_TO_8 |             \
	COLORCONV_24_TO_8 |             \
	COLORCONV_32_TO_8 |             \
	COLORCONV_32A_TO_8)

#define COLORCONV_EXPAND_15_TO_16    COLORCONV_15_TO_16

#define COLORCONV_REDUCE_16_TO_15    COLORCONV_16_TO_15

#define COLORCONV_EXPAND_HI_TO_TRUE (COLORCONV_15_TO_24 |            \
	COLORCONV_15_TO_32 |            \
	COLORCONV_16_TO_24 |            \
	COLORCONV_16_TO_32)

#define COLORCONV_REDUCE_TRUE_TO_HI (COLORCONV_24_TO_15 |            \
	COLORCONV_24_TO_16 |            \
	COLORCONV_32_TO_15 |            \
	COLORCONV_32_TO_16)

#define COLORCONV_24_EQUALS_32      (COLORCONV_24_TO_32 |            \
	COLORCONV_32_TO_24)

#define COLORCONV_TOTAL             (COLORCONV_EXPAND_256 |          \
	COLORCONV_REDUCE_TO_256 |       \
	COLORCONV_EXPAND_15_TO_16 |     \
	COLORCONV_REDUCE_16_TO_15 |     \
	COLORCONV_EXPAND_HI_TO_TRUE |   \
	COLORCONV_REDUCE_TRUE_TO_HI |   \
	COLORCONV_24_EQUALS_32 |        \
	COLORCONV_32A_TO_15 |           \
	COLORCONV_32A_TO_16 |           \
	COLORCONV_32A_TO_24)

#define COLORCONV_PARTIAL           (COLORCONV_EXPAND_15_TO_16 |     \
	COLORCONV_REDUCE_16_TO_15 |     \
	COLORCONV_24_EQUALS_32)

#define COLORCONV_MOST              (COLORCONV_EXPAND_15_TO_16 |     \
	COLORCONV_REDUCE_16_TO_15 |     \
	COLORCONV_EXPAND_HI_TO_TRUE |   \
	COLORCONV_REDUCE_TRUE_TO_HI |   \
	COLORCONV_24_EQUALS_32)

#define COLORCONV_KEEP_ALPHA        (COLORCONV_TOTAL                 \
	& ~(COLORCONV_32A_TO_8 |        \
	COLORCONV_32A_TO_15 |       \
	COLORCONV_32A_TO_16 |       \
	COLORCONV_32A_TO_24))

#endif
