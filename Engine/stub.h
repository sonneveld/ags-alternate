#ifndef __MY_STUB_H
#define __MY_STUB_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <ctype.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_audio.h>
 
 
// typedefs and defines
// ================================================================
 
typedef ALLEGRO_BITMAP BITMAP;
typedef al_fixed fixed;
typedef ALLEGRO_COLOR RGB;
typedef ALLEGRO_COLOR PALETTE[];
#define PALLETE PALETTE

#define PAL_SIZE 256

#define GFX_TEXT                       -1
#define GFX_AUTODETECT                 0
#define GFX_AUTODETECT_FULLSCREEN      1
#define GFX_AUTODETECT_WINDOWED        2

#define DIGI_AUTODETECT 0
#define MIDI_AUTODETECT 0
#define DIGI_NONE 0

#define SWITCH_NONE           0
#define SWITCH_PAUSE          1
#define SWITCH_AMNESIA        2
#define SWITCH_BACKGROUND     3
#define SWITCH_BACKAMNESIA    4

#define SWITCH_IN             0
#define SWITCH_OUT            1

#define KEY_A ALLEGRO_KEY_A
#define KEY_0	ALLEGRO_KEY_0
#define KEY_ALT	ALLEGRO_KEY_ALT
#define KEY_BACKSLASH	ALLEGRO_KEY_BACKSLASH
#define KEY_BACKSPACE	ALLEGRO_KEY_BACKSPACE
#define KEY_CLOSEBRACE	ALLEGRO_KEY_CLOSEBRACE
#define KEY_COMMA	ALLEGRO_KEY_COMMA
#define KEY_DEL	ALLEGRO_KEY_DELETE
#define KEY_DOWN	ALLEGRO_KEY_DOWN
#define KEY_END	ALLEGRO_KEY_END
#define KEY_ENTER	ALLEGRO_KEY_ENTER
#define KEY_EQUALS	ALLEGRO_KEY_EQUALS
#define KEY_ESC	ALLEGRO_KEY_ESCAPE
#define KEY_STOP	ALLEGRO_KEY_FULLSTOP
#define KEY_HOME	ALLEGRO_KEY_HOME
#define KEY_INSERT	ALLEGRO_KEY_INSERT
#define KEY_LCONTROL	ALLEGRO_KEY_LCTRL
#define KEY_LEFT	ALLEGRO_KEY_LEFT
#define KEY_LSHIFT	ALLEGRO_KEY_LSHIFT
#define KEY_MAX	ALLEGRO_KEY_MAX
#define KEY_MINUS	ALLEGRO_KEY_MINUS
#define KEY_OPENBRACE	ALLEGRO_KEY_OPENBRACE
#define KEY_0_PAD	ALLEGRO_KEY_PAD_0
#define KEY_1_PAD	ALLEGRO_KEY_PAD_1
#define KEY_2_PAD	ALLEGRO_KEY_PAD_2
#define KEY_3_PAD	ALLEGRO_KEY_PAD_3
#define KEY_4_PAD	ALLEGRO_KEY_PAD_4
#define KEY_6_PAD	ALLEGRO_KEY_PAD_6
#define KEY_7_PAD	ALLEGRO_KEY_PAD_7
#define KEY_8_PAD	ALLEGRO_KEY_PAD_8
#define KEY_9_PAD	ALLEGRO_KEY_PAD_9
#define KEY_DEL_PAD	ALLEGRO_KEY_PAD_DELETE
#define KEY_ENTER_PAD	ALLEGRO_KEY_PAD_ENTER
#define KEY_MINUS_PAD	ALLEGRO_KEY_PAD_MINUS
#define KEY_PLUS_PAD	ALLEGRO_KEY_PAD_PLUS
#define KEY_SLASH_PAD	ALLEGRO_KEY_PAD_SLASH
#define KEY_PGDN	ALLEGRO_KEY_PGDN
#define KEY_PGUP	ALLEGRO_KEY_PGUP
#define KEY_QUOTE	ALLEGRO_KEY_QUOTE
#define KEY_RCONTROL	ALLEGRO_KEY_RCTRL
#define KEY_RIGHT	ALLEGRO_KEY_RIGHT
#define KEY_RSHIFT	ALLEGRO_KEY_RSHIFT
#define KEY_SCRLOCK	ALLEGRO_KEY_SCROLLLOCK
#define KEY_SEMICOLON	ALLEGRO_KEY_SEMICOLON
#define KEY_SLASH	ALLEGRO_KEY_SLASH
#define KEY_SPACE	ALLEGRO_KEY_SPACE
#define KEY_TAB	ALLEGRO_KEY_TAB
#define KEY_UP	ALLEGRO_KEY_UP

#define MIDI_NONE 0

#define AL_CONST const


#define BEGIN_COLOR_DEPTH_LIST void;
#define   COLOR_DEPTH_8  void;
#define   COLOR_DEPTH_15 void;
#define   COLOR_DEPTH_16 void;
#define   COLOR_DEPTH_24 void;
#define   COLOR_DEPTH_32 void;
#define END_COLOR_DEPTH_LIST void;


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


#define FLI_ERROR       -2
#define AL_ID(a,b,c,d)     (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))
#define U_ASCII         AL_ID('A','S','C','8')
#define U_UNICODE       AL_ID('U','N','I','C')


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


#define END_OF_MAIN()

#define LOCK_VARIABLE(x) void
#define LOCK_FUNCTION(x) void

#define END_OF_FUNCTION(x)			void x##_end(void) { }  

#define MASK_COLOR_8       0
#define MASK_COLOR_15      0x7C1F
#define MASK_COLOR_16      0xF81F
#define MASK_COLOR_24      0xFF00FF
#define MASK_COLOR_32      0xFF00FF

#define TIMERS_PER_SECOND     1193181L
#define SECS_TO_TIMER(x)      ((long)(x) * TIMERS_PER_SECOND)
#define MSEC_TO_TIMER(x)      ((long)(x) * (TIMERS_PER_SECOND / 1000))
#define BPS_TO_TIMER(x)       (TIMERS_PER_SECOND / (long)(x))
#define BPM_TO_TIMER(x)       ((60 * TIMERS_PER_SECOND) / (long)(x))


// structs
// ================================================================

struct al_ffblk {
    int attrib;  //     - actual attributes of the file found
      time_t time;  //    - modification time of file
      char name[512];  // - name of file
  };


struct _al_normal_packfile_details
{
   int hndl;                           /* DOS file handle */
   int flags;                          /* PACKFILE_FLAG_* constants */
   unsigned char *buf_pos;             /* position in buffer */
   int buf_size;                       /* number of bytes in the buffer */
   long todo;                          /* number of bytes still on the disk */
   struct PACKFILE *parent;            /* nested, parent file */
   //struct LZSS_PACK_DATA *pack_data;   /* for LZSS compression */
   //struct LZSS_UNPACK_DATA *unpack_data; /* for LZSS decompression */
   char *filename;                     /* name of the file */
   char *passdata;                     /* encryption key data */
   char *passpos;                      /* current key position */
   //unsigned char buf[F_BUF_SIZE];      /* the actual data buffer */
};

struct PACKFILE                           /* our very own FILE structure... */
{
   //AL_CONST PACKFILE_VTABLE *vtable;
   void *userdata;
   int is_normal_packfile;

   /* The following is only to be used for the "normal" PACKFILE vtable,
    * i.e. what is implemented by Allegro itself. If is_normal_packfile is
    * false then the following is not even allocated. This must be the last
    * member in the structure.
    */
   struct _al_normal_packfile_details normal;
};

typedef struct AUDIOSTREAM
{
   int voice;                          /* the voice we are playing on */
   //struct SAMPLE *samp;                /* the sample we are using */
   int len;                            /* buffer length */
   int bufcount;                       /* number of buffers per sample half */
   int bufnum;                         /* current refill buffer */
   int active;                         /* which half is currently playing */
   void *locked;                       /* the locked buffer */
} AUDIOSTREAM;

typedef struct {
   unsigned char data[32][32][32];
} RGB_MAP;

typedef struct {
   unsigned char data[PAL_SIZE][PAL_SIZE];
} COLOR_MAP;


typedef struct GFX_VTABLE        /* functions for drawing onto bitmaps */
{
   int color_depth;
   int mask_color;
   void *unwrite_bank;  /* C function on some machines, asm on i386 */
  
} GFX_VTABLE;


extern volatile long midi_pos;
extern PALETTE black_palette;
extern char allegro_error[1000];
extern volatile int key_shifts;
extern volatile int mouse_z;
extern RGB_MAP * rgb_map;
extern  COLOR_MAP * color_map;
extern BITMAP * screen;
extern int  _rgb_r_shift_15;
extern int  _rgb_g_shift_15;
extern int  _rgb_b_shift_15;
extern int  _rgb_r_shift_16;
extern int  _rgb_g_shift_16;
extern int  _rgb_b_shift_16;
extern int  _rgb_r_shift_24;
extern int  _rgb_g_shift_24;
extern int  _rgb_b_shift_24;
extern int  _rgb_r_shift_32;
extern int  _rgb_g_shift_32;
extern int  _rgb_b_shift_32;
extern int  _rgb_a_shift_32;

// from ac.cpp
extern char dataDirectory[512];
extern char appDirectory[512];
extern long int filelength(int fhandle);
extern int _rgb_scale_5[];
extern int _rgb_scale_6[];

// mouse
// ================================================================

extern void set_mouse_range (int x1, int y_1, int x2, int y2) ;
extern int poll_mouse();
extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_b;
extern void  position_mouse (int x, int y);
extern int install_mouse();

// drawing
// ================================================================
extern void set_alpha_blender() ;
extern void draw_trans_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) ;
extern void set_clip (BITMAP *bitmap, int x1, int y_1, int x2, int y2);
extern void set_clip_rect (BITMAP *bitmap, int x1, int y_1, int x2, int y2);
extern void destroy_bitmap(BITMAP *bitmap);
extern BITMAP *create_bitmap_ex(int color_depth, int width, int height);
inline int  bitmap_color_depth (BITMAP *bmp) {
    return 32;
}

// bitmap helper macros
// ================================================================

#ifdef TRUE
#define BMP_W(bmp) al_get_bitmap_width(bmp)
#define BMP_H(bmp) al_get_bitmap_height(bmp)
inline GFX_VTABLE *BMP_VTABLE(BITMAP *bmp) {
    abort();
    return NULL;
}
inline unsigned char **BMP_LINE(BITMAP *bmp) {
    abort();
    return NULL;
}

#define BMP_CB(bmp) _bitmap_get_cb(bmp)

#else
#define BMP_W(bmp) bmp->w
#define BMP_H(bmp) bmp->h
#define BMP_LINE(bmp) bmp->line
#define BMP_CB(bmp) bmp->cb
#define BMP_VTABLE(bmp) bmp->vtable
#endif





// misc externs
// ================================================================

extern void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) ;
extern void draw_sprite_h_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) ;
extern void draw_sprite_v_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) ;
extern void draw_sprite_vh_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) ;
extern void stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int source_width, int source_height, 
  int dest_x, int dest_y, int dest_width, int dest_height);
extern void clear_bitmap(BITMAP *bitmap);
extern void clear(BITMAP *bitmap);
extern void clear_to_color(BITMAP *bitmap, int color);
extern BITMAP * create_bitmap (int width, int height);
extern int _bitmap_get_cb(BITMAP *bitmap);
extern int osx_sys_question(AL_CONST char *msg, AL_CONST char *but1, AL_CONST char *but2);
extern  bool PlayMovie (const char *name, int skip) ;



// misc inlines
// ================================================================
  

#define stricmp strcasecmp
#define strnicmp strncasecmp

inline char *strlwr(char *str) {
    char *p = str;
    for ( ; *p; ++p) *p = tolower(*p);
    return str;
}

inline char *strupr(char *str) {
    char *p = str;
    for ( ; *p; ++p) *p = toupper(*p);
    return str;
}
  
inline char *fix_filename_case(char *path) {
    // do nothing if not DOS
    return path;
}

inline char *fix_filename_slashes(char *path) {
    for(char *p = path; *p; p++) {
        if (*p == '\\')
            *p = '/';
    }
    return path;
}

inline char *get_filename( char *path) {
     char *filename = path;
    for(;;) {
        char *result = strpbrk(path, "\\/");
        if (result != NULL)
            filename = result;
    }
    return filename;
}

inline char *append_filename (char *dest, const char *path, const char *filename, int size) {
    // hack, barely works, no checks
    char buffer[500];
    sprintf(buffer, "%s/%s", path, filename);
    strcpy(dest, buffer);
}

inline void hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b) {
    float rf, gf, bf;
    al_color_hsv_to_rgb(h, s, v , &rf, &gf, &bf);
    *r = (int)(rf*255.0f);
    *g = (int)(gf*255.0f);
    *b = (int)(bf*255.0f);
}

inline void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v) {
    float rf, gf, bf;
   rf = ((float)r)/255.0f;
   gf = ((float)r)/255.0f;
   bf = ((float)r)/255.0f;
   al_color_rgb_to_hsv(rf, gf, bf, h, s, v);
}

inline void rectfill ( BITMAP *bmp, int x1, int y_1, int x2, int y2, int color) { printf("stub.h@%d\n", __LINE__); }
inline void putpixel ( BITMAP *bmp, int x, int y, int color){ printf("stub.h@%d\n", __LINE__); }
#define _putpixel putpixel
inline int getpixel ( BITMAP *bmp, int x, int y){ printf("stub.h@%d\n", __LINE__); return 0;}
inline int _getpixel ( BITMAP *bmp, int x, int y){ printf("stub.h@%d\n", __LINE__); return 0;}
inline void acquire_bitmap(BITMAP *bmp) { printf("stub.h@%d\n", __LINE__); }
inline void release_bitmap(BITMAP *bmp) { printf("stub.h@%d\n", __LINE__); }
inline PACKFILE *pack_fopen(char *name, char *mode) { printf("stub.h@%d\n", __LINE__); return  NULL; }
inline void pack_fclose(PACKFILE*file) { }
inline int exists(const char *filename) {    return al_filename_exists(filename);}
inline void do_line(BITMAP *bmp, int x1, int y1,int x2,int y2, int d, void (*proc)(BITMAP *bmp, int x, int y, int d)) { printf("stub.h@%d\n", __LINE__); }
inline int is_memory_bitmap(BITMAP *bmp) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void floodfill(BITMAP *bmp, int x, int y, int color) { printf("stub.h@%d\n", __LINE__); }
inline fixed itofix(int x ) { return  al_itofix(x);}
inline fixed fixdiv(int x, int y) { return al_fixdiv(x,y);}
inline fixed fdiv(int x, int y) { return al_fixdiv(x,y);}
inline fixed fixmul(int x, int y) { return al_fixmul(x,y);}
inline fixed fmul(int x, int y) { return al_fixmul(x,y);}
inline fixed fsin(int x) { return al_fixsin(x);}
inline fixed fcos(int x) { return al_fixcos(x);}
inline fixed fatan(int x) { return al_fixatan(x);}
inline int is_linear_bitmap(BITMAP *bmp) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void set_trans_blender(int r, int g, int b, int a) { }
inline void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle) {  }
inline void set_palette_range(const PALETTE p, int from, int to, int vsync) { }
inline int bitmap_mask_color(BITMAP *bmp) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getr_depth(int color_depth, int c){ printf("stub.h@%d\n", __LINE__); return  0; }
inline int getg_depth(int color_depth, int c){ printf("stub.h@%d\n", __LINE__); return  0; }
inline int getb_depth(int color_depth, int c){ printf("stub.h@%d\n", __LINE__); return  0; }
inline int geta_depth(int color_depth, int c){ printf("stub.h@%d\n", __LINE__); return  0; }
inline int makecol_depth(int color_depth, int r, int g, int b) { printf("stub.h@%d\n", __LINE__); return  0;}
inline int install_sound(int digi, int midi, const char *cfg_path) { return al_install_audio(); }
inline int makeacol_depth(int color_depth, int r, int g, int b, int a) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) { printf("stub.h@%d\n", __LINE__); }
inline void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) { printf("stub.h@%d\n", __LINE__); }
inline int al_findfirst(const char *pattern, struct al_ffblk *info, int attrib) { printf("stub.h@%d\n", __LINE__); return  1;}
inline int al_findnext(struct al_ffblk *info) { printf("stub.h@%d\n", __LINE__); return  1;}
inline void al_findclose(struct al_ffblk *info) { printf("stub.h@%d\n", __LINE__); }
inline void triangle(BITMAP *bmp, int x1,int y1,int x2,int y2,int x3,int y3, int color) { printf("stub.h@%d\n", __LINE__); }
inline BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height) {
    return al_create_sub_bitmap(parent, x, y, width, height);
}
inline void set_color_depth(int depth) { printf("stub.h@%d\n", __LINE__); }
inline void unselect_palette() { printf("stub.h@%d\n", __LINE__); }
inline void hline(BITMAP *bmp, int x1, int y, int x2, int color) { printf("stub.h@%d\n", __LINE__); }
inline void set_window_title(const char *name) { printf("stub.h@%d\n", __LINE__); }
inline void set_volume_per_voice(int scale) { printf("stub.h@%d\n", __LINE__); }
inline void set_volume(int digi_volume, int midi_volume) { printf("stub.h@%d\n", __LINE__); }
inline void set_leds(int leds) { printf("stub.h@%d\n", __LINE__); }
inline int set_close_button_callback(void (*proc)(void)) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void select_palette(const PALETTE p) { printf("stub.h@%d\n", __LINE__); }
inline int save_bitmap(const char *filename, BITMAP *bmp, const ALLEGRO_COLOR *pal) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void reserve_voices(int digi_voices, int midi_voices) { printf("stub.h@%d\n", __LINE__); }
inline void request_refresh_rate(int rate);
inline void remove_sound() { printf("stub.h@%d\n", __LINE__); }
inline void put_backslash(char *filename) { printf("stub.h@%d\n", __LINE__); }
inline int poll_keyboard() { printf("stub.h@%d\n", __LINE__); return  0;}
inline int play_fli(const char *filename, BITMAP *bmp, int loop, int (*callback)()) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void pivot_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle) { printf("stub.h@%d\n", __LINE__); }
inline int pack_fseek(PACKFILE *f, int offset) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int midi_seek(int target) { printf("stub.h@%d\n", __LINE__); return  1;}
inline int makecol8(int r, int g, int b) { printf("stub.h@%d\n", __LINE__); return  0;}
inline int makecol15(int r, int g, int b) { printf("stub.h@%d\n", __LINE__); return  0;}
inline int makecol16(int r, int g, int b) { printf("stub.h@%d\n", __LINE__); return  0;}
inline int makecol24(int r, int g, int b){ printf("stub.h@%d\n", __LINE__); return  0;}
inline int makecol32(int r, int g, int b){ printf("stub.h@%d\n", __LINE__); return  0;}
inline BITMAP *load_pcx(const char *filename, ALLEGRO_COLOR *pal) { printf("stub.h@%d\n", __LINE__); return  NULL; }
inline BITMAP *load_bitmap(const char *filename, ALLEGRO_COLOR *pal) {printf("stub.h@%d\n", __LINE__); return  NULL; }
inline int keyboard_needs_poll() { printf("stub.h@%d\n", __LINE__); return  FALSE ;}
inline int install_timer() { printf("stub.h@%d\n", __LINE__); return  0; }
inline int install_keyboard() { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getr8(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getg8(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getb8(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getr15(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getg15(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getb15(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getr16(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getg16(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getb16(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getr24(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getg24(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getb24(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getr32(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getg32(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int getb32(int c) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void get_palette_range(PALETTE p, int from, int to) { printf("stub.h@%d\n", __LINE__); }
inline void get_palette(PALETTE p) { printf("stub.h@%d\n", __LINE__); }
inline int get_desktop_resolution(int *width, int *height) { printf("stub.h@%d\n", __LINE__); return  0; }
inline double fixtof(fixed x) { return al_fixtof(x); }
inline int file_exists(const char *filename, int attrib, int *aret) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void fade_interpolate(const PALETTE source, const PALETTE dest, PALETTE output, int pos, int from, int to) { printf("stub.h@%d\n", __LINE__); }
inline void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) { printf("stub.h@%d\n", __LINE__); }
inline void draw_lit_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int color) { printf("stub.h@%d\n", __LINE__); }
inline void circlefill(BITMAP *bmp, int x, int y, int radius, int color) { printf("stub.h@%d\n", __LINE__); }
inline int bestfit_color(const PALETTE pal, int r, int g, int b) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int allegro_init() { printf("stub.h@%d\n", __LINE__); return  0; }
inline void allegro_exit() { printf("stub.h@%d\n", __LINE__); }
inline void stop_audio_stream(AUDIOSTREAM *stream) { printf("stub.h@%d\n", __LINE__); }
inline BITMAP *screen_bitmap(ALLEGRO_DISPLAY *display) { return al_get_backbuffer(display); }
inline int set_gfx_mode(int card, int w, int h, int v_w, int v_h) { printf("stub.h@%d\n", __LINE__); return  0; }
inline int makeacol32(int r, int g, int b, int a) { printf("stub.h@%d\n", __LINE__); return  0; }
inline void set_palette(const PALETTE p) { printf("stub.h@%d\n", __LINE__); }
inline void create_light_table(COLOR_MAP *table, const PALETTE pal, int r, int g, int b, void (*callback)(int pos)) { printf("stub.h@%d\n", __LINE__); }
inline long file_size(const char *filename) { printf("stub.h@%d\n", __LINE__); return  0L; }
inline void stretch_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int w, int h) { printf("stub.h@%d\n", __LINE__); }
//inline int install_sound(int digi, int midi, const char *cfg_path) { return 0; }
inline int isdigit(char ch) {    return ch >= '0' && ch <= '9';}
inline void voice_set_pan(int voice, int pan) { printf("stub.h@%d\n", __LINE__); }
inline void voice_start(int voice) { printf("stub.h@%d\n", __LINE__); }
inline void voice_stop(int voice) { printf("stub.h@%d\n", __LINE__); }
inline void cd_exit(void) { printf("stub.h@%d\n", __LINE__); }

typedef unsigned long (*BLENDER_FUNC)(unsigned long x, unsigned long y, unsigned long n);
inline void set_blender_mode (BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24, int r, int g, int b, int a) { printf("stub.h@%d\n", __LINE__); }

inline unsigned long _blender_trans15(unsigned long x, unsigned long y, unsigned long n) { printf("stub.h@%d\n", __LINE__); }
inline unsigned long _blender_trans16(unsigned long x, unsigned long y, unsigned long n) { printf("stub.h@%d\n", __LINE__); }
inline unsigned long _blender_trans24(unsigned long x, unsigned long y, unsigned long n) { printf("stub.h@%d\n", __LINE__); }

inline int mkdir(const char *filename) {
#if defined(LINUX_VERSION) || defined(MAC_VERSION)
    return mkdir(filename, 0766);
#else
    return mkdir(filename);
#endif
}


inline int readkey() { printf("stub.h@%d\n", __LINE__); return  0; }
inline void rest(unsigned int time) { printf("%s@%d\n", __FILE__, __LINE__); }
inline int win_get_window(void){ printf("%s@%d\n", __FILE__, __LINE__);  return 0; } 

inline void aa_stretch_sprite (BITMAP* dst, BITMAP* src,
			  int dx, int dy, int dw, int dh)  { printf("%s@%d\n", __FILE__, __LINE__); }
inline int set_display_switch_mode(int mode) { printf("%s@%d\n", __FILE__, __LINE__);  return 0; } 
inline int set_display_switch_callback(int dir, void (*cb)()) { printf("%s@%d\n", __FILE__, __LINE__);  return 0; } 

inline int get_uformat(void) { printf("%s@%d\n", __FILE__, __LINE__); }
inline char *uconvert(const char *s, int type, char *buf, int newtype, int size){ printf("%s@%d\n", __FILE__, __LINE__);  return 0; } 
inline void set_uformat(int type){ printf("%s@%d\n", __FILE__, __LINE__); }

inline void set_color_conversion(int mode) { printf("%s@%d\n", __FILE__, __LINE__); }
inline void create_rgb_table(RGB_MAP *table, const PALETTE pal, void (*callback)(int pos)) { printf("%s@%d\n", __FILE__, __LINE__); } 
inline int install_int_ex(void (*proc)(), int speed) { printf("stub@%s:%d\n", __FILE__, __LINE__);  return 0; } 




#endif